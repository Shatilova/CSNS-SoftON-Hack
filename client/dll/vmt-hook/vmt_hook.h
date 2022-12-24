#pragma once

#include <Windows.h>

class VHookTable
{
private:

	DWORD dwPtrPtrTable;
	DWORD dwPtrNewTable;
	DWORD dwPtrOldTable;
	DWORD dwSizeTable;

public:

	bool HookTable(DWORD dwTablePtrPtr);
	void HookIndex(DWORD dwIndex, PVOID pAddress);

	DWORD RetHookIndex(DWORD dwIndex, PVOID pAddress);

	void UnHook();
	void ReHook();
};