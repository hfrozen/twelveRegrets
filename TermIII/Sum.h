#if !defined(AFX_SUM_H__BA584D03_5FEB_11D5_83EC_00C0262A4671__INCLUDED_)
#define AFX_SUM_H__BA584D03_5FEB_11D5_83EC_00C0262A4671__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Sum.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSum dialog

class CSum : public CDialog
{
// Construction
public:
	CSum(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_CSUM_DIALOG };
	int		m_nSumType;
	int		m_nForm;
	int		m_nLength;
	BOOL	m_bTimeDisp;
	BOOL	m_bHexDisp;

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	//afx_msg void OnSelectSum();
	//afx_msg void OnSelectForm();
	//afx_msg void OnSelectLength();
	afx_msg void OnBnClickedOk();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUM_H__BA584D03_5FEB_11D5_83EC_00C0262A4671__INCLUDED_)
