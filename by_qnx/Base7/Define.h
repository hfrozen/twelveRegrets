#ifndef _DEFINE_H
#define _DEFINE_H

#ifndef	TRUE
#define	TRUE	1
#endif
#ifndef	FALSE
#define	FALSE	0
#endif
#ifndef	NULL
#define	NULL	'\0'
#endif

/*#ifdef  BOOL
#undef  BOOL
#endif
*/

#ifndef	_LITTLE_ENDIAN
#ifndef	BOOLTYPE
#define	BOOLTYPE
typedef unsigned char	BOOL;
#endif
#endif

#ifdef  BYTE
#undef  BYTE
#endif
typedef unsigned char   BYTE;

#ifdef	SHORT
#undef	SHORT
#endif
typedef short	SHORT;

#ifdef  WORD
#undef  WORD
#endif
typedef unsigned short	WORD;

#ifdef  LWORD
#undef  LWORD
#endif
typedef unsigned short	LWORD;

#ifdef  BWORD
#undef  BWORD
#endif
typedef unsigned short	BWORD;

#ifdef	UINT
#undef	UINT
#endif
typedef unsigned int	UINT;

#ifdef  DWORD
#undef  DWORD
#endif
typedef unsigned long   DWORD;

#ifdef  PSZ
#undef  PSZ
#endif
typedef char*  PSZ;

#ifdef  PUSZ
#undef  PUSZ
#endif
typedef unsigned char*  PUSZ;

#ifdef	PVOID
#undef	PVOID
#endif
typedef void*	PVOID;

#ifdef	_TV_
#undef	_TV_
#endif
#define	_TV_	SHORT	// 2 byte, very important size, use to TEBE value

#define M_PI		3.14159265358979323846

#define	CURV	SHORT
#define	UCURV	WORD

#define	MAX_TRAIN_LENGTH	8

typedef struct _tagTRACEARCH {
	BYTE	nState;
#define	TRACESTATE_LOAD		1		// tc:trace start, cc:trace start from tc
#define	TRACESTATE_END		2		// tc:trace end
#define	TRACESTATE_SAVE		4		// not used
#define	TRACESTATE_SAVEEND	8		// not used
#define	TRACESTATE_LOADDEV	0x10	// cc:trace start to device

	BYTE	nDeb;
#define	DEB_TRACING			5

	WORD	wLength;	// block length after debouncing
	WORD	wCur;		// total length = block length * block size
	WORD	wDev;		// total length after debouncing
	WORD	wiCc;		// index of transmission
	WORD	wiTc;		// index of transmission
} TRACEARCH, *PTRACEARCH;

#include <stdlib.h>
#include <stdarg.h>

#define	KILL(p)	{ if (p != NULL) { delete p; p = NULL; }}
#define	MAIL_MAX	1024

#define	IsValid(x)	((x) >= 0)
#define	IsString(x)	(x != NULL && *x != 0)

#define	BUNCH(name, b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0)\
union {\
	struct {\
		BYTE	b15 : 1;\
		BYTE	b14 : 1;\
		BYTE	b13 : 1;\
		BYTE	b12 : 1;\
		BYTE	b11 : 1;\
		BYTE	b10 : 1;\
		BYTE	b9 : 1;\
		BYTE	b8 : 1;\
		BYTE	b7 : 1;\
		BYTE	b6 : 1;\
		BYTE	b5 : 1;\
		BYTE	b4 : 1;\
		BYTE	b3 : 1;\
		BYTE	b2 : 1;\
		BYTE	b1 : 1;\
		BYTE	b0 : 1;\
	} b;\
	WORD	a;\
} name

#define	PARCELS(name, x, b7, b6, b5, b4, b3, b2, b1, b0)\
union {\
	struct {\
		BYTE	b7 : 1;\
		BYTE	b6 : 1;\
		BYTE	b5 : 1;\
		BYTE	b4 : 1;\
		BYTE	b3 : 1;\
		BYTE	b2 : 1;\
		BYTE	b1 : 1;\
		BYTE	b0 : 1;\
	} b;\
	BYTE	a;\
} name[x]

#define	PARCEL(name, b7, b6, b5, b4, b3, b2, b1, b0)\
union {\
	struct {\
		BYTE	b7 : 1;\
		BYTE	b6 : 1;\
		BYTE	b5 : 1;\
		BYTE	b4 : 1;\
		BYTE	b3 : 1;\
		BYTE	b2 : 1;\
		BYTE	b1 : 1;\
		BYTE	b0 : 1;\
	} b;\
	BYTE	a;\
} name

#define	HALFB(name, ms, ls)\
union {\
	struct {\
		BYTE	ms : 4;\
		BYTE	ls : 4;\
	} n;\
	BYTE	a;\
} name

#define	GETLWORD(w)	(((w & 0xff) << 8) | ((w & 0xff00) >> 8))
#define	GETBWORD(w)	(w)
#define	PUTLWORD(v, w)	v = (((w & 0xff) << 8) | ((w & 0xff00) >> 8))
#define	PUTBWORD(v, w)	v = w

#define	GETLDWORD(dw)	((GETLWORD(dw & 0xffff) << 16) | (GETLWORD(dw & 0xffff0000) >> 16))

#pragma pack(1)
typedef struct _tagTBTIME {
	WORD	year;		// 0-1
	BYTE	month;		// 2
	BYTE	day;		// 3
	BYTE	hour;		// 4
	BYTE	minute;		// 5
	BYTE	sec;		// 6
	WORD	msec;		// 7-8
} TBTIME, *PTBTIME;

typedef struct _tagDEVTIME {
	BYTE	year;		// 0
	BYTE	month;		// 1
	BYTE	day;		// 2
	BYTE	hour;		// 3
	BYTE	minute;		// 4
	BYTE	sec;		// 5
} DEVTIME, *PDEVTIME;
#pragma pack()

//#define	SCENE(x, y)	x = y
#define	SHOTB(c, w)			Shot(c, w)
#define	CUTB(c, w)			Cut(c, w, w)
#define	CUTBS(c, w, v)		Cut(c, w, v)
#define	EDITB(c, w, v, r)	Edit(c, w, v, r)
#define	SHOT(c, w)			pTcx->Shot(c, w)
#define	CUT(c, w)			pTcx->Cut(c, w, w)
#define	CUTS(c, w, v)		pTcx->Cut(c, w, v)
#define	EDIT(c, w, v, r)	pTcx->Edit(c, w, v, r)

//enum  {
//		MSGTYPE_EVENT = 0,
//		MSGTYPE_STATE,		// 1, 001
//		MSGTYPE_ALARM,		// 2, 010
//		MSGTYPE_LIGHT,		// 3, 011
//		MSGTYPE_HEAVY,		// 4, 100
//		MSGTYPE_CRITICAL,	// 5, 101
//		MSGTYPE_MAX			// 6, 110
//};
//#define	MSGTYPE_ONLYCODE	7
//#define	MSGTYPE_VALIDCODE	0xf
//
//#define	MSGTYPE_ADVANCE		0x10
//#define	MSGTYPE_HIDDEN		0x08

enum  {	ALARMTYPE_EVENT = 0,
		ALARMTYPE_STATE,		// 1, 001
		ALARMTYPE_TEXT,			// 2, 010
		ALARMTYPE_WARNING,		// 3, 011
		ALARMTYPE_LIGHT,		// 4, 100
		ALARMTYPE_SEVERE,		// 5, 101
		ALARMTYPE_EMERGENCY,	// 6, 110
		ALARMTYPE_MAX
};
#define	ALARMTYPE_ONLYCODE		7
#define	ALARMTYPE_VALIDCODE		0x0f
#define	ALARMTYPE_HIDDEN		0x08

#define	LOWNIBBLE(n)			(n & 0xf)
#define	HIGHNIBBLE(n)			((n >> 4) & 0xf)
#define	LOWBYTE(n)				(n & 0xff)
#define	HIGHBYTE(n)				((n >> 8) & 0xff)
#define	LOWWORD(dw)				(dw & 0xffff)
#define	HIGHWORD(dw)			((dw >> 16) & 0xffff)
#define	MAKEWORD(high, low)		((((WORD)high << 8) & 0xff00) | ((WORD)low & 0xff))
#define	MAKEDWORD(high, low)	((((DWORD)high << 16) & 0xffff0000) | (DWORD)low)
#define	INVERTWORD(w)			(((w & 0xff) << 8) | ((w & 0xff00) >> 8))

#define	INCBYTE(n)\
	do {\
		if (n < 0xff)	++ n;\
	} while(0)

#define	INCWORD(w)\
	do {\
		if (w < 0xffff)	++ w;\
	} while(0)

#define	VERSION_HIGH(n)	(BYTE)(n / 1)
#define	VERSION_LOW(n)	(BYTE)((WORD)(n * 100) % 100)

#define	DEBUG_ALL		0
#define	DEBUG_CLASS		1
#define	DEBUG_INTR		2
#define	DEBUG_DRIVER	3
#define	DEBUG_MAIL		4
#define	DEBUG_NON		5

#ifndef	_LITTLE_ENDIAN
#define	DEBUG	DEBUG_NON
#endif

//#define	LIU_BYPANEL
//#define	DEBUG_TDIR

#define	NAME_TCMQ	(PSZ)"/TcMq"
#define	NAME_CCMQ	(PSZ)"/CcMq"

typedef struct _tagMail {
	int		id;
	PVOID	pClass;
	long	param;
} MAIL, *LPMAIL;

//#define	DO_KILL	0

#define	MA_ACCEPT			1
#define	MA_SENDBUS			3
#define	MA_RECEIVEBUS		5
#define	MA_RECEIVEPERT		7
#define	MA_RECEIVEFAULT		9
#define	MA_RECEIVEDEVICE	11
#define	MA_RECEIVEPART		13
#define	MA_RECEIVELOCAL		15
#define	MA_RECEIVEDUC		17
#define	MA_RECEIVEPOCKET	19

#define	TIME_INTERVAL	10	// 10ms
#define	TIME_FLASH		300 / TIME_INTERVAL

#define	PRIORITY_INTRRECV	23
#define	PRIORITY_TIMER		22	// 23
#define	PRIORITY_TRIGGER	22	// 22
#define	PRIORITY_MAIL		22	// 20
#define	PRIORITY_MSG		16
#define	PRIORITY_POST		16
#define	PRIORITY_BEIL		16

#define	DEFAULT_DUCRECV		6
#define	PARTFLAG_BEGIN		0x1b
#define	PARTFLAG_END		0x0d

#endif

