/*
 * CTerm.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/iomsg.h>
//#include <sys/select.h>

#include "Track.h"
#include "Arrange.h"
#include "CTerm.h"

#define	TIME_TERMGAP	2		// 2ms

CTerm::CTerm()
	: CPump()
{
	c_pWatcher = NULL;
	c_pGappy = NULL;
	c_hThread = INVALID_HANDLE;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.domi.fd = INVALID_HANDLE;
	c_fab.iCh = 0;
	memset(&c_mag, 0, sizeof(MAGAZINE));
}

CTerm::CTerm(PVOID pParent, QHND hQue, PSZ pszOwnerName)
	: CPump(pParent, hQue, pszOwnerName)
{
	c_pWatcher = NULL;
	c_pGappy = NULL;
	c_hThread = INVALID_HANDLE;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.domi.fd = INVALID_HANDLE;
	c_fab.iCh = 0;
	memset(&c_mag, 0, sizeof(MAGAZINE));
}

CTerm::~CTerm()
{
	KILLR(c_pGappy);
	KILLR(c_pWatcher);
	Close();
	if (Validity(c_hThread)) {
		if (pthread_cancel(c_hThread) == EOK) {
			PVOID pResult;
			pthread_join(c_hThread, &pResult);
		}
		c_hThread = INVALID_HANDLE;
	}
}

void CTerm::Elapsed()
{
	if (c_pWatcher != NULL && c_pWatcher->GetRhythm())	c_pWatcher->Release();
	if (c_pGappy != NULL && c_pGappy->GetRhythm())	c_pGappy->Release();
	c_mag.wi = 0;
	SendTag(TAG_RECEIVETERM, c_pParent, TAGPAR(0x8000, 0, c_fab.iCh));
}

void CTerm::Snatched()
{
	if (c_pWatcher != NULL && c_pWatcher->GetRhythm())	c_pWatcher->Release();
	if (c_pGappy != NULL && c_pGappy->GetRhythm())	c_pGappy->Release();
	if (c_mag.wi > 0) {
		c_mag.wLength = c_mag.wi;
		c_mag.wi = 0;
		SendTag(TAG_RECEIVETERM, c_pParent, TAGPAR(c_mag.wLength, 0, c_fab.iCh));
	}
}

PVOID CTerm::ReceiveEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CTerm* pTerm = (CTerm*)pVoid;
	if (pTerm != NULL)	pTerm->Receive();
	return NULL;
}

void CTerm::Receive()
{
//#if	defined(DEBUG_HANDLER)
//	TRACK("%s>TERM:receive handler start.(%d)\n", c_szOwnerName, c_fab.domi.fd);
//#endif
//
	fd_set fs;
	FD_ZERO(&fs);
	FD_SET(c_fab.domi.fd, &fs);

	while (Validity(c_fab.domi.fd)) {
		int res;
		switch (res = select(c_fab.domi.fd + 1, &fs, NULL, NULL, NULL)) {	//&tv)) {		//NULL)) {
		case EINTR :
			TRACK("%s>TERM:select() catch EINTR\n", c_szOwnerName);
			pthread_exit(0);
			TRACK("%s>TERM:handler exit A.\n", c_szOwnerName);
			return;
			break;
		case -1 :
			TRACK("%s>TERM>ERR:select()!(%s)\n", c_szOwnerName, strerror(errno));
			break;
		case 0 :
			TRACK("%s>TERM>ERR:select timeout!\n", c_szOwnerName);
			break;
		default :
			if (FD_ISSET(c_fab.domi.fd, &fs)) {
				res = read(c_fab.domi.fd, &c_mag.cBuf[c_mag.wi], SIZE_GENSBUF - c_mag.wi);
				if (res < 0)	TRACK("%s>TERM>ERR:read()!(%d-%s)\n", c_szOwnerName, c_fab.domi.fd, strerror(errno));
				else if (res == 0)	TRACK("%s>TERM>ERR:read 0 byte(%d)!\n", c_szOwnerName, c_mag.wi);
				else {
					c_pGappy->Release();
					if (c_mag.wi < SIZE_GENSBUF) {
						c_mag.wi += (WORD)res;
						c_pGappy->Trigger(TIME_TERMGAP);
					}
					else	c_mag.wi = 0;
				}
			}
		}
	}
	pthread_exit(0);

#if	_DEBL(DEBUG_HANDLER)
	TRACK("%s>TERM:receive handler exit.\n", c_szOwnerName);
#endif
}

bool CTerm::Open(int iCh, DWORD dwBps, WORD wCycleTime)
{
	if (Validity(c_fab.domi.fd))	close(c_fab.domi.fd);

	BYTE buf[32];
	sprintf((char*)buf, "/dev/ser%d", iCh);
	c_fab.domi.fd = open((char*)buf, O_RDWR | O_NOCTTY | O_NONBLOCK);	// O_NONBLOCK is not attached at select()
	if (!Validity(c_fab.domi.fd)) {
		TRACK("%s>TERM>ERR:can not open %s!(%s)\n", c_szOwnerName, buf, strerror(errno));
		return false;
	}

	c_fab.iCh = iCh;
	c_fab.dwBps = dwBps;
	c_fab.t.wCycle = wCycleTime;
	struct termios tmo;
	tcgetattr(c_fab.domi.fd, &tmo);
	c_tmo = tmo;

	cfmakeraw(&tmo);

	cfsetispeed(&tmo, c_fab.dwBps);
	cfsetospeed(&tmo, c_fab.dwBps);
	tmo.c_cflag &= ~(IHFLOW | OHFLOW);
	tmo.c_cflag |= (CLOCAL | CREAD | CS8);
	if (c_fab.iCh == PSCCHA_ATO)	tmo.c_cflag |= (PARENB | PARODD);
	else	tmo.c_cflag &= ~(PARENB | PARODD);
	tmo.c_cflag |= HUPCL;
	tcsetattr(c_fab.domi.fd, TCSANOW, &tmo);

	c_mag.wi = c_mag.wLength = 0;
	if (!Validity(CreateThread(&c_hThread, &CTerm::ReceiveEntry, (PVOID)this, PRIORITY_INTRTERM, (PSZ)"TERM"))) {
		TRACK("%s>TERM>ERR:can not create receive thread!\r\n", c_szOwnerName);
		Close();
		return false;
	}

	c_pGappy = new CTimer(this);
	if (c_pGappy->Initial(PRIORITY_PERIODTERM, (CTimer::PTIMERFUNC)(&CTerm::Snatched), (CPump*)this, TIMERCODE_BYCH(c_fab.iCh) + 1) != 0) {
		TRACK("%s>TERM>ERR:can not create gappy for %s!\n", c_szOwnerName, buf);
		KILL(c_pGappy);
		Close();
		return false;
	}

	if (c_fab.t.wCycle > 0) {
		c_pWatcher = new CTimer(this);
		if (c_pWatcher->Initial(PRIORITY_PERIODTERM, (CTimer::PTIMERFUNC)(&CTerm::Elapsed), (CPump*)this, TIMERCODE_BYCH(c_fab.iCh)) != 0) {
			TRACK("%s>TERM>ERR:can not create watcher for %s!\n", c_szOwnerName, buf);
			KILL(c_pGappy);
			KILL(c_pWatcher);
			Close();
			return false;
		}
	}

	return true;
}

bool CTerm::Close()
{
	if (Validity(c_fab.domi.fd)) {
		tcsetattr(c_fab.domi.fd, TCSANOW, &c_tmo);
		close(c_fab.domi.fd);
		c_fab.domi.fd = INVALID_HANDLE;
	}
	return true;
}

bool CTerm::Send(BYTE* pBuf, WORD wLength)
{
	if (!Validity(c_fab.domi.fd)) {
		TRACK("%s>TERM>ERR:illegal send!\n", c_szOwnerName);
		return false;
	}
	if (!Validity(write(c_fab.domi.fd, pBuf, wLength))) {
		TRACK("%s>TERM>ERR:send failure!(%d-%s)\n", c_szOwnerName, errno, strerror(errno));
		return false;
	}
	if (c_fab.t.wCycle > 0 && c_pWatcher != NULL)	c_pWatcher->Trigger((int)c_fab.t.wCycle);
	return true;
}

bool CTerm::FreeSend(BYTE* pBuf, WORD wLength)
{
	if (!Validity(c_fab.domi.fd)) {
		TRACK("%s>TERM>ERR:illegal free send!\n", c_szOwnerName);
		return false;
	}
	if (!Validity(write(c_fab.domi.fd, pBuf, wLength))) {
		TRACK("%s>TERM>ERR:free send failure!(%s)\n", c_szOwnerName, strerror(errno));
		return false;
	}
	return true;
}

void CTerm::Bunch(BYTE* pBuf, WORD wLength)
{
	memcpy(pBuf, c_mag.cBuf, wLength);	//c_mag.wLength);
}

int CTerm::GetWatcherConnection()
{
	if (c_pWatcher != NULL)	return c_pWatcher->GetConnectID();
	return -1;
}

int CTerm::GetGappyConnection()
{
	if (c_pGappy != NULL)	return c_pGappy->GetConnectID();
	return -1;
}
