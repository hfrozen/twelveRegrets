/*
 * CPaper.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include <stdlib.h>

#include "Draft.h"
#include "Infer.h"
#include "../Component2/CPop.h"
#include "../Component2/CDtb.h"

#define	GETPAPER(p)	CPaper* p = CPaper::GetInstance()
#define	GETDOZEN(p)\
	do {\
		if (p == NULL) {\
			CPaper* pPaper = CPaper::GetInstance();\
			if (pPaper != NULL)	p = pPaper->GetDozen();\
		}\
		ASSERTP(p);\
	} while (0)

enum enOBLIGETYPE {
	OBLIGE_MAIN = 0,
	OBLIGE_HEAD,
	OBLIGE_ACTIVE,
	OBLIGE_CAB = 7
};

// 180911
#define	VALIDRECIP(id)		(pDoz->recip[id].real.prol.dwOrder != 0)
// 19/12/05
#define	VALIDMRPS()			(GETTI(FID_HEAD, TUDIB_MRPS) && pPaper->GetRouteState(FID_TAIL) && VALIDRECIP(FID_TAIL) && GETTI(FID_TAIL, TUDIB_MRPS))

#define	OBLIGEBF_ONLYPOS	((1 << OBLIGE_MAIN) | (1 << OBLIGE_HEAD))
#define	OBLIGEBF_MASTER		((1 << OBLIGE_MAIN) | (1 << OBLIGE_HEAD))
#define	CAR_FSELF()			(static_cast<enFLOWINDEX>(pPaper->GetFloatID()))
#define	CAR_PSELF()			(static_cast<enFLOWINDEX>(pPaper->GetCarID()))
#define	UNKNOWN_TAILSTICK	0x10

class CPaper
{
private:
	CPaper();
	virtual ~CPaper();

	BYTE	c_uOblige;
	BYTE	c_cFloatID;		// 0:head-main, 1:head-reserve, 2:tail-main, 3:tail-reserve, 4:cc1 ~
	BYTE	c_cLength;		// 10/8/6/4
	BYTE	c_cCarID;		// 0~9
	WORD	c_wPermNo;		// 2XNN
	bool	c_bInLab;
	bool	c_bDevEx;		// 20/02/18, device extension for branch line(24)
	bool	c_bRecipDevEx;	// 20/02/18, device extension command from dtb

	CMutex	c_mtxRed;

public:
	enum enPALLETINDEX {
		PI_CURR,	// 수신 current, 모두 수신 후에 제어, 감시를 하므로 work는 두지 않는다.
		PI_BKUP,	// 통신이 안될 때 de-bounce까지 current를 backup
		//PI_WORK,	// 작업용, 수신 task와 작업 task가 동기가 맞지 않으므로...
		PI_WORKED,
		PI_MAX
	};
//#define	PI_WORK		PI_CURR

private:
	typedef struct _tagPALLETS {
		PDOZEN	pDoz;
		DWORD	dwRoute;
	} PALLETS;
	PALLETS	c_pallets[PI_MAX];
	//DWORD	c_dwCurRoute;

	RECIP	c_shoot;
	WORD	c_wWakeupDtb;
#define	CYCLE_DTBWAKEUP			20		// 2초, 전원 투입, 방향 전환에서 이 시간동안 DTB error 카운터를 하지 않는다.
	WORD	c_wRed[FID_MAX];
	int		c_iDebs[FID_MAX];
	WORD	c_wWakeupEach[FID_MAX];
	// 171019
#define	CYCLE_EACHWAKEUP		20		// 2초,	전원 투입, 방향 전환에서 이 시간동안 Dtb 상태를 false로 하여 각 객차 상태 검사를 하지않게 하여 코장 코드 발생을 막는다.

public:
	enum enROUTEINDEX {
		ROUTE_NON = 0,
		ROUTE_MAIN,		// 1
		ROUTE_AUX,		// 2
		ROUTE_ALL		// 3
	};

	void	Initial();
	void	Setup(BYTE cOblige, BYTE cLength);
	void	SetOblige(BYTE cObID, bool bAdd = true);
	// 170817
	//void	ChangeOblige(BYTE cOblige);
	BYTE	GetHeadCarOffset();
	void	Freshly();
	void	Routing(int iFID, BYTE cRoute);
	void	Repair();
	bool	GetTenor();
	//int		GetTuPID(int iFID);
	bool	GetRouteState(int iFID, enPALLETINDEX pi = PI_CURR);
	void	GetWakeupEach(WORD* pEach);
	int		RecipForm(BYTE* pMesh, BYTE ch, WORD id);
	void	Shoot(PVOID pVoid, WORD wLength);
	void	ClearAllRed();
	bool	GetLabFromRecip();
	int		Bale(bool bType, BYTE* pMesh);
	//void	ClearMark()					{ c_dwMark = 0; }
	bool	IsCab()						{ return (c_uOblige & (1 << OBLIGE_CAB)) ? true : false; }
	bool	IsActive()					{ return (c_uOblige & (1 << OBLIGE_ACTIVE)) ? true : false; }
	bool	IsHead()					{ return (c_uOblige & (1 << OBLIGE_HEAD)) ? true : false; }
	bool	IsMain()					{ return (c_uOblige & (1 << OBLIGE_MAIN)) ? true : false; }
	BYTE	GetFloatID()				{ return c_cFloatID; }
	void	SetFloatID(BYTE cFID)		{ c_cFloatID = cFID; }
	BYTE	GetOblige()					{ return c_uOblige; }
	BYTE	GetLength()					{ return c_cLength; }
	void	SetLength(BYTE cLength)		{ c_cLength = cLength; }
	BYTE	GetCarID()					{ return c_cCarID; }
	void	SetCarID(BYTE cCID)			{ c_cCarID = cCID; }
	WORD	GetPermNo()					{ return c_wPermNo; }
	void	SetPermNo(WORD wPermNo)		{ c_wPermNo = wPermNo; }
	PDOZEN	GetDozen(enPALLETINDEX pi = PI_CURR)	{ return c_pallets[pi].pDoz; }
	DWORD	GetRoute(enPALLETINDEX pi = PI_CURR)	{ return c_pallets[pi].dwRoute; }
	//DWORD	GetCurRoute()				{ return c_dwCurRoute; }
	WORD	GetRed(int id)				{ return c_wRed[id]; }
	bool	GetLaboratory()				{ return c_bInLab; }
	void	SetLaboratory(bool bInLab)	{ c_bInLab = bInLab; }
	bool	GetDeviceEx()				{ return c_bDevEx; }		// to dtb bus
	void	SetDeviceEx(bool bDevEx)	{ c_bDevEx = bDevEx; }		// from jumper or car length
	bool	GetDeviceExFromRecip()		{ return c_bRecipDevEx; }		// to applications
	void	SetDeviceExByRecip(bool bRecipDevEx)	{ c_bRecipDevEx = bRecipDevEx; }	// from dtb bus

	//BYTE*	GetDebs()					{ return &c_cDebs[0]; }
	static CPaper*	GetInstance()		{ static CPaper paper; return &paper; }

	PRIVACY_CONTAINER();
};
