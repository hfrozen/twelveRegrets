#if !defined(AFX_OPEN_H__BA584D01_5FEB_11D5_83EC_00C0262A4671__INCLUDED_)
#define AFX_OPEN_H__BA584D01_5FEB_11D5_83EC_00C0262A4671__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Open.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpen dialog

#include "TerRef.h"

class COpen : public CDialog
{
// Construction
public:
	COpen(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COpen)
	enum { IDD = IDD_OPEN_DIALOG };

	PORTATT	m_pao;
	CComboBox	m_ctrlPort;
	CComboBox	m_ctrlBps;
	int		m_nDls;
	int		m_nSls;
	int		m_nPrs;
	int		m_nDtr;
	int		m_nRts;
	//}}AFX_DATA

	static const DWORD	m_dwBpss[11];
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpen)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPEN_H__BA584D01_5FEB_11D5_83EC_00C0262A4671__INCLUDED_)
