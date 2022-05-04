// Port.h
#pragma once

#include "afxmt.h"
#include <afxwin.h>

#define	WM_TXEMPTY		WM_USER + 2
#define	WM_RXCHAR		WM_USER + 4

class CPort : public CObject
{
	DECLARE_DYNAMIC(CPort)

public:
	CPort();
	virtual ~CPort();

private:
	CWnd*		m_pParent;
	CWinThread*	m_pThread;
	HANDLE		m_hThread;

	HANDLE		m_hPort;
	HANDLE		m_hShutdown;
	HANDLE		m_hSend;
	HANDLE		m_hEvents[3];

	OVERLAPPED	m_ov;
	COMMTIMEOUTS	m_cto;
	DCB			m_dcb;
	CCriticalSection	m_cs;

	int		m_nPort;
	DWORD	m_dwBps;
#define	SIZE_DEVBUFFER	4096
	char	m_szSend[SIZE_DEVBUFFER];
	DWORD	m_dwSend;
	int		m_nRts;
	int		m_nDtr;
	int		m_nRef;
	static const DWORD	m_dwRts[4];
	static const DWORD	m_dwDtr[4];

protected:
	void	ErrorMessage(char* szError);
	static UINT	PortThreadProc(LPVOID pVoid);
	static void	ReceiveChar(CPort* pPort, COMSTAT comstat);
	static void	ReceiveChar(CPort* pPort);
	static void	SendChar(CPort* pPort);
	void	RtsCtrl(bool bCtrl);
	void	DtrCtrl(bool bCtrl);

public:
	UINT	Initial(CWnd* pParent, int nPort, DWORD dwBps, int nRts = 0, int nDtr = 0);
	void	Close();
	UINT	Takeoff();
	void	Landon();
	void	Send(char* pData);
	void	Send(char* pData, DWORD dwLength);
	bool	IsOpen()	{ if (m_nPort > -1) return true; return false; }
	bool	IsRunning()	{ if (m_hThread != NULL) return true; return false; }
};
