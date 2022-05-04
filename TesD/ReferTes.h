// ReferTes.h

#ifndef REFERTES_H_
#define REFERTES_H_

#ifndef TRUE
#define	TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef	NULL
#define NULL	'\0'
#endif

#ifndef BOOL
typedef unsigned char BOOL;
#endif

#define _UNSIGNEDBYTE_

#ifndef BYTE
#ifdef	_UNSIGNEDBYTE_
typedef unsigned char BYTE;
#else
typedef char BYTE;
#endif
#endif

#ifndef WORD
typedef	unsigned int WORD;
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef PVOID
typedef void* PVOID;
#endif

#define FBYTE static unsigned char __attribute__ ((progmem))
#define FWORD static unsigned int __attribute__ ((progmem))

#define MAKEWORD(l, h)	(((WORD)h << 8) | (WORD)l)

#define	HIBYTE(x)		((BYTE)((x >> 8) & 0xff))
#define LOBYTE(x)		((BYTE)(x & 0xff))

#define	HINIBB(x)		((BYTE)((x >> 4) & 0xf))
#define	LONIBB(x)		((BYTE)(x & 0xf))

#define	T1FALL()	PORTE &= (PINE & ~4)
#define	T1RISE()	PORTE |= (PINE | 4)
#define	T2FALL()	PORTB &= (PINB & ~2)
#define	T2RISE()	PORTB |= (PINB | 2)

//===== digital input =====
typedef  struct _tagINPBUF {
	BYTE	uCur[3];
	BYTE	uBuf[3];
	BYTE	uVal[3];
	BYTE	uVrf[3];
	BYTE	uDeb[3];
#define	DEB_INP				20
	BYTE	uReply;
#define	CYCLE_INREPLY		5	// do not respond directly, only after 5 cycle
} INPBUF;

// ===== digital output =====
typedef struct _tagOUTPBUF {
	BYTE	uVal[3];
	BYTE	uVrf[3];
	BYTE	uRef[3];
} OUTPBUF;

// ===== pwm =====
typedef struct _tagPWMBUF {
	WORD	wVal[4];
	WORD	wVrf[4];
	WORD	v;
} PWMBUF;

// ===== serial comm. =====
//#define	SIZE_SCCBUF			(128 - 6)
#define	SIZE_SCCBUF				(128 - 6) / 2
//#define	WAIT_RXCLOSE		30	//, never over than 30(ms)
//#define	WAIT_RXCLOSEA		8
//#define	WAIT_TXCLOSE		2
#define	CYCLE_INHIBITECHO	2	// at async

typedef union _tagSCCTYPE {
	struct {
		BYTE	bps :	4;
		BYTE	mode :	2;	// SCCMODE_
		BYTE	cci :	1;	// cycle count inhibit, at ato mode, cycle count increase inhibit(1) / enable(0).
		BYTE	rrm :	1;	// report motive, report received frames on every receive(1) or host request(0, cmd is 0x15).
	} b;
	BYTE a;
} SCCTYPE;

enum  {
	SCCBPS_9600,
	SCCBPS_19200,
	SCCBPS_38400,
	SCCBPS_115200,
	SCCBPS_256k,
	SCCBPS_512k,
	SCCBPS_1M,
	SCCBPS_1M5,
	SCCBPS_MAX
};

enum  {
	SCCMODE_ASYNC,
	SCCMODE_ASYNCBCC,
	SCCMODE_ASYNCCRC,
	SCCMODE_SYNC,
	SCCMODE_MAX
};

typedef union _tagSCCATTR {
	struct {
		BYTE	id : 4;
		BYTE	dr0 : 1;	// 1:disable respond.
		BYTE	dr1 : 1;	// 1:disable respond (at dcu mode).
		BYTE	dr2 : 1;	// 1:disable respond (at dcu mode).
		BYTE	dr3 : 1;	// 1:disable respond (at dcu mode).
	} b;
	BYTE a;
} SCCATTR;

enum  {
/*00*/	SDID_ATO,				// same to TcmsWork/Qnxw/Common/Arrange.h
/*01*/	SDID_RTD,
/*02*/	SDID_PAU,
/*03*/	SDID_PIS,
/*04*/	SDID_TRS,
/*05*/	SDID_PDU,
/*06*/	SDID_SIV,
/*07*/	SDID_V3F,
/*08*/	SDID_ECU,
/*09*/	SDID_HVAC,
/*10*/	SDID_DCUL,
/*11*/	SDID_DCUR,
/*12*/	SDID_CM,
/*13*/	SDID_ESK,
/*14*/	SDID_PD,
/*15*/	SDID_PAN,
/*16*/	SDID_MAX
};

#define	SDID_ATON	SDID_CM
#define	SDID_TRSN	SDID_ESK

typedef union _tagDUET {
	BYTE	c[2];
	WORD	w;
} _DUET;

typedef struct _tagSCCCONF {
	SCCTYPE	tp;			//					- 1 byte
	SCCATTR	att;		// attr.			- 1 byte
	_DUET	addr;		// device addr		- 2 byte
	// BYTE	sdrl[2];	// tx, rx length
} SCCCONF;

typedef struct _tagSCCCHB {
	BYTE	sBuf[2][SIZE_SCCBUF];	// (128 - 6) / 2 = 61
	BYTE	ci;
	BYTE	cResv;
	BYTE	cCloseWait;
	BYTE	cNull;
} SCCCHB;

typedef struct _tagSCCCHS {
	SCCCHB	rx;			// 126 byte
	SCCCHB	tx;			// 126 byte
	SCCCONF	cf;			// 4 byte
} SCCCHS;

#define RXFBYDICH(di, ch)		(1 << ((di << 1) + ch))

typedef struct _SCCBYF {
	BYTE	cState;
	SCCCHS	pt[8];
} SCCBUF;

typedef union _tagLIMB {
	SCCBUF	scc;
	INPBUF	inp;
	OUTPBUF	outp;
	PWMBUF	pwm;
} LIMB;
//LIMB	lm;

#define	TXB		tx.sBuf
#define	TXI		tx.ci
#define	TXR		tx.cResv
#define	TXCW	tx.cCloseWait

#define	RXB		rx.sBuf
#define	RXI		rx.ci
#define	RXR		rx.cResv
#define	RXCW	rx.cCloseWait

typedef struct _tagFLAGS {
	BYTE	i;
	BYTE	b[64];
} _FLAGS;

#define	DEVCHAR_CTL		0x13
#define	DEVFLOW_REQ		0x20
#define	DEVFLOW_ANS		0x30
#define	DEVTRACE_REQ	0x21
#define	DEVTRACE_ANS	0x31

#define	XWORD(x)	((((x) & 0xff00) >> 8) | (((x) & 0xff) << 8))

#endif /* REFERTES_H_ */

/*
SCCBUF scc	+-	cState
			+-	pt[8]	+-	tx	+-	sBuf[2][61]
						|-	rx	|-	ci
						|		|-	cResv
						|		|-	cCloseWait
						|		|-	cNull
						|
						+-	cf	+-	tp	+-	bps : 4
								|		|-	mode : 1
								|		|-	cci : 1
								|		|-	rrm : 1
								|
								+-	att	+-	id : 4
								|		|-	dr0~dr3 : 1~
								|
								+-	addr(WORD)
*/
