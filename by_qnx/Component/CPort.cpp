/*
 * CPort.cpp
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#include <string.h>
#include <pthread.h>

#include "CPort.h"
#include "PortMap.h"

void CPort::TimeOutHandler(PVOID pVoid)
{
	CPort* pPort = (CPort*)pVoid;
	pPort->TimeOutter();
}

void CPort::TimeOutter()
{
	if (c_pTimeOut != NULL)
		c_pTimeOut->Release();
	if (c_bDirection) {
		c_bDirection = FALSE;
		int res = 0;
		DoSend(res);
	}
	else	DoReceive(FALSE);
}

void CPort::PortHandler(BYTE* pRecv, WORD wLength, BYTE nState, PVOID pVoid)
{
	CPort* pPort = (CPort*)pVoid;
	pPort->Porter(pRecv, wLength, nState);
}

void CPort::Porter(BYTE* pRecv, WORD wLength, BYTE nState)
{
	if (c_pTimeOut != NULL && c_bAutoTrigOut)
		c_pTimeOut->Release();
	c_recv.pBuf = pRecv;
	c_recv.nState = nState;
	c_recv.wLength = wLength;
	DoReceive(TRUE);
	if (c_portState.uRespondTime > 0 && c_pTimeOut != NULL &&
		wLength == sizeof(TCFIRM) && *pRecv == BROADCAST_ADDR) {
		c_bDirection = TRUE;
		c_pTimeOut->Trigger((int)c_portState.uRespondTime);
	}
}

CPort::CPort()
	: CAnt()
{
	c_pParent = NULL;
	memset(&c_portState, 0, sizeof(PORTSTATE));
	c_portState.uCh = 0xff;
	c_pTimeOut = NULL;
	c_bDirection = FALSE;
	c_bAutoTrigOut = FALSE;
	c_prio = 10;
	memset((PVOID)c_pszOwner, 0, SIZE_TITLEBUF);
}

CPort::CPort(PVOID pParent, int mq)
	: CAnt(pParent, mq)
{
	c_pParent = pParent;
	memset(&c_portState, 0, sizeof(PORTSTATE));
	c_portState.uCh = 0xff;
	c_pTimeOut = NULL;
	c_bDirection = FALSE;
	c_bAutoTrigOut = FALSE;
	c_prio = 10;
	memset((PVOID)c_pszOwner, 0, SIZE_TITLEBUF);
}

CPort::CPort(PVOID pParent, int mq, PSZ pszOwner)
	: CAnt(pParent, mq, pszOwner)
{
	c_pParent = pParent;
	memset(&c_portState, 0, sizeof(PORTSTATE));
	c_portState.uCh = 0xff;
	c_pTimeOut = NULL;
	c_bDirection = FALSE;
	c_bAutoTrigOut = FALSE;
	c_prio = 10;
	if (strlen(pszOwner) >= SIZE_TITLEBUF)
		*(pszOwner + SIZE_TITLEBUF) = NULL;
	strcpy((char*)c_pszOwner, (char*)pszOwner);
}

CPort::~CPort()
{
	Erasure((BYTE)c_portState.uCh);
	End();
	if (c_pTimeOut != NULL) {
		c_pTimeOut->Release();
		delete c_pTimeOut;
		c_pTimeOut = NULL;
	}
}

int CPort::Begin(PPORTSTATE pState)
{
	if (c_portState.uCh < MFPORT_MAX) {
		if (c_pTimeOut != NULL) {
			c_pTimeOut->Release();
			delete c_pTimeOut;
			c_pTimeOut = NULL;
		}
		End();
	}

	MFSTATUS status;
	memset(&status, 0, sizeof(MFSTATUS));
	memcpy(&c_portState, pState, sizeof(PORTSTATE));
	if (c_portState.uCh < MFPORT_MAX) {		// comm.
		if (c_portState.uCh < MF_HDLCA) {	// manchester
		}
		else {								// sync or async
			if (c_portState.uMode > MFMODE_ASYNC) {
				MSGLOG("[PORT]ERROR:Unmatch mode.\r\n");
				return PORTERROR_MODE;
			}
			if (c_portState.uBps >= MFBPS_MAX) {
				MSGLOG("[PORT]ERROR:Unknown speed.\r\n");
				return PORTERROR_BPS;
			}
			if (c_portState.uMode == MFMODE_ASYNC &&
				(c_portState.uRecvLength == 0 ||
				c_portState.uRecvLength > MAX_TRANS_BUFF_SIZE)) {
				MSGLOG("[PORT]ERROR:Unreserved rx. length.\r\n");
				return PORTERROR_RECVLENGTH;
			}
		}
		if (c_portState.uCh < MF_HDLCA) {
			if (c_portState.uAttribute == PORTATTR_CC ||
				c_portState.uAttribute == PORTATTR_DEVICE) {
				status.manch.addr = (BYTE)(c_portState.wAddr & 0xff);
				status.manch.addr_enable = MF_ENABLE;
			}
			else	status.manch.addr_enable = MF_DISABLE;
			status.manch.prem_size = 1;
			status.manch.fcs_type = MF_CRC16;
			status.manch.fcs_enable = MF_ENABLE;
		}
		else {
			status.hdlc.addr = (BYTE)(c_portState.wAddr & 0xff);
			status.hdlc.mode = (BYTE)c_portState.uMode;
			status.hdlc.bps = (BYTE)c_portState.uBps;
			status.hdlc.receive_count = (WORD)c_portState.uRecvLength;
			if (c_portState.uMode == MFMODE_ASYNC) {
				status.hdlc.fcs_enable = MF_DISABLE;
				status.hdlc.sync_fsize = MF_STOPBIT1;
			}
			else {
				status.hdlc.modulation = MF_NRZI;
				status.hdlc.fcs_enable = MF_ENABLE;
				status.hdlc.fcs_type = MF_CRC16;
				status.hdlc.sync_fsize = 1;
				status.hdlc.addr_enable = MF_ENABLE;
			}
		}
		int res;
		if ((res = mfOpen(c_portState.uCh, &status)) != MFB_SUCCESS) {
			PrintMfError(res, (PSZ)"[PORT]ERROR:Can not open port");
			c_portState.uCh = 0xff;
			return res;
		}
		if (Register((BYTE)c_portState.uCh, PortHandler, this) != 0) {
			MSGLOG("[PORT]ERROR:Registry %d.\r\n", c_portState.uCh);
			c_portState.uCh = 0xff;
			return PORTERROR_CANNOTREG;
		}
		if (c_portState.uRespondTime > 0 || c_portState.uWatchTime > 0) {
			c_pTimeOut = new CTimer(this);
			c_pTimeOut->SetOwner(c_pParent);
			c_pTimeOut->SetOwnerName((PSZ)c_pszOwner);
			//int prio = getprio(0);
			c_pTimeOut->Initial(PRIORITY_TRIGGER, TimeOutHandler, c_portState.uCh);
			//c_pTimeOut->Initial(c_prio, c_prio, TimeOutHandler, c_portState.uCh);
		}
		return PORTERROR_NON;
	}

	MSGLOG("[PORT]ERROR:Unknown channel.\r\n");
	return PORTERROR_CHANNEL;
}

int CPort::End()
{
	if (c_portState.uCh < MFPORT_MAX) {
		if (c_pTimeOut != NULL) {
			c_pTimeOut->Release();
			delete c_pTimeOut;
			c_pTimeOut = NULL;
		}
		if (Erasure(c_portState.uCh) != 0)
			MSGLOG("[PORT]ERROR:Can not remove vector in ch.%d.\r\n", c_portState.uCh);
		return mfClose(c_portState.uCh);
	}
	return PORTERROR_NON;
}
int CPort::GetStatus(PMFSTATUS pStatus)
{
	if (c_portState.uCh < MFPORT_MAX)
		return ::mfGetStatus(c_portState.uCh, pStatus);
	return PORTERROR_CHANNEL;
}

int CPort::SetStatus(PMFSTATUS pStatus)
{
	if (c_portState.uCh < MFPORT_MAX)
		return mfSetStatus(c_portState.uCh, pStatus);
	return PORTERROR_CHANNEL;
}

int CPort::EnableIntr(BOOL intr)
{
	if (c_portState.uCh < MFPORT_MAX) {
		return mfEnableChannel(c_portState.uCh, intr);
	}
	return PORTERROR_CHANNEL;
	//return 0;
}

int CPort::Writep(BYTE* pBuf, int iWriteLength)
{
	if (c_portState.uCh < MFPORT_MAX) {
		int res = PORTERROR_NON;
		if (pBuf != NULL) {
			res = mfWrite(c_portState.uCh, (char*)pBuf, &iWriteLength);
			if (res != MFB_SUCCESS)
				PrintMfError(res, (PSZ)"[PORT]ERROR:Can not write port");
		}
		if (c_portState.uWatchTime > 0 && c_pTimeOut != NULL) {
			c_bDirection = FALSE;
			res = c_pTimeOut->Trigger((int)c_portState.uWatchTime);
		}
		return res;
	}
	return PORTERROR_CHANNEL;
}

int CPort::WriteWithoutTrigger(BYTE* pBuf, int iWriteLength)
{
	if (c_portState.uCh < MFPORT_MAX) {
		int res = PORTERROR_NON;
		if (pBuf != NULL) {
			res = mfWrite(c_portState.uCh, (char*)pBuf, &iWriteLength);
			if (res != MFB_SUCCESS)
				PrintMfError(res, (PSZ)"[PORT]ERROR:Can not write port");
		}
		return res;
	}
	return PORTERROR_CHANNEL;
}

BOOL CPort::SetTrig()
{
	if (c_portState.uWatchTime > 0 && c_pTimeOut != NULL) {
		c_bDirection = FALSE;
		c_pTimeOut->Trigger((int)c_portState.uWatchTime);
		return TRUE;
	}
	return FALSE;
}

void CPort::KillTrig()
{
	if (c_pTimeOut != NULL)
		c_pTimeOut->Release();
}

void CPort::SetOwnerName(PSZ pszOwner)
{
	if (strlen(pszOwner) >= SIZE_TITLEBUF)
		*(pszOwner + SIZE_TITLEBUF) = NULL;
	strcpy((char*)c_pszOwner, (char*)pszOwner);
}

int CPort::GetTimerConnection()
{
	if (c_pTimeOut != NULL)
		return c_pTimeOut->GetConnection();
	return -2;
}

void CPort::PrintMfError(int res, PSZ msg)
{
	if (res < MFB_SUCCESS)
		MSGLOG("%s%d(%s).\r\n", msg, c_portState.uCh, strerror(errno));
	else	MSGLOG("%s%d(%d).\r\n", msg, c_portState.uCh, res);
}

void CPort::DoSend(int res)
{
}

void CPort::DoReceive(BOOL respond)
{
}
