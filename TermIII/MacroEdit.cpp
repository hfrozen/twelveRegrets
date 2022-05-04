// MacroEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Terminal.h"
#include "MacroEdit.h"
#include "NewMacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMacroEdit dialog


CMacroEdit::CMacroEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CMacroEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMacroEdit)
	//}}AFX_DATA_INIT
	m_nLength = 0;
	m_nDeleteID = -1;
	//m_pParent = pParent;
}

CMacroEdit::CMacroEdit(MACRO* pMacro, CDialog* pParent)
	: CDialog(CMacroEdit::IDD, pParent)
{
	ASSERT(pMacro);
	ASSERT(pParent);

	m_bEdited = FALSE;
	m_nLength = 0;
	m_nDeleteID = -1;
	m_pMacro = pMacro;
	m_pParent = pParent;
}

void CMacroEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMacroEdit)
	DDX_Control(pDX, IDC_MACRO_LIST, m_ctrlList);
	//}}AFX_DATA_MAP
}

void CMacroEdit::Append(int nIndex, UINT nAtt, UINT nKey, CString strContent)
{
	CString strTemp;
	CString strHead = _T("");
	if (nAtt & SHIFT_ATT)	strHead += _T("SHIFT+");
	if (nAtt & CONTROL_ATT)	strHead += _T("CTRL+");
	if (nAtt & ALT_ATT)		strHead += _T("ALT+");
	if (nKey >= VK_F1 && nKey <= VK_F12)
		strTemp.Format(_T("F%d"), nKey - VK_F1 + 1);
	else	strTemp.Format(_T("%c"), nKey);
	strHead += strTemp;

	LV_ITEM listItem;
	for (int i = 0; i < 2; i ++) {
		listItem.mask = LVIF_TEXT;
		listItem.iItem = nIndex;
		listItem.iSubItem = i;
		listItem.pszText = i ? strContent.GetBuffer(50) : strHead.GetBuffer(50);
		if (i)	m_ctrlList.SetItem(&listItem);
		else	m_ctrlList.InsertItem(&listItem);
	}
}

BEGIN_MESSAGE_MAP(CMacroEdit, CDialog)
	//{{AFX_MSG_MAP(CMacroEdit)
	ON_BN_CLICKED(IDC_MACRO_NEW, OnMacroNew)
	ON_BN_CLICKED(IDC_MACRO_DEL, OnMacroDel)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MACRO_LIST, OnItemchangedMacroList)
	ON_BN_CLICKED(IDC_MACRO_END, OnMacroEnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacroEdit message handlers

#define NAME_WIDTH	100

BOOL CMacroEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	LV_COLUMN listColumn;
	m_ctrlList.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
	m_ctrlList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	CRect rect;
	m_ctrlList.GetClientRect(rect);

	for (int i = 0; i < 2; i ++) {
		listColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		listColumn.fmt = LVCFMT_LEFT;
		CString str = i ? _T("String") : _T("Key");
		listColumn.pszText = str.GetBuffer(50);
		listColumn.iSubItem = i;
		listColumn.cx = i ? rect.right - (rect.left + NAME_WIDTH) : NAME_WIDTH;
		m_ctrlList.InsertColumn(i, &listColumn);
	}

	for (m_nLength = 0; (m_pMacro[m_nLength]).nKey != 0; m_nLength ++)
		Append(m_nLength, (m_pMacro[m_nLength]).nAtt,
			(m_pMacro[m_nLength]).nKey,
			(m_pMacro[m_nLength]).strContent);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMacroEdit::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_bEdited) {
		int nCount = m_ctrlList.GetItemCount();
		CString str;
		int n = 0;
		for (int i = 0; i < nCount; i ++) {
			str = m_ctrlList.GetItemText(i, 0);
			(m_pMacro[n]).nAtt = 0;
			if (str.Find(_T("SHIFT+")) > -1)	(m_pMacro[n]).nAtt |= SHIFT_ATT;
			if (str.Find(_T("CTRL+")) > -1)		(m_pMacro[n]).nAtt |= CONTROL_ATT;
			if (str.Find(_T("ALT+")) > -1)		(m_pMacro[n]).nAtt |= ALT_ATT;
			(m_pMacro[n]).nKey = 0;
			int j;
			do {
				j = str.Find(_T('+'));
				if (j > -1)	str = str.Mid(j + 1);
			} while (j > -1);
			if (str.GetLength() < 2) {
				(m_pMacro[n]).nKey = str.GetAt(0);
			}
			else if (str.GetAt(0) == 'F') {
				str = str.Mid(1);
				j = atoi((LPCTSTR)str);
				if (j > 0 && j < 13)
					(m_pMacro[n]).nKey = (VK_F1 + (j - 1));
			}
			(m_pMacro[n]).strContent = m_ctrlList.GetItemText(i, 1);
			(m_pMacro[n + 1]).nKey = 0;
			if ((m_pMacro[n]).nKey != 0)	++ n;
		}
	}
}

void CMacroEdit::OnMacroNew() 
{
	// TODO: Add your control notification handler code here
	if (m_nLength < (MAX_MACRO - 1)) {
		CNewMacro dlg(this);
		if (dlg.DoModal() == IDOK) {
			if (dlg.m_strMacro.IsEmpty() || dlg.m_nKey == 0) {
				MessageBox(_T("추가되는 내용이 없습니다."), _T("매크로 추가 오류"),
					MB_OK | MB_ICONQUESTION);
				return;
			}
			Append(m_nLength ++, dlg.m_nAtt, dlg.m_nKey, dlg.m_strMacro);
			m_bEdited = TRUE;
		}
	}
	else	MessageBox(_T("매크로 버퍼가 없습니다."), _T("매크로 추가 오류"),
				MB_OK | MB_ICONQUESTION);
}

void CMacroEdit::OnMacroDel() 
{
	// TODO: Add your control notification handler code here
	if (m_nDeleteID > -1) {
		m_ctrlList.DeleteItem(m_nDeleteID);
		m_nDeleteID = -1;
		if (m_nLength > 0)	-- m_nLength;
		m_bEdited = TRUE;
	}
	else	MessageBox(_T("삭제할 내용을 지정하십시오"), _T("매크로 삭제 오류"),
		MB_OK | MB_ICONWARNING);
}

void CMacroEdit::OnItemchangedMacroList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (m_nDeleteID != pNMListView->iItem)
		m_nDeleteID = pNMListView->iItem;
	*pResult = 0;
}


BOOL CMacroEdit::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_INSERT) {
			OnMacroNew();
			return TRUE;
		}
		else if (pMsg->wParam == VK_DELETE) {
			OnMacroDel();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CMacroEdit::OnMacroEnd() 
{
	// TODO: Add your control notification handler code here
	DestroyWindow();
	if (m_pParent != NULL)	m_pParent->SendMessage(WM_CLOSEMACRO, IDOK);
}
