// ClientSock.cpp
#include "stdafx.h"
#include "MHost.h"
#include "ClientSock.h"

CClientSock::CClientSock()
{
	CAsyncSocket::CAsyncSocket();
	m_hWnd = NULL;
	m_nID = -1;
}

CClientSock::~CClientSock()
{
}

void CClientSock::OnConnect(int nErrorCode)
{
	if (nErrorCode != 0)	TRACE(L"OnConnect error code = %d.\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCONNECT, (WPARAM)m_nID, (LPARAM)this);
	CAsyncSocket::OnConnect(nErrorCode);
}

void CClientSock::OnReceive(int nErrorCode) 
{
	if (nErrorCode != 0)	TRACE(L"OnConnect error code = %d.\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SRECEIVE, (WPARAM)m_nID, (LPARAM)this);
	CAsyncSocket::OnReceive(nErrorCode);
}

void CClientSock::OnClose(int nErrorCode) 
{
	if (nErrorCode != 0)	TRACE(L"OnConnect error code = %d.\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCLOSE, (WPARAM)m_nID, (LPARAM)this);
	CAsyncSocket::OnClose(nErrorCode);
}
