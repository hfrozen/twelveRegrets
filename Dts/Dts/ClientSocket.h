#pragma once

#define	WM_SACCEPT		WM_USER + 2
#define	WM_SCONNECT		WM_USER + 4
#define	WM_SRECEIVE		WM_USER + 6
#define	WM_SCLOSE		WM_USER + 8

class CClientSocket : public CAsyncSocket
{
public:
	CClientSocket();
	virtual ~CClientSocket();

	HWND	m_hWnd;
	int		m_nID;
	void	SetHwnd(HWND hWnd)			{ m_hWnd = hWnd; }
	void	SetID(int nID)				{ m_nID = nID; }

	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};
