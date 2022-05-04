/*
 * CServer.cpp
 *
 *  Created on: 2010. 10. 6
 *      Author: Che
 */

#include "CServer.h"

CServer::CServer()
{
	c_pParent = NULL;
	c_mq = -1;
	c_pClient = NULL;
}

CServer::CServer(PVOID pParent, int mq)
	: CSocket(pParent, mq, (PSZ)"Server")
{
	c_pParent = pParent;
	c_mq = mq;
	c_pClient = NULL;
}

CServer::~CServer()
{
}

void CServer::DoAccept()
{
	CClient* pClient = new CClient(c_pParent, c_mq);
	if (!Accept(*pClient)) {
		delete pClient;
		return;
	}
	c_pClient = pClient;
	c_pClient->c_bListen = FALSE;
	c_pClient->Listening();
	SendMail(MA_SOCKETACCEPT, c_pParent, 0);
	CSocket::DoAccept();
}

CClient* CServer::GetAccept()
{
	return c_pClient;
}
