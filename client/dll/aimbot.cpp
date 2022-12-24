#include "aimbot.h"

#include "valve-sdk/engine/studio.h"

#include "globals.h"

const size_t NO_TARGET = 0;

// function converts input vector to view angles
// origin - Quake 1 source code:
// https://github.com/id-Software/Quake/blob/bf4ac424ce754894ac8f1dae6a3981954bc9852d/QW/server/pr_cmds.c#L365
//
// yet it has been slightly modified by me because of 
// minor difference	of view angles representation between CS 1.6 and CSN:S
void VectorAngles(const float* forward, float* angles) {
	float tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0) {
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else {
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);

		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI);

		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;

	while (angles[0] < -89) { angles[0] += 180; angles[1] += 180; }
	while (angles[0] > 89) { angles[0] -= 180; angles[1] += 180; }
	while (angles[1] < 0) { angles[1] += 360; }
	while (angles[1] > 360) { angles[1] -= 360; }
}

// function computes angle between two vectors
float AngleBetvenVectors(Vector a, Vector b) {
	float sc = a.x * b.x + a.y * b.y + a.z * b.z;
	return acos(sc / (a.Length() * b.Length())) * (180.0 / M_PI);
}

size_t Target = NO_TARGET;
Vector HeadBone = { 0, 0, 0 };

void UpdateAimBone(cl_entity_s* ent) {
	// compute head bone position only for aimbot target 
	if (ent->index != Target)
		return;

	model_t * pModel = g::Studio.SetupPlayerModel(ent->index);
	if (!pModel)
		return;

	studiohdr_t * pStudioHeader = (studiohdr_t*)g::Studio.Mod_Extradata(pModel);
	typedef float TransformMatrix[MAXSTUDIOBONES][3][4];
	TransformMatrix * pbonetransform = (TransformMatrix*)g::Studio.StudioGetBoneTransform();
	mstudiobone_t * pbones = (mstudiobone_t*)((byte*)pStudioHeader + pStudioHeader->boneindex);

	// there are 11 bones that may be indexes of head bone in CSN:S
	// more information here: https://www.unknowncheats.me/forum/2995316-post1.html
	static const int head_bones[11] = { 6, 7, 8, 9, 13, 15, 16, 21, 52, 54, 57 };
	for (int i : head_bones) {
		// copy bonename to another char array,
		// then convert its case to lowercase,
		// and finally check if there are 'head' in bonename
		char bonename[255];
		strcpy_s(bonename, pbones[i].name);
		_strlwr_s(bonename);

		if (strstr(bonename, "head")) {
			HeadBone = { (*pbonetransform)[i][0][3], (*pbonetransform)[i][1][3], (*pbonetransform)[i][2][3] };
			break;
		}
	}

	// some magic operations about found head bone.
	// don't think about it too much
	Vector angles = { 0.f, ent->angles[1], 0.f }, vu;
	g::Engine.AngleVectors(angles, 0, 0, vu);
	HeadBone = HeadBone + vu;
	HeadBone.z -= 2.75f;
	HeadBone.z -= abs(g::pClientState->punchangle.y) * 5.f;

	// apply prediction to head bone
	if (g::Settings.AimBot.Prediction) {
		Vector velocity = ent->curstate.origin - ent->prevstate.origin;
		float prediction = static_cast<float>(static_cast<float>(g::Settings.AimBot.PredictionFac) / 100.f);

		HeadBone = HeadBone + (velocity * sqrt(g::Players[Target].get_frametime()) * prediction);
	}
}

// totally optimized aimbot written from scratch
void Aimbot() {
	static float best_fov;

	if (!g::Settings.AimBot.Enabled || !g::Local.is_alive())
		return;

	if (GetAsyncKeyState(g::Settings.AimBot.Key) && g::InFocus) {
		// update aimbot target only if it's not set
		if (Target == NO_TARGET) {
			// store id's of three nearest to FOV players
			std::array<size_t, 3> nearest_to_fov = { NO_TARGET, NO_TARGET, NO_TARGET };

			// iterate through players list
			for (Player& player : g::Players) {
				if ((player.get_team() != g::Local.get_team() || g::Settings.AimBot.DM) && player.is_alive()) {
					float cur_fov = 360.f;

					// get minimum player FOV
					for (int step = -100;
						step <= 100;
						step += 25) {
						float tmp = AngleBetvenVectors(g::Local.get_view_vector(ViewVector::FORWARD), player.get_origin(step) - g::Local.get_eye());

						if (tmp < cur_fov)
							cur_fov = tmp;
					}

					// if current FOV less than already existent,
					// update 'nearest to fov'
					if (cur_fov < best_fov && cur_fov > 0.001f) {
						best_fov = cur_fov;

						nearest_to_fov[2] = nearest_to_fov[1];
						nearest_to_fov[1] = nearest_to_fov[0];
						nearest_to_fov[0] = player.get_id();
					}
				}
			}

			// compute player with minimum distance to local player
			// from among the nearest to FOV players
			float best_dist = MAX_POSSIBLE_DISTANCE;
			for (size_t i : nearest_to_fov) {
				if (i == NO_TARGET)
					break;

				if (g::Players[i].get_distance() < best_dist) {
					best_dist = g::Players[i].get_distance();
					Target = i;
				}
			}
		}

		// if there is no players satisfying conditions, just leave the function
		if (Target == NO_TARGET)
			return;

		// otherwise set aim target, and get its head position.
		// it's important moment that head posisition is computed
		// when and only when aimbot target is specified,
		// but it doesn't work in every other time

		// prevent some issues
		const size_t ALLOWED_DIFFERENCE = 50;
		if (HeadBone.Length() == 0.f || HeadBone.Distance(g::Players[Target].get_origin()) > ALLOWED_DIFFERENCE)
			return;

		// calculate vector to aim, and convert it into view angles
		float va[3];
		VectorAngles(HeadBone - g::Local.get_eye(), va);

		// set view angles
		g::Engine.SetViewAngles(va);
	}

	// if current target is dead, or aimbot key isn't pressed
	// reset variables to default values
	if (!GetAsyncKeyState(g::Settings.AimBot.Key) || (Target != NO_TARGET && !g::Players[Target].is_alive())) {
		Target = NO_TARGET;
		best_fov = g::Settings.AimBot.FOV;
	}
}