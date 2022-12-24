#if !defined( PARTICLEDEFH )
#define PARTICLEDEFH
#ifdef _WIN32
#pragma once
#endif

typedef enum {
	pt_static, 
	pt_grav,
	pt_slowgrav,
	pt_fire,
	pt_explode,
	pt_explode2,
	pt_blob,
	pt_blob2,
	pt_vox_slowgrav,
	pt_vox_grav,
	pt_clientcustom   // Must have callback function specified
} ptype_t;

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
typedef struct particle_s
{
// driver-usable fields
	vec3_t		org;
	short		color;
	short		packedColor;
// drivers never touch the following fields
	struct particle_s	*next;
	vec3_t		vel;
	float		ramp;
	float		die;
	ptype_t		type;
	void		(*deathfunc)( struct particle_s *particle );

	// for pt_clientcusttom, we'll call this function each frame
	void		(*callback)( struct particle_s *particle, float frametime );
	
	// For deathfunc, etc.
	unsigned char context;
} particle_t;

#endif
