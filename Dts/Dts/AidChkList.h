#pragma once

class CAidChkList : public CListCtrl
{
	DECLARE_DYNAMIC(CAidChkList)

public:
	CAidChkList();
	CAidChkList(CWnd* pParent);
	virtual ~CAidChkList();

private:
	CWnd*	m_pParent;

private:
	void	InvalidateGrid(int row, int col);

public:
	int		HitTestEx(CPoint& pt, int* pCol);
	void	SetParent(CWnd* pParent)	{ m_pParent = pParent; }

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


