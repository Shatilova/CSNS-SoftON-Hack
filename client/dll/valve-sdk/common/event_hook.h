#pragma once

#include "event_args.h"

typedef void (*pfnEventHook)(event_args_s* args);
typedef struct event_hook_t {
	event_hook_t* next;
	const char* name;
	pfnEventHook pfn;
}event_hook_s;