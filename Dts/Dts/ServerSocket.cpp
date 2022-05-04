// ServerSocket.cpp
#include "stdafx.h"
#include "Dts.h"
#include "ServerSocket.h"

CServerSocket::CServerSocket()
{
	CAsyncSocket::CAsyncSocket();
	m_hWnd = NULL;
	m_nID = -1;
}

CServerSocket::~CServerSocket()
{
}

void CServerSocket::OnAccept(int nErrorCode)
{
	if (nErrorCode != 0)	TRACE(L"Error code = %d\r\n", nErrorCode);

	CClientSocket* pClientSocket = new CClientSocket();
	if (!Accept(*pClientSocket)) {
		TRACE(L"Server accept failer!\r\n");
		delete pClientSocket;
		return;
	}
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SACCEPT, (WPARAM)m_nID, (LPARAM)pClientSocket);
	CAsyncSocket::OnAccept(nErrorCode);
}
