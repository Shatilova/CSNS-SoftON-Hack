#include "player.h"

#include "globals.h"
#include "hack_detector.h"

bool PathFree(float* pflFrom, float* pflTo) {
	pmtrace_t pTrace;

	g::Engine.pEventAPI->EV_SetTraceHull(2);
	g::Engine.pEventAPI->EV_PlayerTrace(pflFrom, pflTo, PM_GLASS_IGNORE | PM_STUDIO_BOX, g::Local.get_id(), &pTrace);

	return (bool)(pTrace.fraction == 1.0f);
}

void Player::update() {
	// old-style alive-check 
	auto alive_check = [](cl_entity_s * ent) -> bool {
		if (ent->index != g::Local.get_id() &&
			!(ent->curstate.effects & EF_NODRAW) &&
			ent->player && (ent->model->name != 0 || ent->model->name != "") &&
			!(ent->curstate.messagenum < g::Engine.GetLocalPlayer()->curstate.messagenum) &&
			ent->curstate.movetype != 6 && ent->curstate.movetype != 0)
			return true;

		return false;
	};

	if (this->id == g::Local.get_id())
		return;

	cl_entity_s * ent = g::Engine.GetEntityByIndex(this->id);

	this->hack_used = std::nullopt;

	this->alive = alive_check(ent);
	if (!this->alive)
		return;

	this->top_z_offset = ent->curstate.mins[2];
	this->bottom_z_offset = ent->curstate.maxs[2];
	this->origin = ent->origin;
	this->distance = this->origin.Distance(g::Local.get_eye());
	
	// check visibleness not with the only origin,
	// but with the origin with different Z-axis.
	// it makes visible check more accurate
	for (int step = -100;
		step <= 100;
		step += 25) {
		this->visible = PathFree(g::Local.get_eye(), this->get_origin(step));
		if (this->visible)
			break;
	}

	this->frametime = ent->curstate.animtime - ent->prevstate.animtime;
	if (this->frametime == 0.f)
		this->frametime = g::Local.get_frametime();

	hud_player_info_t info;
	g::Engine.GetPlayerInfo(this->id, &info);
	this->nickname = info.name != 0 ? info.name : 0;
	this->hack_used = hack_detector::ReceiveHackInfo(this->id);
}

// function returns player's origin with some Z-axis offset
Vector Player::get_origin(int z_offset) {
	// clamping to [-100:100]
	z_offset = (z_offset < -100) ? -100 : (z_offset > 100) ? 100 : z_offset;

	if (z_offset == 0)
		return this->origin;
	else if (z_offset < 0)
		return { this->origin.x, this->origin.y, this->origin.z + this->top_z_offset * (z_offset / 100.f) };
	else if (z_offset > 0)
		return { this->origin.x, this->origin.y, this->origin.z + this->bottom_z_offset * (z_offset / 100.f) };
}