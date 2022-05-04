/*
 * CDevice.cpp
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#include "CDevice.h"

#define	EXTRA_BUFFERLENGTH	4

CDevice::CDevice()
	: CPort()
{
	c_pParent = NULL;
	c_uCh = 0xff;
	c_wAddr = 0;
	c_uAttribute = 0;
	c_uIntrLevel = 0;
	c_uSendLength = 0;
	c_uRecvLength = 0;
	c_uCycleTime = 0;
	c_pSend = NULL;
	c_pRecv = NULL;
}

CDevice::CDevice(PVOID pParent, int mq)
	: CPort(pParent, mq)
{
	c_pParent = pParent;
	c_uCh = 0xff;
	c_wAddr = 0;
	c_uAttribute = 0;
	c_uIntrLevel = 0;
	c_uSendLength = 0;
	c_uRecvLength = 0;
	c_uCycleTime = 0;
	c_pSend = NULL;
	c_pRecv = NULL;
}

CDevice::CDevice(PVOID pParent, int mq, PSZ pszOwner)
	: CPort(pParent, mq, pszOwner)
{
	c_pParent = pParent;
	c_uCh = 0xff;
	c_wAddr = 0;
	c_uAttribute = 0;
	c_uIntrLevel = 0;
	c_uSendLength = 0;
	c_uRecvLength = 0;
	c_uCycleTime = 0;
	c_pSend = NULL;
	c_pRecv = NULL;
}

CDevice::~CDevice()
{
	if (c_pSend) {
		delete [] c_pSend;
		c_pSend = NULL;
	}
	if (c_pRecv) {
		delete [] c_pRecv;
		c_pRecv = NULL;
	}
	if (c_uCh < 0xff)	Close();
}

BOOL CDevice::Open(UCURV uCh, WORD wAddr, UCURV uMode, UCURV uBps,
					UCURV uIntrLevel, UCURV uSendLength, UCURV uRecvLength,
					UCURV uCycleTime)
{
	ASSERT(uCh > MF_MANCHB && uCh < MF_ADCA);
	c_uCh = uCh;
	c_wAddr = wAddr;
	c_uAttribute = PORTATTR_DEVICE;
	c_uBps = uBps;
	ASSERT(uSendLength);
	c_uSendLength = uSendLength;
	ASSERT(uRecvLength);
	c_uRecvLength = uRecvLength;
	c_uCycleTime = uCycleTime;

	PORTSTATE state;
	memset(&state, 0, sizeof(PORTSTATE));
	state.uCh = c_uCh;
	state.wAddr = c_wAddr;
	state.uMode = uMode;
	state.uAttribute = c_uAttribute;
	state.uBps = c_uBps;
	state.uRecvLength = c_uRecvLength;
	state.uRespondTime = 0;
	state.uWatchTime = c_uCycleTime;
	int res;
	if ((res = Begin(&state)) == PORTERROR_NON) {
		SetAutoTrigOut(TRUE);
		if (c_pSend) {
			delete [] c_pSend;
			c_pSend = NULL;
		}
		if (c_pRecv) {
			delete [] c_pRecv;
			c_pRecv = NULL;
		}
		c_pSend = new BYTE[c_uSendLength + EXTRA_BUFFERLENGTH];
		memset((PVOID)c_pSend, 0, c_uSendLength + EXTRA_BUFFERLENGTH);
		c_pRecv = new BYTE[c_uRecvLength + EXTRA_BUFFERLENGTH];
		memset((PVOID)c_pRecv, 0, c_uRecvLength + EXTRA_BUFFERLENGTH);
		return TRUE;
	}
	else {
		// Trace error
		c_uCh = 0xff;
		return FALSE;
	}
}

BOOL CDevice::Close()
{
	int res = End();
	c_uAttribute =
	c_uIntrLevel =
	c_uSendLength =
	c_uRecvLength =
	c_uCycleTime = 0;
	if (c_pSend) {
		delete [] c_pSend;
		c_pSend = NULL;
	}
	if (c_pRecv) {
		delete [] c_pRecv;
		c_pRecv = NULL;
	}
	if (res == PORTERROR_NON)	return TRUE;
	// Trace error
	return FALSE;
}

BOOL CDevice::Send(BYTE* pSrc, UCURV uSendLength)
{
	memset((PVOID)c_pSend, 0, c_uSendLength + EXTRA_BUFFERLENGTH);
	memset((PVOID)c_pRecv, 0, c_uRecvLength + EXTRA_BUFFERLENGTH);
	memcpy((PVOID)c_pSend, (PVOID)pSrc, uSendLength);
	//if (c_uCh >= MF_HDLCE)	MSGLOG("[DEVICE]ERROR:Wrong ch.%d:%02X\n", c_uCh, *(pSrc + 1));
	int res;
	if ((res = Writep(c_pSend, uSendLength)) == PORTERROR_NON)
		return TRUE;
	// Trace error
	return FALSE;
}

void CDevice::Combine(BYTE* pDest, UCURV uRecvLength)
{
	memcpy((PVOID)pDest, (PVOID)c_pRecv, uRecvLength);
}

void CDevice::CalcBcc(BYTE* pDest, UCURV uLength)
{
	BYTE bcc1, bcc2;
	bcc1 = bcc2 = 0;
	*pDest ++ = DEVFLOW_STX;
	for (UCURV n = 0; n < uLength; n ++) {
		if (n & 1)	bcc2 ^= *pDest;
		else	bcc1 ^= *pDest;
		++ pDest;
	}
	*pDest ++ = DEVFLOW_ETX;
	if (uLength & 1) {
		*pDest ++ = bcc1 ^ DEVFLOW_ETX;
		*pDest = bcc2;
	}
	else {
		*pDest ++ = bcc1;
		*pDest = bcc2 ^ DEVFLOW_ETX;
	}
}

UCURV CDevice::CheckBcc(BYTE* pDest, UCURV uLength)
{
	if (uLength < 7)	return 1;	// minumal size inlude stx, addr, flow, etx, bcc1, bcc2

	BYTE bcc1, bcc2;
	bcc1 = bcc2 = 0;
	++ pDest;	// skip stx
	for (UCURV n = 0; n < (uLength - 4); n ++) {
		if (n & 1)	bcc2 ^= *pDest;
		else	bcc1 ^= *pDest;
		++ pDest;
	}
	if (*pDest != DEVFLOW_ETX)	return 2;
	if (uLength & 1)	bcc1 ^= DEVFLOW_ETX;
	else	bcc2 ^= DEVFLOW_ETX;
	++ pDest;		// skip etx
	if (*pDest != bcc1)	return 3;
	++ pDest;		// skip bcc1
	if (*pDest != bcc2)	return 4;
	return 0;
}

void CDevice::DoReceive(BOOL state)
{
	DRECV dr;
	dr.info.nCh = (BYTE)c_uCh;
	if (state) {
		c_uRecvLength = c_recv.wLength;
		c_uRecvState = (UCURV)c_recv.nState;
		dr.info.wLength = (WORD)(c_uRecvLength | 0x8000);
		dr.info.nState = (BYTE)(c_uRecvState & 0xff);
		memcpy((PVOID)c_pRecv, (PVOID)c_recv.pBuf, c_uRecvLength);
		if (dr.info.nState != 0)	MSGLOG("[DEVICE]ERROR:ch(%d) code(%d) leng(%d)\r\n",
				dr.info.nCh, dr.info.nState, dr.info.wLength);
	}
	else {
		dr.info.nState = 0;
		dr.info.wLength = 0;
		//MSGLOG("[DEVICE]ERROR:ch(%d) timeout\r\n", dr.info.nCh);
	}
	SendMail(MA_RECEIVEDEVICE, c_pParent, dr.dw);
}
