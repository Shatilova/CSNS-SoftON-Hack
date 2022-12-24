#include "vmt_hook.h"

bool VHookTable::HookTable(DWORD dwTablePtrPtr)
{
	DWORD dwIndexFunction = 0;

	dwPtrPtrTable = dwTablePtrPtr;
	dwPtrOldTable = *(PDWORD)dwPtrPtrTable;

	for (dwIndexFunction = 0; ((PDWORD) * (PDWORD)dwTablePtrPtr)[dwIndexFunction]; dwIndexFunction++)
		if (IsBadCodePtr((FARPROC)((PDWORD) * (PDWORD)dwTablePtrPtr)[dwIndexFunction]))
			break;

	dwSizeTable = sizeof(DWORD) * dwIndexFunction;

	if (dwIndexFunction && dwSizeTable)
	{
		dwPtrNewTable = (DWORD)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSizeTable);
		memcpy((PVOID)dwPtrNewTable, (PVOID) * (PDWORD)dwTablePtrPtr, dwSizeTable);

		*(PDWORD)dwTablePtrPtr = dwPtrNewTable;

		return true;
	}

	return false;
}

void VHookTable::HookIndex(DWORD dwIndex, PVOID pAddress)
{
	((PDWORD)dwPtrNewTable)[dwIndex] = (DWORD)pAddress;
}

DWORD VHookTable::RetHookIndex(DWORD dwIndex, PVOID pAddress)
{
	DWORD old = ((PDWORD)dwPtrNewTable)[dwIndex];
	((PDWORD)dwPtrNewTable)[dwIndex] = (DWORD)pAddress;
	return old;
}

void VHookTable::UnHook()
{
	if (dwPtrPtrTable)
		* (PDWORD)dwPtrPtrTable = dwPtrOldTable;
}

void VHookTable::ReHook()
{
	if (dwPtrPtrTable)
		* (PDWORD)dwPtrPtrTable = dwPtrNewTable;
}