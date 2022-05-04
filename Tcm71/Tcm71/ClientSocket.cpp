// ClientSocket.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Tcm71.h"
#include "ClientSocket.h"


// CClientSocket

CClientSocket::CClientSocket()
{
	CAsyncSocket::CAsyncSocket();
	m_hWnd = NULL;
}

CClientSocket::~CClientSocket()
{
}


// CClientSocket ��� �Լ�

void CClientSocket::SetHwnd(HWND hwnd)
{
	if (hwnd != NULL)	m_hWnd = hwnd;
}

void CClientSocket::OnConnect(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (nErrorCode > 0)	TRACE(_T("OnConnect error code = %d\r\n"), nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCONNECT, 0, 0);

	CAsyncSocket::OnConnect(nErrorCode);
}

void CClientSocket::OnReceive(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (nErrorCode > 0)	TRACE(_T("OnReceive error code = %d\r\n"), nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SRECEIVE, 0, 0);

	CAsyncSocket::OnReceive(nErrorCode);
}

void CClientSocket::OnClose(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (nErrorCode > 0)	TRACE(_T("OnClose error code = %d\r\n"), nErrorCode);
	if (m_hWnd != NULL)	SendMessage(m_hWnd, WM_SCLOSE, 0, 0);

	CAsyncSocket::OnClose(nErrorCode);
}
