#pragma once

#define	WM_FINDTROUBLECODE		WM_USER + 12
#define	WM_CLOSEFINDTROUBLE		WM_USER + 14

#define SEARCH_QWORDCODE		2000

class CFindTroubles : public CDialogEx
{
	DECLARE_DYNAMIC(CFindTroubles)

public:
	CFindTroubles(CWnd* pParent = NULL);
	CFindTroubles(CWnd* pParent, bool bFolder);
	virtual ~CFindTroubles();

	enum { IDD = IDD_DIALOG_FINDTROUBLES };

private:
	CWnd*	m_pParent;
	bool	m_bFindFolder;
	CEdit	m_editFolder;
	CButton	m_btnFolder;
	CString	m_strTrouble;
	CString	m_strFindPath;
	int		m_nFindLength;

	void	Log(CString strLog, bool bPos = false);
	bool	IsSysFolder(CString strPath, TCHAR* pFile);
	bool	SearchCode(UINT uCode, CString strPath, TCHAR* pFile);
	bool	SearchFiles(UINT uCode, CString strParent);

public:
	void	SearchTrouble();
	void	SetFindPath(CString strPath)		{ m_strFindPath = strPath; }
	CString	GetFindPath()						{ return m_strFindPath; }
	void	SetTroubleText(CString strTrouble)	{ m_strTrouble = strTrouble; }
	CString GetTroubleText()					{ return m_strTrouble; }
	bool	GetFolderType()						{ return m_bFindFolder; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedOK();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonFolder();
	afx_msg void OnBnClickedButtonFind();
	afx_msg void OnBnClickedButtonFindtroublesFind();
};
