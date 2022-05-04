#if !defined(AFX_MACROEDIT_H__1A062481_64EB_11D5_83EC_00C0262A4671__INCLUDED_)
#define AFX_MACROEDIT_H__1A062481_64EB_11D5_83EC_00C0262A4671__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MacroEdit.h : header file
//

#define WM_CLOSEMACRO WM_USER + 8

/////////////////////////////////////////////////////////////////////////////
// CMacroEdit dialog

class CMacroEdit : public CDialog
{
// Construction
public:
	CMacroEdit(CWnd* pParent = NULL);   // standard constructor
	CMacroEdit(MACRO*, CDialog*);

	CDialog* m_pParent;

	MACRO*	m_pMacro;
	BOOL	m_bEdited;	
	int		m_nLength;
	int		m_nDeleteID;
	void	Append(int, UINT, UINT, CString);

// Dialog Data
	//{{AFX_DATA(CMacroEdit)
	enum { IDD = IDD_MACRO_DIALOG };
	CListCtrl	m_ctrlList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMacroEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMacroEdit)
	afx_msg void OnMacroNew();
	afx_msg void OnMacroDel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnItemchangedMacroList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMacroEnd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACROEDIT_H__1A062481_64EB_11D5_83EC_00C0262A4671__INCLUDED_)
