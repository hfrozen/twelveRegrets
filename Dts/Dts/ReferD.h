// ReferD.h
#pragma once

#include <sys/timeb.h>

#define	DEVELOPER_PC	L"FREEZE"

#define	FONTS_LENGTH	4

typedef struct _tagARTICLE {
	WORD	wFbSig[4][2];
	WORD	wNull;
} ARTICLE, *PARTICLE;

enum ARTICLETYPE {
	ARTICLETYPE_NON,
	ARTICLETYPE_BOOL,
	ARTICLETYPE_BYTE,
	ARTICLETYPE_WORD,
	ARTICLETYPE_DWORD,
	ARTICLETYPE_FLOAT,
	ARTICLETYPE_MAX
};

typedef struct _tagARTICLEITEM {
	ARTICLETYPE	type;
	PVOID		pVoid;
	PCWSTR		pstrItem;
} ARTICLEITEM;

enum CARFORM_DIRECTION {
	CARFORM_DIRNON,
	CARFORM_DIRTAIL,
	CARFORM_DIRHEAD,
	CARFORM_DIRLEFT,
	CARFORM_DIRRIGHT,
	CARFORM_DIRMAX
};

enum CARFORM_PANTO {
	CARFORM_PANX,
	CARFORM_PANDN,
	CARFORM_PANUP,
	CARFORM_PANBP,
	CARFORM_PANMAX
};

enum CARFORM_COACH {
	CARFORM_COACHN,
	CARFORM_COACHL,
	CARFORM_COACHR,
	CARFORM_COACHMAX
};

enum CARFORM_WHEEL {
	CARFORM_WHEELU,
	CARFORM_WHEELD,
	CARFORM_WHEELMAX
};

enum CARFORM_DOOR {
	CARFORM_DOORCL,
	CARFORM_DOOROP,
	CARFORM_DOORBP,
	CARFORM_DOORMAX
};

enum CARFORM_SIDE {
	CARFORM_SIDEX,
	CARFORM_SIDENR,
	CARFORM_SIDEFL,
	CARFORM_SIDEMAX
};

enum CARFORM_BP {
	CARFORM_BPNR,
	CARFORM_BPON,
	CARFORM_BPMAX,
};

enum CARFORM_HAND {
	CARFORM_HANDNR,
	CARFORM_HANDON,
	CARFORM_HANDMAX
};

enum CARTYPE {
	CARTYPE_TC = 1,
	CARTYPE_T1 = 2,
	CARTYPE_T2 = 4,
	CARTYPE_M = 8
};

typedef struct _tagBODYSTYLE {
	int		nID;
	CARTYPE	nType;
	CARFORM_DIRECTION	dir;
	CARFORM_PANTO	pan[2];
	CARFORM_COACH	coach;
	CARFORM_WHEEL	wheel;
	CARFORM_DOOR	door[8];
	CARFORM_SIDE	side;
	CARFORM_BP		bypass;
	CARFORM_HAND	ihandle;
	CARFORM_HAND	ehandle;
	BYTE	nLamp;
} BODYSTYLE, *PBODYSTYLE;

enum MONCOM_CH {
	MONCOM_RX = 0,
	MONCOM_TX,
	MONCOM_MAX
};

enum RUDDERKEY {
	RKEY_NEUTRAL = 0,
	RKEY_FORWARD,
	RKEY_REVERSE,
	RKEY_MAX
};

enum CARPOS {
	CARPOS_LEFT = 0,
	CARPOS_M1L,
	CARPOS_M2L,
	CARPOS_T1L,
	CARPOS_M2,
	CARPOS_T2,
	CARPOS_T1R,
	CARPOS_M1R,
	CARPOS_M2R,
	CARPOS_RIGHT,
	CARPOS_MAX
};

#define	CARF_LEFT	(1 << CARPOS_LEFT)
#define	CARF_M1L	(1 << CARPOS_M1L)
#define	CARF_M2L	(1 << CARPOS_M2L)
#define	CARF_T1L	(1 << CARPOS_T1L)
#define	CARF_M2		(1 << CARPOS_M2)
#define	CARF_T2		(1 << CARPOS_T2)
#define	CARF_T1R	(1 << CARPOS_T1R)
#define	CARF_M1R	(1 << CARPOS_M1R)
#define	CARF_M2R	(1 << CARPOS_M2R)
#define	CARF_RIGHT	(1 << CARPOS_RIGHT)

enum OPERMODE {
	OPMODE_NON = 0,
	OPMODE_AR,
	OPMODE_FA,
	OPMODE_AUTO,
	OPMODE_MANU,
	OPMODE_YARD,
	OPMODE_ENER,
	OPMODE_MAX
};

#define	IsAtoCtrl(mode)	(mode > OPMODE_NON && mode < OPMODE_MANU)

enum RUDDERPOS {
	RUDDERPOS_FORWARD = 0,
	RUDDERPOS_NEUTRAL,
	RUDDERPOS_REVERSE,
	RUDDERPOS_MAX
};

#define	STICK_NEUTRAL	RUDDERPOS_NEUTRAL
#define	STICK_FORWARD	RUDDERPOS_FORWARD
#define	STICK_REVERSE	RUDDERPOS_REVERSE

enum MASTERPOS {
	STEERINGPOS_EB = 0,
	STEERINGPOS_B7,	// 1
	STEERINGPOS_B6,	// 2
	STEERINGPOS_B5,	// 3
	STEERINGPOS_B4,	// 4
	STEERINGPOS_B3,	// 5
	STEERINGPOS_B2,	// 6
	STEERINGPOS_B1,	// 7
	STEERINGPOS_C,	// 8
	STEERINGPOS_P1,	// 9
	STEERINGPOS_P2,	// 10
	STEERINGPOS_P3,	// 11
	STEERINGPOS_P4,	// 12
	STEERINGPOS_MAX
};

enum EFFORTTYPE {
	EFFORT_COAST = 0,
	EFFORT_BRAKE,
	EFFORT_POWER,
	EFFORT_NOATC = 255
};

enum CABINCMD {
	CABINCMD_NON,
	CABINCMD_HCR,
	CABINCMD_TCR,
	CABINCMD_MAX
};

enum DOORCMD {
	DOORCMD_CLS,
	DOORCMD_LOPEN,
	DOORCMD_ROPEN,
	DOORCMD_MAX
};

#include "../../../Qnx2/Inform2/Arrange.h"

/*enum DEVID {
	DEVID_SIV = 0,
	DEVID_V3F,
	DEVID_ECU,
	DEVID_HVAC,
	DEVID_DCUL,
	DEVID_DCUR,
	DEVID_CMSB,
	DEVID_ESK,
	DEVID_PAN,
	DEVID_SDOOR,
	DEVID_TERM,
	DEVID_MAX
};
*/
enum LDEVID {
	LDEVID_ATO = DEVID_MAX,
	LDEVID_PIS,
	LDEVID_PA,
	LDEVID_TRS,
	LDEVID_RTD,
	LDEVID_MAX
};

#define	DEVID_DCU	DEVID_DCUL
#define	DEVID_REALMAX	DEVID_DCUR

#define	DEVF_SIV	(1 << DEVID_SIV)
#define	DEVF_V3F	(1 << DEVID_V3F)
#define	DEVF_ECU	(1 << DEVID_ECU)
#define	DEVF_HVAC	(1 << DEVID_HVAC)
#define	DEVF_DCUL	(1 << DEVID_DCUL)
#define	DEVF_DCUR	(1 << DEVID_DCUR)
#define	DEVF_CM		(1 << DEVID_CM)	//DEVF_CMSB	(1 << DEVID_CMSB)
#define	DEVF_ESK	(1 << DEVID_ESK)
#define	DEVF_PAN	(1 << DEVID_PAN)
#define	DEVF_SDOOR	(1 << DEVID_SDOOR)
#define	DEVF_TERM	(1 << DEVID_TERM)

enum  {
/*00*/	SDID_ATC,		// 1st same to Refer32.h
/*01*/	SDID_ATO,		//// same to TcmsWork/Qnxw/Common/Arrange.h
/*02*/	SDID_PAU,
/*03*/	SDID_PIS,
/*04*/	SDID_TRS,
/*05*/	SDID_AVC,
/*06*/	SDID_RFD,
/*07*/	SDID_SIV,
/*08*/	SDID_V3F,
/*09*/	SDID_ECU,
/*10*/	SDID_HVAC,
/*11*/	SDID_DCUL,
/*12*/	SDID_DCUR,
/*13*/	SDID_BMS,
/*14*/	SDID_CMSB,
/*15*/	SDID_FDU,
/*16*/	SDID_RESA,
/*17*/	SDID_RESB,
/*18*/	SDID_MAX
};

enum  {
	PWMCH_TACHO,
	PWMCH_STEER,
	PWMCH_BC,
	PWMCH_BATTERY,
	PWMCH_TEMPERATURE,
	PWMCH_MAX
};

#define	SDID_ATON	SDID_CM
#define	SDID_TRSN	SDID_ESK

enum WHERE_STT {
	WHERE_CURRENT,
	WHERE_NEXT,
	WHERE_DEST
};

typedef struct _tagMODULE MODULE;
typedef CTypedPtrList <CPtrList, MODULE*> EDIFICE, *PEDIFICE;

typedef struct _tagMODULE {
	UINT	nID;
	BYTE	cSize;
	BYTE	cType;
	CString	strName;
	EDIFICE	edif;
} MODULE, *PMODULE;

enum enMODTYPE {
	MODTYPE_BYTE,
	MODTYPE_WORD,
	MODTYPE_DWORD,
	MODTYPE_BUNION,
	MODTYPE_WUNION,
	MODTYPE_DWUNION
};

#define	WM_INTERNAL			WM_USER + 100
#define	WM_BODYCLICK		WM_INTERNAL + 2
#define	WM_DIRECTIONCTRL	WM_INTERNAL + 4
#define	WM_CONNECTDIO		WM_INTERNAL + 6
#define	WM_CONNECTSCU		WM_INTERNAL + 8

#define	COLOR_RED			RGB(255, 0, 0)
#define	COLOR_GREEN			RGB(0, 255, 0)
#define	COLOR_YELLOW		RGB(255, 255, 0)
#define	COLOR_BLUE			RGB(0, 0, 255)
#define	COLOR_BLACK			RGB(0, 0, 0)
#define	COLOR_WHITE			RGB(255, 255, 255)

#define	CREATEFONT(f, h, w, name)\
	do {\
		f.CreateFontW(h, 0, 0, 0, w, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, name);\
	} while (0)

#define	ERASE(f)\
	do {\
		for (int n = 0; n < FONTS_LENGTH; n ++) {\
			if (f[n].m_hObject)	f[n].DeleteObject();\
		}\
	} while (0)

#define	KILL(p)\
	do {\
		if (p != NULL) {\
			delete p;\
			p = NULL;\
		}\
	} while (0)

#define	IsCheckIdc(id, min, max)	(id >= (IDC_CHECK_DEV0 + min) && id <= (IDC_CHECK_DEV0 + max))
#define	UnCheck(id, min, max)\
	do {\
		for (int n = IDC_CHECK_DEV0 + min; n <= (IDC_CHECK_DEV0 + max); n ++) {\
			if (n != id)	((CButton*)GetDlgItem(n))->SetCheck(BST_UNCHECKED);\
		}\
	} while (0)

#define	TOBCD(x)	(((x / 10) << 4) | (x % 10))

#define	CASTONCABIN(ip, id, psrc, length)\
	do {\
		char buf[length + 3];\
		memcpy(&buf[3], psrc, length);\
		buf[0] = length + 2;\
		buf[1] = 0x15;\
		buf[2] = 0x50 | id;\
		CCabin* pCabin = (CCabin*)m_pParent;\
		CPanel* pPanel = pCabin->GetPanel();\
		pPanel->SendToSocket(ip, buf, length + 3);\
	} while (0)

#define	CASTONCAR(ip, id, psrc, length)\
	do {\
		char buf[length + 3];\
		memcpy(&buf[3], psrc, length);\
		buf[0] = length + 2;\
		buf[1] = 0x15;\
		buf[2] = 0x50 | id;\
		CCar* pCar = (CCar*)m_pParent;\
		CPanel* pPanel = pCar->GetPanel();\
		pPanel->SendToSocket(ip, buf, length + 3);\
	} while (0)

#define	CASTONCABINA(ip, id, psrc, length)\
	do {\
		char buf[length + 3];\
		memcpy(&buf[3], psrc, length);\
		buf[0] = length + 2;\
		buf[1] = 0x15;\
		buf[2] = 0x40 | id;\
		CCabin* pCabin = (CCabin*)m_pParent;\
		CPanel* pPanel = pCabin->GetPanel();\
		pPanel->SendToSocket(ip, buf, length + 3);\
	} while (0)

#define	CASTONCARA(ip, id, psrc, length)\
	do {\
		char buf[length + 3];\
		memcpy(&buf[3], psrc, length);\
		buf[0] = length + 2;\
		buf[1] = 0x15;\
		buf[2] = 0x40 | id;\
		CCar* pCar = (CCar*)m_pParent;\
		CPanel* pPanel = pCar->GetPanel();\
		pPanel->SendToSocket(ip, buf, length + 3);\
	} while (0)

#define	GetIntro(p)	CIntro* p = (CIntro*)m_pParent

#define	ROOTPATH				L"d:/TcmsWork/VC/DtsDoc"
#define	SCRIPTPATH				L"/Script"
#define	IOFILE					L"/IoName.txt"
#define	ARTICLEFILE				L"/Article.txt"

#define	TEXT_ARTICLEIDENTITY	L"*Article File*"
#define	TEXT_FILEENDIDENTITY	L"END"

#define	TEXT_LEFTDOOROPEN		L"LDOP"
#define	TEXT_RIGHTDOOROPEN		L"RDOP"
#define	TEXT_DOORCLOSE			L"CLOSE"
#define	TEXT_PANTOGRAPHUP		L"PANUP"
#define	TEXT_PANTOGRAPHDOWN		L"PANDN"
//#define	TEXT_DCLOADON			L"DCON"
//#define	TEXT_DCLOADOFF			L"DCOFF"
//#define	TEXT_ACLOADON			L"ACON"
//#define	TEXT_ACLOADOFF			L"ACOFF"

#define	IDC_DEVCTRL_BEGIN		IDC_RADIO_DEV0

#define	MAX_SPEED		100000
#define	INC_SPEED		2000
#define	DEC_SPEED		2400

#define	CARLENGTH		6
//#define	_SIMM_SOCKET_
#define	SETUP_SKIP
#define	PORT_SETUP		true

enum enINFORMTCM {
	INFTCM_LINK = 0,
	INFTCM_OBLIGE,
	INFTCM_SPEED,
	INFTCM_DISTAN,
	INFTCM_MAX
};

#define	BOFDW3(dw)	(BYTE)((dw >> 24) & 0xff)
#define	BOFDW2(dw)	(BYTE)((dw >> 16) & 0xff)
#define	BOFDW1(dw)	(BYTE)((dw >> 8) & 0xff)
#define	BOFDW0(dw)	(BYTE)(dw & 0xff)
