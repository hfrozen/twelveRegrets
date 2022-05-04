#if !defined(AFX_VIEWEDIT_H__D52C22C1_6D6D_11D5_83EC_00C0262A4671__INCLUDED_)
#define AFX_VIEWEDIT_H__D52C22C1_6D6D_11D5_83EC_00C0262A4671__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewEdit window

class CViewEdit : public CEdit
{
// Construction
public:
	CViewEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CViewEdit();

	void AddString(CString);
	void ClearAll(void);
	void SetTextColor(COLORREF);
	void SetBkColor(COLORREF);

protected:
	COLORREF m_crTextColor;
	COLORREF m_crBkColor;
	CBrush m_brBkgnd;

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewEdit)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWEDIT_H__D52C22C1_6D6D_11D5_83EC_00C0262A4671__INCLUDED_)
