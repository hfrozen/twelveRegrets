// Car.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Car.h"
#include "afxdialogex.h"

#include "Intro.h"

#define	SETUPGATE_TIMER		2
#define	TIME_SETUPGATE		100

#define	ECUPRESSURE_MAX		20

IMPLEMENT_DYNAMIC(CCar, CDialogEx)

CCar::CCar(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCar::IDD, pParent)
{
	m_pParent = pParent;
	m_nCID = -1;
	m_cDi = m_cDo = 0;
	m_pSiv = NULL;
	m_pV3f = NULL;
}

CCar::CCar(CWnd* pParent, int nID)
	: CDialogEx(CCar::IDD, pParent)
{
	m_pParent = pParent;
	m_nCID = nID;
	m_cDi = 0;
	m_pSiv = NULL;
	m_pV3f = NULL;
}

CCar::~CCar()
{
	KILL(m_pSiv);
	KILL(m_pV3f);
}

void CCar::BitCtrlDG(UINT id, int nState)
{
	if ((id >= CUDIB_LD0 && id <= CUDIB_LD3) || (id >= CUDIB_RD0 && id <= CUDIB_RD3)) {
		PBODYSTYLE pBs = m_body.GetStyle();
		CARFORM_DOOR door[8];
		memcpy(&door, &(pBs->door), sizeof(CARFORM_DOOR) * 8);

		int n = 0;
		if (id >= CUDIB_LD0 && id <= CUDIB_LD3)	n = id - CUDIB_LD0;
		else	n = id - CUDIB_RD0 + 4;

		//int m = m_nCID < 9 ? n : 7 - n;
		int m = n;
		door[m] = nState != 0 ? CARFORM_DOOROP : CARFORM_DOORCL;
		if (m < 4)	BitCtrlD(CUDIB_LD0 + m, nState != 0 ? BST_CHECKED : BST_UNCHECKED);
		else	BitCtrlD(CUDIB_RD0 + m - 4, nState != 0 ? BST_CHECKED : BST_UNCHECKED);
		m_body.SetDoor(door, true);
	}
	else if (id == CUDIB_DIS) {
		CARFORM_BP bypass = nState != 0 ? CARFORM_BPON : CARFORM_BPNR;
		BitCtrlD(CUDIB_DIS, nState != 0 ? BST_CHECKED : BST_UNCHECKED);
		m_body.SetBypass(bypass, true);
	}
	else if (id == CUDIB_EED || id == CUDIB_EAD) {
	}
	else if (id == CUDIB_EDF) {
	}
	else if (id == CUDIB_PANPS1 || id == CUDIB_PANPS2) {
		PBODYSTYLE pBs = m_body.GetStyle();
		CARFORM_PANTO pan[2];
		memcpy(&pan, &(pBs->pan), sizeof(CARFORM_PANTO) * 2);

		bool bChg = false;
		int n = id == CUDIB_PANPS1 ? 0 : 1;
		pan[n] = nState != 0 ? CARFORM_PANUP : CARFORM_PANDN;
		BitCtrlD(id, nState != 0 ? BST_UNCHECKED : BST_CHECKED);
		m_body.SetPanto(pan, true);
	}
	else	BitCtrlD(id, nState);
}

void CCar::BitCtrlD(UINT id, int nState)
{
	BitCtrl(UDIB2IDC(id), nState);
}

void CCar::BitCtrl(UINT id, int nState)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	CButton* pButton = (CButton*)GetDlgItem(id);
	int check = nState;
	if (pButton != NULL) {
		if (nState == BST_UNCHECKED || nState == BST_CHECKED)
			pButton->SetCheck(nState);
		check = pButton->GetCheck();
	}
	pIntro->BitCtrl(m_nCID, id, check == BST_CHECKED ? true : false);
}

//void CCar::EcuPressureCtrl(bool bFrom)
//{
//	CString str;
//	if (bFrom) {
//		int pos = m_sliderEcuPress.GetPos();
//		str.Format(L"%02d", ECUPRESSURE_MAX - pos);
//		m_editEcuPress.SetWindowTextW(str);
//	}
//	else {
//		m_editEcuPress.GetWindowTextW(str);
//		int pos = atoi((CT2CA)str);
//		if (pos > ECUPRESSURE_MAX)	pos %= ECUPRESSURE_MAX;
//		str.Format(L"%02d", pos);
//		m_editEcuPress.SetWindowTextW(str);
//		m_sliderEcuPress.SetPos(ECUPRESSURE_MAX - pos);
//	}
//}
//
void CCar::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
	m_body.Ready(bCmd);
	if (bCmd) {
		BitCtrlD(CUDIB_DIS, BST_CHECKED);
		BitCtrlD(CUDIB_ECUMF, BST_CHECKED);
		BitCtrlD(CUDIB_BRCS, BST_CHECKED);
		BitCtrlD(CUDIB_BCCS, BST_CHECKED);
		BitCtrlD(CUDIB_ASCS, BST_CHECKED);
		BitCtrlD(CUDIB_FDUF, BST_CHECKED);
		BitCtrlD(CUDIB_MDS, BST_CHECKED);
		if (m_wDevs & DEVF_SIV)	BitCtrlD(CUDIB_SIVK, BST_CHECKED);
		if (m_wDevs & DEVF_PAN) {
			BitCtrlD(CUDIB_PANPS1, BST_CHECKED);
			BitCtrlD(CUDIB_PANPS2, BST_CHECKED);
		}
		//EcuPressureCtrl(true);
	}
}

void CCar::SetBody(BODYSTYLE bs)
{
	//m_nCID = bs.nID;
	m_body.SetStyle(bs);
	CString str;
	str.Format(L"CAR %d", m_nCID);
	m_dcu[0].SetCarName(str);
	m_dcu[1].SetCarName(str);
	if (m_wDevs & DEVF_SIV)	m_pSiv->SetCarName(str);
	if (m_wDevs & DEVF_V3F)	m_pV3f->SetCarName(str);
	m_ecu.SetCarName(str);
	m_hvac.SetCarName(str);

	//str.Format(L"%d", m_nCID);
	//m_editCID.SetWindowTextW(str);
	//m_sliderEcuPress.SetPos(ECUPRESSURE_MAX - 12);
}

void CCar::SetDevices()
{
	m_btnSiv.ShowWindow((m_wDevs & DEVF_SIV) ? SW_SHOW : SW_HIDE);
	m_checkSivk.ShowWindow((m_wDevs & DEVF_SIV) ? SW_SHOW : SW_HIDE);
	m_checkSivf.ShowWindow((m_wDevs & DEVF_SIV) ? SW_SHOW : SW_HIDE);
	m_checkBvr.ShowWindow((m_wDevs & DEVF_SIV) ? SW_SHOW : SW_HIDE);
	m_sttSiv.ShowWindow((m_wDevs & DEVF_SIV) ? SW_SHOW : SW_HIDE);

	m_btnV3f.ShowWindow((m_wDevs & DEVF_V3F) ? SW_SHOW : SW_HIDE);
	m_checkVff.ShowWindow((m_wDevs & DEVF_V3F) ? SW_SHOW : SW_HIDE);
	m_checkIes.ShowWindow((m_wDevs & DEVF_V3F) ? SW_SHOW : SW_HIDE);
	m_sttV3f.ShowWindow((m_wDevs & DEVF_V3F) ? SW_SHOW : SW_HIDE);

	m_checkCmf.ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);
	m_checkCmbp.ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);
	m_checkCmk.ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);
	m_checkCmg.ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);
	m_checkCmn.ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);
	m_lampCmk.ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);
	m_sttCmk.ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);
	m_sttCmsb.ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);
	m_checkNest[3].ShowWindow((m_wDevs & DEVF_CM) ? SW_SHOW : SW_HIDE);

	m_checkEsk.ShowWindow((m_wDevs & DEVF_ESK) ? SW_SHOW : SW_HIDE);
	m_lampEsk.ShowWindow((m_wDevs & DEVF_ESK) ? SW_SHOW : SW_HIDE);
	m_sttEsk.ShowWindow((m_wDevs & DEVF_ESK) ? SW_SHOW : SW_HIDE);
	m_checkNest[2].ShowWindow((m_wDevs & DEVF_ESK) ? SW_SHOW : SW_HIDE);

	m_checkIpdr.ShowWindow((m_wDevs & DEVF_PAN) ? SW_SHOW : SW_HIDE);

	/*m_checkUops.ShowWindow((m_wDevs & DEVF_TERM) ? SW_SHOW : SW_HIDE);
	m_checkUos.ShowWindow((m_wDevs & DEVF_TERM) ? SW_SHOW : SW_HIDE);
	m_checkMrcs.ShowWindow((m_wDevs & DEVF_TERM) ? SW_SHOW : SW_HIDE);*/
}

void CCar::SetPanto(bool bCtrl)
{
	PBODYSTYLE pBs = m_body.GetStyle();
	CARFORM_PANTO pan[2];
	memcpy(&pan, &(pBs->pan), sizeof(CARFORM_PANTO) * 2);

	bool bChg = false;
	for (int n = 0; n < 2; n ++) {
		if (pan[n] != CARFORM_PANBP) {
			if ((pan[n] == CARFORM_PANUP && !bCtrl) || (pan[n] != CARFORM_PANUP && bCtrl)) {
				pan[n] = bCtrl ? CARFORM_PANUP : CARFORM_PANDN;
				BitCtrlD(CUDIB_PANPS1 + n, bCtrl ? BST_UNCHECKED : BST_CHECKED);
				bChg = true;
			}
		}
	}
	if (bChg)	m_body.SetPanto(pan, true);
}

void CCar::SetDoor(DOORCMD cmd, bool bSide)
{
	PBODYSTYLE pBs = m_body.GetStyle();
	CARFORM_DOOR door[8];
	memcpy(&door, &(pBs->door), sizeof(CARFORM_DOOR) * 8);

	bool bChg;
	bChg = false;
	for (int n = 0; n < 8; n ++) {
		int m = m_nCID < 9 ? n : 7 - n;
		if (door[m] != CARFORM_DOORBP) {
			if (cmd == DOORCMD_CLS && door[m] != CARFORM_DOORCL) {
				door[m] = CARFORM_DOORCL;
				if (m < 4)	BitCtrlD(CUDIB_LD0 + m, BST_UNCHECKED);
				else	BitCtrlD(CUDIB_RD0 + (m - 4), BST_UNCHECKED);
				bChg = true;
			}
			else if ((cmd == DOORCMD_LOPEN && n < 4 && door[m] == CARFORM_DOORCL) ||
				(cmd == DOORCMD_ROPEN && n >= 4 && door[m] == CARFORM_DOORCL)) {
				door[m] = CARFORM_DOOROP;
				if (m < 4)	BitCtrlD(CUDIB_LD0 + m, BST_CHECKED);
				else	BitCtrlD(CUDIB_RD0 + (m - 4), BST_CHECKED);
				bChg = true;
			}
		}
	}
	if (bChg)	m_body.SetDoor(door, true);
	if (m_nCID == 9)	bSide ^= true;
	m_dcu[bSide ? 1 : 0].SetFullOpen(cmd == DOORCMD_CLS ? false : true);
}

void CCar::DiReaction(BYTE ch)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	ch &= 0x1f;
	if (m_cDi != ch) {
		PBODYSTYLE pBody = m_body.GetStyle();
		for (int n = 0; n < 5; n ++) {
			if ((m_cDi & (1 << n)) != (ch & (1 << n))) {
				CButton* pCheck;
				switch (n) {
				case 0 :	m_body.SetLamp((pBody->nLamp & 0xfe) | (ch & 1), true);	break;
				case 1 :
					m_body.SetLamp((pBody->nLamp & 0xfd) | (ch & 2), true);
					pCheck = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
					if (pCheck == NULL || pCheck->GetCheck() == BST_CHECKED)
						BitCtrlD(CUDIB_LK1, (ch & 2) ? BST_CHECKED : BST_UNCHECKED);
					break;
				case 2 :
					m_body.SetLamp((pBody->nLamp & 0xfb) | (ch & 4), true);
					pCheck = (CButton*)GetDlgItem(IDC_CHECK_ARROW1);
					if (pCheck == NULL || pCheck->GetCheck() == BST_CHECKED)
						BitCtrlD(CUDIB_LK2, (ch & 4) ? BST_CHECKED : BST_UNCHECKED);
					break;
				case 3 :
					if (m_wDevs & DEVF_CM) {
						m_lampCmk.SetState((ch & 8) ? LS_ON : LS_OFF);
						pCheck = (CButton*)GetDlgItem(IDC_CHECK_ARROW2);
						if (pCheck == NULL || pCheck->GetCheck() == BST_CHECKED)
							BitCtrlD(CUDIB_CMK, (ch & 8) ? BST_CHECKED : BST_UNCHECKED);
					}
					break;
				case 4 :
					if (m_wDevs & DEVF_ESK) {
						m_lampEsk.SetState((ch & 0x10) ? LS_ON : LS_OFF);
						pCheck = (CButton*)GetDlgItem(IDC_CHECK_ARROW3);
						if (pCheck == NULL || pCheck->GetCheck() == BST_CHECKED)
							BitCtrlD(CUDIB_ESK, (ch & 0x10) ? BST_CHECKED : BST_UNCHECKED);
					}
					break;
				default :	break;
				}
			}
		}
		m_cDi = ch;
	}
}

void CCar::FeedIn(CButton* pButton, UINT id, bool bChk)
{
	if (pButton->GetCheck() == BST_CHECKED) {
		CButton* pButton = (CButton*)GetDlgItem(id);
		if (pButton == NULL)	return;
		pButton->SetCheck(bChk ? BST_CHECKED : BST_UNCHECKED);
		BitCtrl(id);
	}
}

void CCar::DoReaction(BYTE cDO)
{
	for (int n = 0; n < 8; n ++) {
		if ((cDO & (1 << n)) && !(m_cDo & (1 << n))) {	// on
			m_cDo |= (1 << n);
			if (n < 3) {
				m_body.SetLampA(1 << n, true, true);
				if (n == 1)	FeedIn(&m_checkNest[0], IDC_CHECK_DEV44, true);
				else if (n == 2)	FeedIn(&m_checkNest[1], IDC_CHECK_DEV45, true);
			}
			else if (n == 3 && (m_wDevs & DEVF_ESK)) {
				m_lampEsk.SetState(LS_ON);
				FeedIn(&m_checkNest[2], IDC_CHECK_DEV29, true);
			}
			else if (n == 4 && (m_wDevs & DEVF_CM)) {
				m_lampCmk.SetState(LS_ON);
				FeedIn(&m_checkNest[3], IDC_CHECK_DEV37, true);
			}
		}
		else if (!(cDO & (1 << n)) && (m_cDo & (1 << n))) {	// off
			m_cDo &= ~(1 << n);
			if (n < 3) {
				m_body.SetLampA(1 << n, false, true);
				if (n == 1)	FeedIn(&m_checkNest[0], IDC_CHECK_DEV44, false);
				else if (n == 2)	FeedIn(&m_checkNest[1], IDC_CHECK_DEV45, false);
			}
			else if (n == 3 && (m_wDevs & DEVF_ESK)) {
				m_lampEsk.SetState(LS_OFF);
				FeedIn(&m_checkNest[2], IDC_CHECK_DEV29, false);
			}
			else if (n == 4 && (m_wDevs & DEVF_CM)) {
				m_lampCmk.SetState(LS_OFF);
				FeedIn(&m_checkNest[3], IDC_CHECK_DEV37, false);
			}
		}
	}
}

void CCar::SdrReaction(BYTE* p, int leng)
{
	WORD wAdd = MAKEWORD(*(p + 1), *(p + 2));
	int ch = (int)(*p & 0xf);
	++ p;
	-- leng;
	switch (ch) {
	case 0 :	// HVAC
		ch ++;
		if (wAdd == LADD_HVAC)	m_hvac.RecvSdr(p, leng);
		break;
	case 1 :	// ECU
		if (wAdd == LADD_ECU)	m_ecu.RecvSdr(p, leng);
		break;
	case 2 :	// SIV/V3F
		if (wAdd == LADD_SIV && m_pSiv != NULL)	m_pSiv->RecvSdr(p, leng);
		else if (wAdd == LADD_V3F && m_pV3f != NULL)	m_pV3f->RecvSdr(p, leng);
		break;
	case 3 :
	case 4 :	// DCU
		if (((*(p + 1) >= 1 && *(p + 1) <= 4) || (*(p + 1) >= 0x11 && *(p + 1) <= 0x14)) && *p == 0x70) {
			m_dcu[*(p + 1) >= 1 && *(p + 1) <= 4 ? 1 : 0].RecvSdr(p, leng);
		}
		break;
	default :	break;
	}
}

void CCar::SetupGates()
{
	m_nGateID = 0;
	SetTimer(SETUPGATE_TIMER, TIME_SETUPGATE, NULL);
}

CPanel* CCar::GetPanel()
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	return pIntro->GetPanel();
}

void CCar::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_EDIT_DEV0, m_editCID);
	//DDX_Control(pDX, IDC_EDIT_DEV1, m_editEcuPress);
	//DDX_Control(pDX, IDC_SLIDER_DEV0, m_sliderEcuPress);
	DDX_Control(pDX, IDC_BUTTON_DEV0, m_btnHidden);
	DDX_Control(pDX, IDC_BUTTON_DEV1, m_btnDcuL);
	DDX_Control(pDX, IDC_BUTTON_DEV2, m_btnDcuR);
	DDX_Control(pDX, IDC_BUTTON_DEV3, m_btnHvac);
	DDX_Control(pDX, IDC_BUTTON_DEV4, m_btnEcu);
	DDX_Control(pDX, IDC_BUTTON_DEV5, m_btnV3f);
	DDX_Control(pDX, IDC_BUTTON_DEV6, m_btnSiv);
	DDX_Control(pDX, IDC_STATIC_DEV0, m_sttEsk);
	DDX_Control(pDX, IDC_STATIC_DEV1, m_sttCmk);
	DDX_Control(pDX, IDC_STATIC_DEV2, m_sttV3f);
	DDX_Control(pDX, IDC_STATIC_DEV3, m_sttSiv);
	DDX_Control(pDX, IDC_STATIC_DEV4, m_sttCmsb);
	DDX_Control(pDX, IDC_CHECK_DEV26, m_checkUops);
	DDX_Control(pDX, IDC_CHECK_DEV27, m_checkMrcs);
	DDX_Control(pDX, IDC_CHECK_DEV28, m_checkUos);
	DDX_Control(pDX, IDC_CHECK_DEV29, m_checkEsk);
	DDX_Control(pDX, IDC_CHECK_DEV30, m_checkVff);
	DDX_Control(pDX, IDC_CHECK_DEV31, m_checkIes);
	DDX_Control(pDX, IDC_CHECK_DEV32, m_checkBvr);
	DDX_Control(pDX, IDC_CHECK_DEV33, m_checkSivk);
	DDX_Control(pDX, IDC_CHECK_DEV34, m_checkSivf);
	DDX_Control(pDX, IDC_CHECK_DEV35, m_checkCmf);
	DDX_Control(pDX, IDC_CHECK_DEV36, m_checkCmbp);
	DDX_Control(pDX, IDC_CHECK_DEV37, m_checkCmk);
	DDX_Control(pDX, IDC_CHECK_DEV38, m_checkCmg);
	DDX_Control(pDX, IDC_CHECK_DEV39, m_checkCmn);
	DDX_Control(pDX, IDC_CHECK_DEV42, m_checkIpdr);
	DDX_Control(pDX, IDC_CHECK_ARROW0, m_checkNest[0]);
	DDX_Control(pDX, IDC_CHECK_ARROW1, m_checkNest[1]);
	DDX_Control(pDX, IDC_CHECK_ARROW2, m_checkNest[2]);
	DDX_Control(pDX, IDC_CHECK_ARROW3, m_checkNest[3]);
}

BEGIN_MESSAGE_MAP(CCar, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_MESSAGE(WM_BODYCLICK, &CCar::OnBodyClick)
END_MESSAGE_MAP()

BOOL CCar::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT(m_nCID > -1);

	m_btnHidden.ShowWindow(SW_HIDE);

	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_HIDDEN0);
	pStatic->ShowWindow(SW_HIDE);
	CRect rc, rt;
	pStatic->GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_body.Create(WS_VISIBLE | WS_CHILD, rc, this);
	rc = m_body.GetRect();

	//m_editCID.SetWindowTextW(L"99");

	CRect rtl;
	pStatic = (CStatic*)GetDlgItem(IDC_STATICLP_DEV0);
	if (pStatic != NULL) {
		pStatic->GetWindowRect(&rtl);
		ScreenToClient(&rtl);
		pStatic->ShowWindow(SW_HIDE);
		m_lampEsk.Create(L"", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rtl, this);
		m_lampEsk.EnableWindow(true);
		m_lampEsk.ShowWindow(SW_SHOW);
		m_lampEsk.Set(0, LS_OFF);
	}
	pStatic = (CStatic*)GetDlgItem(IDC_STATICLP_DEV1);
	if (pStatic != NULL) {
		pStatic->GetWindowRect(&rtl);
		ScreenToClient(&rtl);
		pStatic->ShowWindow(SW_HIDE);
		m_lampCmk.Create(L"", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rtl, this);
		m_lampCmk.EnableWindow(true);
		m_lampCmk.ShowWindow(SW_SHOW);
		m_lampCmk.Set(1, LS_OFF);
	}
	//pStatic = (CStatic*)GetDlgItem(IDC_STATICLP_DEV2);
	//if (pStatic != NULL) {
	//	pStatic->GetWindowRect(&rtl);
	//	ScreenToClient(&rtl);
	//}

	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV30);
	if (pButton != NULL) {
		pButton->GetWindowRect(&rtl);
		ScreenToClient(&rtl);
		CRect rty;
		m_btnEcu.GetWindowRect(&rty);
		ScreenToClient(&rty);
		m_btnSiv.SetWindowPos(NULL, rty.left, rtl.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		m_btnV3f.SetWindowPos(NULL, rty.left, rtl.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	m_wDevs = pIntro->GetDevices(m_nCID);

	m_dcu[0].SetParent(this);
	m_dcu[0].Create(IDD_DIALOG_DDCU, this);
	m_dcu[0].SetTitle(L"CAR", L"DCUL");
	m_dcu[0].EnableWindow(false);
	m_dcu[0].ShowWindow(SW_HIDE);

	m_dcu[1].SetParent(this);
	m_dcu[1].Create(IDD_DIALOG_DDCU, this);
	m_dcu[1].SetTitle(L"CAR", L"DCUR");
	m_dcu[1].EnableWindow(false);
	m_dcu[1].ShowWindow(SW_HIDE);

	//m_sliderEcuPress.SetRange(0, ECUPRESSURE_MAX);
	//m_sliderEcuPress.SetPos(12);
	//m_editEcuPress.SetWindowTextW(L"12");

	if (m_wDevs & DEVF_SIV) {
		m_pSiv = new CDsiv(this);
		m_pSiv->Create(IDD_DIALOG_DSIV, this);
		m_pSiv->SetTitle(L"CAR", L"SIV");
		m_pSiv->EnableWindow(false);
		m_pSiv->ShowWindow(SW_HIDE);
		m_checkUops.EnableWindow(true);
		m_checkUops.ShowWindow(SW_SHOW);
		m_checkMrcs.EnableWindow(true);
		m_checkMrcs.ShowWindow(SW_SHOW);
		m_checkUos.EnableWindow(true);
		m_checkUos.ShowWindow(SW_SHOW);
	}
	else {
		m_checkUops.EnableWindow(false);
		m_checkUops.ShowWindow(SW_HIDE);
		m_checkMrcs.EnableWindow(false);
		m_checkMrcs.ShowWindow(SW_HIDE);
		m_checkUos.EnableWindow(false);
		m_checkUos.ShowWindow(SW_HIDE);
	}

	if (m_wDevs & DEVF_V3F) {
		m_pV3f = new CDv3f(this);
		m_pV3f->Create(IDD_DIALOG_DV3F, this);
		m_pV3f->SetTitle(L"CAR", L"V3F");
		m_pV3f->EnableWindow(false);
		m_pV3f->ShowWindow(SW_HIDE);
	}

	m_ecu.SetParent(this);
	m_ecu.Create(IDD_DIALOG_DECU, this);
	m_ecu.SetTitle(L"CAR", L"ECU");
	m_ecu.EnableWindow(false);
	m_ecu.ShowWindow(SW_HIDE);

	m_hvac.SetParent(this);
	m_hvac.Create(IDD_DIALOG_DHVAC, this);
	m_hvac.SetTitle(L"CAR", L"HVAC");
	m_hvac.EnableWindow(false);
	m_hvac.ShowWindow(SW_HIDE);

	GetClientRect(&rt);
	rt.right = rt.left + rc.Width();
	SetWindowPos(NULL, 0, 0, rt.Width(), rt.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(false);
	}

	SetDevices();
	for (int n = 0; n < 4; n ++)
		m_checkNest[n].SetCheck(BST_CHECKED);

	return TRUE;
}

BOOL CCar::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		//if (pMsg->wParam == VK_RETURN) {
		//	CWnd* pWnd = GetFocus();
		//	HWND hWnd = pWnd->GetSafeHwnd();
		//	if (hWnd == m_editCID.GetSafeHwnd()) {
		//		//CarIDCtrl();
		//	}
		//	else if (hWnd == m_editEcuPress.GetSafeHwnd())	EcuPressureCtrl(false);
		//}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CCar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wNotifyMsg = (int)HIWORD(wParam);
	UINT id = (UINT)LOWORD(wParam);
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	if (wNotifyMsg == CBN_SELCHANGE) {
	}
	else if (wNotifyMsg == EN_KILLFOCUS) {
		//switch (id) {
		//case IDC_EDIT_DEV0 :	/*CarIDCtrl();*/	break;
		//case IDC_EDIT_DEV1 :	EcuPressureCtrl(false);	break;
		//default :	break;
		//}
	}
	else {
		if (id >= IDC_CHECK_DEV0 && id < IDC_CHECK_DEVCASCADE)	BitCtrl(id);
		else if (id == IDC_CHECK_DEVCASCADE) {
		}
		else if (id >= IDC_BUTTON_DEV0 && id <= IDC_BUTTON_DEV7) {
			CButton* pButton = (CButton*)GetDlgItem(id);
			if (pButton != NULL) {
				CRect rc;
				pButton->GetWindowRect(&rc);
				CWnd* pWnd = NULL;
				switch (id) {
				case IDC_BUTTON_DEV1 :
				case IDC_BUTTON_DEV2 :	pWnd = (CWnd*)&m_dcu[id == IDC_BUTTON_DEV1 ? 0 : 1];	break;
				case IDC_BUTTON_DEV3 :	pWnd = (CWnd*)&m_hvac;	break;
				case IDC_BUTTON_DEV4 :	pWnd = (CWnd*)&m_ecu;	break;
				case IDC_BUTTON_DEV5 :	pWnd = (CWnd*)m_pV3f;	break;
				case IDC_BUTTON_DEV6 :	pWnd = (CWnd*)m_pSiv;	break;
				default :	break;
				}
				if (pWnd != NULL && !pWnd->IsWindowVisible()) {
					CRect rcd;
					pWnd->GetWindowRect(&rcd);
					CRect rcw = pIntro->GetWorkRect();
					if ((rc.left + rcd.Width()) > rcw.right)	rc.OffsetRect(-((rc.left + rcd.Width()) - rcw.right), 0);
					if ((rc.top + rcd.Height()) > rcw.bottom)	rc.OffsetRect(0, -((rc.top + rcd.Height()) - rcw.bottom));
					pWnd->SetWindowPos(NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
					pWnd->EnableWindow();
					pWnd->ShowWindow(SW_SHOW);
				}
			}
		}
	}
	return CDialogEx::OnCommand(wParam, lParam);
}

void CCar::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CCar::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CCar::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == SETUPGATE_TIMER) {
		switch (m_nGateID) {
		case 0 :	m_hvac.SetupGate(false, m_nCID, 0);	break;
		case 1 :	m_ecu.SetupGate(false, m_nCID, 1);		break;
		case 2 :	if (m_wDevs & DEVF_SIV)	m_pSiv->SetupGate(false, m_nCID, 2);	break;
		case 3 :	if (m_wDevs & DEVF_V3F)	m_pV3f->SetupGate(false, m_nCID, 2);	break;
		case 4 :	m_dcu[0].SetupGate(false, m_nCID, 5, false);	break;
		case 5 :	m_dcu[1].SetupGate(false, m_nCID, 4, true);	break;
		case 6 :	m_hvac.CollectBuild();	break;
		case 7 :	m_ecu.CollectBuild();	break;
		case 8 :	if (m_wDevs & DEVF_SIV)	m_pSiv->CollectBuild();	break;
		case 9 :	if (m_wDevs & DEVF_V3F)	m_pV3f->CollectBuild();	break;
		case 10 :	m_dcu[0].CollectBuild();	break;
		case 11 :	m_dcu[1].CollectBuild();	break;
		default :	KillTimer(SETUPGATE_TIMER);	break;
		}
		++ m_nGateID;
	}
	CDialogEx::OnTimer(nIDEvent);
}

//void CCar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	CIntro* pIntro = (CIntro*)m_pParent;
//	if (pScrollBar == (CScrollBar*)&m_sliderEcuPress)	EcuPressureCtrl(true);
//
//	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
//}
//
LRESULT CCar::OnBodyClick(WPARAM wParam, LPARAM lParam)
{
	int nButton = (int)HIWORD(wParam);
	int nPart = (int)LOWORD(wParam);

	PBODYSTYLE pBs = m_body.GetStyle();
	if (nPart < CBody::AREA_DOOR) {
		CARFORM_PANTO pan[2];
		memcpy(&pan, &(pBs->pan), sizeof(CARFORM_PANTO) * 2);
		bool bEffect = false;
		if (nButton == 1 && (pan[nPart] == CARFORM_PANDN || pan[nPart] == CARFORM_PANBP)) {
			if (pan[nPart] == CARFORM_PANDN)	pan[nPart] = CARFORM_PANBP;
			else	pan[nPart] = CARFORM_PANDN;
			bEffect = true;
		}
		else if (nButton == 0 && (pan[nPart] == CARFORM_PANDN || pan[nPart] == CARFORM_PANUP)) {
			if (pan[nPart] == CARFORM_PANDN)	pan[nPart] = CARFORM_PANUP;
			else	pan[nPart] = CARFORM_PANDN;
			bEffect = true;
		}
		if (bEffect) {
			m_body.SetPanto(pan, true);
			//BitCtrlD(PANCK1 + nPart, pan[nPart] == CARFORM_PANBP ? BST_CHECKED : BST_UNCHECKED);
			//BitCtrlD(CUDIB_PANPS1 + nPart, pan[nPart] == CARFORM_PANUP ? BST_CHECKED : BST_UNCHECKED);
			BitCtrlD(CUDIB_PANPS1 + nPart, pan[nPart] == CARFORM_PANUP ? BST_UNCHECKED : BST_CHECKED);
		}
	}
	else if (nPart < CBody::AREA_SIDE) {
		CARFORM_DOOR door[8];
		memcpy(&door, &(pBs->door), sizeof(CARFORM_DOOR) * 8);
		int did = nPart - CBody::AREA_DOOR;
		if (nButton == 1) {
			if (door[did] != CARFORM_DOOROP) {
				if (door[did] == CARFORM_DOORBP)	door[did] = CARFORM_DOORCL;
				else	door[did] = CARFORM_DOORBP;
				m_body.SetDoor(door, true);
			}
		}
		else {
			if (door[did] != CARFORM_DOORBP) {
				if (door[did] == CARFORM_DOORCL)	door[did] = CARFORM_DOOROP;
				else	door[did] = CARFORM_DOORCL;
				m_body.SetDoor(door, true);
				if (did < 4)	BitCtrlD(CUDIB_LD0 + did, door[did] == CARFORM_DOOROP ? BST_CHECKED : BST_UNCHECKED);
				else	BitCtrlD(CUDIB_RD0 + (did - 4), door[did] == CARFORM_DOOROP ? BST_CHECKED : BST_UNCHECKED);
			}
		}
	}
	else if (nPart < CBody::AREA_BYPASS) {
		CARFORM_SIDE side;
		memcpy(&side, &(pBs->side), sizeof(CARFORM_SIDE));
		if (side == CARFORM_SIDENR)	side = CARFORM_SIDEFL;
		else	side = CARFORM_SIDENR;
		m_body.SetSide(side, true);
		//BitCtrlD(CUDIB_EDF + did, side[did] == CARFORM_SIDEFL ? BST_CHECKED : BST_UNCHECKED);
		BitCtrlD(CUDIB_EDF, side == CARFORM_SIDEFL ? BST_CHECKED : BST_UNCHECKED);
	}
	else if (nPart == CBody::AREA_BYPASS) {
		CARFORM_BP bp = pBs->bypass;
		if (bp == CARFORM_BPNR)	bp = CARFORM_BPON;
		else	bp = CARFORM_BPNR;
		m_body.SetBypass(bp, true);
		BitCtrlD(CUDIB_DIS, bp == CARFORM_BPON ? BST_UNCHECKED : BST_CHECKED);
	}
	else if (nPart == CBody::AREA_IHANDLE) {
		CARFORM_HAND hand = pBs->ihandle;
		if (hand == CARFORM_HANDNR)	hand = CARFORM_HANDON;
		else	hand = CARFORM_HANDNR;
		m_body.SetIHandle(hand, true);
		BitCtrlD(CUDIB_EED, hand == CARFORM_HANDON ? BST_CHECKED : BST_UNCHECKED);
	}
	else if (nPart == CBody::AREA_EHANDLE) {
		CARFORM_HAND hand = pBs->ehandle;
		if (hand == CARFORM_HANDNR)	hand = CARFORM_HANDON;
		else	hand = CARFORM_HANDNR;
		m_body.SetEHandle(hand, true);
		BitCtrlD(CUDIB_EAD, hand == CARFORM_HANDON ? BST_CHECKED : BST_UNCHECKED);
	}
	return 0;
}
