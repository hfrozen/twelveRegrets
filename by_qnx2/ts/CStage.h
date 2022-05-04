/*
 * CStage.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "../Inform2/DevInfo2/HduInfo.h"
#include "Draft.h"
#include "Infer.h"
#include "Mm.h"
#include "../Component2/CPop.h"

#include "../Stage/CUsual.h"
#include "../Stage/CDump.h"

class CStage
{
public:
	CStage();
	virtual ~CStage();

private:
	PVOID	c_pParent;
	//const bool	c_bConvey = false;
	bool	c_bAuxPipe;		// 0:main<->hdu,	1:aux<->hdu
	bool	c_bListing;
	const bool	c_bDownloadType = true;

	BYTE	c_cWakeupSeq;
	WORD	c_wChapter;
	WORD	c_wPage;
	WORD	c_wChapterBkupAtInsp;
	//WORD	c_wChapterBkupAtFire;
	WORD	c_wPageBkup;
	WORD	c_wStead;
	_QUARTET	c_inspTotal;
	DWORD	c_dwDoorTestMap;
	WORD	c_wWatch;
	WORD	c_wMaxWatch;
	bool	c_bSkip4Blank;

	typedef struct _tagPISMANUALREQ {
		bool	bRecog;		// hdu를 통해 pis 제어가 바뀌었음.
		bool	bStatus;
	} PISMANUALREQ;
	PISMANUALREQ	c_pisReq;

	//typedef struct _tagREFLECTION {
	//	bool	bAlter;
	//	bool	bScan;
	//	WORD	wLength;
	//	WORD	wCount;
	//	WORD	wMax;
	//} REFLECTION;
	//REFLECTION	c_reflection;

	// 170930, remove
	//bool	c_bHdb, c_bSva, c_bSvb, c_bIom, c_bLdp, c_bTli, c_bSli, c_bIsp, c_bLcn, c_bEnt;

	typedef struct _tagV3FBCCLR {		// ecmd.b.clrvb를 설정하고 유지하기 위한...
		bool	bClear;
		WORD	wWait;
	} V3FBCCLR;
	V3FBCCLR	c_vbcc;
#define	WAIT_V3FBCCLR			REAL2PERIOD(3000)

	enum {
		LIST_STATE = 0,
		LIST_TROUBLE,
		LIST_SORTTROUBLE,
		LIST_MAX
	};
	WORD	c_wListPage;

	typedef struct _tagLISTPAGE {
		WORD	wID;
		WORD	wCur;
		WORD	wTotal;
	} LISTPAGE, *PLISTPAGE;
	LISTPAGE	c_list[LIST_MAX];

	typedef struct _tagDEVICEINCAR {
		WORD	wDev;
		WORD	wCar;
	} DEVIC;
	DEVIC		c_dicID;

	typedef struct _tagREADSCRAP {
		BYTE	cCmd;
		BYTE	cResp;
		WORD	wHduAddr;
		WORD	wHduLength;
	} READSCRAP;
	READSCRAP	c_rdScrap;
	WORD	c_wBookingPage;
	//WORD	c_wInsertPage;

	typedef struct _tagRESERVEEDIT {
		WORD	wTimer;
		int		nItem;
	} RESERVEEDIT;
	RESERVEEDIT	c_redit;

	enum enFLINDEX {
		FLI_RTD,
		FLI_MM,
		FLI_MAX
	};
	FILELIST	c_fl[FLI_MAX];			// 0:rtd, 1:mm

public:
	enum enDOWNLOADITEM {
		DOWNLOADITEM_NON,
		DOWNLOADITEM_LOGBOOKENTRY,		// 1
		DOWNLOADITEM_INSPECTENTRY,		// 2
		DOWNLOADITEM_TRACEENTRY,		// 3
		DOWNLOADITEM_MAKEDESTINATION,	// 4
		DOWNLOADITEM_GETLOGBOOKLIST,	// 5
		DOWNLOADITEM_NEXTLOGBOOKLIST,	// 6
		DOWNLOADITEM_COPYLOGBOOKTEXT,	// 7
		DOWNLOADITEM_GETLOGBOOKTEXT,	// 8
		DOWNLOADITEM_PUTLOGBOOKTEXT,	// 9
		DOWNLOADITEM_NEXTINSPECTENTRY,	// 10
		DOWNLOADITEM_GETINSPECTTEXT,	// 11
		DOWNLOADITEM_PUTINSPECTTEXT,	// 12
		DOWNLOADITEM_MAX
	};

	typedef struct _tagDOWNLOADINFO {
		int		iEntryIndex;
		int		iFileIndex;
		WORD	wMsg;
		//WORD	wResult;
		WORD	wItem;
		WORD	wItemClone;
		//WORD	wProgress;
		DWORD	dwIndexClone;
		bool	bContinue;
		bool	bCancel;
		bool	bFailur;
		struct {
			uint64_t	cur;
			uint64_t	total;
		} size;
	} DOWNLOADINFO, *PDOWNLOADINFO;
	DOWNLOADINFO	c_dlInfo;

	// 210720
	typedef struct _tagHVACINSPMON {
		bool	bInsp;
		bool	bHorc;
		__BC(	each[CID_MAX],
					blk,	1,	// sdr.blk
					tsr,	1,	// sdr.tsr
					s0,		1,	// sda.s0
					s1,		1,	// sda.s1
					u1,		1,	// sda.u1ok
					u2,		1,	// sda.u2ok
					hok,	1,	// sda.hok
					hng,	1);	// sda.hng
	} HVACINSPMON;
	HVACINSPMON	c_him;

private:
	CUsual	c_usual;
	CDump	c_dump;

	//_SHEAF		c_sheaf;
	CMutex		c_mtx;

	enum enWPAGEINDEX {
		WPG_NULL = 0,
		WPG_HEADBOX,
		WPG_SERVICEA,
		WPG_SERVICEB,
		//WPG_TROUBLES,			// 171207
		WPG_TROUBLELIST,
		WPG_STATUSLIST,
		WPG_IOSTATEDUMP,
		WPG_LINETEXTDUMP,
		WPG_LINESTATEDUMP,
		WPG_OLDTROUBLELIST,
		WPG_LOGBOOKENTRYLIST,
		WPG_INSPECTENTRYLIST,
		WPG_CURRENTINSPECT,
		WPG_READYEDIT,
		WPG_MAX
	};

	WORD	c_wElapsedTime[WPG_MAX][2];

#define	SECTION_MAX	4
	static const BYTE	c_cMatrix[SECTION_MAX][10];

	void	InitialDownloadInfo();
	bool	IsActiveRoad();
	PHDURINFO	GetSendBuf(bool bClr = true);
	PSHEAF	GetSheaf();
	void	ClearSheaf();
	void	ClearEntrySentence();
	void	ClearListSentence();
	void	Carry(WORD* pCmp, WORD wHduAddr, WORD wHduLength);
	void	Convey(BYTE* pCmp, WORD wHduAddr, WORD wHduLength);
	void	ReadPage();
	void	ReadPage(WORD wAddr, WORD wLength);
	void	OpenPage();
	void	MakeBlank();
	void	BaseSketch();
	void	HeadLine();
	void	ServiceA();
	void	ServiceB();
	//void	ReadyListup();
	//void	Reflect();
	void	DumpPioState();
	void	DumpLineText();
	void	DumpLineState();
	void	DumpList(bool bTrouble, bool bAll);
	void	DumpOldTrouble();
	void	DumpEntryList();	//int nEntryID);
	void	DumpSetItem();
	//void	InspectItemClear();
	void	CurrentInspect();
	void	SectionDossier();
	void	ReceiveUserDetail();
	void	DownloadInspect();
	bool	MakeDestinationFromEntry();
	void	DownloadEnd(WORD wReItem, WORD wMsg);
	//int		NextDownloadFromEntry(int iIndex, bool bLog);

public:
	void	Initial(int iFrom);
	void	InitialPage();
	void	InspectReset(WORD wItem);
	void	Monitor();		// 210720
	void	Arteriam();
	void	Pave();
	void	ReceiveUserRequest(BYTE cCmd);
	//void	TurnFireDialog();
	void	ConfirmSize();
	void	SetParent(PVOID pVoid);
	bool	GetDesiredPisMode()					{ return c_pisReq.bStatus; }
	bool	GetPisModeRequest()					{ return c_pisReq.bRecog; }
	void	ClearPisModeRequest()				{ c_pisReq.bRecog = false; }
	DWORD	GetDoorTestMap()					{ return c_dwDoorTestMap; }
	bool	GetV3fBreakerClearCmd()				{ return c_vbcc.bClear; }
	//WORD	GetListupWait()						{ return c_reflection.wMax; }
	//WORD	GetInitiativeMsg()					{ return c_wInitiativeMsg; }
	//POLDTROUBLEB	GetOldTroubleBlock(int iID)	{ return &c_otl.ot.blk[iID]; }
	PDOWNLOADINFO	GetDownloadInfo()			{ return &c_dlInfo; }
	WORD	GetDownloadItem()					{ return c_dlInfo.wItem; }
	void	SetDownloadMsg(WORD wMsg)			{ c_dlInfo.wMsg = wMsg; }
	void	ReserveEdit(WORD wTime, int nItem)	{ c_redit.wTimer = wTime; c_redit.nItem = nItem; }
	void	RegisterPage(WORD wPage)			{ c_wBookingPage = wPage; }
	//void	PageInsert(WORD wPage)				{ c_wInsertPage = wPage; }
	//BYTE*	GetTray()							{ return (BYTE*)&c_rHdu; }

	PUBLICY_CONTAINER();
};
