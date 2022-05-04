// TerminalDlg.h : header file
//

#if !defined(AFX_TERMINALDLG_H__8C434A27_5FE4_11D5_83EC_00C0262A4671__INCLUDED_)
#define AFX_TERMINALDLG_H__8C434A27_5FE4_11D5_83EC_00C0262A4671__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Port.h"
#include "ViewEdit.h"
#include "TerRef.h"

#define MAX_BUFF 1024

#define RXGAP_TIMER 2
#define RXGAP_TIME 1000

class CMacroEdit;

/////////////////////////////////////////////////////////////////////////////
// CTerminalDlg dialog
const BYTE flowByte[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x15, 0xd, 0xa, 0};

class CTerminalDlg : public CDialog
{
// Construction
public:
	CTerminalDlg(CWnd* pParent = NULL);	// standard constructor

	CMenu m_menu;
	HACCEL m_hAccelTable;

	CPort m_port;
	int m_nPort;
	WORD m_wSum;
	UINT m_nRi;
	BOOL m_bRxWaiting;
	BOOL m_bSumWithoutCtrl;
	char m_tbuf[MAX_BUFF];
	char m_rbuf[MAX_BUFF];

	PORTATT	m_pat;

	typedef struct tagSUMFORM {
		int		nSumType;
		int		nForm;
		int		nLength;
		BOOL	bTimeDisp;
		BOOL	bHexDisp;
	} SUM_FORM;
	SUM_FORM m_sumForm;

	MACRO	m_macro[MAX_MACRO];
	CMacroEdit* m_pMacroEdit;
	CString	m_strMacroFileName;

	enum	{ SOH, STX, ETX, EOT, ENQ, ACK, NAK, CR, LF, SUM };
	BYTE	m_flowByte[SUM + 1];
	int		m_nMacroLength;

	void Send(CString);
	void SendH(CString);
	void CalcSum(TCHAR);
	int MacroFind(UINT, UINT);
	void Log(CString strLog, COLORREF color = RGB(0, 0, 0));
	void Log(BOOL bPlace, char* pszStr, int leng = 0);
	void Log(UINT leng);
	void Log(unsigned char ch);
	void Print(BOOL, char*, int leng = 0);
	void Print(UINT);
	void Print(unsigned char);
	BYTE atoh(BYTE ch);

// Dialog Data
	//{{AFX_DATA(CTerminalDlg)
	enum { IDD = IDD_TERMINAL_DIALOG };
	CEdit	m_editTx;
	CRichEditCtrl	m_editRx;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerminalDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void Serialize(CArchive& ar);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTerminalDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpen();
	afx_msg void OnControlChar();
	afx_msg void OnChksum();
	afx_msg void OnMacroEdit();
	afx_msg void OnDestroy();
	afx_msg void OnClearMonitor();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LONG OnRxChar(WPARAM, LPARAM);
	afx_msg LONG OnCloseMacro(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERMINALDLG_H__8C434A27_5FE4_11D5_83EC_00C0262A4671__INCLUDED_)
