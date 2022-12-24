// hack-independent utils to work with binary cfg-file,
// and display messages about errors

#pragma once

#include <Windows.h>
#include <sstream>

namespace utils {
	void SaveCFG();
	void LoadCFG();

	// the following functions looks like 'print' function in Python:
	// it takes variadic number of arguments, 
	// and put them to stringstream, and use that stringstream
	// as a message for MessageBox.
	// after each argument whitespace is put automatically
	template<typename ... Args>
	void Warning(Args&& ... args) {
		std::stringstream ss;
		((ss << std::forward<Args>(args) << " "), ...);

		const std::string tmp_str = ss.str();
		MessageBoxA(NULL, tmp_str.c_str(), "SoftON Internal Warning", MB_OK | MB_ICONINFORMATION);
	}

	template<typename ... Args>
	void TerminateGame(Args && ... args) {
		std::stringstream ss;
		((ss << std::forward<Args>(args) << " "), ...);

		const std::string tmp_str = ss.str();
		MessageBoxA(NULL, tmp_str.c_str(), "SoftON Internal Error", MB_OK | MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), 0);
	}
}
