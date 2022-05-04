/**
 * @file	CGate.h
 * @brief
 * @details
 * @author
 * @date
 */
#pragma once
#include "../Base/CPump.h"
#include "../wm/WmRefer.h"

class CGate :	public CPump
{
public:
	CGate();
	virtual ~CGate();

private:
	QHND		c_hWmq;
	bool		c_bOpen;
	bool		c_bIdle;
	uint64_t	c_runIdle;

	WMMSG		c_wmm;

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

protected:
	void	TakeLinkWm(DWORD dw);
	void	TakeUnlinkWm(DWORD dw);
	void	TakeDebugWm(DWORD dw);
	PUBLISH_TAGTABLE();
};
