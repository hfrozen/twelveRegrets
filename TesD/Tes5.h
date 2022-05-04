// Tes5.h

#ifndef TES5_H_
#define TES5_H_

#include "ReferTes.h"

//#define DEF_SOCKMAC			"\x00\x08\xdc\x03\x02\x01"	// Default Mac Address : 00.08.dc.03.02.01
#define	DEF_SOCKGATE		0xc1a80001	// Default Gateway = 193.168.0.1
//#define	DEF_SOCKSIP			0xc1a80032	// Default Source IP = 193.168.00.50
#define	DEF_SOCKSIP			0xc1a80050	// Default Source IP = 193.168.00.80
#define	DEF_SOCKMASK		0xffffff00	// Default Subnet Mask = 255.255.255.0
#define	DEF_SOCKDNS			0x00000000	// Default DNS Server IP = 0.0.0.0
#define DEF_SOCKLPT			5000		// Default Listen port = 5000
#define DEF_SOCKCPT			3000		// Default Connect port = 3000
#define	DEF_SOCKHTTP		80			// Default Http port = 80

// 128 clk=18.432, UX=0

#define CSTT_1MS	-144		// by clk/128

#define BPS115200	9
#define BPS38400    29
#define BPS19200    59
#define BPS9600     119
#define BPS4800     239
#define BPS2400     479

#define	ReloadTimer()	TCNT0 = (BYTE)(CSTT_1MS & 0xff)

#define DEF_TCCR1B	(1 << WGM13) | (1 << WGM12)
#define	DEF_TCCR3B	(1 << WGM33) | (1 << WGM32)

// uTesState;
#define	TESSTATE_AWAKE			1
#define	TESSTATE_SCANINB		2
#define	TESSTATE_CONNECTCLIENT	4
#define	TESSTATE_WAITLOCAL		8
#define	TESSTATE_MONIT			0x80

// uDebug;
#define	DEBUG_MONITSOCK			1
#define	DEBUG_MONITBUS			2
#define DEBUG_MONITCOMM			4
#define	DEBUG_MONITCOMMA		8
#define	DEBUG_MONITCOMMA2		0x10
#define	DEBUG_INP				0x20

#define	SockM()		(cDebug & DEBUG_MONITSOCK)
#define	BusM()		(cDebug & DEBUG_MONITBUS)
#define CommM()		(cDebug & DEBUG_MONITCOMM)
#define	ComaM()		(cDebug & DEBUG_MONITCOMMA)
#define	ComaM2()	((cDebug & (DEBUG_MONITCOMMA | DEBUG_MONITCOMMA2)) == (DEBUG_MONITCOMMA | DEBUG_MONITCOMMA2))
#define	InM()		(cDebug & DEBUG_INP)

/*//#define	TRACE(fmt, ...)\
//	do {\
//		printf_P(PSTR("[%02d:%02d:%03d]"), time.min, time.sec, time.ms);\
//		printf_P(fmt, ##__VA_ARGS__);\
//	} while (0)*/

#define	TRACE(fmt, ...)\
	do {\
		printf_P(fmt, ##__VA_ARGS__);\
	} while (0)

#define	NTRACE(fmt, ...)	printf_P(fmt, ##__VA_ARGS__)

#define	PRINTT()	printf_P(PSTR("[%02d:%02d:%03d]"), time.min, time.sec, time.ms)

enum  {
	CTYPE_TRAFFIC = 0,
	CTYPE_COMM,
	CTYPE_INP,
	CTYPE_OUTP,
	CTYPE_PWM,
	CTYPE_MAX
};

#define	DEVLENG_MAX		16

#define	SIZE_DLEBUF		128
BYTE	cDleBuf[SIZE_DLEBUF];

#define SOCK_SERVER		0

#define SIZE_ENVELOPE	256
typedef struct _tagENVELOPE {
	int		i;
	BYTE	s[SIZE_ENVELOPE];
} ENVELOPE;

typedef struct _tagHOST {
	ENVELOPE	r;
	ENVELOPE	t;
} HOST;

typedef struct _tagLOCAL {
	BYTE		cCurInb;
	BYTE		cMaxInb;
	BYTE		cScanInb;
	BYTE		cWaitResp;
#define	TIME_LOCALWAITRESPOND	10
#define	TIME_LOCALRXCONTINUE	3
	ENVELOPE	r;
	ENVELOPE	t;
} LOCAL;

typedef struct _tagMONC {
	ENVELOPE	r;
} MONC;
//#define MONCMD_BEGINF		'\t'
//#define MONCMD_ENDF		'\n'

typedef union _tagPIECES {
	struct {
		BYTE l : 4;
		BYTE h : 4;
	} b;
	BYTE a;
} PIECES;

typedef struct _tagHOSTCMDHEAD {
	BYTE*	p;
	BYTE	left;
	BYTE	leng;
	PIECES	cmd;
	BYTE	cid;
} HCH, *PHCH;

enum  {
	HCMD_GRRESET = 1,
	HCMD_GRSREPORT,
	HCMD_SCRESET = 0x10,
	HCMD_SCINIT,		// 0x11
	HCMD_SCCONFIG,		// 0x12
	HCMD_SCSREPORT,		// 0x13
	HCMD_SCSEND,		// 0x14
	HCMD_SCRESVSEND,	// 0x15
	HCMD_SCRREPORT,		// 0x16
	HCMD_DIRESET = 0x20,
	HCMD_DIRREPORT,
	HCMD_DICREPORT,
	HCMD_DORESET = 0x30,
	HCMD_DORREPORT,
	HCMD_DOCREPORT,
	HCMD_DOABSPUT,
	HCMD_DOSELPUT,
	HCMD_PMRESET = 0x40,
	HCMD_PMRREPORT,
	HCMD_PMCREPORT,
	HCMD_PMABSPUT,
	HCMD_MAX = 0x50
};

typedef struct _tagTIMES {
	BYTE	hour;
	BYTE	min;
	BYTE	sec;
	WORD	ms;
} TIMES;

void HostCommand(void);
void CommInit(BYTE ch);
HCH AbstractCmd(BYTE* p, BYTE leng, BOOL cmd);
void Shift(BYTE* p, int sn);
void PrintIP(DWORD dwIp);
void RespSock(BYTE cId);
void PreSend(BYTE cId);
void CidReport(void);
void StatusReport(BYTE ch, BYTE cmd);
void RecvReport(BYTE ch);
void InpReport(void);
void InpNullReport(void);
void Output(void);
void OutpReport(void);
void OutpNullReport(void);
void PwmCtrl(BYTE ch);
void PwmReport(BYTE ch);
void PwmNullReport(BYTE ch);
void InvalidCommand(BYTE* p);
BYTE DleEncoder(BYTE* pSrc, BYTE cLength);
BYTE DleDecoder(BYTE* pDest, BYTE cLength);
void PutsLB(BYTE *p, BYTE leng);
void PutCharLB(BYTE c);
//static int PutChar(char c, FILE* stream);
void SendByte(BYTE c);
void SendChar(BYTE c);
BYTE ctoa(BYTE c);
BYTE Turnup(BYTE c);
void Delay(WORD delay);
void InitialDev(void);
void Monit(void);

#endif /* TES5_H_ */