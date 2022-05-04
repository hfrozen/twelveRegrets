#pragma once

class CLog : public CDialogEx
{
	DECLARE_DYNAMIC(CLog)

public:
	CLog(CWnd* pParent = NULL);
	virtual ~CLog();

	enum { IDD = IDD_DIALOG_LOG };
protected:
	CWnd*	m_pParent;
	CRichEditCtrl m_edit;

public:
	void	Repos(int cx, int cy);
	void	Trace(CString strLog, COLORREF crText = COLOR_BLACK);
	void	Track(CString strLog, COLORREF crText = COLOR_BLACK);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
