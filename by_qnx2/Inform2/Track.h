/**
 * @file	Track.h
 * @brief	디버그를 위한 printf
 * @details	날짜와 시간을 포함한 문장이 콘솔로 표현되고 같은 문장이 파일로도 저장된다.
 *			사용이 편리하도록 클래스로 하지않고 글로벌로 구성한다.
 * @author	Che-follo
 * @date	2015-11-XX
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
int		CmpDouble(const void* a, const void* b);

#ifdef __cplusplus
}
#endif
