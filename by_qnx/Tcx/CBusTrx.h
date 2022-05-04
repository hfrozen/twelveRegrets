/*
 * CBusTrx.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	CBUSTRX_H_
#define	CBUSTRX_H_

#include <CPort.h>

#define	RECEIVE_TIMEOUT		2
#define	RECEIVE_FRAMEZERO	1

class CBusTrx	: public CPort
{
public:
	CBusTrx();
	CBusTrx(PVOID pParent, int mq);
	CBusTrx(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CBusTrx();

private:
	DWORD	c_dwOrderID;
	PVOID	c_pParent;
	BOOL	c_bLine;
	BYTE	c_nDuty;
	UCURV	c_uMfch;
	WORD	c_wAddr;
	UCURV	c_uAttribute;
	UCURV	c_uCarLength;
	UCURV	c_uRespondTime;
	UCURV	c_uWatchTime;
	WORD	c_wCycleTime;
	uint64_t	c_syncTime;
	TCDOZ	c_doz;
	TCPERT	c_pert;

public:
	int		Open(UCURV uMfch, BYTE nDuty, WORD wAddr, UCURV uCarLength, BOOL bLine);
	int		Close();
	int		Send(PVOID pVoid, DWORD dwOrderID = 0);
	int		Dispatch(WORD wAddr, PVOID pVoid);
	void	CombineZero(BYTE* pDest);
	void	Combine(BYTE* pDest);
	void	ClearFirm();
	void	ClearFirmWithoutZero();
	//void	SetCps(float cps)	{ c_cps = cps; }
	DWORD	GetOrderID()	{ return c_dwOrderID; }
	PTCPERT	GetPert()		{ return &c_pert; }
	void	SetCycleTime(WORD ct)	{ c_wCycleTime = ct; }
	uint64_t	GetSyncTime()		{ return c_syncTime; }
	virtual void	DoSend(int res);
	virtual void	DoReceive(BOOL state);
};

#endif	/* CBUSTRX_H_ */
