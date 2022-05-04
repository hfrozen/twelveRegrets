/*
 * CDtb.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include <string.h>
#include <stdio.h>

#include "Draft.h"
#include "Infer.h"
#include "CPop.h"
#include "CPort.h"

//#define	_DIRECT_CALL_

class CDtb :	public CPort
{
public:
	CDtb(PVOID pParent, QHND hReservoir);
	CDtb(PVOID pParent, QHND hReservoir, PSZ pszOwnerName);
	virtual ~CDtb();

	virtual void	DoSend();
	virtual void	DoReceive(bool bReal);

private:
	bool	c_bLine;
	BYTE	c_cProperID;
	BYTE	c_cID;
	BYTE	c_cCarLength;		// car length
	DWORD	c_dwOrder;
	DOZEN	c_doz;

	WORD	c_wSendWatch;
	WORD	c_wErrBitmap;

public:
	void	Arteriam();
	bool	OpenBus(bool bLine, int iProperID, int iID, int iCarLength, WORD wCycleTime = 0);
	bool	ModifyBus(int iID);
	void	CloseBus();
	bool	SendBus(PVOID pVoid, DWORD dwOrder, bool bSend = true);		// 190110
	// 190813
	void	SetCarLength(BYTE cCarLength)	{ c_cCarLength = cCarLength; }
	void	ClearAllDozen()					{ memset(&c_doz, 0, sizeof(DOZEN)); }
	void	Bunch(BYTE* pDest)				{ memcpy(pDest, &c_doz, sizeof(DOZEN)); }
	void	Demount(BYTE* pDest, int iID)	{ memcpy(pDest, &c_doz.recip[iID], SIZE_RECIP); }
	DWORD	GetRecvOrder()					{ return c_dwOrder; }

	PUBLICY_CONTAINER();
};
