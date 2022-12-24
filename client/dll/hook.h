// collection of functions to hook:
// 1) Game Event:      createsmoke, g3sg1, mac10, etc...
// 2) Console Command: cl_seethru, gamma, impulse, etc...
// 3) User Message:    BanPick, IGFactor, MonShootSnd, etc...

#pragma once

#include "globals.h"

namespace hook {
	pfnEventHook Event(const char* name, pfnEventHook func);
	xcommand_t Command(const char* name, xcommand_t func);
	pfnUserMsgHook UserMsg(const char* name, pfnUserMsgHook func);
}