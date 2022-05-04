/*
 * Printz.c
 *
 *  Created on: 2010. 09. XX
 *      Author: Che
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <time.h>

#include "Printz.h"

/*#ifdef	_CTCX
#include "../Tcx/CTcx.h"
extern CTcx tcx;
#endif
*/
void printt()
{
	time_t cur = time(NULL);
	struct tm* lc = localtime(&cur);
	printf("%02d:%02d:%02d", lc->tm_hour, lc->tm_min, lc->tm_sec);
}

void mprint(const char* fmt, ...)
{
	char buf[1024];

	va_list alist;
	va_start(alist, fmt);
	sprintf(buf, fmt, alist);
	printf(buf);
/*#ifdef	_CTCX
	tcx.OsMsg(buf);
#endif*/
	va_end(alist);
}

void printz(const char* fmt, ...)
{
	va_list alist;
	va_start(alist, fmt);
	printf(fmt, alist);
	va_end(alist);
}
