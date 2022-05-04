/**
 * @file	CAsync.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/iomsg.h>
//#include <sys/select.h>

#include "../Base/Track.h"
#include "CAsync.h"

#define	TIME_ASYNCSETTLE	100		//30		// 2ms

CAsync::CAsync()
	: CPump()
{
	c_pWatcher = NULL;
	c_pSettler = NULL;
	c_hThread = INVALID_HANDLE;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.subject.fd = INVALID_HANDLE;
	c_fab.iCh = 0;
	memset(&c_mag, 0, sizeof(MAGAZINE));
}

CAsync::CAsync(PVOID pParent, QHND hQue, PSZ pszOwnerName)
	: CPump(pParent, hQue, pszOwnerName)
{
	c_pWatcher = NULL;
	c_pSettler = NULL;
	c_hThread = INVALID_HANDLE;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.subject.fd = INVALID_HANDLE;
	c_fab.iCh = 0;
	memset(&c_mag, 0, sizeof(MAGAZINE));
}

CAsync::~CAsync()
{
	KILLR(c_pSettler);
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

void CAsync::Watch()
{
	if (c_pWatcher != NULL && c_pWatcher->GetRhythm())	c_pWatcher->Release();
	if (c_pSettler != NULL && c_pSettler->GetRhythm())	c_pSettler->Release();
	c_mag.wi = 0;
	SendTag(TAG_RECEIVEASYNC, c_pParent, MAKEDWORD(0x8000, MAKEWORD(RECV_TIMEOUT, c_fab.iCh)));
}

void CAsync::Settle()
{
	if (c_pWatcher != NULL && c_pWatcher->GetRhythm())	c_pWatcher->Release();
	if (c_pSettler != NULL && c_pSettler->GetRhythm())	c_pSettler->Release();
	if (c_mag.wi > 0) {
		c_mag.wLength = c_mag.wi;
		c_mag.wi = 0;
		SendTag(TAG_RECEIVEASYNC, c_pParent, MAKEDWORD(c_mag.wLength, MAKEWORD(RECV_NORMAL, c_fab.iCh)));
	}
}

PVOID CAsync::ReceiveEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CAsync* pAsync = (CAsync*)pVoid;
	if (pAsync != NULL)	pAsync->Receive();
	pthread_exit(NULL);
	return NULL;
}

void CAsync::Receive()
{
//#if	defined(DEBUG_HANDLER)
//	TRACK("%s>ASYNC:receive handler start.(%d)\n", c_szOwnerName, c_fab.domi.fd);
//#endif
//
	fd_set fs;
	FD_ZERO(&fs);
	FD_SET(c_fab.subject.fd, &fs);

	while (Validity(c_fab.subject.fd)) {
		int res;
		switch (res = select(c_fab.subject.fd + 1, &fs, NULL, NULL, NULL)) {	//&tv)) {		//NULL)) {
		case EINTR :
			TRACK("%s>ASYNC:select() catch EINTR\n", c_szOwnerName);
			pthread_exit(0);
			TRACK("%s>ASYNC:handler exit A.\n", c_szOwnerName);
			return;
			break;
		case -1 :
			TRACK("%s>ASYNC>ERR:select()!(%s)\n", c_szOwnerName, strerror(errno));
			break;
		case 0 :
			TRACK("%s>ASYNC>ERR:select timeout!\n", c_szOwnerName);
			break;
		default :
			if (FD_ISSET(c_fab.subject.fd, &fs)) {
				res = read(c_fab.subject.fd, &c_mag.cBuf[c_mag.wi], SIZE_GENSBUF - c_mag.wi);
				if (res < 0)	TRACK("%s>ASYNC>ERR:read()!(%d-%s)\n", c_szOwnerName, c_fab.subject.fd, strerror(errno));
				else if (res == 0)	TRACK("%s>ASYNC>ERR:read 0 byte(%d)!\n", c_szOwnerName, c_mag.wi);
				else {
					c_pSettler->Release();
					if (c_mag.wi < SIZE_GENSBUF) {
						//TRACK("ASYNC:read %d %d\n", c_mag.wi, res);		// ?????
						c_mag.wi += (WORD)res;
						c_pSettler->Trigger(TIME_ASYNCSETTLE);
					}
					else	c_mag.wi = 0;
				}
			}
		}
	}
	pthread_exit(0);

#if	_DEBL(DEBUG_HANDLER)
	TRACK("%s>ASYNC:receive handler exit.\n", c_szOwnerName);
#endif
}

bool CAsync::Open(int iCh, DWORD dwBps, enPARITY parity, WORD wCycleTime)
{
	if (Validity(c_fab.subject.fd))	close(c_fab.subject.fd);

	BYTE buf[32];
	sprintf((char*)buf, "/dev/ser%d", iCh);
	c_fab.subject.fd = open((char*)buf, O_RDWR | O_NOCTTY | O_NONBLOCK);	// O_NONBLOCK is not attached at select()
	if (!Validity(c_fab.subject.fd)) {
		TRACK("%s>ASYNC>ERR:can not open %s!(%s)\n", c_szOwnerName, buf, strerror(errno));
		return false;
	}
	tcflush(c_fab.subject.fd, TCIFLUSH);

	c_fab.iCh = iCh;
	c_fab.dwBps = dwBps;
	c_fab.time.wCycle = wCycleTime;
	struct termios tmo;
	tcgetattr(c_fab.subject.fd, &tmo);
	c_tmo = tmo;

	cfmakeraw(&tmo);

	cfsetispeed(&tmo, c_fab.dwBps);
	cfsetospeed(&tmo, c_fab.dwBps);
	tmo.c_cflag &= ~(IHFLOW | OHFLOW);
	tmo.c_cflag |= (CLOCAL | CREAD | CS8);
	tmo.c_cflag &= ~(PARENB | PARODD);
	if (parity & PARITY_EN)		tmo.c_cflag |= PARENB;
	if (parity & PARITY_ODD)	tmo.c_cflag |= PARODD;
	tmo.c_cflag |= HUPCL;
	tcsetattr(c_fab.subject.fd, TCSANOW, &tmo);

	c_mag.wi = c_mag.wLength = 0;
	if (!Validity(CreateThread(&c_hThread, &CAsync::ReceiveEntry, (PVOID)this, PRIORITY_INTRASYNC, (PSZ)"ASYNC"))) {
		TRACK("%s>ASYNC>ERR:can not create receive thread!\r\n", c_szOwnerName);
		Close();
		return false;
	}

	c_pSettler = new CTimer(this);
	if (c_pSettler->Initial(PRIORITY_PERIODASYNC, (CTimer::PTIMERFUNC)(&CAsync::Settle), TIMERCODE_BYCH(c_fab.iCh) + 1) != 0) {
		TRACK("%s>ASYNC>ERR:can not create gappy for %s!\n", c_szOwnerName, buf);
		KILL(c_pSettler);
		Close();
		return false;
	}

	if (c_fab.time.wCycle > 0) {
		c_pWatcher = new CTimer(this);
		if (c_pWatcher->Initial(PRIORITY_PERIODASYNC, (CTimer::PTIMERFUNC)(&CAsync::Watch), TIMERCODE_BYCH(c_fab.iCh)) != 0) {
			TRACK("%s>ASYNC>ERR:can not create watcher for %s!\n", c_szOwnerName, buf);
			KILL(c_pSettler);
			KILL(c_pWatcher);
			Close();
			return false;
		}
	}

	return true;
}

bool CAsync::Close()
{
	if (Validity(c_fab.subject.fd)) {
		tcsetattr(c_fab.subject.fd, TCSANOW, &c_tmo);
		close(c_fab.subject.fd);
		c_fab.subject.fd = INVALID_HANDLE;
	}
	return true;
}

bool CAsync::Send(BYTE* pBuf, WORD wLength)
{
	if (!Validity(c_fab.subject.fd)) {
		TRACK("%s>ASYNC>ERR:illegal send!\n", c_szOwnerName);
		return false;
	}
	if (!Validity(write(c_fab.subject.fd, pBuf, wLength))) {
		TRACK("%s>ASYNC>ERR:send failure!(%d-%s)\n", c_szOwnerName, errno, strerror(errno));
		return false;
	}
	if (c_fab.time.wCycle > 0 && c_pWatcher != NULL)	c_pWatcher->Trigger((int)c_fab.time.wCycle);
	return true;
}

bool CAsync::FreeSend(BYTE* pBuf, WORD wLength)
{
	if (!Validity(c_fab.subject.fd)) {
		TRACK("%s>ASYNC>ERR:illegal free send!\n", c_szOwnerName);
		return false;
	}
	if (!Validity(write(c_fab.subject.fd, pBuf, wLength))) {
		TRACK("%s>ASYNC>ERR:free send failure!(%s)\n", c_szOwnerName, strerror(errno));
		return false;
	}
	return true;
}

void CAsync::Bunch(BYTE* pBuf, WORD wLength)
{
	memcpy(pBuf, c_mag.cBuf, wLength);	//c_mag.wLength);
}

int CAsync::GetWatcherConnection()
{
	if (c_pWatcher != NULL)	return c_pWatcher->GetConnectID();
	return -1;
}

int CAsync::GetSettlerConnection()
{
	if (c_pSettler != NULL)	return c_pSettler->GetConnectID();
	return -1;
}
