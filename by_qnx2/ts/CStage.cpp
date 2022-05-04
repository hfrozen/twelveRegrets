/*
 * CStage.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <string.h>

#include "Track.h"
#include "Insp.h"
#include "Mm.h"
#include "../Component2/CTool.h"
#include "../Component2/CBand.h"
#include "CVerse.h"
#include "CProse.h"
#include "CPaper.h"
#include "CSch.h"
#include "CArch.h"
#include "CReview.h"
#include "CLand.h"
#include "CPel.h"
#include "CStage.h"

#define	HDUFRAME_MINLENGTH	5		// 5 word

const BYTE CStage::c_cMatrix[SECTION_MAX][10] = {
		{	WPG_HEADBOX,	WPG_SERVICEA,		WPG_SERVICEB,		WPG_TROUBLELIST,	WPG_STATUSLIST,			0xff,	0xff,	0xff,	0xff,	0xff	},
		{	WPG_HEADBOX,	WPG_IOSTATEDUMP,	WPG_LINETEXTDUMP,	WPG_LINESTATEDUMP,	WPG_OLDTROUBLELIST,		0xff,	0xff,	0xff,	0xff,	0xff	},
		{	WPG_HEADBOX,	WPG_IOSTATEDUMP,	WPG_LINETEXTDUMP,	WPG_LINESTATEDUMP,	WPG_CURRENTINSPECT,		0xff,	0xff,	0xff,	0xff,	0xff	},
		{	WPG_READYEDIT,	WPG_READYEDIT,		WPG_READYEDIT,		WPG_READYEDIT,		WPG_READYEDIT,			0xff,	0xff,	0xff,	0xff,	0xff	},
};

CStage::CStage()
{
	c_pParent = NULL;
	//memset(&c_sheaf, 0, sizeof(_SHEAF));
	Initial(INTROFROM_KEY);
}

CStage::~CStage()
{
}

void CStage::Initial(int iFrom)
{
	if (iFrom != INTROFROM_OTRN) {
		c_bAuxPipe = false;
		c_pisReq.bRecog = false;		// init. to auto
		c_pisReq.bStatus = false;
		c_cWakeupSeq = 0;
		c_wWatch = c_wMaxWatch = 0;
		c_bSkip4Blank = false;
		c_vbcc.bClear = false;
		c_vbcc.wWait = 0;
		for (int n = 0; n < WPG_MAX; n ++)	c_wElapsedTime[n][0] = c_wElapsedTime[n][1] = 0;
	}

	InitialPage();
	if (iFrom != INTROFROM_OTRN)	c_usual.Initial();
}

void CStage::InitialPage()
{
	c_wListPage = 0xff;
	c_bListing = false;
	// 181003
	//memset(&c_list, 0, sizeof(LISTPAGE) & LIST_MAX);	// !!!!! very serious bug !!!!!
	memset(&c_list, 0, sizeof(LISTPAGE) * LIST_MAX);
	c_dicID.wCar = HK_CAR0;
	c_dicID.wDev = HK_SIDE;
	c_wBookingPage = 0xffff;
	c_wChapter = c_wPage = 0;
	c_wChapterBkupAtInsp = c_wPageBkup = c_wStead = 0;
	c_dwDoorTestMap = 0;
	//c_wChapterBkupAtFire = 0;
	memset(&c_redit, 0, sizeof(RESERVEEDIT));
	memset(&c_fl, 0, sizeof(FILELIST) * FLI_MAX);
	memset(&c_rdScrap, 0, sizeof(READSCRAP));
	InitialDownloadInfo();
}

void CStage::InitialDownloadInfo()
{
	c_dlInfo.iEntryIndex = -1;
	c_dlInfo.iFileIndex = -1;
	c_dlInfo.wMsg = 0;
	c_dlInfo.wItem = c_dlInfo.wItemClone = DOWNLOADITEM_NON;
	//c_dlInfo.wProgress = 0;
	c_dlInfo.bContinue = c_dlInfo.bCancel = c_dlInfo.bFailur = false;
	c_dlInfo.size.cur = c_dlInfo.size.total = 0;
}

bool CStage::IsActiveRoad()
{
	GETPAPER(pPaper);
	if ((pPaper->IsMain() && !c_bAuxPipe) || (!pPaper->IsMain() && c_bAuxPipe))	return true;
	return false;
}

PHDURINFO CStage::GetSendBuf(bool bClr)
{
	GETVERSE(pVerse);
	PHDURINFO pHdur = (PHDURINFO)pVerse->GetInfo(false, PDEVID_HDU, LI_CURR);
	if (bClr)	memset(pHdur, 0, sizeof(HDURINFO));
	return pHdur;
}

PSHEAF CStage::GetSheaf()
{
	GETVERSE(pVerse);
	return pVerse->GetSheaf();
}

void CStage::ClearSheaf()
{
	GETVERSE(pVerse);
	pVerse->ClearSheaf();
}

void CStage::ClearEntrySentence()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&entb, 0, sizeof(ENTRYSENTENCE));
}

void CStage::ClearListSentence()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&tlib, 0, sizeof(ELISTSENTENCE));
}

void CStage::Carry(WORD* pCmp, WORD wHduAddr, WORD wHduLength)
{
	// !!!CAUTION: wLength is word length
	HDURINFO hdur;

	GETVERSE(pVerse);
	PHDURINFO pHdur = (PHDURINFO)pVerse->GetInfo(false, PDEVID_HDU, LI_CURR);
	memcpy(&hdur, pHdur, sizeof(HDURINFO));
	WORD* pSrc = (WORD*)&hdur.t;
	WORD* pDest = (WORD*)&pHdur->t;		// _BUNDLE

	WORD same, addr, leng;
	same = addr = leng = 0;
	for (WORD w = 0; w < wHduLength; w ++) {
		*(pDest + leng) = *pSrc;
		if (*pSrc != *pCmp) {
			*pCmp = *pSrc;
			++ leng;
			same = 0;
			if (!addr)	addr = wHduAddr;
		}
		else {
			if (leng > 0) {
				++ leng;
				if (++ same >= HDUFRAME_MINLENGTH) {
					Convey(NULL, addr, leng - same);
					same = addr = leng = 0;
				}
			}
		}
		++ wHduAddr;
		++ pSrc;
		++ pCmp;
	}
	if (leng > 0)	Convey(NULL, addr, leng - same);
}

void CStage::Convey(BYTE* pCmp, WORD wHduAddr, WORD wHduLength)
{
	// !!!CAUTION: wLength is word length
	GETVERSE(pVerse);
	PHDURINFO pHdur = (PHDURINFO)pVerse->GetInfo(false, PDEVID_HDU, LI_CURR);
	pHdur->cEsc = 0x1b;
	pHdur->cRW = 'W';
	pHdur->wAddr = XWORD(wHduAddr);
	pHdur->wLength = XWORD(wHduLength);

	wHduLength *= sizeof(WORD);
	BYTE sum = 0;
	BYTE* p = (BYTE*)&pHdur->cEsc;
	for (int n = 0; n < (int)(wHduLength + 6); n ++)	// append 3 word
		sum += *p ++;
	*p ++ = sum;
	*p ++ = '\r';	// cr
	*p = '\n';		// lf

	if (IsActiveRoad()) {
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		//pLand->SendHdu();
		pLand->SendHduA();
	}
	if (pCmp != NULL)	memcpy(pCmp, &pHdur->t, wHduLength * sizeof(WORD));
	//if (wHduAddr == ADDR_TROUBLESSENTENCE) {
	//	BYTE* p = pVerse->GetDump();
	//	memcpy(p, &pHdur->cEsc, sizeof(TRBSSTCTAIL) + 9);
	//}
}

void CStage::ReadPage()
{
	GETVERSE(pVerse);
	PHDURINFO pHdur = (PHDURINFO)pVerse->GetInfo(false, PDEVID_HDU, LI_CURR);
	pHdur->cEsc = 0x1b;
	pHdur->cRW = 'R';
	pHdur->wAddr = XWORD(c_rdScrap.wHduAddr);
	pHdur->wLength = XWORD(c_rdScrap.wHduLength);

	BYTE sum = 0;
	BYTE* p = (BYTE*)&pHdur->cEsc;
	for (int n = 0; n < 6; n ++)	sum += *p ++;
	*p ++ = sum;
	*p ++ = '\r';
	*p = '\n';

	if (IsActiveRoad()) {
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		//pLand->SendHdu((BYTE*)&pHdur->cEsc, (WORD)9);
		pLand->SendHduA((BYTE*)&pHdur->cEsc, (WORD)9);
	}
	//TRACK("HDU:read page %d - %d\n", c_rdScrap.wAddr, c_rdScrap.wLength);
}

void CStage::ReadPage(WORD wAddr, WORD wLength)
{
	GETVERSE(pVerse);
	PHDURINFO pHdur = (PHDURINFO)pVerse->GetInfo(false, PDEVID_HDU, LI_CURR);
	pHdur->cEsc = 0x1b;
	pHdur->cRW = 'R';
	pHdur->wAddr = XWORD(wAddr);
	pHdur->wLength = XWORD(wLength);

	BYTE sum = 0;
	BYTE* p = (BYTE*)&pHdur->cEsc;
	for (int n = 0; n < 6; n ++)	sum += *p ++;
	*p ++ = sum;
	*p ++ = '\r';
	*p = '\n';

	if (IsActiveRoad()) {
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		//pLand->SendHdu((BYTE*)&pHdur->cEsc, (WORD)9);
		pLand->SendHduA((BYTE*)&pHdur->cEsc, (WORD)9);
	}
}

void CStage::OpenPage()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&oppb, 0, sizeof(OPENPAGESENTENCE));

	oppb.wPage = XWORD(c_wBookingPage);
	pVerse->CopyBundle(&oppb, sizeof(OPENPAGESENTENCE));

	Convey(NULL, ADDR_OPENPAGESENTENCE, sizeof(OPENPAGESENTENCE) / sizeof(WORD));
}

void CStage::MakeBlank()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&mblb, 0, sizeof(MBLANKSENTENCE));

	// 170904reset이 안될 때는 명령 워드를 지우고 보내고 다시 설정하여 보내야 한다.
	mblb.wBlankCmd = (c_cWakeupSeq & 1) ? 0 : 0x700;
	pVerse->CopyBundle(&mblb, sizeof(MBLANKSENTENCE));

	Convey(NULL, ADDR_MBLANKSENTENCE, sizeof(MBLANKSENTENCE) / sizeof(WORD));
	c_bSkip4Blank = true;
	GETPAPER(pPaper);
	if (pPaper->IsMain())	printf("HDU:reset.\n");		//TRACK("HDU:reset.\n");
	// 170823, 자세한 원인은 알 수 없으나 위에서 TRACK을 하게되면 300ms이상이 소요된다.
}

void CStage::BaseSketch()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&basb, 0, sizeof(BASESENTENCE));

	GETPAPER(pPaper);
	//PDOZEN pDoz = pPaper->GetDozen();

	basb.wLineNo = XWORD(2);
	WORD w = (WORD)pPaper->GetLength();
	basb.wLength = XWORD(w);
	//BYTE c = GETCIB(CAR_SELF, CUDIB_CT01);
	BYTE c = pVerse->GetPermNo();
	GETTOOL(pTool);
	c = pTool->ToHex(c);
	basb.wProperNo = XWORD(c);
	pVerse->CopyBundle(&basb, sizeof(BASESENTENCE));

	Convey(NULL, ADDR_BASESENTENCE, sizeof(BASESENTENCE) / sizeof(WORD));
	ClearSheaf();

	if (pPaper->IsMain())	TRACK("HDU:base sketch.\n");
}

void CStage::HeadLine()
{
	c_usual.HeadLine();

	//c_wScreenOut = c_usual.GetScreenOut();

	PSHEAF pSheaf = GetSheaf();
	//c_sheaf.gnd.cab.a = 0;
	pSheaf->hdb.wSecond = 0xff;		// 시계가 부드럽게 넘어가는 걸 보기위해 이 줄은 필요하다!!!

	pSheaf->hdb.wLineNo = 0xff;		// 지워야 된다
	pSheaf->hdb.wLength = 0xff;
	pSheaf->hdb.wProperNo = 0xff;

	//GETVERSE(pVerse);
	//PBUNDLES pBund = pVerse->GetBundle();
	//if (hdbb.wTextPresent != 0)	pSheaf->hdb.wTextPresent = 0;

	//if (c_bConvey || !c_bHdb) {
		Convey((BYTE*)&pSheaf->hdb, ADDR_HEADLINESENTENCE, sizeof(HEADLINESENTENCE) / sizeof(WORD));
	//	c_bHdb = true;
	//}
	//else	Carry((WORD*)&pSheaf->hdb, ADDR_HEADLINESENTENCE, sizeof(HEADLINESENTENCE) / sizeof(WORD));
}

void CStage::ServiceA()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		c_usual.ServiceA();
		PSHEAF pSheaf = GetSheaf();
		//if (c_bConvey || !c_bSva) {
			Convey((BYTE*)&pSheaf->una, ADDR_UNITAPSENTENCE, sizeof(UNITAPSENTENCE) / sizeof(WORD));
		//	c_bSva = true;
		//}
		//else	Carry((WORD*)&pSheaf->sva, ADDR_UNITAPSENTENCE, sizeof(UNITAPSENTENCE) / sizeof(WORD));
	}
}

void CStage::ServiceB()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		c_usual.ServiceB();
		PSHEAF pSheaf = GetSheaf();
		//if (c_bConvey || !c_bSvb) {
			Convey((BYTE*)&pSheaf->unb, ADDR_UNITBPSENTENCE, sizeof(UNITBPSENTENCE) / sizeof(WORD));
		//	c_bSvb = true;
		//}
		//else	Carry((WORD*)&pSheaf->svb, ADDR_UNITBPSENTENCE, sizeof(UNITBPSENTENCE) / sizeof(WORD));
	}
}

//void CStage::ReadyListup()
//{
//	if (c_reflection.bAlter) {
//		c_reflection.bAlter = false;
//		c_reflection.bScan = true;
//		ReadPage(ADDR_TROUBLESSENTENCE + 80, 1);			// read TROUBLEARCH.wLength
//	}
//	else	c_reflection.bAlter = true;	// 1sec
//}
//
//void CStage::Reflect()
//{
//	GETVERSE(pVerse);
//	PBUNDLES pBund = pVerse->GetBundle();
//	memset(&trbb, 0, sizeof(TRBSSTCTAIL));
//	PSHEAF pSheaf = GetSheaf();
//	GETARCHIVE(pArch);
//	GETTOOL(pTool);
//	int leng = 0;
//	TROUBLEARCH troub;
//	while (pArch->GetStillOnFilm(&troub, SPARKLE_TROUBLEDU)) {
//		if ((troub.b.type & TROUBLETYPE_ONLY) >= TROUBLE_STATUS) {
//			_DUET du;
//			du.c[0] = pTool->ToBcd((BYTE)troub.b.mon);
//			du.c[1] = pTool->ToBcd((BYTE)troub.b.day);
//			trbb.tb[leng].wMD = du.w;
//			if (troub.b.down) {
//				du.c[0] = pTool->ToBcd((BYTE)troub.b.dhour);
//				du.c[1] = pTool->ToBcd((BYTE)troub.b.dmin);
//				trbb.tb[leng].wHM = du.w;
//				du.c[0] = (troub.b.type & TROUBLETYPE_ONLY) > TROUBLE_STATUS ? 0x81 : 0x80;
//				du.c[1] = pTool->ToBcd((BYTE)troub.b.dsec);
//				trbb.tb[leng].wST = du.w;
//			}
//			else {
//				du.c[0] = pTool->ToBcd((BYTE)troub.b.uhour);
//				du.c[1] = pTool->ToBcd((BYTE)troub.b.umin);
//				trbb.tb[leng].wHM = du.w;
//				du.c[0] = (troub.b.type & TROUBLETYPE_ONLY) > TROUBLE_STATUS ? 1 : 0;
//				du.c[1] = pTool->ToBcd((BYTE)troub.b.usec);
//				trbb.tb[leng].wST = du.w;
//			}
//			WORD cid = (WORD)troub.b.cid;
//			du.w = ((cid << 12) & 0xf000) | (WORD)(troub.b.code & 0xfff);
//			trbb.tb[leng].wCidCode = XWORD(du.w);
//
//			++ leng;
//		}
//		if (leng >= MAX_TROUBLEBLOCK)	break;
//	}
//	if (leng > 0) {
//		c_reflection.wLength = leng;
//		trbb.wLength = XWORD(c_reflection.wLength);
//		pVerse->CopyBundle(&trbb, sizeof(TROUBLESSENTENCE));
//
//		pSheaf->trb.wLength = 0;
//		Convey((BYTE*)&pSheaf->trb, ADDR_TROUBLESSENTENCE, sizeof(TROUBLESSENTENCE) / sizeof(WORD));
//	}
//}
//
void CStage::DumpPioState()
{
	c_dump.PioState();

	PSHEAF pSheaf = GetSheaf();
	//if (c_bConvey || !c_bIom) {
		Convey((BYTE*)&pSheaf->icd, ADDR_IOCDUMPSENTENCE, sizeof(IOCDUMPSENTENCE) / sizeof(WORD));
	//	c_bIom = true;
	//}
	//else	Carry((WORD*)&pSheaf->iom, ADDR_IODUMPSENTENCE, sizeof(IODUMPSENTENCE) / sizeof(WORD));
}

void CStage::DumpLineText()
{
	//c_usual.LineText(c_dicID.wCar, c_dicID.wDev);
	c_dump.LineText(c_dicID.wCar, c_dicID.wDev);

	PSHEAF pSheaf = GetSheaf();
	//if (c_bConvey || !c_bLdp) {
		Convey((BYTE*)&pSheaf->led, ADDR_LKEDUMPSENTENCE, sizeof(LKEDUMPSENTENCE) / sizeof(WORD));
	//	c_bLdp = true;
	//}
	//else	Carry((WORD*)&pSheaf->ldp, ADDR_LDUMPSENTENCE, sizeof(LDUMPSENTENCE) / sizeof(WORD));
}

void CStage::DumpLineState()
{
	c_dump.LineState();

	PSHEAF pSheaf = GetSheaf();
	pSheaf->lks.wUpdate = 0;
	//if (c_bConvey || !c_bLcn) {
		Convey((BYTE*)&pSheaf->lks, ADDR_LKSTATESENTENCE, sizeof(LKSTATESENTENCE) / sizeof(WORD));
	//	c_bLcn = true;
	//}
	//else	Carry((WORD*)&pSheaf->lcn, ADDR_LCONDSENTENCE, sizeof(LCONDSENTENCE) / sizeof(WORD));
}

void CStage::DumpList(bool bTrouble, bool bAll)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		c_bListing = true;
		PLISTPAGE pList = bTrouble ? &c_list[LIST_TROUBLE] : &c_list[LIST_STATE];
		//pList->wTotal = c_dump.GetSortList(pList->wCur, bTrouble, bAll);
		pList->wTotal = c_dump.GetSortListByPage(pList->wCur, bTrouble);

		PSHEAF pSheaf = GetSheaf();
		if (bTrouble)	Convey((BYTE*)&pSheaf->tli, ADDR_TROUBLELIST, sizeof(ELISTSENTENCE) / sizeof(WORD));
		else	Convey((BYTE*)&pSheaf->sli, ADDR_STATELIST, sizeof(ELISTSENTENCE) / sizeof(WORD));
		c_bListing = false;
	}
}

void CStage::DumpOldTrouble()
{
	GETPAPER(pPaper);
	//GETARCHIVE(pArch);
	//PBULKPACK pBulk = pArch->GetBulk();
	//if (pPaper->GetOblige() & (1 << OBLIGE_MAIN)) {		// 171110 && (pBulk->wState & (1 << BULKSTATE_OLDTROUBLE))) {
	if (pPaper->IsMain()) {
		c_bListing = true;
		if (c_wListPage == LIST_SORTTROUBLE) {
			//c_list[LIST_SORTTROUBLE].wTotal = c_dump.GetTroubleForEachCar(c_list[LIST_SORTTROUBLE].wID, c_list[LIST_SORTTROUBLE].wCur);
			c_list[LIST_SORTTROUBLE].wTotal = c_dump.GetEachListByPage(c_list[LIST_SORTTROUBLE].wID, c_list[LIST_SORTTROUBLE].wCur);
			//TRACK("STG:old %d %d %d.\n", c_list[LIST_OLDTROUBLE].wID, c_list[LIST_OLDTROUBLE].wCur, c_list[LIST_OLDTROUBLE].wTotal);
		}

		PSHEAF pSheaf = GetSheaf();
		Convey((BYTE*)&pSheaf->tli, ADDR_TROUBLELIST, sizeof(ELISTSENTENCE) / sizeof(WORD));
		c_bListing = false;
	}
}

void CStage::DumpEntryList()		//int nEntryID)
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	pVerse->CopyBundle(&entb, sizeof(ENTRYSENTENCE));

	PSHEAF pSheaf = GetSheaf();
	Convey((BYTE*)&pSheaf->ent, ADDR_ENTRYSENTENCE, sizeof(ENTRYSENTENCE) / sizeof(WORD));
	// entry가 적을 때는 DU가 메모리를 초기화하기 전에 이 함수를 실행하므로 Carry를 사용하면 안된다!!!
}

void CStage::DumpSetItem()
{
	c_dump.SetItemText();

	PSHEAF pSheaf = GetSheaf();

	Convey((BYTE*)&pSheaf->uss, ADDR_USERSETSENTENCE, sizeof(USERSETSENTENCE) / sizeof(WORD));
}

void CStage::CurrentInspect()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();

	//GETPAPER(pPaper);
	//PDOZEN pDoz = pPaper->GetDozen();

	GETREVIEW(pRev);
	WORD w = pRev->GetInspectStep();
	ispb.wStep = XWORD(w);
	w = pRev->GetInspectItem();
	if (w & (1 << INSPITEM_PDT))			ispb.wItem = XWORD(1);
	else if (w & (1 << INSPITEM_DAILY))		ispb.wItem = XWORD(2);
	else if (w & (1 << INSPITEM_MONTHLY))	ispb.wItem = XWORD(3);
	else	ispb.wItem = 0;

	w = pRev->GetInspectTime();
	GETTOOL(pTool);
	BYTE sec = pTool->ToBcd((BYTE)(w % 60));
	BYTE min = pTool->ToBcd((BYTE)(w / 60));
	ispb.wTime = MAKEWORD(min, sec);

	GETARCHIVE(pArch);
	PINSPREPORT pInsp = pArch->GetInspectReport();
	ispb.wSubItem = pInsp->wSubItem;
	ispb.wMsg = pInsp->wMsg;
	memcpy(&ispb.ics, &pInsp->ics, sizeof(INSPECTCOLUMNS));
	if (pInsp->wEcui >= 0 && pInsp->wEcui < 4)	memcpy(&ispb.ics.ecu, &pInsp->ecu[pInsp->wEcui], sizeof(ECUINSPECTCOL));

	// 210720
	ispb.ics.wHvacMon[0] = (WORD)((c_him.each[0].a << 8) | c_him.each[1].a);
	ispb.ics.wHvacMon[1] = (WORD)((c_him.each[2].a << 8) | c_him.each[3].a);
	ispb.ics.wHvacMon[2] = (WORD)((c_him.each[4].a << 8) | c_him.each[4].a);
	ispb.ics.wHvacMon[3] = c_him.bInsp ? 0x100 : 0;
	ispb.ics.wHvacMon[3] |= c_him.bHorc ? 0x200 : 0;

	pVerse->CopyBundle(&ispb, sizeof(INSPECTSENTENCE));

	PSHEAF pSheaf = GetSheaf();
	//if (c_bConvey || !c_bIsp) {
		Convey((BYTE*)&pSheaf->isp, ADDR_INSPECTSENTENCE, sizeof(INSPECTSENTENCE) / sizeof(WORD));
	//	c_bIsp = true;
	//}
	//else	Carry((WORD*)&pSheaf->isp, ADDR_INSPECTSENTENCE, sizeof(INSPECTSENTENCE) / sizeof(WORD));
}

void CStage::InspectReset(WORD wItem)
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();

	hdlb.insp.b.ros = false;
	//ispb.insp.b.ros = false;
	ispb.wItem = XWORD(wItem & INSP_MODE);
	ispb.wSubItem = 0;
	ispb.wMsg = 0;
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_TCL))) {
		ispb.ics.wTcRes = XWORD(0xddd);		// 0000 1101 1101 1101 "시험 진행 중"
		ispb.ics.wTul = XWORD(0xff);		// "시험"
		ispb.ics.wCul = 0xffff;
		ispb.ics.wPole = 0xffff;
		ispb.ics.wSvf = 0xffff;
		ispb.ics.wEcu = 0xffff;
		ispb.ics.wHvac = 0xffff;
		ispb.ics.wEhv = XWORD(0xff);
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_V3F))) {
		ispb.ics.wV3fRes = XWORD(0xd);
		ispb.ics.wHb = 0;
		ispb.ics.wLb = 0;
		ispb.ics.wCpb = XWORD(0x155);
		for (int n = 0; n < V3F_MAX; n ++)
			ispb.ics.wPwmv[n] = ispb.ics.wFc[n] = 0;
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_SIV))) {
		ispb.ics.wSivRes = XWORD(0xd);
		for (int n = 0; n < SIV_MAX; n ++)
			ispb.ics.wEo[n] = ispb.ics.wFreq[n] = 0;
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_ECU))) {
		ispb.ics.ecu.wEcuRes = XWORD(0xd);
		ispb.ics.ecu.wEcuL = 0xffff;
		ispb.ics.ecu.wEcuH = XWORD(0xf);
		for (int n = 0; n < 10; n ++)
			ispb.ics.ecu.wBcp[n] = ispb.ics.ecu.wPwme[n] = 0;
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_COOL))) {
		ispb.ics.wCoolRes = XWORD(0xd);
		ispb.ics.wCool1 = ispb.ics.wCool2 = 0xffff;
		ispb.ics.wCool12 = XWORD(0xff);
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_HEAT))) {
		ispb.ics.wHeatRes = XWORD(0xd);
		ispb.ics.wHeatL = 0xffff;
		ispb.ics.wHeatH = XWORD(0xf);
	}
}

void CStage::ReceiveUserDetail()
{
	GETVERSE(pVerse);
	GETPAPER(pPaper);
	//if (c_reflection.bScan) {
	//	c_reflection.bScan = false;
	//	bool bEmpty = false;
	//	WORD wRecv = pVerse->GetHduRecvLength();
	//	if (wRecv >= 4)	{
	//		BYTE* p = (BYTE*)pVerse->GetInfo(true, PDEVID_HDU, LI_CURR);
	//		p += 2;		// skip esc-'A'
	//		_DUET du;
	//		du.c[1] = *p ++;
	//		du.c[0] = *p;
	//		if (du.w == 0)	bEmpty = true;
	//		else	TRACK("STG>WARN:not empty!(%d)\n", du.w);
	//	}
	//	if (bEmpty) {
	//		c_reflection.wCount = 0;
	//		Reflect();
	//	}
	//	else {
	//		if (++ c_reflection.wCount > c_reflection.wMax)	c_reflection.wMax = c_reflection.wCount;
	//	}
	//	return;
	//}
	if (c_rdScrap.cResp == HK_SETENVIRONS || c_rdScrap.cResp == HK_SETWHEEL || c_rdScrap.cResp == HK_SETTIME || c_rdScrap.cResp == HK_SETCAREER) {
		WORD wRecv = pVerse->GetHduRecvLength();
		TRACK("HDU:user demand %d bytes.\n", wRecv);
		if (wRecv < sizeof(USERSETSENTENCE))	return;

		BYTE* p = (BYTE*)pVerse->GetInfo(true, PDEVID_HDU, LI_CURR);
		p += 2;		// skip esc-'A'

		USERSETSENTENCE setv[2];
		memcpy(&setv[1].c[0], p, sizeof(USERSETSENTENCE));
		for (int n = 0; n < (int)(sizeof(USERSETSENTENCE) / sizeof(WORD)); n ++)
			setv[0].ws[n] = XWORD(setv[1].ws[n]);
		GETARCHIVE(pArch);
		bool bSaveEnv = false;
		switch (c_rdScrap.cResp) {
		case HK_SETCAREER :
			{
				PCAREERS pCrr = pArch->GetCareers();
				_QUARTET quar;
				quar.dw = setv[0].sv.dwDistance;
				pCrr->real.dbDistance = (double)(((DWORD)quar.w[0] << 16) | ((DWORD)quar.w[1])) * 1000.f;
				//TRACK("STG:dist = %.2f\n", pCrr->real.dbDistance);
				for (int n = 0; n < SIV_MAX; n ++) {
					quar.dw = setv[0].sv.dwSivPwr[n];
					pCrr->real.dbSivPower[n] = (double)(((DWORD)quar.w[0] << 16) | ((DWORD)quar.w[1])) * 1000.f;
					//TRACK("STG:sivp%d = %.2f\n", n, pCrr->real.dbSivPower[n]);
				}
				for (int n = 0; n < V3F_MAX; n ++) {
					quar.dw = setv[0].sv.dwV3fPwr[n];
					pCrr->real.dbV3fPower[n] = (double)(((DWORD)quar.w[0] << 16) | ((DWORD)quar.w[1])) * 1000.f;
					//TRACK("STG:v3fp%d = %.2f\n", n, pCrr->real.dbV3fPower[n]);
					quar.dw = setv[0].sv.dwV3fRev[n];
					pCrr->real.dbV3fReviv[n] = (double)(((DWORD)quar.w[0] << 16) | ((DWORD)quar.w[1])) * 1000.f;
					//TRACK("STG:v3fr%d = %.2f\n", n, pCrr->real.dbV3fReviv[n]);
				}
				if (pPaper->IsMain()) {
					CLand* pLand = (CLand*)c_pParent;
					ASSERTP(pLand);
					pLand->StirCareerMatch();
					pArch->Shot(CAR_PSELF(), 180);
				}
			}
			break;
		case HK_SETTIME :
			{
				DTSTIME dt;
				dt.year = setv[0].sv.year;
				dt.mon = setv[0].sv.month;
				dt.day = setv[0].sv.day;
				dt.hour = setv[0].sv.hour;
				dt.min = setv[0].sv.minute;
				dt.sec = 0;
				TRACK("HDU:set time %04X/%02X/%02X %02X:%02X\n", dt.year, dt.mon, dt.day, dt.hour, dt.min);
				pArch->SetSysTimeByBcdDts(dt);		// 여기서 시간 설정을 한다
				if (pPaper->IsMain()) {
					CLand* pLand = (CLand*)c_pParent;
					ASSERTP(pLand);
					pLand->StirTimeMatch(false);
					pArch->Shot(CAR_PSELF(), 176);
					bSaveEnv = true;
				}
			}
			break;
		case HK_SETWHEEL :
			{
				TRACK("HDU:set wheel %d %d %d %d %d %d %d\n",
						setv[0].sv.wheeldiam[0], setv[0].sv.wheeldiam[1], setv[0].sv.wheeldiam[2], setv[0].sv.wheeldiam[3],
						setv[0].sv.wheeldiam[4], setv[0].sv.wheeldiam[5], setv[0].sv.wheeldiam[6]);
				PENVIRONS pEnv = pArch->GetEnvirons();
				for (int n = 0; n < LENGTH_WHEELDIA; n ++)
					pEnv->real.cWheelDiai[n] = setv[0].sv.wheeldiam[n] > MIN_WHEELDIA ? (BYTE)(setv[0].sv.wheeldiam[n] - MIN_WHEELDIA) : 0;
				if (pPaper->IsMain()) {
					CLand* pLand = (CLand*)c_pParent;
					ASSERTP(pLand);
					pLand->StirEnvironMatch();
					pArch->Shot(CAR_PSELF(), 178);
					bSaveEnv = true;
				}
			}
			break;
		case HK_SETENVIRONS :
			{
				TRACK("HDU:set temp 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d 8:%d 9:%d V:%d M:%d-%d\n",
						setv[0].sv.temp[0], setv[0].sv.temp[1], setv[0].sv.temp[2], setv[0].sv.temp[3], setv[0].sv.temp[4],
						setv[0].sv.temp[5], setv[0].sv.temp[6], setv[0].sv.temp[7], setv[0].sv.temp[8], setv[0].sv.temp[9],
						setv[0].sv.venttime, setv[0].sv.pisManual, setv[0].sv.pisRequest);
				PENVIRONS pEnv = pArch->GetEnvirons();
				for (int n = 0; n < 10; n ++) {
					if (setv[0].sv.temp[n] < MIN_TEMPSV || setv[0].sv.temp[n] > MAX_TEMPSV)
						pEnv->real.cTempSv[n] = DEF_TEMPSV;
					else	pEnv->real.cTempSv[n] = (BYTE)setv[0].sv.temp[n];
				}
				pEnv->real.cVentTime = (BYTE)setv[0].sv.venttime;
				//pVerse->SetPisManual(setv[0].sv.pismode != 0 ? true : false);	// 171115
				// 171129
				// PIS CONTROL CODE #4 : get pis mode sw from hdu
				BYTE block = pVerse->GetLineBlocks();
				if (!(block & ((1 << PDEVID_PAU) | (1 << PDEVID_PIS)))) {
					// 180717
					c_pisReq.bStatus = setv[0].sv.pisManual != 0 ? true : false;	// manual : auto
					c_pisReq.bRecog = true;
				}

				if (pPaper->IsMain()) {
					CLand* pLand = (CLand*)c_pParent;
					ASSERTP(pLand);
					pLand->StirEnvironMatch();
					pArch->Shot(CAR_PSELF(), 179);
					bSaveEnv = true;
				}
			}
			break;

		default :	break;
		}
		if (bSaveEnv)	pArch->SaveEnvirons();
	}
	else if (c_rdScrap.cResp == HK_INSPRUN) {
		//if (pPaper->GetOblige() & (1 << OBLIGE_HEAD)) {
		if (pPaper->IsHead()) {
			WORD wRecv = pVerse->GetHduRecvLength();
			TRACK("HDU:user demand %d bytes.\n", wRecv);
			if (wRecv < 4)	return;
			BYTE* p = (BYTE*)pVerse->GetInfo(true, PDEVID_HDU, LI_CURR);
			//for (WORD w = 0; w < wRecv; w ++)	printf("0x%02X ", *(p + w));
			//printf("\n");
			p += 2;		// skip esc-'A'
			_DUET du;
			du.c[1] = *p ++;	// XWORD
			du.c[0] = *p;
			WORD w = du.w;
			//printf("0x%04X\n", du.w);

			if ((du.w & (1 << INSPITEM_PDT)) && (du.w & (1 << INSPITEM_ALL)))
				du.w = ((1 << INSPITEM_PDT) | (1 << INSPITEM_TCL) | (1 << INSPITEM_V3F) | (1 << INSPITEM_ECU));
			else if ((du.w & (1 << INSPITEM_DAILY)) && (du.w & (1 << INSPITEM_ALL)))
				du.w = ((1 << INSPITEM_DAILY) | (1 << INSPITEM_TCL) | (1 << INSPITEM_V3F) | (1 << INSPITEM_ECU));
			else if ((du.w & (1 << INSPITEM_MONTHLY)) && (du.w & (1 << INSPITEM_ALL)))
				du.w = ((1 << INSPITEM_MONTHLY) | (1 << INSPITEM_TCL) | (1 << INSPITEM_V3F) | (1 << INSPITEM_SIV) | (1 << INSPITEM_ECU));

			if (w & (1 << INSPITEM_COOLSEL)) {
				du.w &= ~(1 << INSPITEM_COOLSEL);
				du.w |= (1 << INSPITEM_COOL);
			}
			if (w & (1 << INSPITEM_HEATSEL)) {
				du.w &= ~(1 << INSPITEM_HEATSEL);
				du.w |= (1 << INSPITEM_HEAT);
			}
			//printf("0x%04X\n", du.w);

			GETREVIEW(pRev);
			pRev->InspectRegister(du.w);
			TRACK("HDU:inspect items %04X.\n", du.w);
		}
	}
	else if (c_rdScrap.cResp == HK_DOWNLOAD) {
		WORD wRecv = pVerse->GetHduRecvLength();
		TRACK("HDU:user demand %d byte.\n", wRecv);
		BYTE* p = (BYTE*)pVerse->GetInfo(true, PDEVID_HDU, LI_CURR);
		//for (WORD w = 0; w < wRecv; w ++)	printf("0x%02x ", *(p + w));
		//printf("\n");
		if (c_dlInfo.wItem == DOWNLOADITEM_LOGBOOKENTRY) {
			p += 2;		// skip esc-'a'
			_QUARTET qu;
			qu.c[3] = *p ++;
			qu.c[2] = *p ++;
			qu.c[1] = *p ++;
			qu.c[0] = *p;
			InitialDownloadInfo();
			for (int n = 0; n < SIZE_ENTRYSENTENCE; n ++) {
				if (qu.dw & (1 << n)) {
					PENTRYSHAPE pEntry = c_dump.GetEntry(n);
					if (pEntry != NULL && pEntry->szName[0] != 0) {
						c_dlInfo.size.total += (int)pEntry->dwSize;
						TRACK("STG:download logbook. %d : %s(%d)\n", n, pEntry->szName, pEntry->dwSize);
					}
					else	qu.dw &= ~(1 << n);
					//else	TRACK("%d : null\n");
				}
			}
			if (qu.dw != 0) {
				TRACK("STG:download logbook. %lld\n", c_dlInfo.size.total);
				c_dump.SetEntrySelMap(qu.dw);
				c_dlInfo.iEntryIndex = 0;
				c_dlInfo.wItemClone = DOWNLOADITEM_GETLOGBOOKTEXT;
				c_dlInfo.size.cur = 0;
				MakeDestinationFromEntry();
			}
		}
		else if (c_dlInfo.wItem == DOWNLOADITEM_TRACEENTRY) {
			// * TRACE 2 :
			// 트레이스 항목을 검사한다.
			p += 6;	// skip esc-'a'-es0-es1-es2-es3
			_QUARTET qu;
			qu.c[1] = *p ++;	// wSvfSelMap, b0~b2:siv, b3~7:v3f
			qu.c[0] = *p ++;
			qu.c[3] = *p ++;	// wEcuSelMap, b0~b9:ecu
			qu.c[2] = *p;
			TRACK("STG:trace down load item %04X %04X\n", qu.w[0], qu.w[1]);	// fix value - w[0](siv:0x07,v3f:0xf8),	w[1](ecu:0x3ff)

			WORD wTraceLengthFromCID[CID_MAX];
			memset(&wTraceLengthFromCID, 0, sizeof(WORD) * CID_MAX);
			GETPAPER(pPaper);
			PDOZEN pDoz = pPaper->GetDozen();
			GETSCHEME(pSch);

			int ldi = 0;
			if ((qu.w[0] & 7) != 0)				ldi = DEVID_SIV;
			else if ((qu.w[0] & 0xf8) != 0)		ldi = DEVID_V3F;
			else if ((qu.w[1] & 0x3ff) != 0)	ldi = DEVID_ECU;

			bool bResult = false;
			for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
				WORD wDevMap = pSch->GetLDeviceMap(n);
				WORD wLeng = 0;
				if ((ldi == DEVID_SIV && (wDevMap & DEVBF_SIV)) || (ldi == DEVID_V3F && (wDevMap & DEVBF_V3F)) || ldi == DEVID_ECU) {
					int fid = pSch->C2F(pPaper->GetTenor(), n);
					// 선택된 디바이스들을 표시할 필요가 있다. 7호선에선 프로그레스 바를 각 디바이스마다 따로 두었으니 괜찮았지만. 여기서도...
					switch (ldi) {
					// 200218
					//case DEVID_SIV :	wLeng = (WORD)pDoz->recip[fid].real.svf.s.a.t.s.trc.b.leng;	break;
					case DEVID_SIV :	wLeng = !pPaper->GetDeviceExFromRecip() ?
												(WORD)pDoz->recip[fid].real.svf.s.a.t.s.trc.b.leng :
												(WORD)pDoz->recip[fid].real.svf.s2.a.t.s.trc.b.leng;	break;
					case DEVID_V3F :	wLeng = (WORD)pDoz->recip[fid].real.svf.v.a.t.s.trc.b.leng;	break;
					default :			wLeng = !pPaper->GetDeviceExFromRecip() ?
												(WORD)pDoz->recip[fid].real.ecu.co.a.t.s.trc.b.leng :
												(WORD)pDoz->recip[fid].real.ecu.ct.a.t.s.trc.b.leng;	break;
					}
				}
				wTraceLengthFromCID[n] = wLeng;
				if (wTraceLengthFromCID[n] > 0)	bResult = true;
			}
			if (bResult) {
				CLand* pLand = (CLand*)c_pParent;
				ASSERTP(pLand);
				pLand->LeadTraceSphere(ldi, &wTraceLengthFromCID[0]);
			}
		}
	}
	else if (c_rdScrap.cResp == HK_DOORTEST) {
		BYTE* p = (BYTE*)pVerse->GetInfo(true, PDEVID_HDU, LI_CURR);
		p += 2;		// skip esc-'a'
		_QUARTET qu;
		qu.c[1] = *p ++;
		qu.c[0] = *p ++;
		qu.c[3] = *p ++;
		qu.c[2] = *p;
		TRACK("STG:door test %08X\n", qu.dw);
		c_dwDoorTestMap = qu.dw & 0xfffff;
		if (c_dwDoorTestMap != 0)	c_dwDoorTestMap |= 0x100000;
	}
}

void CStage::DownloadInspect()
{
	InitialDownloadInfo();
	DWORD dwSelMap = 0;
	for (int n = 0; n < MAXLENGTH_ENTRY; n ++) {
		PENTRYSHAPE pEntry = c_dump.GetEntry(n);
		if (pEntry != NULL && pEntry->szName[0] != 0) {
			c_dlInfo.size.total += (int)pEntry->dwSize;
			dwSelMap |= (1 << n);
			TRACK("STG:download inspect %d : %s(%d)\n", n, pEntry->szName, pEntry->dwSize);
		}
		//else	TRACK("%d : null\n");
	}
	c_dump.SetEntrySelMap(dwSelMap);
	if (dwSelMap != 0) {
		TRACK("STG:download inspect %lld.\n", c_dlInfo.size.total);
		c_dlInfo.iEntryIndex = 0;
		c_dlInfo.wItemClone = DOWNLOADITEM_GETINSPECTTEXT;
		c_dlInfo.size.cur = 0;
		MakeDestinationFromEntry();
	}
}

bool CStage::MakeDestinationFromEntry()
{
	GETARCHIVE(pArch);
	DWORD dwSelMap = c_dump.GetEntrySelMap();
	for ( ; c_dlInfo.iEntryIndex < MAXLENGTH_ENTRY; c_dlInfo.iEntryIndex ++) {
		PENTRYSHAPE pEntry = c_dump.GetEntry(c_dlInfo.iEntryIndex);
		if (pEntry != NULL && pEntry->szName[0] != 0 && (dwSelMap & (1 << c_dlInfo.iEntryIndex))) {
			c_dlInfo.wItem = DOWNLOADITEM_MAKEDESTINATION;
			pArch->MakeDestination(pEntry->szName, c_dlInfo.wItemClone == DOWNLOADITEM_GETLOGBOOKTEXT ? true : false);
			PBULKPACK pBulk = pArch->GetBulk();
			pBulk->uiCurSize = 0;
			return true;
		}
	}
	return false;
}

//int CStage::NextDownloadFromEntry(int iEntryIndex, bool bLog)
//{
//	GETARCHIVE(pArch);
//	DWORD dwSelMap = c_dump.GetEntrySelMap();
//	for ( ; iEntryIndex < MAXLENGTH_ENTRY; iEntryIndex ++) {
//		PENTRYSHAPE pEntry = c_dump.GetEntry(iEntryIndex);
//		if (pEntry != NULL && pEntry->szName[0] != 0 && (dwSelMap & (1 << iEntryIndex))) {
//			if (bLog)	pArch->GetLogbook(pEntry->szName);
//			else	pArch->GetInspect(pEntry->szName);
//			return iEntryIndex;
//		}
//	}
//	return -1;
//}
//
void CStage::DownloadEnd(WORD wReItem, WORD wMsg)
{
	c_dlInfo.wItem = wReItem;
	c_dlInfo.iEntryIndex = -1;
	c_dlInfo.iFileIndex = -1;
	c_dlInfo.bCancel = false;
	c_dlInfo.wMsg = wMsg;
}

void CStage::Monitor()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	c_him.bInsp = pDoz->recip[FID_HEAD].real.sign.insp.b.hvac;
	c_him.bHorc = pDoz->recip[FID_HEAD].real.sign.insp.b.horc;
	GETSCHEME(pSch);
	for (int cid =  0; cid < (int)pPaper->GetLength(); cid ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), cid);
		if (pPaper->GetRouteState(fid)) {
			c_him.each[cid].b.blk = pDoz->recip[fid].real.sign.scmdEx.b.hicmd;
			if (pDoz->recip[fid].real.hcb.cb.wSign != SIGN_CMSBFLAP) {
				c_him.each[cid].b.tsr = pDoz->recip[fid].real.hcb.hv.r.t.s.tst.b.tsr;
				c_him.each[cid].b.s0 = pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.seq & 1 ? true : false;
				c_him.each[cid].b.s1 = pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.seq & 2 ? true : false;
				c_him.each[cid].b.u1 = pDoz->recip[fid].real.hcb.hv.a.t.s.stb.b.u1tok;
				c_him.each[cid].b.u2 = pDoz->recip[fid].real.hcb.hv.a.t.s.stb.b.u2tok;
				c_him.each[cid].b.hok = pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.ok;
				c_him.each[cid].b.hng = pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.ng;
			}
		}
	}
}

void CStage::Arteriam()
{
	GETARCHIVE(pArch);
	PBULKPACK pBulk = pArch->GetBulk();
	if (pBulk == NULL)	return;

	CLand* pLand = (CLand*)c_pParent;
	ASSERTP(pLand);
	BYTE cProperID = pLand->GetProperID();

	// 아래는 HDU에서 시작된 프로세서이므로 여기서 처리한다...
	DWORD dwIndex;
	if ((pBulk->m[BPID_OCCASN].bBegin && pBulk->m[BPID_OCCASN].bEnd) || c_dlInfo.bContinue) {
		pBulk->m[BPID_OCCASN].bBegin = pBulk->m[BPID_OCCASN].bEnd = false;
		c_dlInfo.bContinue = false;
		switch (c_dlInfo.wItem) {
		case DOWNLOADITEM_LOGBOOKENTRY :
			// pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.f[n].szName에는 패스없이 파일이름만 들어있다.
			// 여기서 처음 'd'나 'p'등의 이니셜을 빼고 숫자만 표시한다.
			if ((dwIndex = c_dump.MonoEntryList((PVOID)pBulk)) >= pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal) {
				c_wStead = WPG_LOGBOOKENTRYLIST;
				c_dlInfo.wMsg = (cProperID & 1) ? 48 : 41;		// "USB 메모리를 찾고 있습니다."
				//pArch->UsbScaning(true);
			}
			else	pArch->GetLogbookEntry(dwIndex, pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal);
			break;
		case DOWNLOADITEM_INSPECTENTRY :
			if (pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex == 0) {
				c_inspTotal.dw = pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal;
				pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal = (DWORD)(c_inspTotal.c[0] + c_inspTotal.c[1] + c_inspTotal.c[2]);
				//TRACK("STG:take insp entry %d.\n", pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal);
			}
			if ((dwIndex = c_dump.TriEntryList((PVOID)pBulk, c_inspTotal)) >= pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal) {
				//TRACK("STG:tri list %d.\n", dwIndex);
				c_wStead = WPG_INSPECTENTRYLIST;
				c_dlInfo.wMsg = (cProperID & 1) ? 48 : 41;		// "USB 메모리를 찾고 있습니다."
				//pArch->UsbScaning(true);
			}
			else	pArch->GetInspectEntry(dwIndex, pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal);
			break;
		case DOWNLOADITEM_TRACEENTRY :
			if (c_dlInfo.wMsg == 40)	c_dlInfo.wMsg = (cProperID & 1) ? 48 : 41;
			break;
		case DOWNLOADITEM_MAKEDESTINATION :
			if (pBulk->m[BPID_OCCASN].iResult == BULKRES_OK) {
#if	defined(_DEBUG_DOWNLOAD_)
				TRACK("STG:make destination dir.\n");
#endif
				if (c_dlInfo.wItemClone == DOWNLOADITEM_GETLOGBOOKTEXT) {
					memset(&c_fl[FLI_MM], 0, sizeof(FILELIST));
					c_dlInfo.wItem = DOWNLOADITEM_GETLOGBOOKLIST;
					PENTRYSHAPE pEntry = c_dump.GetEntry(c_dlInfo.iEntryIndex);		// 이니셜을 포함한 파일 이름
					pArch->GetLogbookList(pEntry->szName, 0, 0);
				}
				else if (c_dlInfo.wItemClone == DOWNLOADITEM_GETINSPECTTEXT) {
					c_dlInfo.iEntryIndex = -1;
					c_dlInfo.size.cur = 0;
					pBulk->uiCurSize = 0;
					c_dlInfo.wItem = DOWNLOADITEM_NEXTINSPECTENTRY;
					c_dlInfo.bContinue = true;
				}
				else {
					TRACK("STG:Make destination but not continue!(%d)\n", c_dlInfo.wItem);
					InitialDownloadInfo();
					c_dlInfo.bFailur = true;
				}
			}
			else {
				TRACK("STG:Make destination failure!(%d)\n", c_dlInfo.wItem);
				InitialDownloadInfo();
				c_dlInfo.bFailur = true;
			}
			break;
		case DOWNLOADITEM_GETLOGBOOKLIST :
			c_fl[FLI_MM].wLength = (WORD)pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex;
			if (pBulk->m[BPID_OCCASN].bt.pad.sp.wLength > 0) {
				WORD w = 0;
				while (w < pBulk->m[BPID_OCCASN].bt.pad.sp.wLength) {
					if (c_fl[FLI_MM].wLength < MAX_FILELIST)
						memcpy(&c_fl[FLI_MM].list[c_fl[FLI_MM].wLength ++], &pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[w], sizeof(ENTRYSHAPE));
					++ w;
				}
			}
			if (pBulk->m[BPID_OCCASN].bt.pad.sp.wLength < SIZE_ENTRYBLOCK) {
#if	defined(_DEBUG_DOWNLOAD_)
				TRACK("STG:file length %d\n", c_fl[FLI_MM].wLength);
#endif
				if (c_fl[FLI_MM].wLength > 0) {
					c_dlInfo.iFileIndex = -1;
					c_dlInfo.size.cur = 0;
					pBulk->uiCurSize = 0;
					c_dlInfo.wItem = DOWNLOADITEM_NEXTLOGBOOKLIST;
					c_dlInfo.bContinue = true;
				}
				else {
					DownloadEnd(DOWNLOADITEM_LOGBOOKENTRY, 45);
					TRACK("STG:logbook empty.\n");
				}
			}
			else {
				DWORD dw = (WORD)pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex + SIZE_ENTRYBLOCK;
				pArch->GetLogbookList(NULL, dw, pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal);
			}
			break;
		case DOWNLOADITEM_NEXTLOGBOOKLIST :
			if (++ c_dlInfo.iFileIndex < (int)c_fl[FLI_MM].wLength) {
				if (c_bDownloadType) {
					pBulk->uiCurSize = c_dlInfo.size.cur;		// get을 하다가 copy로 넘어올 때 필요하다.
					pArch->CopyLogbook(c_fl[FLI_MM].list[c_dlInfo.iFileIndex].szName);		// 이니셜이 포함된 파일 이름
					c_dlInfo.wItem = DOWNLOADITEM_COPYLOGBOOKTEXT;
#if	defined(_DEBUG_DOWNLOAD_)
					TRACK("STG:copy %s", c_fl[FLI_MM].list[c_dlInfo.iFileIndex].szName);
#endif
				}
				else {
					pArch->GetLogbook(c_fl[FLI_MM].list[c_dlInfo.iFileIndex].szName);
					c_dlInfo.wItem = DOWNLOADITEM_GETLOGBOOKTEXT;
#if	defined(_DEBUG_DOWNLOAD_)
					TRACK("STG:get %s", c_fl[FLI_MM].list[c_dlInfo.iFileIndex].szName);
#endif
				}
			}
			else {
				DownloadEnd(DOWNLOADITEM_LOGBOOKENTRY, 45);
				TRACK("STG:logbook download end.\n");
			}
			break;
		case DOWNLOADITEM_COPYLOGBOOKTEXT :
			if (c_dlInfo.bCancel)	DownloadEnd(DOWNLOADITEM_LOGBOOKENTRY, 47);
			else {
				c_dlInfo.wItem = DOWNLOADITEM_NEXTLOGBOOKLIST;
				c_dlInfo.bContinue = true;
			}
			break;
		case DOWNLOADITEM_GETLOGBOOKTEXT :
			if (c_dlInfo.bCancel)	DownloadEnd(DOWNLOADITEM_LOGBOOKENTRY, 47);
			else {
				c_dlInfo.wItem = DOWNLOADITEM_PUTLOGBOOKTEXT;
				pArch->PutLogbook(NULL);
#if	defined(_DEBUG_DOWNLOAD_)
				TRACK("STG:get %d\n", pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex);
#endif
			}
			break;
		case DOWNLOADITEM_PUTLOGBOOKTEXT :
			if (c_dlInfo.bCancel)	DownloadEnd(DOWNLOADITEM_LOGBOOKENTRY, 47);
			else {
				c_dlInfo.size.cur += (int)pBulk->m[BPID_OCCASN].bt.pad.sp.wLength;
				if (!(pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal & (1 << BULKFID_FILEEND))) {
					c_dlInfo.wItem = DOWNLOADITEM_GETLOGBOOKTEXT;
					pArch->GetLogbook();
				}
				else {
					c_dlInfo.wItem = DOWNLOADITEM_NEXTLOGBOOKLIST;
					c_dlInfo.bContinue = true;
				}
			}
			break;
		case DOWNLOADITEM_NEXTINSPECTENTRY :
			TRACK("STG:next inspect entry\n");
			do {
				++ c_dlInfo.iEntryIndex;
				PENTRYSHAPE pEntry = c_dump.GetEntry(c_dlInfo.iEntryIndex);
				if (pEntry != NULL && pEntry->szName[0] != 0)	break;
			} while (c_dlInfo.iEntryIndex < MAXLENGTH_ENTRY);

			if (c_dlInfo.iEntryIndex >= MAXLENGTH_ENTRY)	DownloadEnd(DOWNLOADITEM_INSPECTENTRY, 46);
			else {
				PENTRYSHAPE pEntry = c_dump.GetEntry(c_dlInfo.iEntryIndex);
				c_dlInfo.wItem = DOWNLOADITEM_GETINSPECTTEXT;
				pArch->GetInspect(pEntry->szName);
#if	defined(_DEBUG_DOWNLOAD_)
				TRACK("STG:get %s", pEntry->szName);
#endif
			}
			break;
		case DOWNLOADITEM_GETINSPECTTEXT :
			if (c_dlInfo.bCancel)	DownloadEnd(DOWNLOADITEM_INSPECTENTRY, 47);
			else {
				c_dlInfo.wItem = DOWNLOADITEM_PUTINSPECTTEXT;
				pArch->PutInspect(NULL);
#if	defined(_DEBUG_DOWNLOAD_)
				TRACK("STG:get %d\n", pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex);
#endif
			}
			break;
		case DOWNLOADITEM_PUTINSPECTTEXT :
			if (c_dlInfo.bCancel)	DownloadEnd(DOWNLOADITEM_INSPECTENTRY, 47);
			else {
				c_dlInfo.size.cur += (int)pBulk->m[BPID_OCCASN].bt.pad.sp.wLength;
				if (!(pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal & (1 << BULKFID_FILEEND))) {
					c_dlInfo.wItem = DOWNLOADITEM_GETINSPECTTEXT;
					pArch->GetInspect();
				}
				else {
					c_dlInfo.wItem = DOWNLOADITEM_NEXTINSPECTENTRY;
					c_dlInfo.bContinue = true;
				}
			}
			break;
		default :	break;
		}
	}

	if (c_dlInfo.wItem == DOWNLOADITEM_LOGBOOKENTRY || c_dlInfo.wItem == DOWNLOADITEM_INSPECTENTRY || c_dlInfo.wItem == DOWNLOADITEM_TRACEENTRY) {
		if ((pBulk->wState & (1 << BULKSTATE_SEIZEUSB)) && (c_dlInfo.wMsg == 41 || c_dlInfo.wMsg == 48))
			c_dlInfo.wMsg = c_dlInfo.wItem == DOWNLOADITEM_INSPECTENTRY ? 43 : 42;
	}
	else if (c_dlInfo.wItem == DOWNLOADITEM_COPYLOGBOOKTEXT) {
		PBULKPACK pBulk = pArch->GetBulk();
		c_dlInfo.size.cur = pBulk->uiCurSize;
	}

	c_dlInfo.dwIndexClone = pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex;
	if (c_redit.wTimer > 0 && -- c_redit.wTimer == 0)	c_wChapter = 3;

	if (c_vbcc.wWait > 0 && -- c_vbcc.wWait == 0)	c_vbcc.bClear = false;
}

void CStage::Pave()
{
	GETPAPER(pPaper);
	CUCUTOFF();

	if (c_bSkip4Blank) {
		c_bSkip4Blank = false;
		return;
	}

	_TWATCHCLICK();
	WORD page = c_wPage;
	if (c_cWakeupSeq < 9) {
		MakeBlank();
		++ c_cWakeupSeq;
		page = 5;
	}
	else if (c_cWakeupSeq == 10 || c_cWakeupSeq == 11) {
		BaseSketch();
		c_cWakeupSeq = 12;
		page = 6;
	}
	else {
		if (c_rdScrap.cCmd != 0) {
			c_rdScrap.cCmd = 0;
			ReadPage();
			page = 7;
		}
		else if (c_wBookingPage < 0xffff) {
			OpenPage();
			if (c_wBookingPage == INSPECTENTRY_PAGE)	c_wChapter = c_wChapterBkupAtInsp;		// 171226, 1;		//InspectItemClear();

			c_wBookingPage = 0xffff;
			page = 8;
		}
		else {
			BYTE pg;
			if (c_wStead > WPG_NULL && c_wStead < WPG_MAX) {
				pg = (BYTE)c_wStead;
				c_wStead = WPG_NULL;
				page = 9;
			}
			else	pg = c_cMatrix[c_wChapter][c_wPage];
			switch (pg) {
			//case WPG_NULL :				break;
			case WPG_HEADBOX :			HeadLine();				break;
			case WPG_SERVICEA :			ServiceA();				break;
			case WPG_SERVICEB :			ServiceB();				break;
			//case WPG_TROUBLES :			ReadyListup();			break;
			case WPG_TROUBLELIST :		DumpList(true, true);	break;
			case WPG_STATUSLIST :		DumpList(false, true);	break;
			case WPG_IOSTATEDUMP :		DumpPioState();			break;
			case WPG_LINETEXTDUMP :		DumpLineText();			break;
			case WPG_LINESTATEDUMP :	DumpLineState();		break;
			case WPG_OLDTROUBLELIST :	DumpOldTrouble();		break;
			case WPG_LOGBOOKENTRYLIST :	DumpEntryList();		break;
			case WPG_INSPECTENTRYLIST :	DumpEntryList();		break;
			case WPG_CURRENTINSPECT :	CurrentInspect();		break;
			case WPG_READYEDIT :
				{
					DumpSetItem();
					c_wChapter = 1;
					if (c_redit.nItem < MERROR_OUTRANGETEMPSV0)	RegisterPage(TIMEWHEELEDIT_PAGE);	// 들어가면서 틀린 항목을 가르킬 수단이 필요하다.
					else	RegisterPage(TEMPOTHEREDIT_PAGE);
				}
				break;
			case WPG_NULL :	break;
			default :
				//TRACK("HDU>ERR:work page!(%d - %d)\n", c_wChapter, c_wPage);
				break;
			}
		}
		if (c_cMatrix[c_wChapter][++ c_wPage] == 0xff)	c_wPage = 0;
	}
	_TWATCHMEASURE(WORD, c_wWatch, 5);
	if (c_wWatch > c_wMaxWatch)	c_wMaxWatch = c_wWatch;
	c_wElapsedTime[page][0] = c_wWatch;
	if (c_wElapsedTime[page][1] < c_wElapsedTime[page][0])
		c_wElapsedTime[page][1] = c_wElapsedTime[page][0];

	PDOZEN pDoz = pPaper->GetDozen();
	if (!pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin)	ClearSheaf();
}

void CStage::ConfirmSize()
{
	TRACK("HDU\n");
	TRACK("HEADLINE     %d\n", sizeof(HEADLINESENTENCE));
	TRACK("UNITA PART   %d\n", sizeof(UNITAPSENTENCE));
	TRACK("UNITB PART   %d\n", sizeof(UNITBPSENTENCE));
	TRACK("IO & CAREER  %d\n", sizeof(IOCDUMPSENTENCE));
	TRACK("LINK & ENV.  %d\n", sizeof(LKEDUMPSENTENCE));
	TRACK("LINK STATE   %d\n", sizeof(LKSTATESENTENCE));
	TRACK("ERRORLIST    %d\n", sizeof(ELISTSENTENCE));
	TRACK("INSPECT      %d\n", sizeof(INSPECTSENTENCE));
	TRACK("ENTRYLIST    %d\n", sizeof(ENTRYSENTENCE));
	TRACK("USERSET      %d\n", sizeof(USERSETSENTENCE));
}

void CStage::ReceiveUserRequest(BYTE cCmd)
{
	GETPAPER(pPaper);
	GETARCHIVE(pArch);

//#if	defined(DEBUG_HDURECV)
//	if (!c_reflection.bScan)	TRACK("HDU:user request(%d)\n", cCmd);
//#endif

	switch (cCmd) {
	case HK_WAKEUPTRIGGER :
		c_cWakeupSeq = 0;
		break;
	case HK_WAKEUPEDNORMAL :
		c_cWakeupSeq = 10;
		break;
	case HK_WAKEUPEDOVERHAUL :
		c_cWakeupSeq = 11;
		break;
	case HK_BKSPACE :		break;
	case HK_ENTER :			break;
	case HK_UP :
	case HK_DOWN :
		if (!c_bListing && c_wListPage < LIST_MAX) {
			if (cCmd == HK_UP) {
				if (c_list[c_wListPage].wCur > 0)	-- c_list[c_wListPage].wCur;
			}
			else {
				if (c_list[c_wListPage].wCur < c_list[c_wListPage].wTotal)	++ c_list[c_wListPage].wCur;
				TRACK("STG:pg dn %d %d.\n", c_list[c_wListPage].wCur, c_list[c_wListPage].wTotal);
			}
		}
		break;
	case HK_LEFT :			break;
	case HK_RIGHT :			break;
	case HK_PGUP :			break;
	case HK_PGDOWN :		break;
	case HK_ESC :			break;
	// 180124
	case HK_T0 :	case HK_T1 :
		if (c_wListPage == LIST_SORTTROUBLE) {
			int iID = cCmd == HK_T0 ? 0 : (int)pPaper->GetLength() + 1;
			if (c_list[LIST_SORTTROUBLE].wID != iID) {
				c_list[LIST_SORTTROUBLE].wID = iID;
				c_list[LIST_SORTTROUBLE].wCur = c_list[LIST_SORTTROUBLE].wTotal = 0;
				ClearListSentence();
			}
		}
		break;
	case HK_CAR0 :	case HK_CAR1 :	case HK_CAR2 :	case HK_CAR3 :		// 통신 내용에서 객차 선택
	case HK_CAR4 :	case HK_CAR5 :	case HK_CAR6 :	case HK_CAR7 :
	case HK_CAR8 :	case HK_CAR9 :
		// 180124
		if (c_wListPage == LIST_SORTTROUBLE) {
			int iID = (int)(cCmd - HK_CAR0 + 1);
			if (c_list[LIST_SORTTROUBLE].wID != iID) {
				c_list[LIST_SORTTROUBLE].wID = iID;
				c_list[LIST_SORTTROUBLE].wCur = c_list[LIST_SORTTROUBLE].wTotal = 0;
				ClearListSentence();
			}
		}
		else	c_dicID.wCar = cCmd;
		break;
	case HK_SIDE :	case HK_V3F :	case HK_ECU :	case HK_SIV :	// 통신 내용에서 장치 선택
	case HK_DCU :
		c_dicID.wDev = cCmd;
		if (pPaper->GetTenor())	c_dicID.wCar = c_dicID.wDev == HK_V3F ? HK_CAR8 : HK_CAR9;
		else	c_dicID.wCar = c_dicID.wDev == HK_V3F ? HK_CAR1 : HK_CAR0;
		break;
	case HK_INSPRUN :			// 차상 시험 시작
		//if (pPaper->GetOblige() & (1 << OBLIGE_HEAD)) {
		if (pPaper->IsHead()) {
			c_rdScrap.cCmd = c_rdScrap.cResp = cCmd;
			c_rdScrap.wHduAddr = ADDR_INSPECTSELMAP;
			c_rdScrap.wHduLength = 2;
			// 171226
			c_wChapterBkupAtInsp = c_wChapter;
			c_wChapter = 2;
		}
		break;
	case HK_INSPSTOP :			// 차상 시험 중지
		//if (pPaper->GetOblige() & (1 << OBLIGE_HEAD)) {
		if (pPaper->IsHead()) {
			GETREVIEW(pRev);
			if (pRev->GetInspectItem() != 0)	pRev->SetInspectStep(INSPSTEP_CANCELREADY);
		}
		break;
	case HK_INSPRETRY :			// 차상 시험 다시
	case HK_INSPSKIP :			// 차상 시험 건너 뛰기
		{
			GETREVIEW(pRev);
			if (pRev->GetInspectStep() == INSPSTEP_PAUSE) {
				if (cCmd == HK_INSPRETRY)	pRev->InspectRetry();
				else {
					pRev->InspectSkip();
					RegisterPage(pRev->GetInspectHduPage());
				}
			}
		}
		break;
	case HK_CANCEL :
		if (c_dlInfo.iEntryIndex >= 0) {
			pArch->DownloadCancel();
			c_dlInfo.bCancel = true;
			c_dlInfo.iEntryIndex = -1;
			c_dlInfo.iFileIndex = -1;
			c_dlInfo.wMsg = 47;
		}
		else {
			CLand* pLand = (CLand*)c_pParent;
			ASSERTP(pLand);
			CLand::PTRACEINFO pTraceInfo = pLand->GetTraceInfo();
			if (pTraceInfo->wItem != 0)	pLand->SuspendTraceSphere();
		}
		break;
	case HK_DOWNLOAD :				// 받기, 다운로드
		//if (pPaper->GetOblige() & (1 << OBLIGE_MAIN)) {
		//if ((pPaper->IsMain() && !c_bAuxPipe) || (!pPaper->IsMain() && c_bAuxPipe)) {
		if (IsActiveRoad()) {
			if (c_dlInfo.wItem == DOWNLOADITEM_LOGBOOKENTRY || c_dlInfo.wItem == DOWNLOADITEM_INSPECTENTRY || c_dlInfo.wItem == DOWNLOADITEM_TRACEENTRY) {
				c_dlInfo.bCancel = false;
				if (c_dlInfo.wItem == DOWNLOADITEM_LOGBOOKENTRY || c_dlInfo.wItem == DOWNLOADITEM_TRACEENTRY) {
					// * TRACE 1
					// DU에서 받기 단추를 눌렀다 -> 어떤 항목이 선택되었는지 검사한다.
					c_rdScrap.cCmd = c_rdScrap.cResp = cCmd;
					c_rdScrap.wHduAddr = ADDR_ENTRYSELMAP;
					c_rdScrap.wHduLength = 4;
				}
				else	DownloadInspect();
				c_dlInfo.wMsg = 44;
			}
		}
		break;
	case HK_EXAMIN :			// 검수
		InitialPage();
		c_bAuxPipe = false;
		c_wChapter = 1;
		pArch->UsbScaning(true);		//(false); 검수 상태에서는 항상 USB를 검사한다.
		break;
	case HK_GENERAL :			// 운전 정보
		InitialPage();
		c_wChapter = 0;
		c_dwDoorTestMap = 0;
		pArch->UsbScaning(false);		// 운전 상태에서는 USB 검사를 취소한다.
		break;
	case HK_CLEARV3FBCNT :
		{
			c_vbcc.bClear = true;
			c_vbcc.wWait = WAIT_V3FBCCLR;
		}
		break;
	case HK_DOORTEST :			// 출입문 시험
		//if (pPaper->GetOblige() & (1 << OBLIGE_HEAD)) {
		if (pPaper->IsHead()) {
			c_rdScrap.cCmd = c_rdScrap.cResp = cCmd;
			c_rdScrap.wHduAddr = ADDR_DOORTESTMAP;
			c_rdScrap.wHduLength = 2;
		}
		break;
	case HK_LINELIST :			// 놓쳤다. 찾을 것
		c_dicID.wCar = HK_CAR0;
		c_dicID.wDev = HK_SIDE;
		break;
	case HK_LISTSORTTROUBLE :
		c_wListPage = LIST_SORTTROUBLE;
		// 180124
		//c_list[LIST_SORTTROUBLE].wID = c_list[LIST_SORTTROUBLE].wCur = c_list[LIST_SORTTROUBLE].wTotal = 0;
		{
			PDOZEN pDoz = pPaper->GetDozen();
			c_list[LIST_SORTTROUBLE].wID = (pDoz->recip[FID_HEAD].real.prol.cProperID < PID_RIGHT) ? 0 : pPaper->GetLength() + 1;
		}
		c_list[LIST_SORTTROUBLE].wCur = c_list[LIST_SORTTROUBLE].wTotal = 0;
		ClearListSentence();
		break;
	case HK_LOGBOOKENTRY :
		c_bAuxPipe = false;
		//if (pPaper->GetOblige() & (1 << OBLIGE_MAIN)) {
		if (pPaper->IsMain()) {
			ClearEntrySentence();
			c_dump.ClearEntryList();
			InitialDownloadInfo();
			c_dlInfo.wItem = DOWNLOADITEM_LOGBOOKENTRY;
			c_dlInfo.wMsg = 40;						// "목록을 가져오고 있습니다. 잠시 기다립시오."
			pArch->GetLogbookEntry((DWORD)0, (DWORD)LENGTH_LOGBOOKENTRY);
			//TRACK("STG:Msg is 40\n");
		}
		break;
	case HK_LOGBOOKENTRYAUX :
		{
			if (pPaper->IsMain()) {
				int fid = (CAR_FSELF() < FID_TAIL) ? FID_HEADBK : FID_TAILBK;
				if (pPaper->GetRouteState(fid))	c_bAuxPipe = true;	// 180911, 전원 투입 후, 확인
			}
			else {
				c_bAuxPipe = true;
				ClearEntrySentence();
				c_dump.ClearEntryList();
				InitialDownloadInfo();
				c_dlInfo.wItem = DOWNLOADITEM_LOGBOOKENTRY;
				c_dlInfo.wMsg = 40;
				pArch->GetLogbookEntry((DWORD)0, (DWORD)LENGTH_LOGBOOKENTRY);
				//TRACK("STG:Msg is 40\n");
			}
		}
		break;
	case HK_INSPECTENTRY :
		//if (pPaper->GetOblige() & (1 << OBLIGE_MAIN)) {
		if (pPaper->IsMain()) {
			ClearEntrySentence();
			c_dump.ClearEntryList();
			InitialDownloadInfo();
			c_dlInfo.wItem = DOWNLOADITEM_INSPECTENTRY;
			c_dlInfo.wMsg = 40;						// "목록을 가져오고 있습니다. 잠시 기다립시오."
			pArch->GetInspectEntry((DWORD)0, (DWORD)((LENGTH_MONTHLYENTRY << 16) | (LENGTH_DAILYENTRY << 8) | LENGTH_PDTENTRY));
		}
		break;
	case HK_TRACEENTRY :
		//if (pPaper->GetOblige() & (1 << OBLIGE_MAIN)) {
		if (pPaper->IsMain()) {
			if (c_dlInfo.wItem != DOWNLOADITEM_TRACEENTRY) {
				InitialDownloadInfo();
				c_dlInfo.wItem = DOWNLOADITEM_TRACEENTRY;

				CLand* pLand = (CLand*)c_pParent;
				ASSERTP(pLand);
				BYTE cProperID = pLand->GetProperID();
				c_dlInfo.wMsg = (cProperID & 1) ? 48 : 41;					// "USB 메모리를 찾고 있습니다."
			}
		}
		break;
	case HK_TRACECLEAR :
		{
			CLand* pLand = (CLand*)c_pParent;
			ASSERTP(pLand);
			pLand->SetTraceClear(true);
		}
		break;
	case HK_RELEASEBLOCK :
		{
			CLand* pLand = (CLand*)c_pParent;
			ASSERTP(pLand);
			// PIS CONTROL CODE #3
			pLand->ReleaseServiceBlock();
		}
		break;
	case HK_LISTSTATUS :		// 상태 정보
		c_wListPage = LIST_STATE;
		c_list[LIST_STATE].wID = c_list[LIST_STATE].wCur = c_list[LIST_STATE].wTotal = 0;
		break;
	case HK_LISTTROUBLE :		// 고장 정보
		c_wListPage = LIST_TROUBLE;
		c_list[LIST_TROUBLE].wID = c_list[LIST_TROUBLE].wCur = c_list[LIST_TROUBLE].wTotal = 0;
		break;
	case HK_EXITEDIT :
		{
			int res = pArch->CheckEnvirons();
			if (res == MERROR_NON) {
				// 181003, it is really useless
				//InitialPage();
				//RegisterPage(OVERHAUL_PAGE);
			}
			else if (res < MERROR_OUTRANGETEMPSV0)	RegisterPage(TIMEWHEELEDIT_PAGE);	// 들어가면서 틀린 항목을 가르킬 수단이 필요하다.
			else	RegisterPage(TEMPOTHEREDIT_PAGE);
		}
		break;
	case HK_CMCHARGE :
		{
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			if (pLsv->sivcf.wCorrectMap != 0 && pLsv->wCmgBitmap != 0 && pLsv->wCmkBitmap == 0) {
				if (!pLsv->bManualCmg) {
					pLsv->bManualCmg = true;
					pLsv->wManualCmgCounter = COUNT_MANUALCMG;
				}
			}
		}
		break;
	case HK_LAMPTEST :
		{
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			pLsv->bManualAlbo = !pLsv->bManualAlbo;
		}
		break;
	case HK_SETENVIRONS :
	case HK_SETWHEEL :
	case HK_SETTIME :
	case HK_SETCAREER :
		c_rdScrap.cCmd = c_rdScrap.cResp = cCmd;
		c_rdScrap.wHduAddr = ADDR_USERSETSENTENCE;
		c_rdScrap.wHduLength = sizeof(USERSETSENTENCE) >> 1;
		break;
	// 180511
	// 171107
	//case HK_NEXTTROUBLE :
	//case HK_AGREESEVERE :
	//	{
	//		GETVERSE(pVerse);		// 180511, some trouble
	//		PBUNDLES pBund = pVerse->GetBundle();
	//		WORD code = XWORD(hdlb.wTroubleCode);
	//		WORD cid = XWORD(hdlb.wTroubleID);
	//		pArch->Hide((int)cid, code);
	//	}
	//	break;

	// 200218
	case HK_CLEARLINEFAULTCOUNTS :
		{
			CLand* pLand = (CLand*)c_pParent;
			ASSERTP(pLand);
			pLand->StirClearLfc();
		}
		break;
	case HK_FIREAGREE0 :	case HK_FIREAGREE1 :	case HK_FIREAGREE2 :	case HK_FIREAGREE3 :
	case HK_FIREAGREE4 :	case HK_FIREAGREE5 :	case HK_FIREAGREE6 :	case HK_FIREAGREE7 :
	case HK_FIREAGREE8 :	case HK_FIREAGREE9 :
		{
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			// 180626
			//pLsv->fire.wUserCmd[cCmd - HK_FIREAGREE0] = CProse::UCF_AGREE;
			if (!pPaper->GetDeviceExFromRecip())
				pLsv->fire.wUserCmd = CProse::UCF_AGREE;
			else {
				pLsv->fire.bAck = true;
				TRACK("FDS4: driver's ack button\n");
			}
		}
		break;
	case HK_FIRECANCEL0 :	case HK_FIRECANCEL1 :	case HK_FIRECANCEL2 :	case HK_FIRECANCEL3 :
	case HK_FIRECANCEL4 :	case HK_FIRECANCEL5 :	case HK_FIRECANCEL6 :	case HK_FIRECANCEL7 :
	case HK_FIRECANCEL8 :	case HK_FIRECANCEL9 :
		{
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			// 180626
			//pLsv->fire.wUserCmd[cCmd - HK_FIRECANCEL0] = CProse::UCF_IGNORE;
			// FD STEP 4: cature driver action
			if (!pPaper->GetDeviceExFromRecip())	pLsv->fire.wUserCmd = CProse::UCF_IGNORE;
			else {
				pLsv->fire.bAck = false;
				TRACK("FDS5: driver's cancel button\n");
			}
		}
		break;
	case HK_FIREBUZZ :
		if (pPaper->GetDeviceExFromRecip()) {
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			pLsv->fire.bMbCancel ^= true;
			TRACK("FDS6: driver's mbc button\n");
		}
		break;
	case HK_FIRECLOSE :
		{
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			pLsv->fire.wSeq = CProse::FS_CLOSE;
			TRACK("FDS7: driver's mbc button\n");
		}
		break;
	//case HK_TLISTTU0 :	case HK_TLISTCU0 :	case HK_TLISTCU1 :	case HK_TLISTCU2 :
	//case HK_TLISTCU3 :	case HK_TLISTCU4 :	case HK_TLISTCU5 :	case HK_TLISTCU6 :
	//case HK_TLISTCU7 :	case HK_TLISTCU8 :	case HK_TLISTCU9 :	case HK_TLISTTU9 :
	//	{
	//		int iID = (int)(cCmd - HK_TLISTTU0);
	//		if (c_list[LIST_SORTTROUBLE].wID != iID) {
	//			c_list[LIST_SORTTROUBLE].wID = iID;
	//			c_list[LIST_SORTTROUBLE].wCur = c_list[LIST_SORTTROUBLE].wTotal = 0;
	//			ClearListSentence();
	//		}
	//	}
	//	break;
	case HK_TEXT :	ReceiveUserDetail();	break;
	default :	break;
	}
}

//void CStage::TurnFireDialog()
//{
//	c_wChapterBkupAtFire = c_wChapter;
//	c_wChapter = 0;
//	c_wBookingPage = NORMAL_PAGE;
//}
//
void CStage::SetParent(PVOID pVoid)
{
	c_pParent = pVoid;
	c_usual.SetProp(pVoid, this);
	c_dump.SetProp(pVoid, this);
}

ENTRY_CONTAINER(CStage)
	//SCOOP(&c_dlInfo.iEntryIndex,		sizeof(int),	"Dli")
	//SCOOP(&c_dlInfo.iFileIndex,			sizeof(int),	"")
	SCOOP(&c_dlInfo.iEntryIndex,		sizeof(int),	"STG")
	SCOOP(&c_dlInfo.wItem,				sizeof(WORD),	"")
	//SCOOP(&c_dlInfo.dwIndexClone,		sizeof(DWORD),	"")
	//SCOOP(&c_dlInfo.size.cur,			sizeof(uint64_t), "cur")
	//SCOOP(&c_dlInfo.size.total,			sizeof(uint64_t), "tol")
	SCOOP(&c_wChapter,					sizeof(WORD),	"chap")
	SCOOP(&c_wPage,						sizeof(WORD),	"")
	SCOOP(&c_wBookingPage,				sizeof(WORD),	"")
	//SCOOP(&c_reflection.wLength,		sizeof(WORD),	"")
	SCOOP(&c_list[LIST_TROUBLE].wCur,	sizeof(WORD),	"lit")
	SCOOP(&c_list[LIST_TROUBLE].wTotal,	sizeof(WORD),	"")
	SCOOP(&c_list[LIST_STATE].wCur,		sizeof(WORD),	"lis")
	SCOOP(&c_list[LIST_STATE].wTotal,	sizeof(WORD),	"")
	SCOOP(&c_wWatch,					sizeof(WORD),	"")
	SCOOP(&c_wMaxWatch,					sizeof(WORD),	"")
	SCOOP(&c_pisReq.bRecog,				sizeof(BYTE),	"")
	SCOOP(&c_pisReq.bStatus,			sizeof(BYTE),	"")
	//SCOOP(&c_wScreenOut,				sizeof(WORD),	"scr")
	SCOOP(&c_wElapsedTime[0][0],		sizeof(WORD),	"st0")
	SCOOP(&c_wElapsedTime[0][1],		sizeof(WORD),	"")
	SCOOP(&c_wElapsedTime[1][0],		sizeof(WORD),	"")
	SCOOP(&c_wElapsedTime[1][1],		sizeof(WORD),	"")
	SCOOP(&c_wElapsedTime[2][0],		sizeof(WORD),	"")
	SCOOP(&c_wElapsedTime[2][1],		sizeof(WORD),	"")
	SCOOP(&c_wElapsedTime[3][0],		sizeof(WORD),	"")
	SCOOP(&c_wElapsedTime[3][1],		sizeof(WORD),	"")
	SCOOP(&c_wElapsedTime[4][0],		sizeof(WORD),	"")
	SCOOP(&c_wElapsedTime[4][1],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[5][0],		sizeof(WORD),	"st5")
	//SCOOP(&c_wElapsedTime[5][1],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[6][0],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[6][1],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[7][0],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[7][1],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[8][0],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[8][1],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[9][0],		sizeof(WORD),	"")
	//SCOOP(&c_wElapsedTime[9][1],		sizeof(WORD),	"")
EXIT_CONTAINER()
