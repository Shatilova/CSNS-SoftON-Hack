#include "utils.h"

#include <windows.h>
#include <iostream>
#include <Tlhelp32.h>
#include <fstream>
#include <limits>

#ifdef max
#undef max
#endif

namespace utils {
	DWORD PIDByName(const char* name) {
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
		PROCESSENTRY32 pEntry;
		pEntry.dwSize = sizeof(pEntry);
		BOOL hRes = Process32First(hSnapShot, &pEntry);
		while (hRes) {
			if (strcmp(pEntry.szExeFile, name) == 0) {
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
					(DWORD)pEntry.th32ProcessID);
				if (hProcess != NULL) {
					CloseHandle(hSnapShot);
					return pEntry.th32ProcessID;
				}
			}
			hRes = Process32Next(hSnapShot, &pEntry);
		}
		CloseHandle(hSnapShot);
	}

	HANDLE ProcessByName(const char* name) {
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
		PROCESSENTRY32 pEntry;
		pEntry.dwSize = sizeof(pEntry);
		BOOL hRes = Process32First(hSnapShot, &pEntry);
		while (hRes) {
			if (strcmp(pEntry.szExeFile, name) == 0) {
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
					(DWORD)pEntry.th32ProcessID);
				if (hProcess != NULL) {
					CloseHandle(hSnapShot);
					return hProcess;
				}
			}
			hRes = Process32Next(hSnapShot, &pEntry);
		}
		CloseHandle(hSnapShot);
		return 0x0;
	}


	int GetLoaderHash() {
		char filename[256];
		GetModuleFileNameA(NULL, filename, 255);

		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		ifs.ignore(std::numeric_limits<std::streamsize>::max());
		int length = ifs.gcount();
		ifs.close();

		std::hash<int> hash_int;
		return hash_int(length);
	}
}