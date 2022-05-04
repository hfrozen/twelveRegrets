#pragma once

#include "ClientSock.h"

class CServerSock : public CAsyncSocket
{
public:
	CServerSock();
	virtual ~CServerSock();

	HWND	m_hWnd;
	int		m_nID;

	void	SetHwnd(HWND hWnd)			{ m_hWnd = hWnd; }
	void	SetID(int nID)				{ m_nID = nID; }

	virtual void OnAccept(int nErrorCode);
};
