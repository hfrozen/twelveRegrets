// DtsDlg.h
#pragma once

class CIntro;

class CDtsDlg : public CDialogEx
{
public:
	CDtsDlg(CWnd* pParent = NULL);
	virtual ~CDtsDlg();

	enum { IDD = IDD_DTS_DIALOG };

protected:
	CIntro*	m_pIntro;
	CStatic m_sttDts;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
