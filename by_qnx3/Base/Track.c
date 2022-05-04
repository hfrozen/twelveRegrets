/**
 * @file	Track.c
 * @brief
 * @details
 * @author
 * @date
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdbool.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "Track.h"

static bool g_bInit = false;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
//#if	defined(_TRACK_MAIN_)
static char g_cPrevFile[256] = { 0 };
//#endif
static char g_cCurFile[256] = { 0 };
static char	g_cBuf[1024] = { 0 };

void TrackA(const char* fmt, ...)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	sprintf(g_cBuf, "%02d:%02d:%02d:%03d-", pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tv.tv_usec / 1000);
	int year = (pTm->tm_year + 1900) % 100;
	int mon = pTm->tm_mon + 1;
	int day = pTm->tm_mday;
	sprintf(g_cCurFile, "/h/sd/log/l%02d%02d%02d", year, mon, day);

	va_list vl;
	va_start(vl, fmt);
	vsprintf(&g_cBuf[13], fmt, vl);
	va_end(vl);
	printf(g_cBuf);

	if (!g_bInit) {
		pthread_mutex_init(&g_mutex, NULL);
		g_bInit = true;
	}
	pthread_mutex_lock(&g_mutex);
	FILE* fp = fopen(g_cCurFile, "a");
	if (fp != NULL) {
		fwrite(g_cBuf, 1, strlen(g_cBuf), fp);
		fclose(fp);
	}

//#if	defined(_TRACK_MAIN_)
	if (strcmp(g_cPrevFile, g_cCurFile)) {
		if (mon > 1)	-- mon;
		else {
			mon = 12;
			if (year > 0)	-- year;
		}
		unsigned long tar = (unsigned long)year * 10000 + (unsigned long)mon * 100 + (unsigned long)day;

		char buf[256];
		DIR* pDir = opendir("/h/sd/log");
		if (pDir != NULL) {
			struct dirent* pEntry;
			while ((pEntry = readdir(pDir)) != NULL) {
				if (pEntry->d_name[0] == 'l') {
					unsigned long cur = (unsigned long)atol(&pEntry->d_name[1]);
					if (cur <= tar) {
						sprintf(buf, "/h/sd/log/%s", pEntry->d_name);
						remove(buf);
						printf("delete %s\n", buf);
					}
				}
			}
			closedir(pDir);
		}
		strncpy(g_cPrevFile, g_cCurFile, 256);
	}
//#endif
	pthread_mutex_unlock(&g_mutex);
}
