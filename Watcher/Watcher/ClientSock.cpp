// ClientSock.cpp
#include "stdafx.h"
#include "Watcher.h"
#include "ClientSock.h"

CClientSock::CClientSock()
{
	CAsyncSocket::CAsyncSocket();
	m_hWnd = NULL;
}

CClientSock::~CClientSock()
{
}

void CClientSock::SetHwnd(HWND hwnd)
{
	if (hwnd != NULL)	m_hWnd = hwnd;
}

void CClientSock::OnConnect(int nErrorCode) 
{
	TRACE(L"CClientSock::OnConnect() error code = %d\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCONNECT, 0, 0);

	CAsyncSocket::OnConnect(nErrorCode);
}

void CClientSock::OnReceive(int nErrorCode) 
{
	//TRACE(L"CClientSock::OnReceive() error code = %d\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SRECEIVE, 0, 0);

	CAsyncSocket::OnReceive(nErrorCode);
}

void CClientSock::OnClose(int nErrorCode) 
{
	TRACE(L"CClientSock::OnClose() error code = %d\r\n", nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCLOSE, 0, 0);

	CAsyncSocket::OnClose(nErrorCode);
}
