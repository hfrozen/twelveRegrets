/**
 * @file	Track.c
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "Mm.h"
#include "Track.h"

static bool g_bInit = false;
static pthread_mutex_t	g_mtx = PTHREAD_MUTEX_INITIALIZER;
#if	0
static bool g_bChkDate = true;
static char g_cPrevFile[256] = { 0 };
static char g_cCurFile[256] = { 0 };
static char g_cHead[256];
#endif
static char	g_cBuf[1024];

void TrackA(const char* fmt, ...)
{
#if	0
	bool bHead = false;
#endif
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	sprintf(g_cBuf, "%02d:%02d:%02d:%03d-", pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tv.tv_usec / 1000);

	va_list vl;
	va_start(vl, fmt);
	vsprintf(&g_cBuf[13], fmt, vl);
	va_end(vl);
	printf(g_cBuf);

	if (!g_bInit) {
		pthread_mutex_init(&g_mtx, NULL);
		g_bInit = true;
	}
	pthread_mutex_lock(&g_mtx);

#if	0
	char day;
	if (pTm->tm_mday < 31)	day = (char)((pTm->tm_mday % 10) + '0');
	else	day = 'a';
	sprintf(g_cCurFile, "%s%c", LOG_NAME, day);

	if (g_bChkDate || (g_cPrevFile[0] != 0 && strcmp(g_cPrevFile, g_cCurFile))) {
		struct stat buf;
		if (!stat(g_cCurFile, &buf)) {
			struct tm* pFileTm = localtime(&buf.st_mtime);
			if (pFileTm->tm_mon != pTm->tm_mon || pFileTm->tm_mday != pTm->tm_mday) {
				remove(g_cCurFile);
				bHead = true;	// remove, append head
			}
			//else {			// same date file
			//}
		}
		else	bHead = true;	// not exist, append head
	}
	//else {					// same date file
	//}
	g_bChkDate = false;
	if (strcmp(g_cPrevFile, g_cCurFile))	strncpy(g_cPrevFile, g_cCurFile, 256);

	FILE* fp = fopen(g_cCurFile, "a");
	if (fp != NULL) {
		if (bHead) {
			sprintf(g_cHead, "%04d/%02d/%02d==========\n", pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday);
			fwrite(g_cHead, 1, strlen(g_cHead), fp);
			bHead = false;
		}
		fwrite(g_cBuf, 1, strlen(g_cBuf), fp);
		fclose(fp);
	}
	else	printf("log open fail!\n");
#else
	FILE* fp = fopen(LOG_PATHTMP, "a");
	if (fp != NULL) {
		fwrite(g_cBuf, 1, strlen(g_cBuf), fp);
		fclose(fp);
	}
	else	printf("log open fail!\n");
#endif

	pthread_mutex_unlock(&g_mtx);
}

int CmpDouble(const void* a, const void* b)
{
	if (*(double*)a < *(double*)b)	return -1;
	if (*(double*)a == *(double*)b)	return 0;
	return 1;
}
