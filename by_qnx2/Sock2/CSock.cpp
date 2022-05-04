/*
 * CSock.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <string.h>

#include "Track.h"
#include "CSock.h"

void CSock::SockHandler(union sigval sv)
{
	CSock* pSock = (CSock*)sv.sival_ptr;
	if (pSock == NULL)	return;
	if (!Validity(pSock->c_iSock))	return;

	bool bRework = true;
	if (pSock->c_bListen)	pSock->DoAccept();
	else {
		int res, rleng;
		if (Validity(res = ioctl(pSock->c_iSock, FIONREAD, &rleng))) {
			if (rleng > 0)	pSock->DoReceive();
			else {
				pSock->DoClose();
				bRework = false;
			}
		}
	}
	if (bRework)	pSock->Listening();
}

CSock::CSock()
	: CPump()
{
	Initial();
}

CSock::CSock(PVOID pParent, QHND hQue)
	: CPump(pParent, hQue)
{
	Initial();
}

CSock::CSock(PVOID pParent, QHND hQue, PSZ pszOwnerName)
	: CPump(pParent, hQue, pszOwnerName)
{
	Initial();
}

CSock::~CSock()
{
	if (Validity(c_iSock))	Close();
}

void CSock::Initial()
{
	c_iSock = INVALID_HANDLE;
	c_pid = INVALID_HANDLE;
	c_bListen = false;
	memset(c_cIp, 0, 32);
	c_wPort = 0;
}

bool CSock::Create(unsigned int uPort, int iType)
{
	if (!Validity(c_iSock = socket(AF_INET, iType, 0))) {
		TRACK("SOCK>ERR:socket() failure!(%s)\n", strerror(errno));
		return false;
	}
	if (Bind(uPort))	return true;
	return false;
}

bool CSock::Bind(unsigned int uPort)
{
	if (!Validity(c_iSock))	return false;

	memset((PVOID)&c_sockAddr, 0, sizeof(sockaddr_in));
	c_sockAddr.sin_family = AF_INET;
	c_sockAddr.sin_port = htons(uPort);
	if (!Validity(bind(c_iSock, (sockaddr*)&c_sockAddr, sizeof(c_sockAddr)))) {
		TRACK("SOCK>ERR:bind() failure!(%s)\n", strerror(errno));
		Close();
		return false;
	}
	c_uPort = uPort;
	return true;
}

bool CSock::Listen(int iBacklog)
{
	if (!Validity(c_iSock))	return false;

	if (!Validity(listen(c_iSock, iBacklog))) {
		TRACK("SOCK>ERR:listen() failure!(%s)\n", strerror(errno));
		Close();
		return false;
	}
	int flags = fcntl(c_iSock, F_GETFL, 0);
	if (!Validity(fcntl(c_iSock, F_SETFL, flags | O_NONBLOCK))) {
		TRACK("SOCK>ERR:fcntl() failure!(%s)\n", strerror(errno));
		Close();
		return false;
	}
	if (Listening()) {
		int on = 1;
		if (!Validity(ioctl(c_iSock, FIOASYNC, &on))) {
			TRACK("SOCK>ERR:ioctl() failure!(%s)\n", strerror(errno));
			Close();
			return false;
		}
		c_bListen = true;
		return true;
	}
	Close();
	return false;
}

bool CSock::Listening()
{
	if (!Validity(c_iSock))	return false;

	sigevent event;
	event.sigev_notify = SIGEV_THREAD;
	event.sigev_notify_function = SockHandler;
	event.sigev_notify_attributes = NULL;
	event.sigev_value.sival_ptr = this;
	if (!Validity(ionotify(c_iSock, _NOTIFY_ACTION_TRANARM, _NOTIFY_COND_INPUT, &event))) {
		TRACK("SOCK>ERR:ionotify() failure!(%s)\n", strerror(errno));
		return false;
	}
	return true;
}

bool CSock::Accept(CSock& rSock)
{
	if (!Validity(c_iSock))	return false;

	int sock;
	int saLeng = sizeof(rSock.c_sockAddr);
	if (!Validity(sock = accept(c_iSock, (sockaddr*)&rSock.c_sockAddr, (socklen_t*)&saLeng))) {
		TRACK("SOCK>ERR:accept() failure!(%s)\n", strerror(errno));
		return false;
	}
	rSock.c_iSock = sock;
	return true;
}

bool CSock::Connect(PSZ pIp, WORD wPort)
{
	if (Validity(c_iSock))	close(c_iSock);

	strncpy((char*)c_cIp, (char*)pIp, 32);
	c_wPort = wPort;

	c_iSock = socket(AF_INET, SOCK_STREAM, 0);
	if (!Validity(c_iSock)) {
		TRACK("SOCK>ERR:can not open socket!(%s)\n", strerror(errno));
		return false;
	}

	struct sockaddr_in sadd;
	memset(&sadd, 0, sizeof(sadd));
	sadd.sin_family = AF_INET;
	sadd.sin_addr.s_addr = inet_addr((char*)c_cIp);
	sadd.sin_port = htons(c_wPort);

	int res = connect(c_iSock, (struct sockaddr*)&sadd, sizeof(sadd));
	if (!Validity(res)) {
		if (errno == EINPROGRESS) {
			do {
				fd_set sf;
				FD_ZERO(&sf);
				FD_SET(c_iSock, &sf);

				timeval tv;
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				res = select(c_iSock + 1, NULL, &sf, NULL, &tv);
				if (res == 0) {
					TRACK("SOCK>ERR:connect timeout!\n");
					close(c_iSock);
					c_iSock = INVALID_HANDLE;
					return false;
				}
			} while (res < 0 && errno == EINTR);
		}
	}
	if (!Validity(res)) {
		TRACK("SOCK>ERR:connect failed!(%s)\n", strerror(errno));
		close(c_iSock);
		c_iSock = INVALID_HANDLE;
		return false;
	}

	// for switch
	res = getpeername(c_iSock, (struct sockaddr*)&sadd, (socklen_t*) sizeof(struct sockaddr));
	if (!Validity(res)) {
		TRACK("SOCK>ERR:connect to unknown!(%s)\n", strerror(errno));
		close(c_iSock);
		c_iSock = INVALID_HANDLE;
		return false;
	}

	TRACK("SOCK:connect to %s[%d]\n", c_cIp, c_wPort);
	return Listening();
}

bool CSock::GetPeerName(PSZ pName)
{
	if (!Validity(c_iSock))	return false;
	sprintf((char*)pName, inet_ntoa(c_sockAddr.sin_addr));
	return true;
}

int CSock::Receive(PVOID pBuf, int iLength, int iFlags)
{
	if (!Validity(c_iSock))	return 0;
	return ((int)recv(c_iSock, pBuf, iLength, iFlags));
}

int CSock::Send(PVOID pBuf, int iLength, int iFlags)
{
	if (!Validity(c_iSock))	return 0;
	return ((int)send(c_iSock, pBuf, iLength, iFlags));
}

void CSock::Close()
{
	if (Validity(c_iSock))	close(c_iSock);
	Initial();
}
