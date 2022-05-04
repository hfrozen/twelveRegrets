/**
 * @file	CTdc.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "../Base/CPump.h"
#include "../Part/CPop.h"
#include "../Part/CFio.h"
#include "../Part/CAsync.h"
#include "CGate.h"
#include "../sm/SmRefer.h"
#include "../pi/PiRefer.h"
#include "../up/UpRefer.h"
#include "../dn/DnRefer.h"

#include "TdcRefer.h"

class CTdc:	public CPump
{
public:
	CTdc();
	virtual ~CTdc();

private:
	QHND		c_hStq;
	int			c_hStp;
	PSTPACK		c_pStp;

	// 200212
	//QHND		c_hPtq;
	//int			c_hPtp;
	//PPTPACK		c_pPtp;

	QHND		c_hUtq;
	int			c_hUtp;
	PUTPACK		c_pUtp;

	QHND		c_hDtq;
	int			c_hDtp;
	PDTPACK		c_pDtp;

	bool		c_bServer;		// 200212
	bool		c_bServerPage;

	CMutex		c_mtxps;
	CMutex		c_mtxtw;
	CMutex		c_mtxr;
	CFio		c_fio;
	CGate		c_gate;
	CTimer*		c_pTuner;

#define	MAX_ASYNC		2
	CAsync*		c_pAsync[MAX_ASYNC];

	enum enSTATE {
		STATE_CAPSERV = 0,
		STATE_RECVING,
		STATE_PIDSCANING,
		STATE_UPLOAD,
		STATE_CAPTURENET,
		STATE_UPLOADING,
		STATE_MOUNTEDUSB,
		STATE_DOWNLOADING
	};
	WORD		c_wState;

	enum enDPOS {
		DPOS_RECVMSG = 0,
		DPOS_RECVERR = 6,
		DPOS_RECVSINK,
		DPOS_UDMSG,
		DPOS_PINGMSG = 13,
		DPOS_REPORTMSG = 15,
		DPOS_MAX
	};

	enum enPAGE {
		PAGE_RESET = 1,
		PAGE_RECVNOR,
		PAGE_RECVFERR,
		PAGE_RECVTERR,
		PAGE_PING,
		PAGE_SERVER,		// 200212
		PAGE_REPORTING,
		PAGE_REPORTEND,
		PAGE_UPLOAD,
		PAGE_UPLOADSUCCESS,
		PAGE_UPLOADFAIL,
		PAGE_DOWNLOAD,
		PAGE_DOWNLOADSUCCESS,
		PAGE_DOWNLOADFAIL
	};
	WORD		c_wPage;
	int			c_iTick[2];

#define	MAX_RECVINTERVAL	18		//30		// 30 * 100ms = 3sec
	int			c_wRecvInterval;

	enum enRECVERR {
		RECVERR_ALL,
		RECVERR_UNKNOWNCH,
		RECVERR_TIMEOUT,
		RECVERR_UNMATCHLENGTH,
		RECVERR_CTRLCHAR,
		RECVERR_BCC,
		RECVERR_SINK,
		RECVERR_MAX
	};
	WORD	c_wRecvErr[RECVERR_MAX];
	QWORD	c_qwRecvCnt;

	_ENV	c_env;
	_TODF	c_todf[MAX_ASYNC];
	_RESP	c_resp;

	typedef struct _tagHSCROLL {
		WORD	wInterval;
		int		x;
		int		size;
		int		cpos;
		BYTE	str[256];
		BYTE	text[256];
	} HSCROLL;
	HSCROLL	c_hs;
#define	INTERVAL_HSCROLL		25

	bool		c_bPulse;
	WORD		c_wCycle;
	WORD		c_wWarmTimer;		// by 100ms interval
#define		TIME_WARM		INTERVAL_HSCROLL * 2	//100		// 10sec	//50		// 5sec
	WORD		c_wPauseTimer;		// by 100ms interval
#define		TIME_PAUSE		15		// 1.5sec
	WORD		c_wDnPrintTimer;	// by 20ms interval
#define		TIME_DNPRINT	25		// 20 * 15 = 300ms
	WORD		c_wReportTimer;		// by 20ms interval
#define		TIME_REPORT		25
	WORD		c_wConnectInterval;
#define		INTERVAL_CONNECT	250;

	MSRTIME		c_pulseTime;	// time to execute Pulsate()
	STBAIL		c_stbail;
	//PTBAIL		c_ptbail;
	UTBAIL		c_utbail;
	DTBAIL		c_dtbail;

	SMMSRITEM	c_smMsr;
	//PIMSRITEM	c_piMsr;
	UPMSRITEM	c_upMsr;
	DNMSRITEM	c_dnMsr;

	typedef struct _tagLOCALLAYER {
		int					iCh;
		DWORD				dwBps;
		CAsync::enPARITY	parity;
		WORD				wCycle;
		PSZ					pszName;
	} LOCALLAYER;
	static const LOCALLAYER	c_asyncForm[MAX_ASYNC];
	static PSZ	c_pszBlankInfo;
	static PSZ	c_pszDeviceName;
	static PSZ	c_pszTopInfo;
	static PSZ	c_pszBottomInfo;
	static PSZ	c_pszUploadSuccess;
	static PSZ	c_pszUploadFail;
	static PSZ	c_pszDownloadSuccess;
	static PSZ	c_pszDownloadFail;

	enum enCTRLCHAR {
		STX = 2,
		ETX = 3
	};

	void	Destroy();
	int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int iPriority);
	void	SetChar(int x, BYTE ch);
	void	SetDots(int x, int iLength);
	void	SetText(int x, PSZ pszText);
	void	SetScrollText(int x, int leng, PSZ pszText);
	void	ClrScrollText()				{ c_hs.wInterval = 0; }
	void	PrintV(WORD wPage = 0);
	WORD	CalcBcc(BYTE* p, int length);
	void	RecvError(int iErr);
	//bool	ScanUploadFile();
	void	Respond(int rci, BYTE cCnt);
	void	TimeMonitor(PMSRTIMEDW pDestMtb, PMSRTIMEDW pSrcMtb, PSZ pszTitle);
	void	LoadEnv();
	void	StoreEnv();
	int		FindFile(PSZ pPath, char cSign1, char cSign2 = '\0');
	void	TaskCmd(QHND hQue, WORD wCmd);

public:
	void	Pulsate();
	bool	Launch();

protected:
	void	TakeReceiveAsync(DWORD dwParam);
	PUBLISH_TAGTABLE();

public:
	int		Bale(bool bType, BYTE* pMesh);
	PRIVACY_CONTAINER();
};
