/*
 * CDevice.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef CDEVICE_H_
#define CDEVICE_H_

#include "CPort.h"

class CDevice	: public CPort
{
public:
	CDevice();
	CDevice(PVOID pParent, int mq);
	CDevice(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CDevice();

private:
	PVOID	c_pParent;
	UCURV	c_uCh;
	WORD	c_wAddr;
	UCURV	c_uAttribute;
	UCURV	c_uBps;
	UCURV	c_uIntrLevel;
	UCURV	c_uSendLength;
	UCURV	c_uRecvLength;
	UCURV	c_uCycleTime;
	BYTE*	c_pSend;
	BYTE*	c_pRecv;
	UCURV	c_uRecvState;

public:
	BOOL	Open(UCURV uCh, WORD wAddr, UCURV uMode, UCURV uBps,
					UCURV uIntrLevel, UCURV uSendLength, UCURV uRecvLength,
					UCURV uCycleTime);
	BOOL	Close();
	BOOL	Send(BYTE* pSrc, UCURV uSendLength);
	void	Combine(BYTE* pDest, UCURV uRecvLength);
	void	CalcBcc(BYTE* pDest, UCURV uLength);
	UCURV	CheckBcc(BYTE* pDest, UCURV uLength);
	UCURV	GetState()	{ return c_uRecvState; }
	virtual void	DoReceive(BOOL state);
};

#endif /* CDEVICE_H_ */
