// MHostDlg.h
#pragma once

#include "ServerSock.h"
#include "Node.h"

class CMHostDlg : public CDialogEx
{
public:
	CMHostDlg(CWnd* pParent = NULL);
	virtual ~CMHostDlg();

	enum { IDD = IDD_MHOST_DIALOG };

	CEdit m_editLength;
	CServerSock*	m_pServerSock;
	CPoint	m_pt;
	CRect	m_rectBase;
	CRect	m_rectNode;
	typedef CTypedPtrList <CPtrList, CNode*> TWIG, *PTWIG;
	TWIG	m_twig;

private:
	void	AlignScreen();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnAccept(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNodeClose(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
