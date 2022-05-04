/**
 * @file	PiRefer.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "../Common/Refer.h"

#define	PTQ_STR				"/ptq"			// pi & tdc connect queue
#define	PTPACK_STR			"/ptpk"
#define PINGRESULT_FILE		"/tmp/pingres.txt"

//typedef struct _tagPIMEASURINGITEMAS {
//	MSRTIME	ping;
//} PIMSRITEM;
//
typedef struct _tagPIMEASURINGITEMAS {
	MSRTIMEDW	ping;
} PIMSRITEM;

typedef struct _tagSERVREFER {
	WORD	wCurID;
	QWORD	qwShape;
} SERVREF;

typedef struct _tagPTBAIL {
	WORD		wPiState;
	PIMSRITEM	mi;
	SERVREF		sr;
} PTBAIL;

typedef struct _tagPTPACK {
	PTBAIL		bail;
	_ENV		env;
} PTPACK, *PPTPACK;

enum enPISTATE {
	PISTATE_PINGING = 0,
	PISTATE_NOTHINGMORE
};
