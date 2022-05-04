/*
 * CWicket.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "DuoTypes.h"
#include "Mc.h"
#include "CPump.h"

class CWicket :	public CPump
{
public:
	CWicket();
	virtual ~CWicket();

private:
	QHND		c_hMcq;
	bool		c_bOpen;
	bool		c_bIdle;
	uint64_t	c_runIdle;

	MCMSG		c_mcm;

	static const PSZ	c_szInit;
	static const PSZ	c_szNorm;
	static const PSZ	c_szCont;		// continuity

	void	Send(WORD wLength);

public:
	bool	Open();
	void	Close();
	void	SendContinue();
	void	Send(bool bType = false);
	void	InitMeast();
	bool	GetState()					{ return c_bOpen; }
	//void	MeasTime(int u);

protected:
	void	TakeLinkMc(DWORD dw);
	void	TakeUnlinkMc(DWORD dw);
	void	TakeDebugMc(DWORD dw);
	PUBLISH_TAGTABLE();
};
