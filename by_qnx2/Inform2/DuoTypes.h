/**
 * @file	DuoTypes.h
 * @brief	2호선 프로그램에서 사용되는 기본 단위들을 나열한다.
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */

#include "../../../Common/LineTypes.h"

#pragma once

typedef struct _tagINCERT {
	_OCTET	oct;
	WORD	wCertify;
} INCERT;

typedef struct _tagOUTCERT {
	_DUET	duet;
	WORD	wCertify;
} OUTCERT;

#pragma pack(push, 1)

typedef struct _tagDTSTIME {
	WORD	year;		// 0-1
	BYTE	mon;		// 2
	BYTE	day;		// 3
	BYTE	date;		// 4
	BYTE	hour;		// 5
	BYTE	min;		// 6
	BYTE	sec;		// 7
	WORD	msec;		// 8-9
} DTSTIME, *PDTSTIME;

typedef struct _tagDEVTIME {
	BYTE	year;		// 0, 0~99
	BYTE	mon;		// 1, 1~12
	BYTE	day;		// 2, 1~31
	BYTE	hour;		// 3, 0~23
	BYTE	min;		// 4, 0~59
	BYTE	sec;		// 5, 0~59
} DEVTIME, *PDEVTIME;

typedef union _tagDBFTIME {
	struct {
		DWORD	year	: 6;	// 0~63 -> 2010~2073
		DWORD	mon		: 4;
		DWORD	day		: 5;
		DWORD	hour	: 5;
		DWORD	min		: 6;
		DWORD	sec		: 6;
	} t;						// 2/4
	DWORD dw;
} DBFTIME, *PDBFTIME;

#pragma pack(pop)

#define	SIZE_TRACETEXT		100
// in siv, v3f trace...
// wIndex	: sda에 나타난 트레이스 길이에 따른 순서 (0~트레이스 길이)
// wStripID	: 한 분량의 트레이스 데이터를 100바이트(1회 전송 분)로 나눈 블럭들의 순서
// 한 분량의 트레이스 데이터
// SIV...
// 샘플링 데이터 (8 바이트) * 샘플링 횟수 (201 회) = 1608 바이트, 1608 바이트 + 헤드 (8 바이트) = 1616 바이트
// wPageID	: 0 ~ 16 (100 바이트 * 17회)
// 171114
// 샘플링 데이터 (40 바이트) * 샘플링 횟수 (201 회) = 8040 바이트, 8040 바이트 + 헤드 (8 바이트) = 8048 바이트
// wPageID	: 0 ~ 80 (100 바이트 * 81회)
#define	SIVTRACEPAGE_MAX	81
#define	SIVTRACEREAL_SIZE	8048
// 180626
// 샘플링 데이터 (50 바이트) * 샘플링 횟수 (201 회) = 10050 바이트, 10050 바이트 + 헤드 (8 바이트) = 10058 바이트
// wPageID	: 0 ~ 100 (100 바이트 * 101회)
//#define	SIVTRACEPAGEEX_MAX	101
//#define	SIVTRACEREALEX_SIZE	10058
#define	SIVTRACEPAGEEX_MAX	101
#define	SIVTRACEREALEX_SIZE	10058
// V3F...
// 샘플링 데이터 (50 바이트) * 샘플링 횟수 (700 회) = 35000 바이트, 35000 바이트 + 헤드 (8 바이트) = 35008 바이트
// wPageID : 0 ~ 350 (100바이트 * 351회)
// 170817
// 샘플링 데이터 (40 바이트) * 샘플링 횟수 (201 회) = 8040 바이트, 8040 바이트 + 헤드 (8 바이트) = 8048 바이트
// wPageID : 0 ~ 80 (100바이트 * 81회)
// 171115
// 샘플링 데이터 (40 바이트) * 샘플링 횟수 (201 회) = 8040 바이트, 8040 바이트 + 헤드 (8 바이트) = 8048 바이트
// wPageID : 0 ~ 80 (100바이트 * 17회)
#define	V3FTRACEPAGE_MAX	81
#define	V3FTRACEREAL_SIZE	8048
// ECU...
// 샘플링 데이터 (30 바이트) * 샘플링 횟수 (101 회) = 3030 바이트
// wPageID : 0 ~ 30 (100바이트 * 31회)
#define	ECUTRACEPAGE_MAX	31
#define	ECUTRACEREAL_SIZE	3030

#define	LIMIT_TRACE				16

typedef union _tagECUTRACETEXTS {
	BYTE	c1[LIMIT_TRACE * ECUTRACEPAGE_MAX * SIZE_TRACETEXT];
	BYTE	c2[LIMIT_TRACE][ECUTRACEPAGE_MAX * SIZE_TRACETEXT];
	BYTE	c3[LIMIT_TRACE][ECUTRACEPAGE_MAX][SIZE_TRACETEXT];
} ECUTRACETEXTS;

enum enARRAGEECUTRACESTEP {			// ECU 트레이스 데이터를 모으는 순서
	AETSTEP_EMPTY,
	AETSTEP_WAITARRANGE,
	AETSTEP_WAITLOAD,
	AETSTEP_END
};

enum enSERVICEDEVICECTRLCODEBIN {	// 서비스 장치 제어권 지정 코드
	SVCCODEB_HEAD = 0,
	SVCCODEB_TAIL,
	SVCCODEB_NEUTRAL,
	SVCCODEB_XCHANGE,
};

enum enSERVICEDEVICECODEHEX {
	SVCCODEH_HEAD = 0x31,
	SVCCODEH_TAIL,		// 0x32
	SVCCODEH_NEUTRAL,	// 0x33
	SVCCODEH_XCHANGE,	// 0x34
};

#if	!defined(_WIN32)
#define	SIZE_GENSBUF	1024

#include <mqueue.h>
typedef mqd_t			QHND;

#include <pthread.h>
typedef pthread_t		TRHND;

#include <time.h>
typedef timer_t			TMHND;

//typedef pthread_mutex_t	MUTEX;
//#define	InitMutex(x)	pthread_mutex_init(&x, NULL)
//#define	LockMutex(x)	pthread_mutex_lock(&x)
//#define	UnlockMutex(x)	pthread_mutex_unlock(&x)

#define	INVALID_HANDLE		-1
#endif

#define	CHECK_SIZE(limit, obj)\
	enum { SIZE_##obj = sizeof(obj) };\
	struct __tag##obj {\
		char	ch##obj[limit - SIZE_##obj];\
	};
