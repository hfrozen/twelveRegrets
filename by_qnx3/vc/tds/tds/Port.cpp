// Port.cpp
#include "stdafx.h"
#include "Port.h"

//#define	DEBUG_PORT

IMPLEMENT_DYNAMIC(CPort, CWnd)

const DWORD	CPort::m_dwRts[] = { RTS_CONTROL_DISABLE, RTS_CONTROL_ENABLE, RTS_CONTROL_HANDSHAKE, RTS_CONTROL_TOGGLE	};
const DWORD	CPort::m_dwDtr[] = { DTR_CONTROL_DISABLE, DTR_CONTROL_ENABLE, DTR_CONTROL_HANDSHAKE, DTR_CONTROL_HANDSHAKE };

CPort::CPort()
{
	m_pParent = NULL;
	m_pThread = NULL;
	m_hThread = NULL;
	m_hPort = NULL;
	m_hShutdown = NULL;
	m_hSend = NULL;

	m_ov.hEvent = NULL;
	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;

	m_nPort = -1;
	m_dwBps = 0;
	m_dwSend = 0;
	m_nRts = 0;
	m_nDtr = 0;
	m_nRef = 0;
	ZeroMemory(&m_dcb, sizeof(DCB));
}

CPort::~CPort()
{
	Landon();
	Close();
}

UINT CPort::Initial(CWnd* pParent, int nPort, DWORD dwBps, int nRts, int nDtr)
{
	ASSERT(pParent);
	ASSERT(nPort >= 0);

	m_pParent = pParent;
	m_nPort = nPort;
	m_dwBps = dwBps;
	m_nRts = nRts;
	m_nDtr = nDtr;

	if (m_hThread != NULL) {
		SetEvent(m_hShutdown);
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
		m_pThread = NULL;
	}

	if (m_hShutdown != NULL)	ResetEvent(m_hShutdown);
	m_hShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_ov.hEvent != NULL)	ResetEvent(m_ov.hEvent);
	m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hSend != NULL)	ResetEvent(m_hSend);
	m_hSend = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEvents[0] = m_hShutdown;
	m_hEvents[1] = m_ov.hEvent;
	m_hEvents[2] = m_hSend;

	if (m_hPort != NULL) {
		CloseHandle(m_hPort);
		m_hPort = NULL;
	}

	m_cs.Lock();
	ZeroMemory(m_szSend, SIZE_DEVBUFFER);
	CString strPort;
	if (m_nPort < 10)	strPort.Format(L"COM%d", m_nPort);
	else	strPort.Format(L"\\\\.\\COM%d", m_nPort);
	m_hPort = CreateFile(strPort, GENERIC_READ | GENERIC_WRITE, 0, NULL,
							OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	UINT result = 0;
	if (m_hPort == INVALID_HANDLE_VALUE)	result = 1;
	else {
		m_cto.ReadIntervalTimeout = 1000;
		m_cto.ReadTotalTimeoutMultiplier = 1000;
		m_cto.ReadTotalTimeoutConstant = 1000;
		m_cto.WriteTotalTimeoutMultiplier = 1;	///1000;
		m_cto.WriteTotalTimeoutConstant = 1;	///1000;
		if (SetCommTimeouts(m_hPort, &m_cto)) {
			if (SetCommMask(m_hPort, EV_RXCHAR | EV_TXEMPTY)) {
				DWORD dw;
				GetCommMask(m_hPort, &dw);
				if (GetCommState(m_hPort, &m_dcb)) {
					m_dcb.BaudRate = m_dwBps;
					m_dcb.ByteSize = 8;
					m_dcb.StopBits = ONESTOPBIT;
					m_dcb.Parity = NOPARITY;
					m_dcb.fDtrControl = m_dwDtr[m_nDtr];
					m_dcb.fRtsControl = m_dwRts[m_nRts];
					m_dcb.fOutxDsrFlow = m_nDtr == 2 ? TRUE : FALSE;
					m_dcb.fOutxCtsFlow = m_nRts == 2 ? TRUE : FALSE;
					m_dcb.fInX = FALSE;
					m_dcb.fOutX = FALSE;
					m_dcb.fBinary = TRUE;
					m_dcb.fParity = FALSE;
					m_dcb.EofChar = (char)0xff;
					m_dcb.EvtChar = (char)0xff;
					if (SetCommState(m_hPort, &m_dcb))	result = 0;
					else	result = 2;
				}
				else	result = 3;
			}
			else	result = 4;
		}
		else	result = 5;
		PurgeComm(m_hPort, PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT);
	}
	m_cs.Unlock();

	if (result != 0)	m_nPort = -1;
	return result;
}

void CPort::Close()
{
	// must excuted after landing();
	if (m_hThread != NULL)	Landon();
	if (m_hPort != NULL) {
		DWORD dwFunc = CLRRTS;
		EscapeCommFunction(m_hPort, dwFunc);
		CloseHandle(m_hPort);
		m_hPort = NULL;
		m_nPort = -1;
	}
	if (m_hShutdown != NULL) {
		ResetEvent(m_hShutdown);
		CloseHandle(m_hShutdown);
		m_hShutdown = NULL;
	}
	if (m_ov.hEvent != NULL) {
		ResetEvent(m_ov.hEvent);
		CloseHandle(m_ov.hEvent);
		m_ov.hEvent = NULL;
	}
	if (m_hSend != NULL) {
		ResetEvent(m_hSend);
		CloseHandle(m_hSend);
		m_hSend = NULL;
	}
}

UINT CPort::Takeoff()
{
	ResetEvent(m_hShutdown);
	ResetEvent(m_ov.hEvent);
	ResetEvent(m_hSend);

	if (m_pThread != NULL)	return 1;
	else {
		m_pThread = AfxBeginThread(PortThreadProc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		::DuplicateHandle(GetCurrentProcess(), m_pThread->m_hThread,
			GetCurrentProcess(), &m_hThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
		m_pThread->ResumeThread();
	}
	return 0;
}

void CPort::Landon()
{
	if (m_hThread != NULL) {
		SetEvent(m_hShutdown);
		DWORD dw = WaitForSingleObject(m_hThread, 5000);	//INFINITE);
		if (dw != WAIT_OBJECT_0) {
			CString str;
			str.Format(_T("Ref=%d"), m_nRef);
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
		m_pThread = NULL;
	}
}

void CPort::ReceiveChar(CPort* pPort, COMSTAT comstat)
{
	ASSERT(pPort);

	bool bRead = true;
	bool bResult = true;
	DWORD dwError = 0;
	DWORD dwReadByte = 0;
	BYTE nRx;
	HWND hwnd = pPort->m_pParent->GetSafeHwnd();

	while (pPort->m_hThread) {
		pPort->m_cs.Lock();
		bResult = ClearCommError(pPort->m_hPort, &dwError, &comstat) ? true : false;
		pPort->m_cs.Unlock();

		if (comstat.cbInQue == 0)	break;
		pPort->m_cs.Lock();
		if (bRead) {
			bResult = ReadFile(pPort->m_hPort, &nRx, 1, &dwReadByte, &pPort->m_ov) ? true : false;
			if (!bResult) {
				if ((dwError = GetLastError()) != ERROR_IO_PENDING)
					pPort->ErrorMessage("ReadFile()");
			}
			else	bRead = true;
		}
		if (!bRead) {
			bRead = true;
			bResult = GetOverlappedResult(pPort->m_hPort, &pPort->m_ov, &dwReadByte, TRUE) ? true : false;
			if (!bResult)	pPort->ErrorMessage("GetOverlappedResult() in ReadFile()");
		}
		pPort->m_cs.Unlock();
		if (hwnd != NULL)	SendMessage(hwnd, WM_RXCHAR, (WPARAM)nRx, (LPARAM)pPort->m_nPort);
		//if (hwnd != NULL)	PostMessage(hwnd, WM_RXCHAR, (WPARAM)nRx, (LPARAM)pPort->m_nPort);
	}
}

void CPort::ReceiveChar(CPort* pPort)
{
	ASSERT(pPort);
	bool bRead = true;
	BOOL bResult = TRUE;
	DWORD dwError = 0;
	DWORD dwReadByte = 0;
	COMSTAT comstat;
	BYTE nRx;

	while (pPort->m_hThread) {
		pPort->m_cs.Lock();
#if	defined(DEBUG_PORT)
		pPort->m_nRef = 11;
#endif
		bResult = ClearCommError(pPort->m_hPort, &dwError, &comstat);
#if	defined(DEBUG_PORT)
		pPort->m_nRef = 12;
#endif
		pPort->m_cs.Unlock();

		if (comstat.cbInQue == 0)	break;
		pPort->m_cs.Lock();
		if (bRead) {
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 13;
#endif
			bResult = ReadFile(pPort->m_hPort, &nRx, 1, &dwReadByte, &pPort->m_ov);
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 14;
#endif
			if (!bResult) {
				if ((dwError = GetLastError()) != ERROR_IO_PENDING)
					pPort->ErrorMessage("ReadFile()");
#if	defined(DEBUG_PORT)
				pPort->m_nRef = 17;
#endif
			}
			else	bRead = true;
		}
		if (!bRead) {
			bRead = true;
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 15;
#endif
			bResult = GetOverlappedResult(pPort->m_hPort, &pPort->m_ov, &dwReadByte, TRUE);
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 16;
#endif
			if (!bResult)	pPort->ErrorMessage("GetOverlappedResult() in ReadFile()");
		}
		pPort->m_cs.Unlock();
#if	defined(DEBUG_PORT)
		pPort->m_nRef = 18;
#endif
		HWND hwnd = pPort->m_pParent->GetSafeHwnd();
		if (hwnd != NULL)	SendMessage(hwnd, WM_RXCHAR, (WPARAM)nRx, (LPARAM)pPort->m_nPort);
		//if (hwnd != NULL)	PostMessage(hwnd, WM_RXCHAR, (WPARAM)nRx, (LPARAM)pPort->m_nPort);
#if	defined(DEBUG_PORT)
		pPort->m_nRef = 19;
#endif
	}
}

void CPort::SendChar(CPort* pPort)
{
	bool bResult = true;
	DWORD dwSentByte = 0;
	DWORD dwError;
	DWORD dwSend;

	ResetEvent(pPort->m_hSend);

	pPort->m_cs.Lock();
	pPort->m_ov.Offset = 0;
	pPort->m_ov.OffsetHigh = 0;
	if (pPort->m_nRts == 1)	pPort->RtsCtrl(true);
	PurgeComm(pPort->m_hPort, PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT);

	if (pPort->m_dwSend != 0)	dwSend = pPort->m_dwSend;
	else	dwSend = strlen((char*)pPort->m_szSend);
	bResult = WriteFile(pPort->m_hPort, pPort->m_szSend, dwSend, &dwSentByte, &pPort->m_ov) ? true : false;
	if (!bResult) {
		if ((dwError = GetLastError()) != ERROR_IO_PENDING && dwError != ERROR_IO_INCOMPLETE)
			pPort->ErrorMessage("WriteFile()");
		else {
			do {
				dwError = 0;
				bResult = GetOverlappedResult(pPort->m_hPort, &pPort->m_ov, &dwSentByte, FALSE) ? true : false;
				if (!bResult)	dwError = GetLastError();
			} while (!bResult && dwError == ERROR_IO_INCOMPLETE);
			if (!bResult)	pPort->ErrorMessage("GetOverlappedResult() in WriteFile()");
		}
	}
	pPort->m_cs.Unlock();
}

void CPort::Send(char* pData)
{
	ASSERT(m_hPort);
	ZeroMemory(m_szSend, SIZE_DEVBUFFER);
	strcpy_s(m_szSend, pData);
	m_dwSend = 0;
	SetEvent(m_hSend);
}

void CPort::Send(char* pData, DWORD dwLength)
{
	ASSERT(m_hPort);
	ZeroMemory(m_szSend, SIZE_DEVBUFFER);
	m_dwSend = dwLength;
	if (m_dwSend > SIZE_DEVBUFFER)	m_dwSend = SIZE_DEVBUFFER;
	memcpy_s(m_szSend, SIZE_DEVBUFFER, pData, m_dwSend);
	SetEvent(m_hSend);
}

void CPort::RtsCtrl(bool bCtrl)
{
	DWORD dwFunc = bCtrl ? SETRTS : CLRRTS;
	EscapeCommFunction(m_hPort, dwFunc);
}

void CPort::DtrCtrl(bool bCtrl)
{
	DWORD dwFunc = bCtrl ? SETDTR : CLRDTR;
	EscapeCommFunction(m_hPort, dwFunc);
}

void CPort::ErrorMessage(char* szError)
{
	return;
	char* temp = new char[256];
	LPVOID lpBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpBuf, 0, NULL);
	sprintf_s(temp, sizeof(temp), "WARNING:%s failed with following error\r\n%s\r\nport=%d", szError, lpBuf, m_nPort);
	::MessageBoxA(NULL, temp, "Port Error", MB_ICONSTOP);
	LocalFree(lpBuf);
	delete [] temp;
}

UINT CPort::PortThreadProc(LPVOID pVoid)
{
	CPort* pPort = (CPort*)pVoid;
	ASSERT(pPort);

	DWORD dwEvent = 0;
	DWORD dwCommEvent = 0;
	DWORD dwError = 0;
	bool bResult = true;
	COMSTAT comstat;
	ZeroMemory(&comstat, sizeof(COMSTAT));

	//SetCommMask(pPort->m_hPort, EV_RXCHAR | EV_TXEMPTY);
	if (pPort->m_hPort)	PurgeComm(pPort->m_hPort, PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT);
	while (true) {
		dwEvent = EV_RXCHAR | EV_TXEMPTY;
#if	defined(DEBUG_PORT)
		pPort->m_nRef = 1;
#endif
		bResult = WaitCommEvent(pPort->m_hPort, &dwEvent, &pPort->m_ov) ? true : false;
		if (!bResult) {
			if ((dwError = GetLastError()) != ERROR_IO_PENDING && dwError != 87)	// ERROR_INVALID_PARAMETER
				pPort->ErrorMessage("WaitCommEvent()");
		}
		else {
			bResult = ClearCommError(pPort->m_hPort, &dwError, &comstat) ? true : false;
			if (comstat.cbInQue == 0)	continue;
		}

#if	defined(DEBUG_PORT)
		pPort->m_nRef = 2;
#endif
		dwEvent = WaitForMultipleObjects(3, pPort->m_hEvents, FALSE, INFINITE);
#if	defined(DEBUG_PORT)
		pPort->m_nRef = 3;
#endif
		switch (dwEvent) {
		case 0 :
			//ResetEvent(pPort->m_hShutdown);
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 4;
#endif
			//CloseHandle(pPort->m_hThread);
			return 0;
			break;
		case 1 :
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 5;
#endif
			GetCommMask(pPort->m_hPort, &dwCommEvent);
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 7;
#endif
			if (dwCommEvent & EV_TXEMPTY) {
				//HWND hwnd = pPort->m_pParent->GetSafeHwnd();
				//if (hwnd != NULL)	SendMessage(hwnd, WM_TXEMPTY, (WPARAM)0, (LPARAM)pPort->m_nPort);
				//if (pPort->m_nRts == 1)	pPort->RtsCtrl(false);
			}
			if (dwCommEvent & EV_RXCHAR)	ReceiveChar(pPort, comstat);
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 8;
#endif
			break;
		case 2 :
			SendChar(pPort);
#if	defined(DEBUG_PORT)
			pPort->m_nRef = 9;
#endif
			break;
		}
	}
#if	defined(DEBUG_PORT)
	pPort->m_nRef = 10;
#endif
	return 0;
}

