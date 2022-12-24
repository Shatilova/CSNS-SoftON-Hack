#include "kreedz.h"

#include "globals.h"

enum class ButtonState {
	NOT_PRESSED = 4,
	PRESSED = 5,
};
void DeactivateKey(kbutton_s& button) {
	button.state = static_cast<int>(ButtonState::NOT_PRESSED);
}
void ActivateKey(kbutton_s& button) {
	DeactivateKey(button);
	button.state = static_cast<int>(ButtonState::PRESSED);;
}

void GroundStrafe() {
	if (g::Settings.Kreedz.GSType != CFG_GSTypes::NONE) {
		static bool in_gs = false;
		if (GetAsyncKeyState(g::Settings.Kreedz.GSKey) && g::InFocus) {
			// compute some stuff needed only for GroundStrafe inside 'GroundStrafe' function
			pmtrace_t* trace = g::Engine.PM_TraceLine(g::pPlayerMove->origin, Vector(g::pPlayerMove->origin.x, g::pPlayerMove->origin.y, -MAX_POSSIBLE_DISTANCE), 1, (g::pPlayerMove->flags & FL_DUCKING) ? 1 : 0, -1);

			float height = abs(trace->endpos.z - g::pPlayerMove->origin.z);
			float groundAngle = acos(trace->plane.normal[2]) / M_PI * 180;
			float xy_speed = sqrt(POW(g::pPlayerMove->velocity[0]) + POW(g::pPlayerMove->velocity[1]));

			static int gs_state = 0;
			in_gs = true;

			// there are 4 similar types of GroundStrafe.
			// some of them made no sense to implement,
			// because they are noticeably slower than other.
			// approximate speed measurements:
			// GS       - 700 u/s
			// Jump GS  - 750 u/s
			// SGS      - 600 u/s
			// Jump SGS - 650 u/s

			if (g::Settings.Kreedz.GSType == CFG_GSTypes::SGS || g::Settings.Kreedz.GSType == CFG_GSTypes::JUMPSGS) {
				if (g::pPlayerMove->flFallVelocity > 0 && groundAngle < 5.0f && ((g::pPlayerMove->flags & FL_ONGROUND) || height < 0.00001f)) {
					g::Local.set_speed(0.000001);

					if (g::pPlayerMove->flFallVelocity >= 140 && height <= 30)
						ActivateKey(g::pButtonsBits->duck);
				}
			}

			if (gs_state == 0 && (g::pPlayerMove->flags & FL_ONGROUND)) {
				if (groundAngle < 5.0f && ((g::pPlayerMove->flags & FL_ONGROUND) || height < 0.00001f))
					g::Local.set_speed(0.000001);
				ActivateKey(g::pButtonsBits->duck);

				gs_state = 1;
			}
			else if (gs_state == 1) {
				if (groundAngle < 5.0f && ((g::pPlayerMove->flags & FL_ONGROUND) || height < 0.00001f))
					g::Local.set_speed(0.00001f);

				DeactivateKey(g::pButtonsBits->duck);

				if ((g::Settings.Kreedz.GSType == CFG_GSTypes::JUMPGS || g::Settings.Kreedz.GSType == CFG_GSTypes::JUMPSGS) && ((g::pPlayerMove->flags & FL_DUCKING) ? 1 : 0) == 0)
					ActivateKey(g::pButtonsBits->jump);

				gs_state = 0;
			}
		}
		// this piece of code is needed to prevent
		// 'sticking' of duck/jump when gs is stopped
		else if (in_gs) {
			DeactivateKey(g::pButtonsBits->duck);
			DeactivateKey(g::pButtonsBits->jump);

			in_gs = false;
		}
	}
}

void Bhop() {
	if (g::Settings.Kreedz.Bhop) {
		if (GetAsyncKeyState(g::Settings.Kreedz.BhopKey) && g::InFocus) {
			if (g::pPlayerMove->flags & FL_ONGROUND || g::pPlayerMove->waterlevel >= 2 || g::pPlayerMove->movetype == 5)
				ActivateKey(g::pButtonsBits->jump);
			else
				DeactivateKey(g::pButtonsBits->jump);
		}
	}
}
