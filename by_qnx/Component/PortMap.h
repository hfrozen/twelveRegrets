/*
 * PortMap.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef PORTMAP_H_
#define PORTMAP_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <Define.h>
#include <pthread.h>
#include <mqueue.h>
#include <Mfb.h>

#define	MFB_MSGARCH		MFB_MsgBuffObj

pthread_t		g_hThread;
mqd_t			g_mqBell;
MFB_MSGARCH		g_bell;
SHORT			g_map;	// = 0;

typedef void (*MAPFUNC)(BYTE*, WORD, BYTE, PVOID);

typedef struct _tagVECTORS {
	MAPFUNC	pExec;
	PVOID	pBody;
} VECTORS;
#define	SIZE_VECTORS	10
VECTORS	g_vectors[SIZE_VECTORS];

void	InitVectors();
int		Register(BYTE ch, MAPFUNC pExec, PVOID pBody);
int		Erasure(BYTE ch);
void*	MsgHandler();

#ifdef	__cplusplus
}
#endif

#endif /* PORTMAP_H_ */
