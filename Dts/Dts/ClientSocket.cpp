// ClientSocket.cpp
#include "stdafx.h"
#include "Dts.h"
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
	if (nErrorCode != 0)	TRACE(L"OnConnect error code = %d at %d.\r\n", nErrorCode, m_nID);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCONNECT, (WPARAM)m_nID, (LPARAM)nErrorCode);	//this);
	CAsyncSocket::OnConnect(nErrorCode);
}

void CClientSocket::OnReceive(int nErrorCode)
{
	if (nErrorCode != 0)	TRACE(L"OnReceive error code = %d at %d.\r\n", nErrorCode, m_nID);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SRECEIVE, (WPARAM)m_nID, (LPARAM)nErrorCode);	//this);
	CAsyncSocket::OnReceive(nErrorCode);
}

void CClientSocket::OnClose(int nErrorCode)
{
	if (nErrorCode != 0)	TRACE(L"OnClose error code = %d at %d.\r\n", nErrorCode, m_nID);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCLOSE, (WPARAM)m_nID, (LPARAM)nErrorCode);	//this);
	CAsyncSocket::OnClose(nErrorCode);
}
