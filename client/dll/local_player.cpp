#include "local_player.h"

#include "globals.h"

void LocalPlayer::update() {
	auto alive_check = [](cl_entity_s * ent) -> bool {
		const int OBS_NONE = 0;
		if (ent == NULL || !ent->player || ent->curstate.effects & EF_NODRAW ||
			ent->curstate.iuser1 != OBS_NONE || ent->curstate.mins.Length() == NULL ||
			ent->curstate.maxs.Length() == NULL)
			return false;

		return true;
	};

	cl_entity_s* ent = g::Engine.GetLocalPlayer();

	this->id = ent->index;
	this->eye = g::pPlayerMove->origin + g::pPlayerMove->view_ofs;
	this->alive = alive_check(ent);

	float va[3], vf[3], vr[3], vu[3];
	g::pEngine->GetViewAngles(va);
	g::Engine.AngleVectors(va, vf, vr, vu);
	this->view_vector = { vf, vr, vu };

	this->frametime = ent->curstate.animtime - ent->prevstate.animtime;
}

void LocalPlayer::set_speed(double speed) {
	static double LastSpeed = 1;
	if (speed != LastSpeed) {
		*reinterpret_cast<double*>(g::Speed) = (speed * 1000);
		LastSpeed = speed;
	}
}