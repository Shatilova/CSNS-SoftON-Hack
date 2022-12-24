#pragma once

#include "globals.h"

// these two pointers to UserMsg functions is used outside 
extern pfnUserMsgHook pSetFOV;
extern pfnUserMsgHook pReceiveW;

void HookUserMsg();