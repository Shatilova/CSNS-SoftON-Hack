#include "mmap.h"

#include <Windows.h>
#include "utils.h"

typedef HMODULE(__stdcall* pLoadLibraryA)(LPCSTR);
typedef FARPROC(__stdcall* pGetProcAddress)(HMODULE, LPCSTR);
typedef INT(__stdcall* dllmain)(HMODULE, DWORD, LPVOID);

struct loaderdata {
	LPVOID ImageBase;

	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseReloc;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;

	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;
};

DWORD __stdcall LibraryLoader(LPVOID Memory) {
	loaderdata* LoaderParams = (loaderdata*)Memory;

	PIMAGE_BASE_RELOCATION pIBR = LoaderParams->BaseReloc;

	DWORD delta = (DWORD)((LPBYTE)LoaderParams->ImageBase - LoaderParams->NtHeaders->OptionalHeader.ImageBase); // Calculate the delta

	while (pIBR->VirtualAddress) {
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
			int count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			PWORD list = (PWORD)(pIBR + 1);

			for (int i = 0; i < count; i++) {
				if (list[i]) {
					PDWORD ptr = (PDWORD)((LPBYTE)LoaderParams->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
					*ptr += delta;
				}
			}
		}

		pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
	}

	PIMAGE_IMPORT_DESCRIPTOR pIID = LoaderParams->ImportDirectory;

	// Resolve DLL imports
	while (pIID->Characteristics) {
		PIMAGE_THUNK_DATA OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)LoaderParams->ImageBase + pIID->OriginalFirstThunk);
		PIMAGE_THUNK_DATA FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)LoaderParams->ImageBase + pIID->FirstThunk);

		HMODULE hModule = LoaderParams->fnLoadLibraryA((LPCSTR)LoaderParams->ImageBase + pIID->Name);

		if (!hModule)
			return FALSE;

		while (OrigFirstThunk->u1.AddressOfData) {
			if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
				// Import by ordinal
				DWORD Function = (DWORD)LoaderParams->fnGetProcAddress(hModule,
					(LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

				if (!Function)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}
			else {
				// Import by name
				PIMAGE_IMPORT_BY_NAME pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)LoaderParams->ImageBase + OrigFirstThunk->u1.AddressOfData);
				DWORD Function = (DWORD)LoaderParams->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);
				if (!Function)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}
			OrigFirstThunk++;
			FirstThunk++;
		}
		pIID++;
	}

	if (LoaderParams->NtHeaders->OptionalHeader.AddressOfEntryPoint) {
		dllmain EntryPoint = (dllmain)((LPBYTE)LoaderParams->ImageBase + LoaderParams->NtHeaders->OptionalHeader.AddressOfEntryPoint);

		return EntryPoint((HMODULE)LoaderParams->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
	}
	return TRUE;
}

DWORD __stdcall stub() { return 0; }

std::string MMAP(const std::string& process_name, unsigned char* raw) {
#define SAFE_RET(msg) { \
	delete[] raw; \
	return msg; \
	}

	if (!raw)
		SAFE_RET("RAW is broken");

	HANDLE hProc = 0;
	DWORD pID = utils::PIDByName(process_name.c_str());
	hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (!hProc)
		SAFE_RET("Failed to open game process");

	loaderdata LoaderParams;

	// Target Dll's DOS Header
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)raw;
	// Target Dll's NT Headers
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)raw + pDosHeader->e_lfanew);

	// Allocating memory for the DLL
	PVOID ExecutableImage = VirtualAllocEx(hProc, NULL, pNtHeaders->OptionalHeader.SizeOfImage,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// Copy the headers to target process
	int is_ok = WriteProcessMemory(hProc, ExecutableImage, raw,
		pNtHeaders->OptionalHeader.SizeOfHeaders, NULL);
	if (is_ok == 0)
		SAFE_RET("Failed to copy the headers to game process memory");

	// Target Dll's Section Header
	PIMAGE_SECTION_HEADER pSectHeader = (PIMAGE_SECTION_HEADER)(pNtHeaders + 1);
	// Copying sections of the dll to the target process
	for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
		is_ok = WriteProcessMemory(hProc, (PVOID)((LPBYTE)ExecutableImage + pSectHeader[i].VirtualAddress),
			(PVOID)((LPBYTE)raw + pSectHeader[i].PointerToRawData), pSectHeader[i].SizeOfRawData, NULL);
		if (is_ok == 0)
			SAFE_RET("Failed to copy section of the dll to game process memory");
	}

	// Allocating memory for the loader code.
	PVOID LoaderMemory = VirtualAllocEx(hProc, NULL, 4096, MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE); // Allocate memory for the loader code
	if (LoaderMemory == 0)
		SAFE_RET("Failed to allocate memory in game process");

	LoaderParams.ImageBase = ExecutableImage;
	LoaderParams.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)ExecutableImage + pDosHeader->e_lfanew);

	LoaderParams.BaseReloc = (PIMAGE_BASE_RELOCATION)((LPBYTE)ExecutableImage
		+ pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	LoaderParams.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)ExecutableImage
		+ pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	LoaderParams.fnLoadLibraryA = LoadLibraryA;
	LoaderParams.fnGetProcAddress = GetProcAddress;

	// Write the loader information to target process
	is_ok = WriteProcessMemory(hProc, LoaderMemory, &LoaderParams, sizeof(loaderdata),
		NULL);
	if (is_ok == 0)
		SAFE_RET("Failed to write loader information to game process memory");

	// Write the loader code to target process
	is_ok = WriteProcessMemory(hProc, (PVOID)((loaderdata*)LoaderMemory + 1), LibraryLoader,
		(DWORD)stub - (DWORD)LibraryLoader, NULL);
	if (is_ok == 0)
		SAFE_RET("Failed to write the loader code to game process memory");

	// Create a remote thread to execute the loader code
	HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)((loaderdata*)LoaderMemory + 1),
		LoaderMemory, 0, NULL);
	if (hThread == 0)
		SAFE_RET("Fail to create remote thread");

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hProc);

	if (GetLastError())
		SAFE_RET("Hack might be injected, but something went wrong during the injection")
	else
		SAFE_RET("Injected successfully");
}