/**
 * @file	CGate.cpp
 * @brief
 * @details
 * @author
 * @date
 */
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/neutrino.h>
#include "../Base/Track.h"
#include "CTdc.h"
#include "CGate.h"

const PSZ	CGate::c_szInit = (PSZ)"INIT>";
const PSZ	CGate::c_szNorm = (PSZ)"NORL>";
const PSZ	CGate::c_szCont = (PSZ)"CONT>";

CGate::CGate()
	: CPump("WICK")
{
	c_hWmq = INVALID_HANDLE;
	c_bOpen = false;
	c_bIdle = false;
	c_runIdle = 0;
}

CGate::~CGate()
{
	Close();
}

bool CGate::Open()
{
	c_hWmq = mq_open((char*)PULLQ_STR, O_WRONLY);
	if (Validity(c_hWmq)) {
		memset(&c_wmm, 0, sizeof(WMMSG));
		c_wmm.dwLength = (DWORD)this;
		c_wmm.wID = WMMSG_CONTS;
		mq_send(c_hWmq, (char*)&c_wmm, sizeof(WMMSG), 0);		// PRIORITY_POST
		return true;
	}
	TRACK("%s>MON>ERR:can not message queue to socket!(%s)\n", c_szOwnerName, strerror(errno));
	return false;
}

void CGate::Close()
{
	if (Validity(c_hWmq))	mq_close(c_hWmq);
	c_bOpen = false;
}

void CGate::SendContinue()
{
	if (!Validity(c_hWmq))	return;

	memset(&c_wmm, 0, sizeof(WMMSG));
	strncpy((char*)&c_wmm.cBuf[6], c_szCont, strlen(c_szCont));
	Send((WORD)(strlen(c_szCont) + 6));
}

void CGate::Send(bool bType)
{
	if (!c_bOpen || !Validity(c_hWmq))	return;

	memset(&c_wmm, 0, sizeof(WMMSG));
	BYTE* p = &c_wmm.cBuf[6];
	WORD sum = 6;
	if (bType) {
		strncpy((char*)p, c_szInit, strlen(c_szInit));
		p += strlen(c_szInit);
		sum += strlen(c_szInit);

		sprintf((char*)p, "$VAR,512,(");
		while (*p != '\0') {	++ p;	++ sum;	}

		int leng;
		CTdc* pTdc = (CTdc*)c_pParent;
		leng = pTdc->Bale(true, p);
		p += leng;
		sum += leng;
		*p ++ = ')';
		*p ++ = ',';
		sum += 2;

		*p = '\0';
		++ sum;
	}
	else {
		if (c_bIdle) {
			uint64_t clk = ClockCycles();
			double sec = (double)(clk - c_runIdle) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
			if (sec >= 1.0f)	c_bIdle = false;
			else	return;
		}

		strncpy((char*)p, c_szNorm, strlen(c_szNorm));
		p += strlen(c_szNorm);
		sum += strlen(c_szNorm);

		int leng;
		CTdc* pTdc = (CTdc*)c_pParent;
		leng = pTdc->Bale(false, p);
		p += leng;
		sum += leng;

		sum = 512;
	}
	Send((WORD)(sum + 6));
}

void CGate::Send(WORD wLength)
{
	BYTE buf[16];
	sprintf((char*)buf, "%06d", wLength);
	memcpy(c_wmm.cBuf, buf, 6);

	c_wmm.dwLength = wLength;
	c_wmm.wID = WMMSG_SEND;
	mq_send(c_hWmq, (char*)&c_wmm, sizeof(WMMSG), 0);
}

void CGate::TakeLinkWm(DWORD dw)
{
	bool bCmd = c_bOpen ? false : true;
	c_bOpen = true;
	Send(bCmd);
	c_runIdle = ClockCycles();
	c_bIdle = true;
}

void CGate::TakeUnlinkWm(DWORD dw)
{
	c_bOpen = false;
}

void CGate::TakeDebugWm(DWORD dw)
{
	if (c_bOpen) {
	}
}

BEGIN_TAGTABLE(CGate, CPump)
	_TAGMAP(TAG_LINKWM, CGate::TakeLinkWm)
	_TAGMAP(TAG_UNLINKWM, CGate::TakeUnlinkWm)
	_TAGMAP(TAG_DEBUGWM, CGate::TakeDebugWm)
END_TAGTABLE();
