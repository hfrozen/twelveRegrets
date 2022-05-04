/*
 * CDev.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "../Inform2/DevInfo2/DevInfo.h"
#include "CPort.h"

class CDev :	public CPort
{
public:
	CDev();
	CDev(PVOID pParent, QHND hQue);
	CDev(PVOID pParent, QHND hQue, PSZ pszOwnerName);
	virtual ~CDev();

public:
	virtual void	DoSend();
	virtual void	DoReceive(bool bState);

	void	GetPrefix(PLOCALPREFIX pPrefix);
	void	Bunch(BYTE* pDest, WORD wLength);
	WORD	GetHead()						{ return c_mag.buf.f.wAddr; }
};
