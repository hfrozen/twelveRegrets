// ServerSock.cpp
#include "stdafx.h"
#include "MHost.h"
#include "ServerSock.h"

CServerSock::CServerSock()
{
	CAsyncSocket::CAsyncSocket();
	m_hWnd = NULL;
	m_nID = -1;
}

CServerSock::~CServerSock()
{
}

void CServerSock::OnAccept(int nErrorCode) 
{
	if (nErrorCode != 0)	TRACE(L"Error code = %d\r\n", nErrorCode);
	CClientSock* pClient = new CClientSock();
	if (!Accept(*pClient)) {
		TRACE(_T("Server accept error\r\n"));
		delete pClient;
		return;
	}
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SACCEPT, (WPARAM)m_nID, (LPARAM)pClient);
	CAsyncSocket::OnAccept(nErrorCode);
}
