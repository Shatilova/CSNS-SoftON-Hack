#include "anti_re.h"

#include <TlHelp32.h>
#include <algorithm>
#include <vector>
#include <tchar.h>

// to avoid of "GetVersionEx: was declared deprecated" warning
#pragma warning(disable : 4996)

// structures used by library function is moved from header to source file
// because it's needed for library only
#pragma pack(1)
namespace {
	// Grabbed this definition of MSDN and modified one pointer
	// http://msdn.microsoft.com/en-us/library/ms684280(VS.85).aspx
	typedef struct _PROCESS_BASIC_INFORMATION {
		PVOID Reserved1;
		void* PebBaseAddress;
		PVOID Reserved2[2];
		ULONG_PTR UniqueProcessId;
		ULONG_PTR ParentProcessId;
	} PROCESS_BASIC_INFORMATION;

	typedef struct _LSA_UNICODE_STRING {
		USHORT Length;
		USHORT MaximumLength;
		PWSTR Buffer;
	} LSA_UNICODE_STRING, * PLSA_UNICODE_STRING,
		UNICODE_STRING, * PUNICODE_STRING;

	typedef struct _OBJECT_TYPE_INFORMATION {
		UNICODE_STRING TypeName;
		ULONG TotalNumberOfHandles;
		ULONG TotalNumberOfObjects;
	}OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

	// Returned by the ObjectAllTypeInformation class
	// passed to NtQueryObject
	typedef struct _OBJECT_ALL_INFORMATION {
		ULONG NumberOfObjects;
		OBJECT_TYPE_INFORMATION ObjectTypeInformation[1];
	}OBJECT_ALL_INFORMATION, * POBJECT_ALL_INFORMATION;

	typedef struct _PEB_LDR_DATA {
		UINT8 _PADDING_[12];
		LIST_ENTRY InLoadOrderModuleList;
		LIST_ENTRY InMemoryOrderModuleList;
		LIST_ENTRY InInitializationOrderModuleList;
	} PEB_LDR_DATA, * PPEB_LDR_DATA;

	typedef struct _PEB {
		UINT8 _PADDING_[12];
		PEB_LDR_DATA* Ldr;
	} PEB, * PPEB;

	typedef struct _LDR_DATA_TABLE_ENTRY {
		LIST_ENTRY InLoadOrderLinks;
		LIST_ENTRY InMemoryOrderLinks;
		LIST_ENTRY InInitializationOrderLinks;
		VOID* DllBase;
	} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

	typedef struct _UNLINKED_MODULE {
		HMODULE hModule;
		PLIST_ENTRY RealInLoadOrderLinks;
		PLIST_ENTRY RealInMemoryOrderLinks;
		PLIST_ENTRY RealInInitializationOrderLinks;
		PLDR_DATA_TABLE_ENTRY Entry;
	} UNLINKED_MODULE;

	struct FindModuleHandle {
		HMODULE m_hModule;

		FindModuleHandle(HMODULE hModule) :
			m_hModule(hModule)
		{}

		bool operator() (UNLINKED_MODULE const& Module) const {
			return (Module.hModule == m_hModule);
		}
	};

	typedef NTSTATUS(WINAPI* tNtQueryInformationProcess)(HANDLE, UINT, PVOID, ULONG, PULONG);
}
#pragma pack()

// utils
namespace {
	// This function uses the toolhelp32 api to enumerate all running processes
	// on the computer and does a comparison of the process name against the 
	// ProcessName parameter. The function will return 0 on failure.
	DWORD GetProcessIdFromName(LPCTSTR ProcessName) {
		PROCESSENTRY32 pe32;
		HANDLE hSnapshot = NULL;
		ZeroMemory(&pe32, sizeof(PROCESSENTRY32));

		// We want a snapshot of processes
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		// Check for a valid handle, in this case we need to check for
		// INVALID_HANDLE_VALUE instead of NULL
		if (hSnapshot == INVALID_HANDLE_VALUE)
			return 0;

		// Now we can enumerate the running process, also 
		// we can't forget to set the PROCESSENTRY32.dwSize member
		// otherwise the following functions will fail
		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnapshot, &pe32) == FALSE) {
			// Cleanup the mess
			CloseHandle(hSnapshot);
			return 0;
		}

		// Do our first comparison
		if (_tcsicmp(pe32.szExeFile, ProcessName) == FALSE) {
			// Cleanup the mess
			CloseHandle(hSnapshot);
			return pe32.th32ProcessID;
		}

		// Most likely it won't match on the first try so 
		// we loop through the rest of the entries until
		// we find the matching entry or not one at all
		while (Process32Next(hSnapshot, &pe32)) {
			if (_tcsicmp(pe32.szExeFile, ProcessName) == 0) {
				// Cleanup the mess
				CloseHandle(hSnapshot);
				return pe32.th32ProcessID;
			}
		}

		// If we made it this far there wasn't a match
		// so we'll return 0
		CloseHandle(hSnapshot);
		return 0;
	}

	// This function will return the process id of csrss.exe
	// and will do so in two different ways. If the OS is XP or 
	// greater NtDll has a CsrGetProcessId otherwise I'll use 
	// GetProcessIdFromName. Like other functions it will 
	// return 0 on failure.
	DWORD GetCsrssProcessId() {
		// Don't forget to set dw.Size to the appropriate
		// size (either OSVERSIONINFO or OSVERSIONINFOEX)
		OSVERSIONINFO osinfo;
		ZeroMemory(&osinfo, sizeof(OSVERSIONINFO));
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		// Shouldn't fail
		GetVersionEx(&osinfo);

		// Visit http://msdn.microsoft.com/en-us/library/ms724833(VS.85).aspx
		// for a full table of versions however what I have set will
		// trigger on anything XP and newer including Server 2003
		if (osinfo.dwMajorVersion >= 5 && osinfo.dwMinorVersion >= 1) {
			// Gotta love functions pointers
			typedef DWORD(__stdcall * pCsrGetId)();

			// Grab the export from NtDll
			pCsrGetId CsrGetProcessId = (pCsrGetId)GetProcAddress(GetModuleHandle("ntdll.dll"), "CsrGetProcessId");

			if (CsrGetProcessId)
				return CsrGetProcessId();
			else
				return 0;
		}
		else
			return GetProcessIdFromName("csrss.exe");
	}
}

bool CheckDbgPresentCloseHandle() {
	HANDLE Handle = (HANDLE)0x8000;
	__try {
		CloseHandle(Handle);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return true;
	}

	return false;
}

bool CanOpenCsrss()
{
	HANDLE Csrss = 0;

	// If we're being debugged and the process has
	// SeDebugPrivileges privileges then this call
	// will be successful, note that this only works
	// with PROCESS_ALL_ACCESS.
	Csrss = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCsrssProcessId());

	if (Csrss != NULL)
	{
		CloseHandle(Csrss);
		return true;
	}
	else
		return false;
}

void DebugSelf() {
	HANDLE hProcess = NULL;
	DEBUG_EVENT de;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&de, sizeof(DEBUG_EVENT));

	GetStartupInfoA(&si);

	// Create the copy of ourself
	CreateProcessA(NULL, GetCommandLineA(), NULL, NULL, FALSE,
		DEBUG_PROCESS, NULL, NULL, &si, &pi);

	// Continue execution
	ContinueDebugEvent(pi.dwProcessId, pi.dwThreadId, DBG_CONTINUE);

	// Wait for an event
	WaitForDebugEvent(&de, INFINITE);
}

bool NT_Check() {
	static tNtQueryInformationProcess pNtQueryInformationProcess = nullptr;
	if (pNtQueryInformationProcess == nullptr) {
		HINSTANCE hNt = NULL;
		hNt = LoadLibraryA("ntdll.dll");
		pNtQueryInformationProcess = (tNtQueryInformationProcess)GetProcAddress(hNt, "NtQueryInformationProcess");
	}

	HANDLE hDebugObject = 0;
	DWORD NoDebugInherit = 0;
	NTSTATUS debugObjStatus, processDebugStatuc;

	debugObjStatus = pNtQueryInformationProcess(GetCurrentProcess(),
		0x1e, // ProcessDebugObjectHandle
		&hDebugObject, 4, NULL);

	processDebugStatuc = pNtQueryInformationProcess(GetCurrentProcess(),
		0x1f, // ProcessDebugFlags
		&NoDebugInherit, 4, NULL);

	if (debugObjStatus != 0x00000000 || processDebugStatuc != 0x0000000)
		return false;

	if (hDebugObject && NoDebugInherit == FALSE)
		return true;
	else
		return false;
}

bool CheckOutputDebugString(LPCTSTR String) {
	OutputDebugStringA(String);
	if (GetLastError() == 0)
		return true;
	else
		return false;
}

bool CheckForCCBreakpoint(void* pMemory, size_t SizeToCheck) {
	unsigned char* pTmp = (unsigned char*)pMemory;

	for (size_t i = 0; i < SizeToCheck; i++)
		if (pTmp[i] == 0xCC)
			return true;

	return false;
}

int CheckHardwareBreakpoints() {
	unsigned int NumBps = 0;

	// This structure is key to the function and is the 
	// medium for detection and removal
	CONTEXT ctx;
	ZeroMemory(&ctx, sizeof(CONTEXT));

	// The CONTEXT structure is an in/out parameter therefore we have
	// to set the flags so Get/SetThreadContext knows what to set or get.
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	// Get a handle to our thread
	HANDLE hThread = GetCurrentThread();

	// Get the registers
	if (GetThreadContext(hThread, &ctx) == 0)
		return -1;

	// Now we can check for hardware breakpoints, its not 
	// necessary to check Dr6 and Dr7, however feel free to
	if (ctx.Dr0 != 0)
		++NumBps;
	if (ctx.Dr1 != 0)
		++NumBps;
	if (ctx.Dr2 != 0)
		++NumBps;
	if (ctx.Dr3 != 0)
		++NumBps;

	return NumBps;
}

void UnlinkModuleFromPEB(HMODULE hModule) {
#define UNLINK(x)               \
	(x).Flink->Blink = (x).Blink;   \
	(x).Blink->Flink = (x).Flink;

	static std::vector<UNLINKED_MODULE> UnlinkedModules;
	std::vector<UNLINKED_MODULE>::iterator it = std::find_if(UnlinkedModules.begin(), UnlinkedModules.end(), FindModuleHandle(hModule));
	if (it != UnlinkedModules.end())
		return;

#ifdef _WIN64
	PPEB pPEB = (PPEB)__readgsqword(0x60);
#else
	PPEB pPEB = (PPEB)__readfsdword(0x30);
#endif

	PLIST_ENTRY CurrentEntry = pPEB->Ldr->InLoadOrderModuleList.Flink;
	PLDR_DATA_TABLE_ENTRY Current = NULL;

	while (CurrentEntry != &pPEB->Ldr->InLoadOrderModuleList && CurrentEntry != NULL) {
		Current = CONTAINING_RECORD(CurrentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if (Current->DllBase == hModule) {
			UNLINKED_MODULE CurrentModule = { 0 };
			CurrentModule.hModule = hModule;
			CurrentModule.RealInLoadOrderLinks = Current->InLoadOrderLinks.Blink->Flink;
			CurrentModule.RealInInitializationOrderLinks = Current->InInitializationOrderLinks.Blink->Flink;
			CurrentModule.RealInMemoryOrderLinks = Current->InMemoryOrderLinks.Blink->Flink;
			CurrentModule.Entry = Current;
			UnlinkedModules.push_back(CurrentModule);

			UNLINK(Current->InLoadOrderLinks);
			UNLINK(Current->InInitializationOrderLinks);
			UNLINK(Current->InMemoryOrderLinks);

			break;
		}
		CurrentEntry = CurrentEntry->Flink;
	}
#undef UNLINK
}

bool RemoveHeader(HINSTANCE hinstDLL) {
	DWORD ERSize = 0;
	DWORD Protect = 0;
	DWORD dwStartOffset = (DWORD)hinstDLL;

	IMAGE_DOS_HEADER* pDosHeader = (PIMAGE_DOS_HEADER)dwStartOffset;
	IMAGE_NT_HEADERS* pNtHeader = (PIMAGE_NT_HEADERS)(dwStartOffset + pDosHeader->e_lfanew);

	ERSize = sizeof(IMAGE_NT_HEADERS);

	if (VirtualProtect(pDosHeader, ERSize, PAGE_EXECUTE_READWRITE, &Protect)) {
		for (DWORD i = 0; i < ERSize - 1; i++)
			* (PBYTE)((DWORD)pDosHeader + i) = 0x00;
	}
	else return false;

	VirtualProtect(pDosHeader, ERSize, Protect, 0);

	ERSize = sizeof(IMAGE_DOS_HEADER);

	if ((pNtHeader != 0) && VirtualProtect(pNtHeader, ERSize, PAGE_EXECUTE_READWRITE, &Protect)) {
		for (DWORD i = 0; i < ERSize - 1; i++)
			* (PBYTE)((DWORD)pNtHeader + i) = 0x00;
	}
	else return false;

	VirtualProtect(pNtHeader, ERSize, Protect, 0);

	return true;
}