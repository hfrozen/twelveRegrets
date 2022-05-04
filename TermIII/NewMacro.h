#if !defined(AFX_NEWMACRO_H__E649F481_64F1_11D5_83EC_00C0262A4671__INCLUDED_)
#define AFX_NEWMACRO_H__E649F481_64F1_11D5_83EC_00C0262A4671__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewMacro.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewMacro dialog

class CNewMacro : public CDialog
{
// Construction
public:
	CNewMacro(CWnd* pParent = NULL);   // standard constructor

	CBrush	m_whiteBrush;
	UINT	m_nAtt;
	UINT	m_nKey;

// Dialog Data
	//{{AFX_DATA(CNewMacro)
	enum { IDD = IDD_NEW_MACRO_DIALOG };
	CEdit	m_ctrlName;
	CString	m_strMacro;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewMacro)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewMacro)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWMACRO_H__E649F481_64F1_11D5_83EC_00C0262A4671__INCLUDED_)
