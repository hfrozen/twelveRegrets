/*
 * CBusCarx.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	CBUSCARX_H_
#define	CBUSCARX_H_

#include <CPort.h>

class CBusCarx	: public CPort
{
public:
	CBusCarx();
	CBusCarx(PVOID pParent, int mq);
	CBusCarx(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CBusCarx();

private:
	PVOID	c_pParent;
	BOOL	c_bLine;
	UCURV	c_uMfch;
	WORD	c_wAddr;
	UCURV	c_uAttribute;
	UCURV	c_uCarLength;
	UCURV	c_uRespondTime;
	WORD	c_wCycleTime;
	DWORD	c_dwOrderID;
	CCDOZ	c_doz;
	uint64_t	c_syncTime;

public:
	int		Open(UCURV uMfch, WORD wAddr, BOOL bLine);
	int		Close();
	int		Send(PVOID pVoid);
	void	Combine(BYTE* pDest);
	void	ClearFrame();
	DWORD	GetOrderID()	{ return c_dwOrderID; }
	void	SetCycleTime(WORD ct)	{ c_wCycleTime = ct; }
	uint64_t	GetSyncTime()		{ return c_syncTime; }
	virtual void	DoSend(int res);
	virtual void	DoReceive(BOOL state);
};

#endif	/* CBUSCARX_H_ */
