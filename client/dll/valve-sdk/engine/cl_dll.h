typedef unsigned char byte;
typedef unsigned short word;
typedef float vec_t;
typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

#include "util_vector.h"
#define EXPORT	_declspec( dllexport )

#include "../engine/cdll_int.h"
#include "cdll_dll.h"
