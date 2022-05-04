/* CFsc.h	// Foundation to synchronous comm.
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "sys/siginfo.h"

#include "DuoTypes.h"
#include "Prefix.h"
#include "Track.h"
#include "Infer.h"
#include "CPump.h"
#include "CPop.h"
#include "CLabel.h"

#define	GETFSC(p)	CFsc* p = CFsc::GetInstance()

class CFsc :	public CLabel
{
public:
	CFsc();
	virtual ~CFsc();

private:
	PVOID		c_pSpace;
	bool		c_bEnable;
	PVOID		c_pFIO;

//#if	defined(DEBUG_TINT)
//	typedef struct _tagDEBUGTIME {	// UNREACHABLE LINE D가 발생하여 잡기위한 변수
//		double		min;
//		double		max;
//		WORD		wSetTime;
//		WORD		wRemain;
//		uint64_t	clk;
//		struct {
//			int		hour;
//			int		min;
//			int		sec;
//			int		msec;
//		} t;
//	} DEBUGTM;
//	DEBUGTM	c_dtm[2];
//#endif
	typedef union _tagROADMODE {
		struct {
			WORD		flen :	3;	// b0~2, flag length
			WORD		nula :	1;	// b3,
			WORD		crcg :	1;	// b4, crc generate
			WORD		amen :	1;	// b5, address match enable
			WORD		enco :	2;	// b6~7, mode, bit position same to COMSTAT
			//									mode,	NRZ,	NRZI,	manch,	non
			//			COMCST_CMOD1	B7		//		0,		0,		1,		1
			//			COMCST_CMOD0	B6		//		0,		1,		0,		1
			WORD		intr :	1;	// b8, interrupt
			WORD		intrblk : 1;	// b9, interrupt block, read only
			WORD		nulb :	6;	// b10~b15
		} b;
		WORD a;
	} ROADMODE;

	enum enENCMODE	{ ENCM_NRZ = 0,	ENCM_NRZI,	ENCM_MANCH,	ENCM_NON };
	enum enTIMERCH	{ TMR0 = 0, TMR1, TMR_MAX };
	enum enTIMERV	{ SV = 0, PV };

public:
	typedef void (CPump::*PRECVINTR)(BYTE*, WORD, WORD);
	// 181023
	//typedef void (CPump::*PONESINTR)();					// One-shot

private:
	typedef struct _tagROADCONF {
		WORD		wItem;
		int			iCh;
		WORD		wAddr;
		DWORD		dwBps;
		ROADMODE	mode;
		PRECVINTR	pRecvIntr;
		//PONESINTR	pOnesIntr;
		PVOID		pBase;
	} ROADCONF, *PROADCONF;
#define	CLK_BAUDRATE		147456000		// 0x08ca0000	DWORD
#define	MIN_BAUDRATE		9600
#define	MAX_BAUDRATE		4608000

	enum enROADITEM {
		ROADITEM_ADDR =0,
		ROADITEM_BPS,
		ROADITEM_FLAGL,
		ROADITEM_AMATCH,
		ROADITEM_ENCOD,
		ROADITEM_INTR,
		ROADITEM_INTRBLK
	};
#define	ROADITEM_ALL\
	((1 << ROADITEM_ADDR) | (1 << ROADITEM_BPS) | (1 << ROADITEM_AMATCH) | (1 << ROADITEM_ENCOD) |\
	(1 << ROADITEM_FLAGL) | (1 << ROADITEM_INTR))

#define	ROADITEM_MODE	((1 << ROADITEM_AMATCH) | (1 << ROADITEM_ENCOD) | (1 << ROADITEM_INTR) | (1 << ROADITEM_INTRBLK))

#define	MAX_ROADBUF		(SIZE_GENSBUF * 4)
#define	MAX_VHDLCH		12
	typedef struct _tagCHANCONF {
		WORD		wMode;
		bool		bEnIntr;
		PRECVINTR	pRecvIntr;
		//PONESINTR	pOnesIntr;
		PVOID		pBase;
		// 170906
		WORD		wRxSeq;
		WORD		wRxBuf[MAX_ROADBUF];
	} CHANCONF;
	CHANCONF	c_ic[MAX_VHDLCH];
	int			c_iRxChMonit;

#define	MAXFPGA			3
	typedef struct _tagBLOCKCONFIG {
		int			iid;
		int			irq;
		DWORD*		pSpace;
		TRHND		hThread;
		struct sigevent	se;
		CMutex		mtx;
	} BLOCKCONFIG, *PBLOCKCONFIG;
	BLOCKCONFIG		c_bc[MAXFPGA];
#define	LENGTH_FSCMUTEX		MAXFPGA + 1

//#define	MAX_MANCHESTER	4
#define	MAX_PILEBUF		4096
	typedef struct _tagPILE {
		CMutex	mtx;
		WORD	ri;
		WORD	wi;
		char	sBuf[MAX_PILEBUF];
		char	sNull[10];
	} PILE;
	PILE	c_pile;

	static const DWORD	c_dwIntrConst[MAXFPGA][5][2];
	static const int	c_iIntrNo[MAXFPGA];
	static const int	c_iPriority[MAXFPGA];

	typedef PVOID (*PBLOCKHANDLER)(PVOID);
	static const PBLOCKHANDLER	c_pHandlers[MAXFPGA];

	void	PileA(char* pBuf);
	void	Pile0(const char* fmt, ...);
	bool	Clean(int iCh);
	bool	ModeConservationCheck(int iCh);
	bool	SetInterruptBlock(int iBlock);
	bool	GetConfig(ROADCONF& conf);
	bool	SetConfig(ROADCONF conf, bool bModify);
	//void	Elapsed(bool bCh);
	void	ReceiveA(int iCh);
	void	ReceiveB(int iCh);
	void	ReceiveC(int iCh);
	void	BlockHandlerA();
	void	BlockHandlerB();
	void	BlockHandlerC();
	static PVOID	BlockEntryA(PVOID pVoid);
	static PVOID	BlockEntryB(PVOID pVoid);
	static PVOID	BlockEntryC(PVOID pVoid);
	//void	SetTimer(bool bCh, WORD wTime);
	//WORD	GetTimer(bool bCh);

public:
	void	Pile(const char* fmt, ...);
	void	Pull();
	bool	Initial();
	bool	Initial(PVOID pVoid);
	bool	Appear(int iCh, WORD wAddr, DWORD dwBps, bool bMatch, int iFlagLength, PRECVINTR pRecvIntr, CPump* pBase);
	//bool	Appear(int iCh, WORD wAddr, DWORD dwBps, bool bMatch, int iFlagLength, PRECVINTR pRecvIntr, PONESINTR pOnesIntr, CPump* pBase);
	bool	Alter(int iCh, WORD wAddr, bool bMatch);
	bool	Alter(int iCh, WORD wAddr);
	bool	Disappear(int iCh);
	bool	Shoot(int iCh, PVOID p, WORD wLength);
	WORD	GetVersion(int id);
	void	Destroy();
	bool	IsSpaceMap();
	void	SetRxChMonit(int iMonit);
	void	CopyReceiveBuf(int iCh, WORD* pBuf);
	int		Bale(bool bType, BYTE* pMesh);
	WORD	GetRxSeq(int iCh)						{ return c_ic[iCh].wRxSeq; }
	//void	TimerRun(bool bCh, WORD wTime)			{ SetTimer(bCh, wTime); }	// false:ch.0,	true:ch.1
	//void	TimerStop(bool bCh)						{ SetTimer(bCh, 0xffff); }
	void	SetSpace(PVOID pSpace)					{ c_pSpace = pSpace; }
	void	Enable(bool bEnable)					{ c_bEnable = bEnable; }
	void	SetFio(PVOID pVoid)						{ c_pFIO = pVoid; }
	static CFsc*	GetInstance()					{ static CFsc fsc; return &fsc; }

	//PUBLICY_BOWL();
};
