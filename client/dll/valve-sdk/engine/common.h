#pragma once

#include "custom.h"

typedef unsigned __int64 uint64;
typedef unsigned __int16 uint16;
typedef unsigned long long uint64_t;
typedef FILE* FileHandle_t;

typedef struct sizebuf_s
{
	const char* buffername;
	uint16 flags;
	byte* data;
	int maxsize;
	int cursize;
} sizebuf_t;

typedef struct downloadtime_s
{
	qboolean bUsed;
	float fTime;
	int nBytesRemaining;
} downloadtime_t;

typedef struct incomingtransfer_s
{
	qboolean doneregistering;
	int percent;
	qboolean downloadrequested;
	downloadtime_t rgStats[8];
	int nCurStat;
	int nTotalSize;
	int nTotalToTransfer;
	int nRemainingToTransfer;
	float fLastStatusUpdate;
	qboolean custom;
} incomingtransfer_t;