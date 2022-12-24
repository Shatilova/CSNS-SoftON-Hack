// fully encapsulated player class.
// there is the only 'update' method to update player state,
// also there are two friends:
// 1) class PlayersList - used for set player ID 
// 2) TeamInfo function - used for update player team

#pragma once

#include <string>
#include <optional>
#include "valve-sdk/engine/cl_entity.h"

class Player {
public:
	friend class PlayersList;
	friend int TeamInfo(const char* pszName, int iSize, void* pbuf);

	size_t get_id() const { return this->id; }

	Team get_team() const { return this->team; }

	bool is_alive() const { return this->alive; }
	bool is_visible() const { return this->visible; }

	Vector get_origin(int z_offset = 0);

	float get_distance() const { return this->distance; }
	float get_frametime() const { return this->frametime; }

	char* get_nickname() const { return this->nickname; }

	std::optional<std::string> is_hacker() const {
		if (this->hack_used)
			return this->hack_used;

		return std::nullopt;
	}

	void update();

private:
	size_t id = 0;
	Team team = Team::UNK;
	bool alive = false;
	bool visible = false;
	Vector origin = { 0, 0, 0 };
	float distance = 0;
	float fov = 0;
	float frametime = 0;
	float top_z_offset = 0, bottom_z_offset = 0;
	std::optional<std::string> hack_used = std::nullopt;
	char* nickname = 0;
};