// SelectTrouble.cpp
#include "stdafx.h"
#include "Tranzx.h"
#include "SelectTrouble.h"
#include "afxdialogex.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "TranzxView.h"

IMPLEMENT_DYNAMIC(CSelectTrouble, CDialogEx)

CSelectTrouble::CSelectTrouble(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSelectTrouble::IDD, pParent)
{
	m_pParent = pParent;
	m_nSelect = 0;
}

CSelectTrouble::~CSelectTrouble()
{
}

void CSelectTrouble::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_EVENT, m_btnEvent);
	DDX_Control(pDX, IDC_CHECK_STATUS, m_btnStatus);
	DDX_Control(pDX, IDC_CHECK_LIGHT, m_btnLight);
	DDX_Control(pDX, IDC_CHECK_SEVERE, m_btnSevere);
}

BEGIN_MESSAGE_MAP(CSelectTrouble, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ARRANGE, &CSelectTrouble::OnBnClickedButtonArrange)
END_MESSAGE_MAP()

void CSelectTrouble::OnBnClickedButtonArrange()
{
	m_nSelect = CapSelectState();
	if (m_nSelect == 0)	return;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame* pChild = (CChildFrame*)pFrame->GetActiveFrame();
	CTranzxView* pView = (CTranzxView*)pChild->GetActiveView();
	if (pView != NULL)	pView->SendMessage(WM_SELECTTROUBLE, (WPARAM)m_nSelect);
}

UINT CSelectTrouble::CapSelectState()
{
	UINT n = 0;
	if (m_btnEvent.GetCheck() == BST_CHECKED)	n |= 1;
	if (m_btnStatus.GetCheck() == BST_CHECKED)	n |= 2;
	if (m_btnLight.GetCheck() == BST_CHECKED)	n |= 4;
	if (m_btnSevere.GetCheck() == BST_CHECKED)	n |= 8;
	return n;
}

void CSelectTrouble::SetSelectState(UINT nSel)
{
	nSel &= 0xf;
	if (nSel == m_nSelect)	return;
	m_btnEvent.SetCheck((nSel & 1) ? BST_CHECKED : BST_UNCHECKED);
	m_btnStatus.SetCheck((nSel & 2) ? BST_CHECKED : BST_UNCHECKED);
	m_btnLight.SetCheck((nSel & 4) ? BST_CHECKED : BST_UNCHECKED);
	m_btnSevere.SetCheck((nSel & 8) ? BST_CHECKED : BST_UNCHECKED);
	m_nSelect = nSel;
	UpdateData(false);
}
