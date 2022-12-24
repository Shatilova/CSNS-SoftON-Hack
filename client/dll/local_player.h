// fully encapsulated local player class.
// there is the only 'update' method to update player state,
// also there is a friends:
// - TeamInfo function - used for update local player team

#pragma once

#include <array>

#include "valve-sdk/engine/util_vector.h"
#include "valve-sdk/engine/cl_entity.h"

enum class ViewVector {
	FORWARD,
	RIGHT,
	UP,
};

class LocalPlayer {
public:
	friend int TeamInfo(const char* pszName, int iSize, void* pbuf);

	size_t get_id() { return this->id; }

	Team get_team() { return this->team; }

	bool is_alive() { return this->alive; }

	Vector get_eye() { return this->eye; }
	Vector get_view_vector(ViewVector vv) {
		return this->view_vector[static_cast<int>(vv)];
	}

	float get_frametime() { return this->frametime; }

	void set_speed(double speed);
	void update();

private:
	size_t id = 0;
	Team team = Team::UNK;
	bool alive = false;
	Vector eye = { 0, 0, 0 };
	std::array<Vector, 3> view_vector;

	float frametime = 0;
};