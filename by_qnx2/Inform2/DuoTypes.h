/**
 * @file	DuoTypes.h
 * @brief	2ȣ�� ���α׷����� ���Ǵ� �⺻ �������� �����Ѵ�.
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
// wIndex	: sda�� ��Ÿ�� Ʈ���̽� ���̿� ���� ���� (0~Ʈ���̽� ����)
// wStripID	: �� �з��� Ʈ���̽� �����͸� 100����Ʈ(1ȸ ���� ��)�� ���� ������ ����
// �� �з��� Ʈ���̽� ������
// SIV...
// ���ø� ������ (8 ����Ʈ) * ���ø� Ƚ�� (201 ȸ) = 1608 ����Ʈ, 1608 ����Ʈ + ��� (8 ����Ʈ) = 1616 ����Ʈ
// wPageID	: 0 ~ 16 (100 ����Ʈ * 17ȸ)
// 171114
// ���ø� ������ (40 ����Ʈ) * ���ø� Ƚ�� (201 ȸ) = 8040 ����Ʈ, 8040 ����Ʈ + ��� (8 ����Ʈ) = 8048 ����Ʈ
// wPageID	: 0 ~ 80 (100 ����Ʈ * 81ȸ)
#define	SIVTRACEPAGE_MAX	81
#define	SIVTRACEREAL_SIZE	8048
// 180626
// ���ø� ������ (50 ����Ʈ) * ���ø� Ƚ�� (201 ȸ) = 10050 ����Ʈ, 10050 ����Ʈ + ��� (8 ����Ʈ) = 10058 ����Ʈ
// wPageID	: 0 ~ 100 (100 ����Ʈ * 101ȸ)
//#define	SIVTRACEPAGEEX_MAX	101
//#define	SIVTRACEREALEX_SIZE	10058
#define	SIVTRACEPAGEEX_MAX	101
#define	SIVTRACEREALEX_SIZE	10058
// V3F...
// ���ø� ������ (50 ����Ʈ) * ���ø� Ƚ�� (700 ȸ) = 35000 ����Ʈ, 35000 ����Ʈ + ��� (8 ����Ʈ) = 35008 ����Ʈ
// wPageID : 0 ~ 350 (100����Ʈ * 351ȸ)
// 170817
// ���ø� ������ (40 ����Ʈ) * ���ø� Ƚ�� (201 ȸ) = 8040 ����Ʈ, 8040 ����Ʈ + ��� (8 ����Ʈ) = 8048 ����Ʈ
// wPageID : 0 ~ 80 (100����Ʈ * 81ȸ)
// 171115
// ���ø� ������ (40 ����Ʈ) * ���ø� Ƚ�� (201 ȸ) = 8040 ����Ʈ, 8040 ����Ʈ + ��� (8 ����Ʈ) = 8048 ����Ʈ
// wPageID : 0 ~ 80 (100����Ʈ * 17ȸ)
#define	V3FTRACEPAGE_MAX	81
#define	V3FTRACEREAL_SIZE	8048
// ECU...
// ���ø� ������ (30 ����Ʈ) * ���ø� Ƚ�� (101 ȸ) = 3030 ����Ʈ
// wPageID : 0 ~ 30 (100����Ʈ * 31ȸ)
#define	ECUTRACEPAGE_MAX	31
#define	ECUTRACEREAL_SIZE	3030

#define	LIMIT_TRACE				16

typedef union _tagECUTRACETEXTS {
	BYTE	c1[LIMIT_TRACE * ECUTRACEPAGE_MAX * SIZE_TRACETEXT];
	BYTE	c2[LIMIT_TRACE][ECUTRACEPAGE_MAX * SIZE_TRACETEXT];
	BYTE	c3[LIMIT_TRACE][ECUTRACEPAGE_MAX][SIZE_TRACETEXT];
} ECUTRACETEXTS;

enum enARRAGEECUTRACESTEP {			// ECU Ʈ���̽� �����͸� ������ ����
	AETSTEP_EMPTY,
	AETSTEP_WAITARRANGE,
	AETSTEP_WAITLOAD,
	AETSTEP_END
};

enum enSERVICEDEVICECTRLCODEBIN {	// ���� ��ġ ����� ���� �ڵ�
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
