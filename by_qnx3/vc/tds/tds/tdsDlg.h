// tdsDlg.h
#pragma once
#include "Port.h"

#define	CAST_TIMER		2
#define	TIME_CAST		200

class CtdsDlg : public CDialogEx
{
public:
	CtdsDlg(CWnd* pParent = NULL);
	~CtdsDlg();

	enum { IDD = IDD_TDS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON	m_hIcon;
	CRichEditCtrl	m_editLog;
	CEdit	m_editPortNo;
	CEdit	m_editRealDate;
	CEdit	m_editDbfDate;
	CButton	m_btnCast;
	CButton	m_btnReset;
	CButton m_btnOpen;
	CPort	m_port;
	BYTE*	m_pDummy;
	DWORD	m_dwDummy;
	DWORD	m_dwDumOfs;
	BYTE	m_cDumCnt;
	COleDateTime	m_odt;

	typedef union _tagBCDT {
		struct {
			BYTE	one :	4;
			BYTE	ten :	4;
		} b;
		BYTE a;
	} BCDT;

#pragma pack(push, 1)
	typedef union _tagDBFTIME {
		struct {
			DWORD	year	: 6;	// 0~63 -> 2010~2073
			DWORD	mon		: 4;
			DWORD	day		: 5;
			DWORD	hour	: 5;
			DWORD	min		: 6;
			DWORD	sec		: 6;
		} t;						// 2/4
		DWORD dw;
	} DBFTIME;

#define	SIZE_BUBBLEDUMMY	1398	// 1398 + 14 = 1412
	typedef struct _tagBUBBLE {
		BYTE	stx;		// 1
		BYTE	ctrl;		// 2
		BYTE	cnt;		// 3
		struct {
			BCDT	year;	// 4
			BCDT	mon;	// 5
			BCDT	day;	// 6
			BCDT	hour;	// 7
			BCDT	min;	// 8
			BCDT	sec;	// 9
		} t;
		struct {
			BCDT	th;		// 10
			BCDT	to;		// 11
		} n;
		BYTE	dummy[SIZE_BUBBLEDUMMY];
		BYTE	etx;		// 12
		BYTE	bcce;		// 13
		BYTE	bcco;		// 14
	} BUBBLE;
	BUBBLE	m_bubble;
#pragma pack(pop)

	void	Log(CString strLog, COLORREF crText = RGB(0, 0, 0));
	BYTE	ToBcd(BYTE c);

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonCast();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonConv2dbft();

	DECLARE_MESSAGE_MAP()
};
