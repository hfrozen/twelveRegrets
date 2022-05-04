#pragma once

#include "Log.h"

class CDoc : public CWnd
{
	DECLARE_DYNAMIC(CDoc)

public:
	CDoc();
	virtual ~CDoc();

private:
	CWnd*	m_pParent;
	bool	m_bArticle;
	static ARTICLE	m_article;

#define	MAX_ARTICLEITEM	10
	static const ARTICLEITEM	m_articleItems[MAX_ARTICLEITEM];

	void	Log(CString strLog, COLORREF crText = COLOR_BLACK);

public:
	CString	CaptureText(PCWSTR pstrCapture, CString strText);
	void	OpenError(CString strFile, bool bDir, bool bRead);
	bool	MakeDirectory(CString strDirectory);
	CString	MakePath(CString strPath);
	CString	LoadFile(CString strFile);
	void	SaveFile(CString strFile, CString strContext, bool bTail = false);
	void	LoadRule();
	void	SaveRule();
	bool	GetArticleState()			{ return m_bArticle; }
	void	SetParent(CWnd* pParent)	{ m_pParent = pParent; }

protected:
	DECLARE_MESSAGE_MAP()
};


