#pragma once


// CFactPanel

class CFactPanel : public CWnd
{
	DECLARE_DYNAMIC(CFactPanel)

public:
	CFactPanel(CWnd* pParnet);
	virtual ~CFactPanel();

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID = NULL);

protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnPaint();

private:
	CWnd*		m_pParent;
	//CButton		m_button;
	CFont		m_font;

	//CString		m_strFact;
	CString		m_strTime;
	CDC			m_dc;
	COLORREF	m_crBkgnd;
	COLORREF	m_crText;
	CBrush		m_brBkgnd;
	CBrush		m_brText;
	CBitmap		m_bitmap;
	CBitmap*	m_pBitmap;

	void	InvalidateCtrl();

public:
	//void	Draw(CString strFact, CString strTime);
	void	Draw(CString strTime);
	bool	Copy(CDC* pDC, CPoint org, CSize factor, CRect& fRect);
};


