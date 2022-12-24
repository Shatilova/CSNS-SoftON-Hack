#include "hack_detector.h"

#include <unordered_map>

#include "globals.h"
#include "utils.h"

namespace hack_detector {
	std::string key_for_store;
	std::string signature;
	std::unordered_map <std::string, std::string> hacks_list;

	void CorrectnessCheck() {
		if (key_for_store.empty()) {
			utils::Warning("Key for hack detector isn't set. Setting 'bottomcolor'...");
			key_for_store = "bottomcolor";
		}
	}

	void AddHack(const std::string& name, const std::string& key) {
		if (hacks_list.find(name) != hacks_list.end())
			utils::Warning("Hack", name, "already exists with key [", hacks_list[name], "] . Replacing key with [", key, "]");

		hacks_list[key] = name;
	}

	void SetKey(const std::string & key) { key_for_store = key; }

	void SetSignature(const std::string & name) { signature = name; }

	std::optional<std::string> ReceiveHackInfo(size_t player_id) {
		CorrectnessCheck();

		const char* value_of_key = g::Engine.PlayerInfo_ValueForKey(player_id, key_for_store.c_str());
		if (value_of_key != 0 && strlen(value_of_key) > 0) {
			for (const auto& [key, name] : hacks_list)
				if (std::string(value_of_key) == key)
					return name;

			return "Unknown: " + std::string(value_of_key);
		}

		return std::nullopt;
	}

	void SendHackInfo() {
		CorrectnessCheck();

		g::Engine.PlayerInfo_SetValueForKey(key_for_store.c_str(), signature.c_str());
	}
}