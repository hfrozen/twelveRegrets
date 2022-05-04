/**
 * @file	DefTypes.h
 * @brief
 * @details
 * @author
 * @date
 */
#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

#if	!defined(_WIN32)
#include <stdbool.h>
#include <inttypes.h>

// Size of each variables.
// bool =			1
// char =			1
// unsigned char =	1
// short =			2
// unsigned short =	2
// int =			4
// unsigned int =	4
// long =			4
// unsigned long =	4
// int64_t =		8
// long long =		8
// float =			4
// double =			8
// long double =	12
// *void =			4
// *char =			4
// *int =			4
// *long =			4
// *float =			4
// *double =		4

#if	defined(BYTE)
#undef	BYTE
#endif
typedef uint8_t		BYTE;

#if	defined(WORD)
#undef	WORD
#endif
typedef	uint16_t	WORD;

#if	defined(DWORD)
#undef	DWORD
#endif
typedef uint32_t	DWORD;

#if	defined(QWORD)
#undef	QWORD
#endif
typedef	uint64_t	QWORD;

#if	defined(PVOID)
#undef	PVOID
#endif
typedef void*		PVOID;

#if	defined(SHORT)
#undef	SHORT
#endif
typedef short		SHORT;

#if	defined(PSZ)
#undef	PSZ
#endif
typedef const char*		PSZ;

#pragma pack(push, 1)

typedef union	_tagDUET {
	BYTE	c[2];
	WORD	w;
} _DUET;

typedef union	_tagQUARTET {
	BYTE	c[4];
	WORD	w[2];
	DWORD	dw;
} _QUARTET;

typedef union	_tagOCTET {
	BYTE	c[8];
	WORD	w[4];
	DWORD	dw[2];
	QWORD	qw;
} _OCTET;

typedef struct _tagINCERT {
	_OCTET	oct;
	WORD	wCertify;
} INCERT;

typedef struct _tagOUTCERT {
	_DUET	duet;
	WORD	wCertify;
} OUTCERT;

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
} DBFTIMEA, *PDBFTIMEA;

#pragma pack(pop)

#define	SIZE_GENSBUF	2048		// byte, should be same as MAX_ROADBUFW of CFsc		//1024

#include <mqueue.h>
typedef mqd_t			QHND;

#include <pthread.h>
typedef pthread_t		TRHND;

#include <time.h>
typedef timer_t			TMHND;

#include <assert.h>
#define	ASSERT			assert
#define	ASSERTP(p)		assert(p != NULL)

#define	INVALID_HANDLE		-1

#endif

#define	DEBUG_ALL				0
#define	DEBUG_CLASS				1
#define	DEBUG_HANDLER			2
#define	DEBUG_TAG				3
#define	DEBUG_INTR				4
#define	DEBUG_DRIVER			5
#define	DEBUG_COMM				6
#define	DEBUG_INFO				7
#define	DEBUG_NON				8

#define	_DEBUGL_				DEBUG_DRIVER	//DEBUG_CLASS

#define	_DEBL(x)				(_DEBUGL_ <= x)

#ifdef	__cplusplus
}
#endif
