#include "hook.h"

#include <sstream>
#include "valve-sdk/misc/parsemsg.h"

#include "utils.h"

PUserMsg UserMsgByName(const char* name) {
	PUserMsg Ptr = g::pUserMsgBase;

	while (Ptr->next) {
		if (!strcmp(Ptr->name, name))
			return Ptr;

		Ptr = Ptr->next;
	}

	Ptr->pfn = 0;
	return Ptr;
}

event_hook_s* EventByName(const char* name) {
	event_hook_s* pEv = g::pEventBase;
	while (pEv->next) {
		if (!strcmp(pEv->name, name))
			return pEv;

		pEv = pEv->next;
	}

	pEv->pfn = 0;
	return pEv;
}

_pcmd_t CommandByName(const char* name) {
	_pcmd_t pCmd = g::Engine.GetCmdList();

	while (pCmd) {
		if (!strcmp(pCmd->name, name))
			return pCmd;
		pCmd = pCmd->next;
	}

	return NULL;
}

namespace hook {
	pfnEventHook Event(const char* name, pfnEventHook func) {
		pfnEventHook pfnOrig = nullptr;
		event_hook_s* ptr = EventByName(name);

		if (ptr->pfn) {
			pfnOrig = ptr->pfn;
			ptr->pfn = func;
		}
		else
			utils::TerminateGame(name, "game event not found");

		return pfnOrig;
	}

	xcommand_t Command(const char* name, xcommand_t func) {
		_pcmd_t command = CommandByName(name);
		xcommand_t origCommand = command->function;

		if (command != nullptr)
			command->function = func;
		else
			utils::TerminateGame(name, "console command not found");

		return origCommand;
	}

	pfnUserMsgHook UserMsg(const char* name, pfnUserMsgHook func) {
		pfnUserMsgHook orig_func = nullptr;
		PUserMsg Ptr = UserMsgByName(name);

		if (Ptr->pfn) {
			orig_func = Ptr->pfn;
			Ptr->pfn = func;
		}
		else
			utils::TerminateGame(name, "user message not found");

		return orig_func;
	}
}