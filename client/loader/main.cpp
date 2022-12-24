#define _CRT_SECURE_NO_WARNINGS

#include "softon_socket.h"
#include "anti_re.h"
#include "easy_hwid.h"

#include <fstream>
#include <thread>
#include <filesystem>

#include "render.h"
#include "mmap.h"
#include "utils.h"
#include "file_system.h"

namespace fs = std::filesystem;

const std::string IP = "localhost";
const std::string PORT = "12053";

#pragma optimize("", off);
// implementation of ImageButton.
// file_system must contains next three pictures named like:
// 1) "%btn_base_name%_n" - NoActive condition,
// 2) "%btn_base_name%_o" - OnMouse condition,
// 3) "%btn_base_name%_c" - OnClick condition.
bool ImageButton(ImVec2 pos, const std::string& btn_base_name) {
	const internal_fs::Image n_ = internal_fs::GetImageByName(btn_base_name + "_n"),
		o_ = internal_fs::GetImageByName(btn_base_name + "_o"),
		c_ = internal_fs::GetImageByName(btn_base_name + "_c");

	bool pressed = false;
	ImGui::SetCursorPos(pos);
	ImGui::Image((GLuint*)n_.id, ImVec2(n_.w, n_.h));

	if (ImGui::IsItemHovered()) {
		ImGui::SetCursorPos(pos);
		ImGui::Image((GLuint*)o_.id, ImVec2(o_.w, o_.h));
	}

	if (ImGui::IsItemClicked() && !ImGui::GetIO().MouseReleased[0]) {
		ImGui::SetCursorPos(pos);
		ImGui::Image((GLuint*)c_.id, ImVec2(c_.w, c_.h));
		pressed = true;
	}

	return pressed;
}

// check whether data-file exists and is valid
bool DataFound(const std::string& data_path) {
	// existness check
	std::ifstream dat(data_path.c_str(), std::ios_base::binary);
	if (!dat)
		return false;

	// magic number check
	std::string magic;
	dat.read(magic.data(), 4);
	if (magic != "SFTN") {
		dat.close();
		return false;
	}

	dat.close();

	return true;
}

// retrieve data-file contains pictures and font used by dll
void RetrieveData(std::string& resp) {
	CSocket sock;

	auto conn_res = sock.connect(IP, PORT);
	if (conn_res) {
		resp = *conn_res;
		return;
	}

	std::string data_path = getenv("APPDATA");
	data_path += "\\SoftON\\SoftON.dat";

	if (DataFound(data_path)) {
		sock.send("DllData");

		unsigned char* data_file = sock.listen();
		std::ofstream ofs(data_path.c_str(), std::ios::binary);
		ofs.write((const char*)data_file, sock.get_last_listen_size());
		ofs.close();

		delete[] data_file;

		sock.disconnect();
	}
}

unsigned char* RetrieveDll(std::string& resp) {
	CSocket sock;

	auto conn_res = sock.connect(IP, PORT);
	if (conn_res) {
		resp = *conn_res;
		return nullptr;
	}

	std::hash<std::string> hash_string;
	std::string HWID = easy_hwid::CPUHash() + easy_hwid::PhysicalDrive() + easy_hwid::VideoAdapter();
	sock.send("ProcessDll " + std::to_string(hash_string(HWID)) + " " + std::to_string(utils::GetLoaderHash()));

	std::string tmp = (const char*)sock.listen();
	if (!tmp.empty())
		resp = tmp;

	unsigned char* raw = sock.listen();
	sock.disconnect();

	return raw;
}

std::string RetriveHackInfo(std::string & resp) {
	CSocket sock;
	std::string hack_info;
	resp.clear();

	auto conn_res = sock.connect(IP, PORT);
	if (!conn_res) {
		sock.send("HackInfo");

		hack_info = reinterpret_cast<const char*>(sock.listen());
		sock.disconnect();
	} else
		resp = *conn_res;

	return hack_info;
}

void Protection() {
	DebugSelf();

	while (1) {
		if (CanOpenCsrss() ||
			CheckOutputDebugString(" ") ||
			CheckHardwareBreakpoints())
			TerminateProcess(GetCurrentProcess(), 0);

		const size_t THREAD_REST_TIME = 1000;
		std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_REST_TIME));
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	std::thread protection(Protection);
	protection.detach();

	// firstly data-file for loader will be downloaded
	while (!internal_fs::Initialize()) {
		CSocket sock;
		std::string hack_info, resp;
		auto conn_res = sock.connect(IP, PORT);
		if (!conn_res) {
			sock.send("LoaderData");

			unsigned char* data_file = sock.listen();
			std::ofstream ofs(std::string(internal_fs::get_root() + "Loader.dat").c_str(), std::ios::binary);
			ofs.write((const char*)data_file, sock.get_last_listen_size());
			ofs.close();

			sock.disconnect();
		} else
			resp = *conn_res;
	}

	Rendering render;
	GLFWwindow* window = render.get_window();

	ImFont* font;

	const auto& font_file = internal_fs::GetFileByName("RobotoBlack.ttf");
	void* font_raw = reinterpret_cast<void*>(const_cast<unsigned char*>(font_file.raw.data()));
	font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_raw, font_file.header.size, 30.f);

	internal_fs::GenerateImage("bg.png");
	internal_fs::GenerateImage("btn_close_c.png");
	internal_fs::GenerateImage("btn_close_n.png");
	internal_fs::GenerateImage("btn_close_o.png");
	internal_fs::GenerateImage("btn_inject_c.png");
	internal_fs::GenerateImage("btn_inject_n.png");
	internal_fs::GenerateImage("btn_inject_o.png");
	internal_fs::GenerateImage("close.png");
	internal_fs::GenerateImage("hackinfo_bg.png");
	internal_fs::GenerateImage("responce_bg.png");

	std::string response, hack_info = RetriveHackInfo(response);

	HANDLE process_may_be_opened = NULL;

	while (!glfwWindowShouldClose(window)) {
		render.begin();

		ImGui::SetNextWindowSize(ImVec2(512, 320));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowBgAlpha(0.f);
		ImGui::Begin("###Main", (bool*)1, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
		{
			const internal_fs::Image bg = internal_fs::GetImageByName("bg");
			ImGui::GetWindowDrawList()->AddImage((unsigned int*)bg.id, ImVec2(0, 0), ImVec2(bg.w, bg.h));

			// if hack info is not received
			// then something went wrong, and
			// user should try recconnect to the server
			if (hack_info.empty()) {
				if (ImageButton(ImVec2(256, 7), "btn_inject"))
					hack_info = RetriveHackInfo(response);

				ImGui::SetCursorPos(ImVec2(275, 22));
				ImGui::PushFont(font);
				ImGui::Text("RECONNECT");
				ImGui::PopFont();
			}
			// game must be runned
			else if (!utils::ProcessByName("cstrike-online.exe")) {
				ImGui::SetCursorPos(ImVec2(280, 28));
				ImGui::Text("Please run the game first");
			}
			else {
				// check for admin rights
				// admin rights is required for MMAP injection
				static bool admin_check = false;
				if (!admin_check) {
					DWORD pID = utils::PIDByName("cstrike-online.exe");
					process_may_be_opened = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

					admin_check = true;
				}

				// if there is no admin rights
				// user must restart application with it
				if (!process_may_be_opened) {
					ImGui::SetCursorPos(ImVec2(288, 22));
					ImGui::Text("Please run this loader \n    with admin rights");
				}
				// otherwise everything is OK:
				// 1) connection to the server established
				// 2) game is running
				// 3) loader is running with admin rights
				else {
					if (ImageButton(ImVec2(256, 7), "btn_inject")) {
						response.clear();
						RetrieveData(response);
						MMAP("cstrike-online.exe", RetrieveDll(response));
					}

					ImGui::SetCursorPos(ImVec2(305, 22));
					ImGui::PushFont(font);
					ImGui::Text("INJECT");
					ImGui::PopFont();
				}
			}

			// response field: image, title, and content
			const internal_fs::Image responce_bg = internal_fs::GetImageByName("responce_bg");
			ImGui::GetWindowDrawList()->AddImage((unsigned int*)responce_bg.id, ImVec2(261, 72), ImVec2(261 + responce_bg.w, 72 + responce_bg.h));
			ImGui::SetCursorPos(ImVec2(349, 79));
			ImGui::Text("Response");
			ImGui::PushTextWrapPos(500.f);
			ImGui::SetCursorPos(ImVec2(267, 104));
			ImGui::Text(response.c_str());
			ImGui::PopTextWrapPos();

			// hack info field: image, title, and content
			const internal_fs::Image hackinfo_bg = internal_fs::GetImageByName("hackinfo_bg");
			ImGui::GetWindowDrawList()->AddImage((unsigned int*)hackinfo_bg.id, ImVec2(10, 10), ImVec2(10 + hackinfo_bg.w, 10 + hackinfo_bg.h));
			ImGui::SetCursorPos(ImVec2(101, 17));
			ImGui::Text("Hack Info");
			ImGui::PushTextWrapPos(251.f);
			ImGui::SetCursorPos(ImVec2(17, 42));
			ImGui::Text(hack_info.c_str());
			ImGui::PopTextWrapPos();

			// break the loop if close button is pressed
			if (ImageButton(ImVec2(447, 7), "btn_close"))
				break;

			const internal_fs::Image close = internal_fs::GetImageByName("close");
			ImGui::GetWindowDrawList()->AddImage((unsigned int*)close.id, ImVec2(447, 7), ImVec2(447 + close.w, 7 + close.h));
		}
		ImGui::End();

		render.end();
	}

	render.shutdown();

	return 0;
}
#pragma optimize("", on);