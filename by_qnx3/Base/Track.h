/**
 * @file	Track.h
 * @brief
 * @details
 * @author
 * @date
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define	TL_ALL			0
#define	TL_CLASS		1
#define	TL_MAIL			2
#define	TL_ERROR		3
#define	TL_WARNING		4
#define	TL_TRACE		5
#define	TL_INFO			6

#include <sys/time.h>

#define	TRACK(fmt, ...)\
	do {\
		TrackA(fmt, ##__VA_ARGS__);\
	} while (0)

void	TrackA(const char* fmt, ...);

#ifdef __cplusplus
}
#endif
