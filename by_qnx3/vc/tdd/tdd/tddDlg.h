// tddDlg.h
#pragma once

class CtddDlg : public CDialogEx
{
public:
	CtddDlg(CWnd* pParent = NULL);
	virtual ~CtddDlg();

	enum { IDD = IDD_TDD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);


protected:
	HICON m_hIcon;
	CButton m_btnMake;
	CEdit	m_edit;
	CRichEditCtrl	m_log;

	BYTE*	m_pDummy;
	DWORD	m_dwDummy;
	DWORD	m_dwDumOfs;

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

	typedef union	_tagOCTET {
		BYTE	c[8];
		WORD	w[4];
		DWORD	dw[2];
		QWORD	qw;
	} _OCTET;

#define	DUMMYFILE_LENGTH	8928
#define	SIZE_BUBBLEDUMMY	1398	// 1398 + 10 = 1408
	typedef struct _tagBUBBLE {
		BYTE	stx;
		BYTE	ctrl;
		BYTE	cnt;
		struct {
			BCDT	year;
			BCDT	mon;
			BCDT	day;
			BCDT	hour;
			BCDT	min;
			BCDT	sec;
		} t;
		struct {
			BCDT	th;
			BCDT	to;
		} n;
		BYTE	dummy[SIZE_BUBBLEDUMMY];
		BYTE	etx;
		BYTE	bcce;
		BYTE	bcco;
	} BUBBLE;
	BUBBLE	m_bubble;
#pragma pack(pop)

	void	Log(CString strLog, COLORREF crText = RGB(0, 0, 0));
	BYTE	ToBcd(BYTE c);
	void	MakeBlock(BYTE* pBuf, int year, int mon, int day, int hour, int min, int ti);

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonMake();
	afx_msg void OnBnClickedButtonTest();

	DECLARE_MESSAGE_MAP()
};
