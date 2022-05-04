// WatcherDlg.h
#pragma once

#include "ClientSocket.h"
#include "AidStatic.h"

class CWatcherDlg : public CDialogEx
{
public:
	CWatcherDlg(CWnd* pParent = NULL);
	virtual ~CWatcherDlg();

	enum { IDD = IDD_WACHER_DIALOG };

private:
	CClientSocket*	m_pTcmsSocket;
	CClientSocket*	m_pDtsSocket;
	CString			m_strIp;
	int				m_nUnitID;
	int				m_nObligePos;
	int				m_nSpeedPos;
	int				m_nDistancePos;
	//int				m_iLengthHead;
	CFont			m_font[2];

	CButton			m_btnLink;
	CListCtrl		m_list;
	CListCtrl		m_listf;
	CRichEditCtrl	m_log;
	CEdit			m_editCmd;
	CComboBox		m_combo;
	CStatic			m_sttPos;
	CAidStatic		m_sttRecCnt;
#define	SIZE_NAME	512
	CAidStatic*		m_pName[SIZE_NAME];
#define	SIZE_TEXT	1024
	CAidStatic*		m_pText[SIZE_TEXT];

#define	SIZE_BAND	128
	typedef struct _tagBAND {
		WORD	cur;
		WORD	min;		
		WORD	max;
	} _BAND;
	_BAND			m_band[3][SIZE_BAND];

	int				m_nBandWidth;
	int				m_nBandUnit;
	DWORD			m_dwRecCycle;
	WORD			m_wSvcCycle[5];

#define	LENGTH_PAGE	20
	typedef struct _tagPAGE {
		WORD	wSize;
		CString	strConcept;
	} _PAGE;
	_PAGE			m_page[LENGTH_PAGE];
	int				m_nPage;

	bool			m_bConcept;
	bool			m_bBuild;
	unsigned int	m_uRecv;
#define	SIZE_SBUF	32768
	unsigned char	m_rBuf[SIZE_SBUF];
#define	SIZE_REALRECV	8192
	unsigned char	m_cBuf[SIZE_REALRECV];

	//static const char	m_nHeadTitle[];

	typedef union _tagDBFTIME {
		struct {
			DWORD	year	: 6;	// 0~63 -> 2010~2073
			DWORD	mon		: 4;
			DWORD	day		: 5;
			DWORD	hour	: 5;
			DWORD	min		: 6;
			DWORD	sec		: 6;
		} t;						// 2/4
		DWORD	dw;
	} DBFTIME, *PDBFTIME;
	DBFTIME	m_dt;

	WORD	m_wOblige;
	WORD	m_wSpeedK;
	double	m_dbDistance;

	typedef union _tagDUET {
		BYTE	c[2];
		WORD	w;
	} _DUET;

	typedef union _tagDBGDBC {
		BYTE	c[sizeof(double)];
		double	db;
	} DEGDBC;

	int		atoh(BYTE ch);
	int		strtoh(CString str);
	void	Log(CString strLog, COLORREF color = RGB(0, 0, 0));
	CAidStatic*	CreateStatic(bool bType, CRect rt);
	void	ClearPeriod();
	void	InsertPeriod(CString str);
	void	InsertCycle(CString str);
	void	ChgDiffer(CListCtrl* pList, int iItem, int iSubItem, WORD w);
	void	ClearPage();
	void	ClearDrum();
	void	BuildDrum();
	int		FindHead(int i, int iLength);
	CString	PokeText(CString& str);
	CString	GetBlacketText(CString str);
	CString	GetConceptBlock(CString& strRec);
	int		GetConcept(CString strRec);
	void	FindReportPosToDts(CString strConcept);
	void	FillContext(CString strRec);
	void	Assort(int iLength);
	bool	Open(int nID, CString strIp, UINT port);
	bool	Opens(CString strIp);
	void	Close();
	void	SendToTcms(CString strSend, int nID);
	void	SendToDts(CString strSend, int nID);
	void	Demand();
	void	TcmsDebug();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCbnSelchangeCombo();
	afx_msg LRESULT OnConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnLBtnDbClk(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonLink();
	afx_msg void OnBnClickedButtonReport();
	afx_msg void OnBnClickedButtonConv();
	afx_msg void OnBnClickedButtonHex();
	afx_msg void OnBnClickedButtonCmd();
};
