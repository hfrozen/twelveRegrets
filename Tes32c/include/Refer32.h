// refer32.h
#pragma once
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>

#define	I2C_PRIORITY				5

#define	UART_PRIORITY				6
#define	UART_RX_SUBPRIORITY			0
#define	UART_TX_SUBPRIORITY			1

#define	SYSTICK_PRIORITY			4
#define	SYSTICK_SUBPRIORITY			0

#define	PWNIN_PRIORITY				7
#define	PWMIN_SUBPRIORITY			0

#define	EXTI_PRIORITY				7
#define	EXTI_SUBPRIORITY			0

#ifdef	BYTE
#undef	BYTE
#endif
typedef uint8_t		BYTE;

#ifdef	WORD
#undef	WORD
#endif
typedef uint16_t	WORD;

#ifdef	DWORD
#undef	DWORD
#endif
typedef uint32_t	DWORD;

#define	MAKEDWORDA(c0, c1, c2, c3)	((DWORD)c3 << 24 | (DWORD)c2 << 16 | (DWORD)c1 << 8 | (DWORD)c0)
#define	MAKEWORDA(l, h)	((WORD)h << 8 | (WORD)l)
#define	LOBYTEA(x)		(BYTE)(x & 0xff)
#define	HIBYTEA(x)		(BYTE)((x >> 8) & 0xff)
#define	ADDR_RESP		(0xa0 | g_id.cur.address.a)
#define	MAX_PWMRATE		1000			// 100.0%

typedef union _tagDUET {
	BYTE	c[2];
	WORD	w;
} _DUAL;

typedef union _tagQUAD {
	BYTE	c[4];
	WORD	w[2];
	DWORD	dw;
} _QUAD;

typedef union _tagPIXEL8 {
	struct {
		BYTE	b0 : 1;		BYTE	b1 : 1;		BYTE	b2 : 1;		BYTE	b3 : 1;
		BYTE	b4 : 1;		BYTE	b5 : 1;		BYTE	b6 : 1;		BYTE	b7 : 1;
	} b;
	struct {
		BYTE	l : 4;
		BYTE	h : 4;
	} n;
	struct {
		BYTE	ad : 5;
		BYTE	c0 : 3;
	} ex;
	BYTE	a;
} PIXEL8;

typedef union _tagPIXEL32 {
	struct {
		DWORD	b0 : 1;		DWORD	b1 : 1;		DWORD	b2 : 1;		DWORD	b3 : 1;
		DWORD	b4 : 1;		DWORD	b5 : 1;		DWORD	b6 : 1;		DWORD	b7 : 1;
		DWORD	b8 : 1;		DWORD	b9 : 1;		DWORD	b10 : 1;	DWORD	b11 : 1;
		DWORD	b12 : 1;	DWORD	b13 : 1;	DWORD	b14 : 1;	DWORD	b15 : 1;
		DWORD	b16 : 1;	DWORD	b17 : 1;	DWORD	b18 : 1;	DWORD	b19 : 1;
		DWORD	b20 : 1;	DWORD	b21 : 1;	DWORD	b22 : 1;	DWORD	b23 : 1;
		DWORD	b24 : 1;	DWORD	b25 : 1;	DWORD	b26 : 1;	DWORD	b27 : 1;
		DWORD	b28 : 1;	DWORD	b29 : 1;	DWORD	b30 : 1;	DWORD	b31 : 1;
	} b;
	DWORD	a;
} PIXEL32;

typedef struct _tagIDENTITY {
	PIXEL8	role;
	PIXEL8	address;
} IDENTITY;

enum  {
	ROLE_TR = 0,		// traffic
	ROLE_SC,			// serial i/f
	ROLE_DI,			// di
	ROLE_DO,			// do
	ROLE_PO,			// pwm out
	ROLE_PI,			// pwm in
	ROLE_MAX
};

typedef struct _tagIDDRAW {
	IDENTITY	cur;
	IDENTITY	buf;
	int			deb;
} IDDRAW;
#define	DEB_IDENTITY	20

#define	SOCK_SERVER				0
#define	SOCK_PORT				5000
#define SIZE_ENVELOPE	1024
typedef struct _tagENVELOPE {
	WORD	i;
	BYTE	s[SIZE_ENVELOPE];
} EVELOPE, *PENVELOPE;

typedef struct _tagHOST {
	EVELOPE	r;
	EVELOPE	t;
} HOST;

typedef struct _tagBUSCMDHEAD {
	BYTE*	p;
	int		left;
	int		leng;
	PIXEL8	cmd;
	PIXEL8	cid;
} BCHEAD, *PBCHEAD;

enum enBUSCMD {
	BUSCMD_NULL,
	BUSCMD_GRESET,
	BUSCMD_GREPORT,

	BUSCMD_SCRESET =	0x10,
	BUSCMD_SCINIT,		// 0x11
	BUSCMD_SCCONFIG,	// 0x12
	BUSCMD_SCSREPORT,	// 0x13
	BUSCMD_SCSEND,		// 0x14
	BUSCMD_SCCAP,		// 0x15
	BUSCMD_SCRREPORT,	// 0x16
	BUSCMD_SCINITEX,	// 0x17
	BUSCMD_SCCONFIGEX,	// 0x18
	BUSCMD_SCSREPORTEX,	// 0x19

	BUSCMD_DIRESET =	0x20,
	BUSCMD_DIRREPORT =	0x25,
	BUSCMD_DICREPORT,

	BUSCMD_DORESET =	0x30,
	BUSCMD_DORREPORT =	0x35,
	BUSCMD_DOCREPORT,	// 0x36
	BUSCMD_DOPUTDWORD,	// 0x37
	BUSCMD_DOPUTBIT,	// 0x38

	BUSCMD_PORESET =	0x40,
	BUSCMD_PORREPORT =	0x45,
	BUSCMD_POCREPORT,
	BUSCMD_POPUT,

	BUSCMD_PIRESET =	0x50,
	BUSCMD_PIRREPORT =	0x55,
	BUSCMD_PICREPORT,

	BUSCMD_MAX
};

enum enBUSRESPOND {
	BUSRESP_DIACT = BUSCMD_DIRREPORT,
	BUSRESP_DINON = BUSCMD_DICREPORT,
	BUSRESP_DOACT = BUSCMD_DORREPORT,
	BUSRESP_DONON = BUSCMD_DOCREPORT,
	BUSRESP_PIACT = BUSCMD_PIRREPORT,
	BUSRESP_PINON = BUSCMD_PICREPORT,
	BUSRESP_POACT = BUSCMD_PORREPORT,
	BUSRESP_PONON = BUSCMD_POCREPORT,
};

// for g_wShape
enum enSHAPE {
	SHAPE_AWAKE = 0,
	SHAPE_SCANINGINPS,
	SHAPE_CONNECTCLIENT,
	SHAPE_WAITLOCAL,
	SHAPE_DEBUGSOCKET,
	SHAPE_DEBUGTRBUS,
	SHAPE_DEBUGTRTX,
	SHAPE_DEBUGDPIO
};

#define	DEBUGSOCKET()	(g_wShape & (1 << SHAPE_DEBUGSOCKET))
#define	DEBUGTRBUS()	(g_wShape & (1 << SHAPE_DEBUGTRBUS))
#define	DEBUGTRTX()		(g_wShape & (1 << SHAPE_DEBUGTRTX))
#define	DEBUGDPIO()		(g_wShape & (1 << SHAPE_DEBUGDPIO))

typedef struct _tagTRXSTREAM {
	BYTE	c;
	BYTE	wait;		// TR, SC:	wait cycle end, other:	wait receive end
	EVELOPE	r;
	EVELOPE	t;
} TRXSTREAM;
#define	TIME_RXOFF			3
#define	TIME_CYCLEOFF		7

typedef struct _tagUARTHANDLE {
	UART_HandleTypeDef	hu;
	DMA_HandleTypeDef	hd;
	TRXSTREAM			buf;
} UARTHND, *PUARTHND;

typedef struct _tagPWMFABRIC {
	char*			pName;
	TIM_TypeDef*	pHt;
	uint32_t		dwCh;
} PWMFABRIC, *PPWMFABRIC;
#define	PWMOUT_LENGTH		8		//16
#define	PWMIN_LENGTH		2

typedef struct _tagPWMDUTY {
	//DWORD	dwPeriod[2];
	DWORD	dwPeriod;
	DWORD	dwWidth;
	WORD	wTimer;				// set to 0 when no interrupt for a certain period
	struct {
		WORD	bFall :		1;
		WORD	bAlter :	1;
		WORD	bRes :		14;
	} state;
} PWMDUTY;
#define	TIME_ZEROPERIOD		5

/*=============================== FPGA REGISTER ===============================
	0x00:	(r)		rx buffer	/ (w)	tx buffer

	0x02:	(r)		rx length

	0x04:						/ (w)	0x0008: tx enable
										0x0009: tx disable
										0x0080: tx abort
										0x0090: tx continue
										0x0800:	rx enable
										0x0900: rx disable

	0x06:	(r/w)	b15:	b14:	b13:
					b12: hdlc mode
					b11: dpll enable
					b10: address match	/ stop length
					b09: manchester		/ odd parity
					b08: nrzi			/ even parity
					b07: fcs enable
					b06:
					b05: tx abort
					b04: tx enable
					b03:	b02:	b01:
					b00: rx enable

	0x08:	(r)		b15:	b14:	b13:	b12:	b11:	b10:
					b09: tx halt
					b08: tx empty
					b07: rx abort
					b06: rx parity error
					b05: rx overflow error
					b04: rx overlap error
					b03: rx overrun error
					b02: rx frame error
					b01: rx fcs error
					b00: rx full

	0x0a:	(r/w)	b15:	b14:	b13:	b12:	b11:	b10:
					b09: tx halt interrupt enable
					b08: tx empty interrupt enable
					b07: rx abort interrupt enable
					b06: rx overrun error interrupt enable
					b05: rx overlap error interrupt enable
					b04: rx overflow error interrupt enable
					b03: rx frame error interrupt enable
					b02: rx fcs error interrupt enable
					b01: rx parity error interrupt enable
					b00: rx full interrupt enable

	0x0c:	(r)		b15:	b14:	b13:	b12:	b11:	b10:
 					b09: tx halt interrupt status
					b08: tx empty interrupt status
					b07: rx abort interrupt status
					b06: rx overrun error interrupt status
					b05: rx overlap error interrupt status
					b04: rx overflow error interrupt status
					b03: rx frame error interrupt status
					b02: rx fcs error interrupt status
					b01: rx parity error interrupt status
					b00: rx full interrupt status

	0x0e:	(r/w)	hdlc address

	0x10:	(r/w)	tx length

	0x12:	(r/w)	hdlc sync. flag length

	0x14:	(r/w)	baudrate = 147456000 / (bps * 32) - 1
					bps = 147456000 / (baudrate constant + 1) * 32

	0x16:	(r)		only 0x375a
=============================================================================*/

enum enPORTBASE {
	PORT_BASE	= 0x60000000
};

enum enPORTREGOFFSETS {
	PROFS_BUFFER		= 0,
	PROFS_RXLENGTH		= 2,
	PROFS_COMMAND		= 4,
	PROFS_CONFIG		= 6,
	PROFS_STATUS		= 8,
	PROFS_INTRENABLE	= 10,	// 0xa
	PROFS_INTRSTATE		= 12,	// 0xc
	PROFS_ADDRESS		= 14,	// 0xe
	PROFS_TXLENGTH		= 16,	// 0x10
	PROFS_SYNCLENGTH	= 18,	// 0x12
	PROFS_BAUDRATE		= 20,	// 0x14
	PROFS_RESET			= 22,	// 0x16
	PROFS_XLIVECODE		= 24,	// 0x18
	PROFS_MANUALCTRL	= 26	// 0x1a
};

//#define	PREG(x, reg)		*((__IO uint16_t*)(PORT_BASE + (x << 8) + reg))
//#define	DREG(reg)			*((__IO uint16_t*)(PORT_BASE + reg))
#define	PREG(x, reg)		*((__IO WORD*)(PORT_BASE + (x << 8) + reg))
#define	DREG(reg)			*((__IO WORD*)(PORT_BASE + reg))
#define DWREG(x, reg)		(DWORD)(PORT_BASE + (x << 8) + reg)

enum enPORTCMD {
	PCW_TXENABLE		= 0x8,
	PCW_TXDISABLE		= 0x9,
	PCW_TXABORT			= 0x80,
	PCW_TXCONTINUE		= 0x90,
	PCW_RXENABLE		= 0x800,
	PCW_RXDISABLE		= 0x900
};

enum enHDLC_ATTRIBUTE {
	PAB_RXENABLE = 0,
	PAB_TXENABLE = 4,
	PAB_TXABORT = 5,
	PAB_FCSENABLE = 7,
	PAB_NRZI,
	PAB_MANCHESTER,
	PAB_ADDRESSMATCHENABLE,
	PAB_DPLLENABLE,
	PAB_HDLC
};

enum enASC_ATTRIBUTE {
	PAB_EVENPARITY = 8,
	PAB_ODDPARITY,
	PAB_STOPLENGTH
};

enum enPORT_STATUS {
	PSB_RXFULL = 0,
	PSB_RXFCSERROR,
	PSB_RXFRAMEERROR,
	PSB_RXOVERRUNERROR,
	PSB_RXOVERLAPERROR,
	PSB_RXOVERFLOWERROR,
	PSB_RXPARITYERROR,
	PSB_RXABORT,
	PSB_TXEMPTY,
	PSB_TXHALT
};

enum enPORT_RXSTATUS {
	PSW_RXSTATUS = (1 << PSB_RXFULL) | (1 << PSB_RXFCSERROR) | ( 1 << PSB_RXFRAMEERROR) | (1 <<PSB_RXOVERRUNERROR) |
					(1 << PSB_RXOVERLAPERROR) | (1 << PSB_RXOVERFLOWERROR) | (1 << PSB_RXPARITYERROR) | (1 << PSB_RXABORT)
};
enum enPORT_INTERRUPTS {
	PIB_RXFULL = 0,
	PIB_RXPARITYERROR,
	PIB_RXFCSERROR,
	PIB_RXFRAMEERROR,
	PIB_RXOVERFLOWERROR,
	PIB_RXOVERLAPERROR,
	PIB_RXOVERRUNERROR,
	PIB_RXABORT,
	PIB_TXEMPTY,
	PIB_TXHALT
};

// di, do
typedef struct _tagDIOBUF {
	_QUAD	cur;		// current
	_QUAD	prev;		// previous
} _DIOB;

// pi, po
typedef struct _tagPWMB {
	_QUAD	period;
	_QUAD	width;
} _PWMB;

typedef struct _tagPIOBUF {
	_PWMB	cur[PWMOUT_LENGTH];
	_PWMB	prev[PWMOUT_LENGTH];
} _PIOB;

// serial
#define MAX_PORT		8

enum enPROJID {
	PRJ_LINE2,
	PRJ_LINE7,
	PRJ_MAX
};

enum  enSDID {
/*00*/	SDID_ATC,
/*01*/	SDID_ATO,				// same to TcmsWork/Qnxw/Common/Arrange.h
/*02*/	SDID_PAU,
/*03*/	SDID_PIS,
/*04*/	SDID_TRS,
/*05*/	SDID_AVC,
/*06*/	SDID_RFD,
/*07*/	SDID_SIV,
/*08*/	SDID_V3F,
/*09*/	SDID_ECU,
/*10*/	SDID_HVAC,
/*11*/	SDID_DCUL,
/*12*/	SDID_DCUR,
/*13*/	SDID_BAT,
/*14*/	SDID_CMSB,
/*15*/	SDID_FDU,
/*16*/	SDID_RESA,
/*17*/	SDID_RESB,
/*18*/	SDID_MAX
};

enum enSDIDN {
	SDID_NON = SDID_MAX
};

enum enDEVCHAR {
	DEVCHAR_CTL = 0x13,
	DEVFLOW_REQ = 0x20,
	DEVFLOW_ANS = 0x30,
	DEVTRACE_REQ = 0x21,
	DEVTRACE_ANS = 0x31
};

typedef struct _tagDEVMOLD {
	WORD	wAddr;
	BYTE	cBpss;
	BYTE	cMode;
} DEVMOLD;

enum _enHDLCMODE {
	SCMODE_HDLC,
	SCMODE_MATCH,
	SCMODE_MANCHE,
	SCMODE_NRZI,
	SCMODE_FCS
};

enum _enASYNCMODE {
	SCMODE_ASYNC,
	SCMODE_STOPL,
	SCMODE_ODDP,
	SCMODE_EVENP,
	SCMODE_CRCA,
	SCMODE_CRCB
};

enum _enBPS {
	SCBPS_9600,
	SCBPS_19200,
	SCBPS_38400,
	SCBPS_115200,
	SCBPS_256K,
	SCBPS_512K,
	SCBPS_1152K,
	SCBPS_1536K,
	SCBPS_2304K,
	SCBPS_4608K
};

typedef struct _tagSCMOLD {
	BYTE	cDevID;
	_DUAL	addr;
	PIXEL8	mode;
	union {
		struct {
			BYTE	bps : 4;
			BYTE	cnt : 1;
			BYTE	dish : 1;
			BYTE	res : 2;
		} b;
		BYTE	a;
	} conf;
	union {
		struct {
			BYTE	disr : 4;
			BYTE	scl : 4;
		} b;
		BYTE	a;
	} enr;
} SCMOLD;

typedef struct _tagFRMLENGTH {
	WORD	r;		// rx length
	WORD	t;		// tx length
	WORD	c0;		// compare begin offset
	WORD	l0;		// compare length
	WORD	c1;
	WORD	l1;
	WORD	tlo;	// trace length data position offset
	BYTE	tlm;	// trace length data mask
	BYTE	tls;	// trace length data shift
} FRMLENGTH;

enum enSBUFSERIES {
	SB_CURRENT,
	SB_STORAGE,
	SB_MAX
};

#define	SIZE_SCBUF		512
typedef struct _tagSCFRAME {
	BYTE	sBuf[SB_MAX][SIZE_SCBUF];
	int		i;
} SCFRM;

typedef struct _tagSCBUF {
	SCMOLD	m;
	SCFRM	r;
	SCFRM	t;
	WORD	wStatus;
	WORD	wRespt;			// respond delay time
} SCBUF;
#define	RESPOND_DELAY		2	//5

enum enSCBUFSTATE {
	SCBS_RXFULL = 15
};

// bunch
typedef union _tagLIMB {
	SCBUF	sc[MAX_PORT];
	_DIOB	dio;
	_PIOB	pio;
} LIMB;
