#pragma once

/*
 * platform.h
 *  Hides the differences between POSIX (Linux, macOS) and Windows (MSVC).
 *  Include this before any file system call.
 */

#include <stdint.h>

#ifdef _WIN32

#include <malloc.h>
#include <time.h>

#ifndef alloca
#define alloca _alloca
#endif

/* the current time in microseconds: for the collector's timing and the timestamp primitive 161 */
inline int64_t lstTimeUsec()
{
	timespec now;

	timespec_get( &now, TIME_UTC );

	return ( (int64_t) 1000000 * now.tv_sec ) + ( now.tv_nsec / 1000 );
}

#else

#include <alloca.h>
#include <sys/time.h>

/* the current time in microseconds: for the collector's timing and the timestamp primitive 161 */
inline int64_t lstTimeUsec()
{
	timeval now;

	gettimeofday( &now, nullptr );

	return ( (int64_t) 1000000 * now.tv_sec ) + now.tv_usec;
}

#endif
