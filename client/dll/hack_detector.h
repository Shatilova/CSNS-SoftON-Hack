// if you want, you can talk to other hack's developers,
// and add mutual detection of your hacks by each other
// using ingame server-side cvar to store any data.
// in our case, to store hack's ID/name
// 
// it must to be said that it's unsafe at all to store something
// inside serverside cvars

#pragma once

#include <string>
#include <optional>

namespace hack_detector {
	void AddHack(const std::string& name, const std::string& key);
	void SetKey(const std::string& key);
	void SetSignature(const std::string& name);
	std::optional<std::string> ReceiveHackInfo(size_t player_id);
	void SendHackInfo();
}