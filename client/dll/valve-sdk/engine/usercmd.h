#ifndef USERCMD_H
#define USERCMD_H
#ifdef _WIN32
#pragma once
#endif

typedef struct usercmd_s
{
	short	lerp_msec;      // Interpolation time on client
	byte	msec;           // Duration in ms of command
		
	vec3_t	viewangles;     // Command view angles.

// intended velocities
	float	forwardmove;    // Forward velocity.
	float	sidemove;       // Sideways velocity.
	float	upmove;         // Upward velocity.
	byte	lightlevel;     // Light level at spot where we are standing.
	unsigned short  buttons;  // Attack buttons
	byte    impulse;          // Impulse command issued.
	byte	weaponselect;	// Current weapon id, NULL IN NZ

// Experimental player impact stuff.
	int		impact_index;
	///int unk1;
	vec3_t	impact_position; //LITERALLY VIEWANG
} usercmd_t;

#endif // USERCMD_H