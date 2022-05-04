#pragma once

#include "Leds.h"
#include "ClientSocket.h"

//#define	TCPSEQOPEN

#define	UDIB2IDC(x)		(x + IDC_CHECK_DEV0)
#define	IDC2UDIB(x)		(x - IDC_CHECK_DEB0)

#define	MAX_PANELS		12
#define	MAX_FLOORS		16		// 14

#define	TIME_DEVPROC	1000

class CPanel : public CDialogEx
{
	DECLARE_DYNAMIC(CPanel)

public:
	CPanel(CWnd* pParent = NULL);
	virtual ~CPanel();

	enum { IDD = IDD_DIALOG_PANEL };

protected:
	CWnd*	m_pParent;
	bool	m_bAlarmMsg;

	CCriticalSection	m_cs;
	CRichEditCtrl	m_editLog;
	CButton	m_btnClear;
	CButton	m_btnExit;

	CLeds*	m_pLeds[MAX_PANELS];
	BYTE	m_cDies[10];

#define	SIZE_SOCKRECVBUFFER		4096
#define	SIZE_SOCKSENDBUFFER		256
	typedef struct tagFLOORCTRL {
		CClientSocket*	pSock;
		CCriticalSection	cs;
		bool	bConnect;
		bool	bInitial;
		int		nRi;
		int		nConCycle;
		char	sRbuf[SIZE_SOCKRECVBUFFER];
		char	sSbuf[SIZE_SOCKSENDBUFFER];
	} FLOORCTRL;
	FLOORCTRL	m_floor[MAX_FLOORS];
#define	CYCLE_CONNECT		3

#if	defined(TCPSEQOPEN)
	int		m_nConnSeq;
#endif

	static PCWSTR	m_pstrPanelNames[MAX_PANELS];
	static const int	m_nDioMap[2][MAX_LEDS];

	typedef struct tagIPPORT {
		CString	strName;
		CString	strIp;
		UINT	nPort;
	} IPPORT;
	static const IPPORT	m_ips[MAX_FLOORS];

	typedef union _tagLCHTYPE{
		struct {
			BYTE	bps : 4;
			BYTE	null : 3;
			BYTE	mode : 1;
		} b;
		BYTE	a;
	} LCHTYPE;

	void	Log(CString strLog, COLORREF crText = COLOR_BLACK);
public:
	void	InterLog(CString strLog, COLORREF crText = COLOR_BLUE);
	void	InterLog(int id, BYTE* pBuf, int leng, COLORREF crText = COLOR_BLUE);
private:
	void	GetDies(CString strDies);
#if	defined(TCPSEQOPEN)
	void	Connect();
#else
	bool	Connect(int nID);
#endif

public:
	int		GetDioMap(UINT cid, UINT sid);
	void	UnitClr(UINT cid);
	void	BitCtrl(UINT cid, UINT sid, bool bCtrl);
	bool	GetBit(UINT cid, UINT sid);
	void	PwmCtrl(UINT cid, UINT sid, UINT uPwm);
	void	Open();
	void	Close();
	void	SendToSocket(int id, char* pBuf, int leng);
	void	SendToSocket(int id, CString strSend);
	void	SendToSocket(int id, int leng, ...);
	void	SendToSocket(int id, int leng, va_list vargs);
	void	EnableAlarm(bool bAlarm)		{ m_bAlarmMsg = bAlarm; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnClickLed(WPARAM wParam, LPARAM lParam);
};
