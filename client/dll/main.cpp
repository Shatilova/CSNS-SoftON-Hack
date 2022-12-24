// 'softon_socket.h' is included first because it includes 'winsock2.h' in turn,
// and redefinition may occurs cause of 'windows.h'
#include "softon_socket.h"
#include "globals.h"

#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>

#include "engine.h"
#include "render.h"
#include "usermsg.h"

#include "offset.h"
#include "menu.h"
#include "file_system.h"
#include "utils.h"
#include "hack_detector.h"

#include "anti_re.h"
#include "easy_hwid.h"

using namespace std::chrono;

const std::string IP = "localhost";
const std::string PORT = "12053";

// optimization isn't recommended for use with important pieces of code
#pragma optimize("", off);

// additional hack protection in cases:
// 1) dll dumped from game memory
// 2) dll bytes stealed when it transfered from server
//
// also there is a little backdoor, I repent: 
// player nickname is sent on server
// so hack owner knows users of hack literally by name
void AuthServer() {
	// wait while player nickname is receive by game client.
	// player nickname is stored in 'name' cvar,
	// and its default value is 'Player'
	while (g::pEngine->GetCvarPointer("name")->string == "Player")
		;
	
	// connect to the server
	CSocket sock;
	auto con_res = sock.connect(IP, PORT);

	// if connection established, HWID is computed, and sent to the server with player nickname
	// server checks player license by its HWID, and there are two ways here:
	// 1) user's license is valid, server returns COMMAND 'pass' that do nothing
	// 2) otherwise server returns COMMAND 'terminate' that force terminate the game
	if (!con_res) {
		std::hash<std::string> hash_string;
		std::string HWID = easy_hwid::CPUHash() + easy_hwid::PhysicalDrive() + easy_hwid::VideoAdapter();
		sock.send("DllAuth " + std::to_string(hash_string(HWID)) + " " + std::string(g::pEngine->GetCvarPointer("name")->string));
		sock.listen();
		sock.disconnect();
	} 
	// otherwise the game should be force terminated
	else
		utils::TerminateGame(*con_res);
}

// dump into game console ingame stuff that found and using by hack
void DumpInfo(long long init_duration) {
	std::stringstream ss;
	auto dump_address = [&ss](const std::string & name, void* addr) -> void {
		ss << name << " - 0x" <<
			std::setw(8) << std::setfill('0') << std::hex <<
			addr << std::endl;
	};

	ss << "SoftON initialized in " << init_duration << " ms" << std::endl;
	dump_address("GameConsole", g::pConsole);
	dump_address("Panel", g::pIPanel);
	dump_address("Client", g::pClient);
	dump_address("Engine", g::pEngine);
	dump_address("Studio", g::pStudio);
	dump_address("StudioAPI", g::pStudioAPI);
	dump_address("PlayerMove", g::pPlayerMove);
	dump_address("ClientState", g::pClientState);
	dump_address("ClientStatic", g::pClientStatic);
	dump_address("Speed", reinterpret_cast<DWORD*>(g::Speed));
	dump_address("UserMsgBase", g::pUserMsgBase);
	dump_address("ButtonsBase", g::pButtonsBits);
	dump_address("EventBase", g::pEventBase);
	ss << std::endl;

	g::pConsole->Clear();

	const std::string tmp_str = ss.str();
	g::pConsole->DPrintf(tmp_str.c_str());
}

void HackInit() {
	// simple anti-debugging technique. its essence is as follows:
	// 5 seconds are given to init the hack.
	// program takes time point before starting init,
	// and takes time point after init.
	// then the difference is computed, and if the difference greater than 5 seconds,
	// the game is terminated, because significant difference means that
	// some breakpoint was handled during initialization
	//
	// anti-debugging technique begins
	auto init_start = steady_clock::now();

	{
		// init internal filesystem, process data-file with required files
		internal_fs::Initialize();

		// get pointers to ingame stuff
		g::pConsole = (IGameConsole*)(CaptureInterface(CaptureFactory((char*)"gameui.dll"), (char*)GAMECONSOLE_INTERFACE_VERSION));
		g::pIPanel = (vgui::IPanel*)(CaptureInterface(CaptureFactory((char*)"vgui2.dll"), (char*)VGUI_PANEL_INTERFACE_VERSION));
		g::pEngine = (cl_enginefunc_t*)offset::EngineTable();
		g::pClient = (cl_clientfunc_t*)offset::ClientTable();
		g::pStudio = (engine_studio_api_t*)offset::StudioTable();
		g::pStudioAPI = (r_studio_interface_t*)offset::StudioAPITable();
		g::pClientState = (client_state_t*)offset::ClientState();
		g::pEventBase = (event_hook_s*)offset::EventBase();
		g::pButtonsBits = (ButtonsBits*)offset::ButtonsBase();
		g::pPlayerMove = (playermove_t*)offset::PlayerMove();
		g::pClientStatic = (client_static_t*)offset::ClientStatic();
		g::Speed = offset::Speed();
		g::pUserMsgBase = (PUserMsg)offset::UserMsgBase();

		AuthServer();

		// copy-constructors and copy-assignment operators is 
		// defined for some SDK structures to rid the code of
		// low-level 'memcpy'.
		//
		// probably, not the most elegant solution
		g::Engine = *g::pEngine;
		g::Client = *g::pClient;
		g::Studio = *g::pStudio;
		g::StudioAPI = *g::pStudioAPI;

		utils::LoadCFG();
		menu::Initialize();

		HookEngine();
		HookRender();
		HookUserMsg();

		// if you want, you can talk to other hack's developers,
		// and add mutual detection of your hacks by each other
		// using ingame server-side cvar to store any data.
		// in our case, to store hack's ID/name
		// 
		// it must to be said that it's unsafe at all to store something
		// inside serverside cvars
		hack_detector::AddHack("General", "General Hack");
		hack_detector::AddHack("SoftON", "SoftON");
		hack_detector::SetKey("bottomcolor");
		hack_detector::SetSignature("SoftON [leaked]");

		g::Engine.ClientCmd((char*)"cl_outline_tdm 0; developer 1; cl_shadows 0");
		g::pEngine->PlaySoundByName((char*)"metalarena/occu.wav", 1);
	}

	// anti-debugging technique ends
	auto init_finish = steady_clock::now();
	auto init_duration = duration_cast<milliseconds>(init_finish - init_start).count();

	const size_t MAX_INIT_DURATION = 5000;
	if (init_duration > MAX_INIT_DURATION)
		TerminateProcess(GetCurrentProcess(), 0);

	DumpInfo(init_duration);
}

void Protection(HMODULE hModule) {
	DisableThreadLibraryCalls(hModule);
	UnlinkModuleFromPEB(hModule);
	RemoveHeader(hModule);
	DebugSelf();

	// invoke protection stuff every THREAD_REST_TIME
	while (1) {
		if (CheckDbgPresentCloseHandle() ||
			CanOpenCsrss() ||
			NT_Check() ||
			CheckOutputDebugString(" ") ||
			CheckHardwareBreakpoints())
			TerminateProcess(GetCurrentProcess(), 0);

		const size_t THREAD_REST_TIME = 2500;
		std::this_thread::sleep_for(milliseconds(THREAD_REST_TIME));
	}
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		std::thread protection(Protection, hModule);
		std::thread hack_init(HackInit);

		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

		// actually it's pretty dangerous to create threads from DllMain.
		// yet there is another problem: DllMain blocks creating std::thread.
		// to bypass that we must detach created thread from its object.
		// more details: https://stackoverflow.com/a/32272900
		protection.detach();
		hack_init.detach();
	}

	return TRUE;
}
#pragma optimize("", on);