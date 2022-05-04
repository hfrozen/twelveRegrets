/*
 * CDuDoc.h
 *
 *  Created on: 2011. 1. 30
 *      Author: Che
 */

#ifndef CDUDOC_H_
#define CDUDOC_H_

#include <Draft.h>

class CDuDoc
{
public:
	CDuDoc();
	virtual ~CDuDoc();

protected:
	PTCREFERENCE	c_pTcRef;
	PLCFIRM		c_pLcFirm;
	PTCDOZ		c_pDoz;
	PDUCINFO	c_pDucInfo;

private:
	PVOID	c_pParent;
	WORD	c_wReset;
	WORD	c_wMsg;
	DWORD	c_dwTbh;
	DWORD	c_dwTbt;
#define	MAX_DEV_LENGTH	10
	WORD	c_wDevTraceLength[TRACEID_MAX][MAX_DEV_LENGTH];
	WORD	c_wDevTraceProgress[TRACEID_MAX][MAX_DEV_LENGTH];

#define	SIZE_OSMSG		80
	BYTE	c_nOsMsg[SIZE_OSMSG];
	BYTE	c_nHeadBk[SIZE_HEADBK];
	BYTE	c_nNormalBk[SIZE_NORMALBK];
	static const WORD	c_wPattern[3][10];
	static const BYTE	c_nHeadPile[];
	static const BYTE	c_nTailPile[];

	BYTE	WrenchByte(BYTE n);
	DWORD	XchAtZero(DWORD dw);
	BOOL	IsValidTc(UCURV ti);
	BOOL	IsValidCc(UCURV ci);
	void	TuningCode(WORD code, WORD cid, PTMFAULTINFO pFi);
	BYTE	GetTcInfo();
	//BYTE	GetTcFrontMap();
	//BYTE	GetV3fMap(BYTE id);
	BYTE	GetCmsbMap(BYTE id);
	BYTE	GetAuxMap(BYTE id);
public:
	BYTE	GetDoorBypass(UCURV id);
private:
	WORD	GetDoorFault(UCURV id);
	BYTE	CalcSum(WORD size, BOOL type = TRUE);
	BYTE	CalcSum(BYTE* p, BYTE size);
	BYTE	GetRealCid(BYTE cid, BOOL all);
	void	Operate();
	void	TcBusDraw(BYTE state, BYTE flag, WORD node, WORD bus);
	void	DevMonit();
	void	StatusA();
	void	StatusB();
	void	CommMonit();
	void	DirectoryByDay();
	void	OpenPage(WORD addr);
	void	OpenNormalPage();
	void	OpenOverhaulPage();
	void	OpenInspectPage();
	void	OpenRescuePage();
	void	OpenTimeSetPage();
	void	Inspect();
	void	Enviro();
	void	AlarmList();
	void	TraceView();
	void	ComMonit();
	void	HeadPile();
	void	TailPile();
	void	GetEnviro();
	void	GetWheel();
	void	GetInspectItem();
	void	GetTraceItem();
	void	Setting(WORD wStart, WORD wLength);
	void	Conditional(DWORD* pDw, WORD head, WORD tail, BYTE id);
	BOOL	DevToTrace(UCURV di, WORD* pwTrace);
	BOOL	DirectorySelected(BOOL bCmd);
	// Modified 2013/11/02
	//void	SetEntireInfo(BYTE* p);
	void	SetEntireInfo(BYTE* p, BOOL bEnv);
	void	SetTrainNo(BYTE* p);
	void	SetWheelInfo(BYTE* p);
	void	Exchange(WORD* pw1, WORD* pw2)
	{
		WORD w = *pw1;
		*pw1 = *pw2;
		*pw2 = w;
	}

public:
	void	InitialModule(PVOID pVoid);
	void	Format(UCURV page);
	// Appended 2013/11/02
	void	AlarmScroll();
	PVOID	GetSendInfo();
	WORD	GetSendLength(UCURV page);
	WORD	GetRecvLength(UCURV page);
	void	SetTraceLength(UCURV di, UCURV ci, WORD length);
	void	SetTraceProgress(UCURV di, UCURV ci, WORD progress);
	void	AppendHead();
	void	ExitFromDownload();
	void	OsMsg(BYTE* p);
	WORD	GetTraceLength(UCURV di, UCURV ci)	{ return c_wDevTraceLength[di][ci]; }
	void	KeyAction(BYTE* p);
	void	Hangup(WORD msg)		{ c_wMsg = msg; }
	WORD	Hangdown()				{ return c_wMsg; }
	void	ResetMotion(WORD n)		{ c_wReset = n; }
	WORD	GetResetMotion()		{ return c_wReset; }
};

#endif /* CDUDOC_H_ */
