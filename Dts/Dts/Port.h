#pragma once

#include "afxmt.h"
#include <afxwin.h>

//#define	_DEBUG_PORT_
#define	WM_RECEIVECHAR		WM_USER + 10

class CPort : public CObject
{
	DECLARE_DYNAMIC(CPort)

public:
	CPort();
	virtual ~CPort();

	UINT	Initial(CWnd* pParent, int nPort, DWORD dwBps, int nRts = 0, int nDtr = 0);
	void	Close();
	UINT	Takeoff();
	void	Landing();
	void	Send(char* pData);
	void	Send(char* pData, DWORD dwLength);
	bool	IsOpen()	{ if (m_nPort > -1) return true; return false; }
	bool	IsRunning()	{ if (m_hThread != NULL) return true; return false; }

private:
	CWnd*	m_pParent;
	CWinThread*	m_pThread;
	HANDLE	m_hThread;

	HANDLE	m_hPort;
	HANDLE	m_hShutdown;
	HANDLE	m_hSend;
	HANDLE	m_hEvents[3];

	OVERLAPPED	m_ov;
	COMMTIMEOUTS	m_cto;
	DCB		m_dcb;

	CCriticalSection	m_cs;

	int		m_nPort;
	DWORD	m_dwBps;

#define	SIZE_PORTBUFFER		1024
	char	m_szSend[SIZE_PORTBUFFER];
	DWORD	m_dwSend;
	int		m_nRts;
	int		m_nDtr;
#if	defined(_DEBUG_PORT_)
	int		m_nRef;
#endif
	static const DWORD	m_dwRts[4];
	static const DWORD	m_dwDtr[4];

protected:
	void	ErrorMessage(char* szError);
	static UINT	PortThreadProc(LPVOID pVoid);
	static void	ReceiveChar(CPort* pPort, COMSTAT comstat);
	static void	ReceiveChar(CPort* pPort);
	static void	SendChar(CPort* pPort);
	void	RtsCtrl(BOOL bCtrl);
};
