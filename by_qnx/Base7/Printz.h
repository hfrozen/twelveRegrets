/*
 * Printz.h
 *
 *  Created on: 2010. 09. XX
 *      Author: Che
 */

#ifndef PRINTZ_H_
#define PRINTZ_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "Define.h"

#define	MSGLOG(fmt, ...) \
do {\
	printt();\
	printf(fmt, ##__VA_ARGS__);\
} while(0)

void	printt();
void	mprint(const char* fmt, ...);
void	printz(const char* fmt, ...);

#ifdef	__cplusplus
}
#endif

#endif /* PRINTZ_H_ */
