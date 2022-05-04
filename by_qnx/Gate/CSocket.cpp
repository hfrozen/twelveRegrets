/*
 * CSocket.cpp
 *
 *  Created on: 2010. 09. XX
 *      Author: Che
 */

#include "CSocket.h"

void CSocket::SocketHandler(union sigval sv)
{
	CSocket* pSocket = (CSocket*)sv.sival_ptr;
	if (pSocket == NULL)	return;
	if (pSocket->c_iSocket < 0)	return;

	BOOL bRework = TRUE;
	if (pSocket->c_bListen)	pSocket->DoAccept();
	else {
		int res, rleng;
		if ((res = ioctl(pSocket->c_iSocket, FIONREAD, &rleng)) >= 0) {
			if (rleng > 0)	pSocket->DoReceive();
			else {
				pSocket->DoClose();
				bRework = FALSE;
			}
		}
	}
	if (bRework)	pSocket->Listening();
}

CSocket::CSocket() : CAnt()
{
	c_pParent = NULL;
	Initial();
}

CSocket::CSocket(PVOID pParent, int mq, PSZ pszOwner)
	: CAnt(pParent, mq, pszOwner)
{
	c_pParent = pParent;
	Initial();
}

CSocket::~CSocket()
{
	if (IsValid(c_iSocket))	Close();
}


void CSocket::Initial()
{
	c_iSocket = -1;
	c_pid = -1;
	c_bListen = FALSE;
}

BOOL CSocket::Create(UINT uPort, int iType)
{
	if ((c_iSocket = socket(AF_INET, iType, 0)) < 0)	return FALSE;
	if (Bind(uPort))	return TRUE;
	return FALSE;
}

BOOL CSocket::Bind(UINT uPort)
{
	if (c_iSocket < 0) {
		return FALSE;
	}
	sockaddr_in c_sockAddr;
	memset((PVOID)&c_sockAddr, 0, sizeof(c_sockAddr));
	c_sockAddr.sin_family = AF_INET;
	c_sockAddr.sin_port = htons(uPort);
	if (bind(c_iSocket, (sockaddr*)&c_sockAddr, sizeof(c_sockAddr)) < 0) {
		Close();
		return FALSE;
	}
	c_uPort = uPort;
	return TRUE;
}

BOOL CSocket::Listen(int iBacklog)
{
	if (c_iSocket < 0) {
		return FALSE;
	}
	if (listen(c_iSocket, iBacklog) < 0) {
		Close();
		return FALSE;
	}
	int flags = fcntl(c_iSocket, F_GETFL, 0);
	if (fcntl(c_iSocket, F_SETFL, flags | O_NONBLOCK) < 0) {
		Close();
		return FALSE;
	}
	if (Listening()) {
		int on = 1;
		if (ioctl(c_iSocket, FIOASYNC, &on) < 0) {
			Close();
			return FALSE;
		}
		c_bListen = TRUE;
		return TRUE;
	}
	Close();
	return FALSE;
}

BOOL CSocket::Listening()
{
	if (c_iSocket < 0) {
		return FALSE;
	}
	sigevent event;
	event.sigev_notify = SIGEV_THREAD;
	event.sigev_notify_function = SocketHandler;
	event.sigev_notify_attributes = NULL;
	event.sigev_value.sival_ptr = this;
	if (ionotify(c_iSocket, _NOTIFY_ACTION_TRANARM, _NOTIFY_COND_INPUT, &event) < 0) {
		return FALSE;
	}
	return TRUE;
}

BOOL CSocket::Accept(CSocket& rSock)
{
	if (c_iSocket < 0) {
		return FALSE;
	}
	int sock;
	int saLeng = sizeof(rSock.c_sockAddr);
	if ((sock = accept(c_iSocket, (sockaddr*)&rSock.c_sockAddr, &saLeng)) < 0) {
		return FALSE;
	}
	rSock.c_iSocket = sock;
	return TRUE;
}

BOOL CSocket::GetPeerName(PSZ pName)
{
	if (c_iSocket < 0) {
		return FALSE;
	}
	sprintf(pName, inet_ntoa(c_sockAddr.sin_addr));
	return TRUE;
}

int CSocket::Receive(PVOID pBuf, int nLength, int nFlags)
{
	if (c_iSocket < 0) {
		return 0;
	}
	return ((int)recv(c_iSocket, pBuf, nLength, nFlags));
}

int CSocket::Send(PVOID pBuf, int nLength, int nFlags)
{
	if (c_iSocket < 0) {
		return 0;
	}
	return ((int)send(c_iSocket, pBuf, nLength, nFlags));
}

void CSocket::Close()
{
	ASSERT(c_iSocket >= 0);
	close(c_iSocket);
	Initial();
}

//===== CSocket override

void CSocket::DoAccept()
{
}

void CSocket::DoClose()
{
}

void CSocket::DoSend()
{
}

void CSocket::DoReceive()
{
}
