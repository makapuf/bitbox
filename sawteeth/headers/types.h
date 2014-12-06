/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#ifndef TYPES__JNG
#define TYPES__JNG

#include <stdint.h>

#ifdef __HAIKU__
#include <SupportDefs.h>
#else
typedef	int8_t		int8;
typedef uint8_t		uint8;

typedef	int16_t		int16;
typedef uint16_t	uint16;

typedef	int32_t		int32;
typedef uint32_t	uint32;

typedef int32_t		status_t;
#endif

const	int JNG_OK = 0;
const	int JNG_ERROR = -1;
const	int JNG_ERROR_VERSION = -2;

#ifndef PRIu32
#define PRIu32 "lu"
#endif

#endif
