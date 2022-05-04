#pragma once

#include "resource.h"

#define	WM_SELECTTROUBLE	WM_USER + 10

class CSelectTrouble : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectTrouble)

public:
	CSelectTrouble(CWnd* pParent = NULL);
	virtual ~CSelectTrouble();

	enum { IDD = IDD_DIALOG_SELECT_TROUBLE };

protected:
	CWnd*	m_pParent;
	CButton	m_btnEvent;
	CButton	m_btnStatus;
	CButton	m_btnLight;
	CButton	m_btnSevere;

	UINT	m_nSelect;

	virtual void DoDataExchange(CDataExchange* pDX);

	UINT	CapSelectState();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonArrange();

	void	SetSelectState(UINT nSel);
	UINT	GetSelectState()			{ return m_nSelect; }
};
