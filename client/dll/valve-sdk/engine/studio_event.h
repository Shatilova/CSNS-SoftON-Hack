#if !defined( STUDIO_EVENTH )
#define STUDIO_EVENTH
#pragma once

typedef struct mstudioevent_s
{
	int 				frame;
	int					event;
	int					type;
	char				options[64];
} mstudioevent_t;

#endif // STUDIO_EVENTH