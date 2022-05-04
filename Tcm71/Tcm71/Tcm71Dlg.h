// Tcm71Dlg.h : 헤더 파일
//

#pragma once

#include "ClientSocket.h"
#include "AidStatic.h"
#include "afxcmn.h"
#include "afxwin.h"

#define	TIMER_WATCH		2
#define	WATCH_TIME		10000

// CTcm71Dlg 대화 상자
class CTcm71Dlg : public CDialog
{
// 생성입니다.
public:
	CTcm71Dlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_TCM71_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCbnSelchangeCombo();

	CListCtrl		m_list;
	CRichEditCtrl	m_edit;
	CComboBox		m_combo;
	CEdit			m_editCycle;
	CEdit			m_editMask;
	CEdit			m_editDest;
	CButton			m_chkCmp;

	afx_msg LRESULT OnConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);

	~CTcm71Dlg();

private:
	CClientSocket*	m_pClientSock;
	CString		m_strIp;
	CString		m_strDestHead;
	int		m_nTextSize;
	int		m_nCurFrame;
	int		m_nChgFrame;
	int		m_nChkFrame;
	DWORD	m_dwCycle;
	int		m_uRecvi;
	BOOL	m_bChkFrame;
	BOOL	m_bOpen;
	BOOL	m_bTimer;
	int		m_nUnMatch;

#define	SIZE_MASK	1024
	unsigned char	m_nMask[SIZE_MASK];
	unsigned char	m_nMaskDummy[SIZE_MASK];

#define	SIZE_TEXT	1024
	CAidStatic*	m_pText[SIZE_TEXT];

#define	SIZE_TITLE	100
	CAidStatic*	m_pTitle[SIZE_TITLE];

#define	SIZE_DASH	100
	CAidStatic*	m_pDash[SIZE_DASH];

#define	SIZE_INTERVAL	128
	typedef struct _tagINTERVAL {
		double	cur;
		double	avr;
		double	min;
		double	max;
	} BINTERVAL;
	BINTERVAL	m_interval[SIZE_INTERVAL];

#define	SIZE_NAME	64
	typedef struct _tagBLOCK {
		char	name[SIZE_NAME];
		int		nSize;
	} BLOCK;

#define	SIZE_BLOCK	128
	typedef struct _tagARCH {
		int		nSize;
		int		nLength;
		BLOCK	block[SIZE_BLOCK];
	} ARCH;

#define	SIZE_ARCH	32
	ARCH	m_arch[SIZE_ARCH];
	int		m_nArchSize;

#define	SIZE_SBUF	32768
	unsigned char	m_rBuf[SIZE_SBUF];
	unsigned char	m_cBuf[SIZE_SBUF];

	void	ClearTexts();
	void	ClearIntervals();
	void	ClearArches();

	void	Scribbling(CString str, COLORREF color = RGB(16, 16, 16));
	int		FindHead(int n, int length);

	BOOL	Open();
	void	Assort();
	void	Initial();
	void	InitialInterval(CString str);
	void	InsertList(CString str);
	void	InitialFrame(CString str);
	void	BuildFrame();
	void	Monitor();
	void	MonitorInterval(CString str);
	void	EmptyInterval(int n);
	void	PutInterval(int n, double v);
	void	MonitorFrame(int si);
	void	CheckFrame(int page, int si);
	BOOL	LoadMask();
	BOOL	StoreDestination(CString strBuf);
	void	MakeDestHead(int id);
	CString	MakeDestField(unsigned char* p, int si, int ei);

public:
	afx_msg void OnBnClickedButtonMask();
	afx_msg void OnBnClickedButtonDest();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCheckCmp();
	CEdit m_editDu;
};
