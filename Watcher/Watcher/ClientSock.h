// ClientSock.h
#pragma once

#include "AfxSock.h"

#define	WM_SACCEPT		WM_USER + 2
#define	WM_SCONNECT		WM_USER + 4
#define	WM_SRECEIVE		WM_USER + 6
#define	WM_SSEND		WM_USER + 8
#define	WM_SCLOSE		WM_USER + 10

class CClientSock : public CAsyncSocket
{
public:
	CClientSock();
	virtual ~CClientSock();

	HWND	m_hWnd;
	void	SetHwnd(HWND hwnd);

	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};
