#pragma once

#include "ClientSocket.h"

class CServerSocket : public CAsyncSocket
{
public:
	CServerSocket();
	virtual ~CServerSocket();

	HWND	m_hWnd;
	int		m_nID;

	void	SetHwnd(HWND hWnd)		{ m_hWnd = hWnd; }
	void	SetID(int nID)			{ m_nID = nID; }

	virtual void OnAccept(int nErrorCode);
};


