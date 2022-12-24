#include "offset.h"

#include <unordered_map>

#include "globals.h"
#include "utils.h"

// universal representation for game modules
struct GameModule {
	DWORD start, end;

	GameModule() = default;

	// construct module with its start and end address
	GameModule(DWORD start_, DWORD end_) :
		start(start_),
		end(end_)
	{}

	// construct module with its name
	GameModule(const std::string& name) {
		auto get_module_size = [](DWORD address) -> DWORD {
			return PIMAGE_NT_HEADERS(address + (DWORD)PIMAGE_DOS_HEADER(address)->e_lfanew)->OptionalHeader.SizeOfImage;
		};

		start = (DWORD)GetModuleHandleA(std::string(name + ".dll").c_str());

		if (start == NULL)
			utils::TerminateGame("Module", name, "isn't loaded");

		end = start + get_module_size(start) - 1;
	}
};

// GameModules is a wrapper-class to make
// work with game modules more comfortable.
// to get module by name overloaded 'operator[]' is used.
// that operator also constructs module if there is no module with given name
class GameModules {
public:
	const GameModule& operator [] (const std::string& name) {
		auto el = this->modules.find(name);
		if (el != this->modules.end())
			return el->second;

		this->modules[name] = { name };
		return this->modules[name];
	}

private:
	std::unordered_map<std::string, GameModule> modules;
};

#define CompareMemory(Buff1, Buff2, Size) __comparemem((const UCHAR *)Buff1, (const UCHAR *)Buff2, (UINT)Size)
#define FindMemoryClone(Module, Clone, Size) __findmemoryclone((const ULONG)Module.start, (const ULONG)Module.end, (const ULONG)Clone, (UINT)Size)
#define FindReference(Module, Address) __findreference((const ULONG)Module.start, (const ULONG)Module.end, (const ULONG)Address)

BOOL __comparemem(const UCHAR* buff1, const UCHAR* buff2, UINT size) {
	for (UINT i = 0; i < size; i++, buff1++, buff2++)
	{
		if ((*buff1 != *buff2) && (*buff2 != 0xFF))
			return FALSE;
	}
	return TRUE;
}

ULONG __findmemoryclone(const ULONG start, const ULONG end, const ULONG clone, UINT size) {
	for (ULONG ul = start; (ul + size) < end; ul++) {
		if (CompareMemory(ul, clone, size))
			return ul;
	}
	return NULL;
}

ULONG __findreference(const ULONG start, const ULONG end, const ULONG address) {
	UCHAR Pattern[5];
	Pattern[0] = 0x68;
	*(ULONG*)& Pattern[1] = address;
	GameModule tmp = { static_cast<DWORD>(start), static_cast<DWORD>(end) };
	return FindMemoryClone(tmp, Pattern, sizeof(Pattern) - 1);
}

DWORD FindPattern(PCHAR pattern, PCHAR mask, const GameModule & mod) {
	size_t patternLength = strlen(pattern);
	bool found = false;

	for (DWORD i = mod.start; i < mod.end - patternLength; i++) {
		found = true;

		for (size_t idx = 0; idx < patternLength; idx++) {
			if (mask[idx] == 'x' && pattern[idx] != *(PCHAR)(i + idx)) {
				found = false;
				break;
			}
		}

		if (found)
			return i;
	}

	return 0;
}

// offsets by: Shatilova, h4rdee, JusicP
namespace offset {
	GameModules modules;

	DWORD ClientTable() {
		DWORD addr = (DWORD)FindMemoryClone(modules["hw"], "ScreenFade", strlen("ScreenFade"));
		return *(DWORD*)(FindReference(modules["hw"], addr) + 0x13);
	}

	DWORD EngineTable() {
		DWORD addr = (DWORD)FindMemoryClone(modules["hw"], "ScreenFade", strlen("ScreenFade"));
		return *(DWORD*)(FindReference(modules["hw"], addr) + 0x0D);
	}

	DWORD StudioTable() {
		return *(DWORD*)((DWORD)g::pClient->HUD_GetStudioModelInterface + 0x34);
	}

	DWORD StudioAPITable() {
		return *(DWORD*)((DWORD)g::pClient->HUD_GetStudioModelInterface + 0x3A);
	}

	DWORD UserMsgBase() {
		DWORD addr = (DWORD)FindMemoryClone(modules["hw"], "UserMsg: Not Present on Client %d", strlen("UserMsg: Not Present on Client %d"));
		return *(DWORD*)* (DWORD*)(FindReference(modules["hw"], addr) - 0x14);
	}

	DWORD EventBase() {
		return *(DWORD*)(*(DWORD*)((DWORD)g::pEngine->HookEvent + 0x77));
	}

	DWORD Speed() {
		DWORD addr = (DWORD)FindMemoryClone(modules["hw"], "Texture load: %6.1fms", strlen("Texture load: %6.1fms"));
		DWORD ptr = *(DWORD*)(FindReference(modules["hw"], addr) - 0x09);

		DWORD old_prot;
		VirtualProtect((void*)ptr, sizeof(double), PAGE_READWRITE, &old_prot);
		return ptr;
	}

	DWORD ButtonsBase() {
		return *(DWORD*)(FindPattern((PCHAR)"\x0F\x44\xCA\x8B\xD1\x83\xCA\x20\x24\x03\x0F\x44\xD1\x83\x3D\x00\x00\x00\x00\x00\x74",
			(PCHAR)"xxxxxxxxxxxxxxx?????x", modules["client"]) - 0x04) - 0x08;
	}


	DWORD PlayerMove() {
		DWORD addr = (DWORD)FindMemoryClone(modules["hw"], "ScreenFade", strlen("ScreenFade"));
		return *(DWORD*)(FindReference(modules["hw"], addr) + 0x24);
	}

	DWORD ClientState() {
		return *(DWORD*)((DWORD)g::pEngine->SetScreenFade + 0x26) - 0x44C;
	}

	DWORD ClientStatic() {
		DWORD addr = (DWORD)FindMemoryClone(modules["hw"], "WARNING:  Connection Problem", strlen("WARNING:  Connection Problem"));
		return *(DWORD*)(FindReference(modules["hw"], addr) + 0xDE) - 0x08;
	}
}