/**
 * @file	Mc.h
 * @brief	����׿� �޽��� ť�̸��� ũ�� ���� ���Ѵ�.
 * @details	CWicket(ts)�� CMoc(mc)������ �޽����� �ְ� �޴� ����� �����Ѵ�.
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "DuoTypes.h"

#define	MCQ_STR				"/mcq"		// mc pump queue
#define	ENTQ_STR			"/ctq"		// mc & ts connect queue
#define	SIZE_MCMSG			8192
#define	SIZE_MCRTIME		28		// "\tVARS:MSG:00-00-00 00:00:00"
#define	SIZE_GENERALBUF		SIZE_MCMSG - SIZE_MCRTIME
#define	MAX_MCMSG			16

typedef struct _tagMCMSG {
	WORD	wID;
	DWORD	dwLength;
	BYTE	cBuf[SIZE_MCMSG];
} MCMSG, *PMCMSG;

#define	MCMSG_CONTS			2
//#define	MCMSG_CONCU		3
#define	MCMSG_ACCEPT		4
#define	MCMSG_SEND			6
#define	MCMSG_RECEIVE		8
#define	MCMSG_CLOSE			10
