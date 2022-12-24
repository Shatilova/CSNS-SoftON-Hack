#pragma once

#include "../engine/cl_dll.h"

typedef struct TUserMsg
{
	int number;
	int size;
	char name[16];
	TUserMsg* next;
	pfnUserMsgHook pfn;
} *PUserMsg;