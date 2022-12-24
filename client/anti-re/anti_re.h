/*
Author: Joshua Jackson
Date: Nov 09, 2008

Modified by Shatilova. Changelog:
- PEB (Process Environment Block) public stuff added
- the code is split into header and source code files
- some features removed
Date: Feb 22, 2021
*/

#pragma once

#include <Windows.h>

// CheckCloseHandle will call CloseHandle on an invalid
// DWORD aligned value and if a debugger is running an exception
// will occur and the function will return true otherwise it'll
// return false
bool CheckDbgPresentCloseHandle();

// The function will attempt to open csrss.exe with 
// PROCESS_ALL_ACCESS rights if it fails we're 
// not being debugged however, if its successful we probably are
bool CanOpenCsrss();

// Debug self is a function that uses CreateProcess
// to create an identical copy of the current process
// and debugs it
void DebugSelf();

// This function uses NtQuerySystemInformation
// to try to retrieve a handle to the current
// process's debug object handle. If the function
// is successful it'll return true which means we're
// being debugged or it'll return false if it fails
// or the process isn't being debugged.
// CheckProcessDebugFlags will return true if 
// the EPROCESS->NoDebugInherit is == FALSE, 
// the reason we check for false is because 
// the NtQueryProcessInformation function returns the
// inverse of EPROCESS->NoDebugInherit so (!TRUE == FALSE)
bool NT_Check();

// CheckOutputDebugString checks whether or 
// OutputDebugString causes an error to occur
// and if the error does occur then we know 
// there's no debugger, otherwise if there IS
// a debugger no error will occur
bool CheckOutputDebugString(LPCTSTR String);

// INT 3h breakpoints are represented in 
// the IA-32 instruction set with the opcode CC (0xCC)
bool CheckForCCBreakpoint(void* pMemory, size_t SizeToCheck);

// CheckHardwareBreakpoints returns the number of hardware 
// breakpoints detected and on failure it returns -1.
int CheckHardwareBreakpoints();

// Hiding module
void UnlinkModuleFromPEB(HMODULE hModule);

// This function will erase the current images
// PE header from memory preventing a successful image
// if dumped
bool RemoveHeader(HINSTANCE hinstDLL);