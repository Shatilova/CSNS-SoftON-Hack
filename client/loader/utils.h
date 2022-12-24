// some internal functions to:
// 1) terminating application
// 2) work with processes: get PID, get process handle
// 3) get loader hash

#pragma once

#include <Windows.h>
#include <sstream>
#include <string>

namespace utils {
	// this function looks like 'print' function in Python:
	// it takes variadic number of arguments, 
	// and put them to stringstream, and use that stringstream
	// as a message for MessageBox.
	// after each argument whitespace is put automatically
	template<typename ... Args>
	void Terminate(Args&& ... args) {
		std::stringstream ss;
		((ss << std::forward<Args>(args) << " "), ...);

		const std::string tmp_str = ss.str();
		MessageBoxA(NULL, tmp_str.c_str(), "SoftON Internal Error", MB_OK | MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), 0);
	}

	DWORD PIDByName(const char* name);
	HANDLE ProcessByName(const char* name);
	int GetLoaderHash();
}
