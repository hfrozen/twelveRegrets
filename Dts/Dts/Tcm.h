#pragma once

#include "ClientSocket.h"

#define	WM_INFORMTCM	WM_USER + 12
#define	WM_CLOSETCM		WM_USER + 14

class CTcm : public CDialogEx
{
	DECLARE_DYNAMIC(CTcm)

public:
	CTcm(CWnd* pParent = NULL);
	virtual ~CTcm();

	enum { IDD = IDD_DIALOG_TCM };

	CWnd*			m_pParent;
	CRichEditCtrl	m_log;
	CClientSocket*	m_pClientSocket;
	int				m_nID;	// 100~
	WORD			m_wSpeed;
	DWORD			m_dwDistan;

	void	Log(CString strLog, COLORREF crText = RGB(0, 0, 0));

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClickedButtonClear();
	afx_msg void OnClickedButtonHide();
	afx_msg LRESULT OnConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
};
