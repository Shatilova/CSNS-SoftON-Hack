#if !defined( STUDIO_EVENTH )
#define STUDIO_EVENTH
#ifdef _WIN32
#pragma once
#endif

typedef struct mstudioevent_s
{
	int 				frame;
	int					event;
	int					type;
	char				options[64];
} mstudioevent_t;

#endif // STUDIO_EVENTH
