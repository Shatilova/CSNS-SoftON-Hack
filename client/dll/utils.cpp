#include "utils.h"

#include <fstream>
#include <string>

#include "globals.h"
#include "file_system.h"
#include "usermsg.h"

namespace utils {
	// pure binary cfg saver
	void SaveCFG() {
		std::string filepath = std::string(internal_fs::GetRoot() + "SoftON.cfg");
		std::ofstream ofs(filepath.c_str(), std::ios::binary);
		ofs.write(
			reinterpret_cast<char*>(&g::Settings),
			sizeof(g::Settings)
		);
		ofs.close();
	}

	// pure binary cfg loader
	void LoadCFG() {
		std::string filepath = std::string(internal_fs::GetRoot() + "SoftON.cfg");
		std::ifstream ifs(filepath.c_str(), std::ios::binary);
		if (ifs) {
			ifs.read(reinterpret_cast<char*>(&g::Settings),
				sizeof(g::Settings)
			);
			ifs.close();

			if (pSetFOV && pReceiveW) {
				(*pSetFOV)("SetFOV", 1, &g::Settings.Visual.FOV);
				(*pReceiveW)("ReceiveW", 1, &g::Settings.Visual.Rain);
			}

			g::Settings.Visual.NoFog ? g::Engine.ClientCmd((char*)"gl_fog 0") : g::Engine.ClientCmd((char*)"gl_fog 1");
		}
	}
}