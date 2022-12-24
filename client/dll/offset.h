#pragma once

#include <Windows.h>

namespace offset {
	DWORD ClientTable();
	DWORD EngineTable();
	DWORD StudioTable();
	DWORD StudioAPITable();
	DWORD UserMsgBase();
	DWORD EventBase();
	DWORD Speed();
	DWORD ButtonsBase();
	DWORD PlayerMove();
	DWORD ClientState();
	DWORD ClientStatic();
}