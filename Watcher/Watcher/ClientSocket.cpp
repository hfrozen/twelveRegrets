// ClientSocket.cpp
#include "stdafx.h"
#include "Watcher.h"
#include "ClientSocket.h"

CClientSocket::CClientSocket()
{
	CAsyncSocket::CAsyncSocket();
	m_hWnd = NULL;
	m_nID = -1;
}

CClientSocket::~CClientSocket()
{
}

void CClientSocket::OnConnect(int nErrorCode) 
{
	TRACE(L"CClientSocket::OnConnect() error code = %d\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCONNECT, (WPARAM)m_nID, (LPARAM)this);
	CAsyncSocket::OnConnect(nErrorCode);
}

void CClientSocket::OnReceive(int nErrorCode) 
{
	//TRACE(L"CClientSocket::OnReceive() error code = %d\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SRECEIVE, (WPARAM)m_nID, (LPARAM)this);
	CAsyncSocket::OnReceive(nErrorCode);
}

void CClientSocket::OnClose(int nErrorCode) 
{
	TRACE(L"CClientSocket::OnClose() error code = %d\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCLOSE, (WPARAM)m_nID, (LPARAM)this);
	CAsyncSocket::OnClose(nErrorCode);
}
