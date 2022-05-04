/**
 * @file	CServer.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#include "CServer.h"

CServer::CServer()
	: CSock()
{
	c_hQue = INVALID_HANDLE;
}

CServer::CServer(PVOID pParent, QHND hQue)
	: CSock(pParent, hQue)
{
	c_hQue = hQue;
}

CServer::CServer(PVOID pParent, QHND hQue, PSZ pszOwnerName)
	: CSock(pParent, hQue, pszOwnerName)
{
	c_hQue = hQue;
}

CServer::~CServer()
{
}

void CServer::DoAccept()
{
	CClient* pClient = new CClient(c_pParent, c_hQue, (PSZ)"CLNT");
	if (!Accept(*pClient)) {
		delete pClient;
		return;
	}
	pClient->c_bListen = false;
	pClient->Listening();
	SendTag(TAG_SOCKACCEPT, c_pParent, (DWORD)pClient);
	CSock::DoAccept();
}
