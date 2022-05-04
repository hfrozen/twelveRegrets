// Rule.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Rule.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CRule, CDialogEx)

CRule::CRule(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRule::IDD, pParent)
{
	m_pParent = pParent;
	ZeroMemory(&m_article, sizeof(ARTICLE));
}

CRule::~CRule()
{
}

void CRule::MakeCheckList(CAidChkList* pList, int col, WORD* pW)
{
	LV_COLUMN lc;
	for (int n = 0; n < (col + 1); n ++) {
		lc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lc.fmt = LVCFMT_CENTER;
		CString str;
		if (!n) {
			str = L"";
			lc.cx = pList->GetStringWidth(L"WWWWWW");
		}
		else {
			str.Format(L"%d", n - 1);
			lc.cx = pList->GetStringWidth(L"WW");
		}
		lc.pszText = str.GetBuffer(str.GetLength());
		lc.iSubItem = n;
		pList->InsertColumn(n, &lc);
		str.ReleaseBuffer();
	}
	LV_ITEM li;
	li.mask = LVIF_TEXT;
	for (int n = 0; n < 2; n ++) {
		for (int m = 0; m < (col + 1); m ++) {
			li.iItem = n;
			li.iSubItem = m;
			CString str;
			if (!m)	str = n ? L"FALL" : L"RISE";
			else	str = (*pW & (1 << (m - 1))) ? L"1" : L"0";
			li.pszText = str.GetBuffer(str.GetLength());
			if (!m)	pList->InsertItem(&li);
			else	pList->SetItem(&li);
			str.ReleaseBuffer();
		}
		++ pW;
	}
}

void CRule::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRule, CDialogEx)
	ON_BN_CLICKED(IDOK, &CRule::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CRule::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < 4; n ++) {
		m_list[n].SubclassDlgItem(IDC_LIST_RULE0 + n, this);
		m_list[n].SetExtendedStyle(LVS_EX_GRIDLINES);
		m_list[n].GetHeaderCtrl()->EnableWindow(false);
		m_list[n].ShowWindow(SW_SHOW);
		MakeCheckList(&m_list[n], 10, &(m_article.wFbSig[n][0]));
	}

	return TRUE;
}

BOOL CRule::PreTranslateMessage(MSG* pMsg)
{
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CRule::OnBnClickedOk()
{
	for (int n = 0; n < 4; n ++) {
		m_article.wFbSig[n][0] = m_article.wFbSig[n][1] = 0;
		for (int m = 0; m < 2; m ++) {
			for (int k = 0; k < 10; k ++) {
				CString str = m_list[n].GetItemText(m, k + 1);
				if (str != L"0")	m_article.wFbSig[n][m] |= (WORD)(1 << k);
			}
		}
	}
	CDialogEx::OnOK();
}
