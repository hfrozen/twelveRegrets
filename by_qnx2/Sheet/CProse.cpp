/*
 * CProse.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <string.h>

#include "Track.h"
#include "CPaper.h"
#include "CArch.h"
#include "CSch.h"
#include "CVerse.h"
#include "CReview.h"

#include "CProse.h"

// 201116
#define	UGLYDEB_DIRTYLOCAL		20

const WORD CProse::c_wFlapLength1[DEVID_CM][LL_MAX] = {
		{	SIZE_SIVSR,		sizeof(SIVAINFOTRCEX),	LINEDEB_LOCAL	},
		{	SIZE_V3FSR,		sizeof(V3FAINFOTRC),	LINEDEB_LOCAL	},
		{	SIZE_ECUSR,		SIZE_ECUSA,				LINEDEB_LOCAL	},
		{	SIZE_HVACSR,	SIZE_HVACSA,			LINEDEB_LOCAL	},
		// 200218
		{	SIZE_CMSBSR,	SIZE_CMSBSA,			LINEDEB_LOCAL	},
		{	SIZE_DCUSR,		SIZE_DCUSA,				LINEDEB_LOCAL	},
		{	SIZE_DCUSR,		SIZE_DCUSA,				LINEDEB_LOCAL	}
};

const WORD CProse::c_wFlapLength2[DEVID_CM][LL_MAX] = {
		{	SIZE_SIVSR,		sizeof(SIVAINFOTRCEX),	LINEDEB_LOCAL	},
		{	SIZE_V3FSR,		sizeof(V3FAINFOTRC),	LINEDEB_LOCAL	},
		{	SIZE_ECUSR,		SIZE_ECUSA,				UGLYDEB_DIRTYLOCAL	},
		{	SIZE_HVACSR,	SIZE_HVACSA,			UGLYDEB_DIRTYLOCAL	},
		{	SIZE_CMSBSR,	SIZE_CMSBSA,			UGLYDEB_DIRTYLOCAL	},
		{	SIZE_DCUSR,		SIZE_DCUSA,				LINEDEB_LOCAL	},
		{	SIZE_DCUSR,		SIZE_DCUSA,				LINEDEB_LOCAL	}
};

const BYTE CProse::c_cEcuMajfs1[] = {
	0,		// BYTE	cFlow;			// 0
	0,		// __BC(	ref,		// 1
			//				b,		1,
			//				p,		1,
			//				fsb,	1,
			//				hb,		1,
			//				eb,		1,
			//				sp,		2,
			//				slide,	1);
	0,		// BYTE	cBe;			// 2, brake effort, 0~100%(0~0xff)
	0,		// BYTE	cAsp;			// 3, 0~10bar(0~0xff), 1bar = 1.01972kg/cm^2
	0,		// BYTE	cLw;			// 4, 20~75t(0~0xff)
	0,		// BYTE	cBed;			// 5, demand, 0~120kN(0~0xff)
	0,		// BYTE	cBea;			// 6, attain, 0~120kN(0~0xff)
	0,		// BYTE	cEp;			// 7, 0~120kN(0~0xff)
	0,		// BYTE	cBcp;			// 8, 0~10bar(0~0xff)
	0,		// BYTE	cSpr;			// 9
	2,		// __BC(	ev,				// 10
			//				min,	1,
			//				*maj,	1,
			//				sp,		2,
			//				astg,	1,		// anti-skid testing
			//				astr,	1,		// anti-skid test result
			//				btg,	1,		// brake testing
			//				btr,	1);		// brake test result
	4,		// __BC(	fa,				// 11
			//				dvfr,	1,
			//				dvff,	1,
			//				*emvf,	1,
			//				lwf,	1,
			//				rbedf,	1,
			//				sp,		2,
			//				pbuf,	1);
	0,		// __BC(	fb,				// 12
			//				a4as,	1,
			//				a3as,	1,
			//				a2as,	1,
			//				a1as,	1,
			//				tpsf,	1,
			//				cpsf,	1,
			//				sp,		2);
	0,		// __BC(	fc,				// 13
			//				rbeaf,	1,
			//				tllf,	1,
			//				bcf,	1,
			//				ebrf,	1,
			//				sp,		4);
	0x46,	// __BC(	fd,				// 14
			//				alwf,	1,
			//				*mb04f,	1,
			//				*eb01f,	1,
			//				eb03f,	1,
			//				cb07f,	1,
			//				sp,		1,
			//				*r1hf,	1,
			//				r2hf,	1);
	0x9d,	// __BC(	fe,				// 15
			//				*r5hf,	1,
			//				r6hf,	1,
			//				*cmvf,	1,
			//				*vmvf,	1,
			//				*cvpsf,	1,
			//				rpsf,	1,
			//				cif,	1,
			//				*lrpd,	1);
	0, 0, 0, 0,	//BYTE cSpr16[14];		// 16~19
	0, 0, 0, 0, 0, 0, 0, 0,				// 20~27
	0, 0								// 28~29
};

const BYTE CProse::c_cEcuMajfs2[] = {
	0,		// BYTE	cFlow;			// 0
	0,		// __BC(	ref,		// 1
			//				b,		1,
			//				p,		1,
			//				fsb,	1,
			//				hb,		1,
			//				eb,		1,
			//				sp,		2,
			//				slide,	1);
	0,		// BYTE	cBe;			// 2, brake effort, 0~100%(0~0xff)
	0, 0, 0, 0,		// BYTE	wAsp[2];		// 3~6, 0~10bar(0~0xff), 1bar = 1.01972kg/cm^2
	0,		// BYTE	cLw;			// 7, 20~75t(0~0xff)
	0,		// BYTE	cBed;			// 8, demand, 0~120kN(0~0xff)
	0,		// BYTE	cBea;			// 9, attain, 0~120kN(0~0xff)
	0,		// BYTE	cEp;			// 10, 0~120kN(0~0xff)
	0,		// BYTE	cBcp;			// 11, 0~10bar(0~0xff)
	0,		// BYTE	cSpr;			// 12
	2,		// __BC(	ev,				// 13
			//				min,	1,
			//				*maj,	1,
			//				sp,		2,
			//				astg,	1,		// anti-skid testing
			//				astr,	1,		// anti-skid test result
			//				btg,	1,		// brake testing
			//				btr,	1);		// brake test result
	4,		// __BC(	fa,				// 14
			//				dvfr,	1,
			//				dvff,	1,
			//				*emvf,	1,
			//				lwf,	1,
			//				rbedf,	1,
			//				sp,		2,
			//				pbuf,	1);
	0,		// __BC(	fb,				// 15
			//				a4as,	1,
			//				a3as,	1,
			//				a2as,	1,
			//				a1as,	1,
			//				tpsf,	1,
			//				cpsf,	1,
			//				sp,		2);
	0,		// __BC(	fc,				// 16
			//				rbeaf,	1,
			//				tllf,	1,
			//				bcf,	1,
			//				ebrf,	1,
			//				sp,		4);
	0x46,	// __BC(	fd,				// 17
			//				alwf,	1,
			//				*mb04f,	1,
			//				*eb01f,	1,
			//				eb03f,	1,
			//				cb07f,	1,
			//				sp,		1,
			//				*r1hf,	1,
			//				r2hf,	1);
	0x9d,	// __BC(	fe,				// 18
			//				*r5hf,	1,
			//				r6hf,	1,
			//				*cmvf,	1,
			//				*vmvf,	1,
			//				*cvpsf,	1,
			//				rpsf,	1,
			//				cif,	1,
			//				*lrpd,	1);
	0,		//BYTE cSpr16[14];		// 19
	0, 0, 0, 0, 0, 0, 0, 0,				// 20~27
	0, 0								// 28~29
};

const BYTE CProse::c_cSivDets1[] = {
	0,				//	BYTE	cFlow;			// 0
	0,				//	__BC(	tst,			// 1
					//				st,		2,	// 01:testing, 10:test end
					//				vng,	1,	// voltage test result NG
					//				vok,	1,	// voltage test result OK
					//				fng,	1,	// frequency test result NG
					//				fok,	1,	// frequency test result OK
					//				sp,		1,
					//				sts,	1);
	0xe0,			//	__BC(	sta,			// 2
					//				sp0,	1,
					//				off9,	1,	// siv 9 off
					//				off5,	1,	// siv 5 off
					//				off0,	1,	// siv 0 off
					//				cttf,	1,	// contactor fail
					//				*gfux2,	1,
					//				*gfvy2,	1,
					//				*gfwz2,	1);
	0x40,			//	__BC(	stb,			// 3
					//				lk,		1,
					//				chk,	1,
					//				sivk,	1,
					//				sp,		2,
					//				start,	1,
					//				*pcd,	1,
					//				ivs,	1);
	0xff,			//	__BC(	stc,			// 4
					//				*ivoc,	1,
					//				*acoc,	1,
					//				*psft,	1,
					//				*bcoc,	1,
					//				*gfux1,	1,
					//				*gfvy1,	1,
					//				*gfwz1,	1,
					//				*thf,	1);
	0xff,			//	__BC(	std,			// 5
					//				*dcov,	1,
					//				*fcov,	1,
					//				*unft,	1,
					//				*acov,	1,
					//				*acuv,	1,
					//				*bcov,	1,
					//				*ovld,	1,
					//				*mjf,	1);
	0,				//	BYTE	cEs;			// 6, line voltage				0~3000V / 0~0xfa
	0,				//	BYTE	cIs;			// 7, invertor input current	0~500A / 0~0xfa
	0,				//	BYTE	cVo;			// 8, output voltage			0~750V / 0~0xfa
	0,				//	BYTE	cIo;			// 9, output current			0~750A / 0~0xfa
	0,				//	BYTE	cFo;			// 10, output frequency			0~125Hz / 0~0xfa
	0,				//	BYTE	cVs;			// 11, invertor input voltage	0~3000V / 0~0xfa
	0,				//	__BC(	trc,			// 12
					//				cok,	2,
					//				sp,		1,
					//				col,	1,
					//				leng,	4);
	1,				//	__BC(	ste,			// 13
					//				*igdf,	1,
					//				llvd,	1,
					//				sp,		2,
					//				iom,	4);
	0,				//	BYTE	cBatV;			// 170731, cSpr14;			// 14
					// 180626
	0, 0,			//	WORD	wVer;			// 15~16
	0, 0, 0, 0,		//	WORD	wSpr[4];		// 17~18/19~20/21~22/23~24
	0, 0, 0, 0
};

const BYTE CProse::c_cSivDets2[] = {
	0,				//BYTE	cFlow;			// 0
	0,				//__BC(	tst,			// 1
					//			st,		2,	// 01:testing, 10:test end
					//			vng,	1,	// voltage test result NG
					//			vok,	1,	// voltage test result OK
					//			fng,	1,	// frequency test result NG
					//			fok,	1,	// frequency test result OK
					//			sp,		1,
					//			sts,	1);
	0xf0,			//__BC(	sta,			// 2
					//			off9,	1,	// siv 9 off
					//			off5,	1,	// siv 5 off
					//			off0,	1,	// siv 0 off
					//			cttf,	1,	// contactor fail
					//			inov,	1,
					//			inoc,	1,
					//			chpfov,	1,
					//			chpfuv,	1);
					//__BC(	stb,			// 3
	0,				//			lk,		1,
					//			chk,	1,
					//			sivk,	1,
					//			bcchk,	1,
					//			bcik,	1,
					//			sp,		1,
					//			start,	1,
					//			ivs,	1);
	0xff,			//__BC(	stc,			// 4
					//			chpov,	1,
					//			chpoc,	1,
					//			invcov,	1,
					//			dcoc,	1,
					//			invoc,	1,
					//			acov,	1,
					//			acuv,	1,
					//			lovd,	1);
	0x3f,			//__BC(	std,			// 5
					//			acub,	1,
					//			acoc,	1,
					//			acol,	1,
					//			bcnoc,	1,
					//			bcoc,	1,
					//			bcov,	1,
					//			sp,		2);
	0xc7,			//__BC(	ste,			// 6
					//			chkf,	1,
					//			lkf,	1,
					//			sivkf,	1,
					//			sp,		3,
					//			stf,	1,
					//			igdf,	1);
	0,				//BYTE	cEs;			// 7, line voltage				0~3000V / 0~0xfa
	0,				//BYTE	cIs;			// 8, invertor input current	0~500A / 0~0xfa
	0,				//BYTE	cVo;			// 9, output voltage			0~750V / 0~0xfa
	0, 0,			//WORD	wIo;			// 10~11, output current			0~750A / 0~0x2ee
	0,				//BYTE	cFo;			// 12, output frequency			0~125Hz / 0~0xfa
	0,				//BYTE	cVs;			// 13, invertor input voltage	0~3000V / 0~0xfa
	0x3f,			//__BC(	stf,			// 14
					//			gdfc1,	1,
					//			gdfc2,	1,
					//			gdfru,	1,
					//			gdfrv,	1,
					//			gdfbu,	1,
					//			gdfbv,	1,
					//			sp,		2);
	0xc7,			//__BC(	stg,			// 15
					//			gdfu,	1,
					//			gdfv,	1,
					//			gdfw,	1,
					//			sp,		3,
					//			thf,	1,
					//			cplv,	1);
	0,				//__BC(	trc,			// 16
					//			cok,	2,
					//			sp,		1,
					//			col,	1,
					//			leng,	4);
	0,				//BYTE	cBatV;			// 17
	0, 0,			//WORD	wVer;			// 18~19
	0, 0, 0, 0		//BYTE	cSpr[5];		// 20~24
};

CProse::CProse()
{
	c_bSiv = c_bV3f = false;
	c_iFinalDcuID[0] = c_iFinalDcuID[1] = 0xff;
	c_bFinalAlter = false;
	c_cLocalRedID = 0;
	//c_ecuNote.cTotalLength = 0;
	c_eculr.bSync = true;
	c_eculr.wTimer = TIME_ECULOADRATESYNC;
	//c_bSivFound = false;
	//c_bSivEx = true;
	memset(&c_wDebs, 0, sizeof(WORD) * REALDEV_MAX);
	memset(&c_sivfltEx, 0, sizeof(SIVFLAPTRCEX) * LI_MAX);
	memset(&c_sivflt2, 0, sizeof(SIVFLAPTRC2) * LI_MAX);
	memset(&c_v3fflt, 0, sizeof(V3FFLAPTRC) * LI_MAX);
	memset(&c_ecufl, 0, sizeof(ECUFLAPS) * LI_MAX);
	memset(&c_hvacfl, 0, sizeof(HVACFLAP) * LI_MAX);
	memset(&c_cmsbfl, 0, sizeof(CMSBFLAP) * LI_MAX);
	memset(&c_dcufl, 0, sizeof(DCUFLAP) * LI_MAX * 8);
	// 200218
	memset(&c_wRed, 0, sizeof(WORD) * REALDEV_MAX);
	Initial();
}

CProse::~CProse()
{
}

void CProse::Initial()
{
	c_inspCmdBuf.a = 0;
	c_cSivTraceLength = c_cV3fTraceLength = c_cEcuTraceLength = 0;
	c_ecuNote.wPreID = c_ecuNote.wPreLength = c_ecuNote.wSufLength = 0;
	c_sivNote.wPreID = c_sivNote.wPreLength = c_sivNote.wSufLength = 0;
	c_sivNote2.wPreID = c_sivNote2.wPreLength = c_sivNote2.wSufLength = 0;
	c_bEcuTraceUpload = false;
	// 200218
	//memset(&c_wRed, 0, sizeof(WORD) * REALDEV_MAX);
	memset(&c_lsv, 0, sizeof(LSHARE));
	c_dcSw.Clear();
	c_acSw.Clear();

	c_lsv.sivcf.wWarmUpTimer = TIME_SIVWARMUP;
	c_lsv.v3fcf.wWarmUpTimer = TIME_V3FWARMUP;
	c_lsv.ecucf.wWarmUpTimer = TIME_ECUWARMUP;
	c_lsv.hvaccf.wWarmUpTimer = TIME_HVACWARMUP;
	c_lsv.cmsbcf.wWarmUpTimer = TIME_CMSBWARMUP;
	c_lsv.dcucf.wWarmUpTimer = TIME_DCUWARMUP;
	c_wEcuBlindCnt = 0;
}

//WORD CProse::GetSivType()
//{
//	if (!c_bSivFound)	return 0;
//	if (!c_bSivEx)	return 1;
//	return 2;
//}
//
//void CProse::SetSivType(bool bType)
//{
//	c_bSivEx = bType;
//	c_bSivFound = true;
//}
//
//WORD CProse::GetDebounce(int ldi)
//{
//	GETPAPER(pPaper);
//	PDOZEN pDoz = pPaper->GetDozen();
//	if (ldi == DEVID_ECU && !pDoz->recip[FID_HEAD].real.sign.scmdEx.b.lab)	return 1;
//	return c_wFlapLength[ldi][LL_DEB];
//}
//
int CProse::GetSerialID(int ldi, int iSubi)
{
	if (ldi < DEVID_DCUL)		return ldi;
	else if (ldi < DEVID_DCUR)	return ldi + iSubi;
	else if (ldi < DEVID_CM)	return ldi + iSubi + 3;
	else	return ldi + 6;
}

PVOID CProse::GetInfo(bool bDir, int ldi, int iSubi, int li)
{
	if (ldi < DEVID_CM && iSubi < 4) {
		if (ldi == DEVID_SIV)	c_bSiv = true;
		if (ldi == DEVID_V3F)	c_bV3f = true;

		switch (ldi) {
		case DEVID_SIV :
			{
				GETPAPER(pPaper);
				if (!pPaper->GetDeviceExFromRecip())
					return bDir ? (PVOID)&c_sivfltEx[li].ta : (PVOID)&c_sivfltEx[li].r;
				else	return bDir ? (PVOID)&c_sivflt2[li].ta : (PVOID)&c_sivflt2[li].r;
			}
			break;
		case DEVID_V3F :	return bDir ? (PVOID)&c_v3fflt[li].ta : (PVOID)&c_v3fflt[li].r;						break;
		case DEVID_ECU : {
				GETPAPER(pPaper);
				if (pPaper->GetDeviceExFromRecip())	return bDir ? (PVOID)&c_ecufl[li].ct.a : (PVOID)&c_ecufl[li].ct.r;
				else	return bDir ? (PVOID)&c_ecufl[li].co.a : (PVOID)&c_ecufl[li].co.r;
			}
			break;
		case DEVID_HVAC :	return bDir ? (PVOID)&c_hvacfl[li].a : (PVOID)&c_hvacfl[li].r;						break;
		case DEVID_CMSB :	return bDir ? (PVOID)&c_cmsbfl[li].a : (PVOID)&c_cmsbfl[li].r;						break;
		case DEVID_DCUL :	return bDir ? (PVOID)&c_dcufl[iSubi][li].a : (PVOID)&c_dcufl[iSubi][li].r;			break;
		default :			return bDir ? (PVOID)&c_dcufl[iSubi + 4][li].a : (PVOID)&c_dcufl[iSubi + 4][li].r;	break;
		}
	}
	TRACK("PROSE>%s>ERR:impossible reference!(ldi=%d, si=%d, li=%d)\n", __FUNCTION__, ldi, iSubi, li);
	return NULL;
}

WORD CProse::GetLength(bool bDir, int ldi)
{
	GETPAPER(pPaper);
	if (pPaper->GetDeviceExFromRecip())
		return c_wFlapLength2[ldi][bDir ? 1 : 0];
	return c_wFlapLength1[ldi][bDir ? 1 : 0];
}

WORD CProse::GetWarmup(int ldi)
{
	switch (ldi) {
	case DEVID_SIV :	return c_lsv.sivcf.wWarmUpTimer;	break;
	case DEVID_V3F :	return c_lsv.v3fcf.wWarmUpTimer;	break;
	case DEVID_ECU :	return c_lsv.ecucf.wWarmUpTimer;	break;
	case DEVID_HVAC :	return c_lsv.hvaccf.wWarmUpTimer;	break;
	case DEVID_CMSB :	return c_lsv.cmsbcf.wWarmUpTimer;	break;
	case DEVID_DCU :	return c_lsv.dcucf.wWarmUpTimer;	break;
	default :	break;
	}
	return 0;
}

void CProse::TriggerFireDetectionRelease()
{
	c_lsv.fire.wReleaseTimer = TIME_FIREDETECTIONRELEASE;
	GETPAPER(pPaper);
	if (pPaper->GetLabFromRecip())	TRACK("PRS:trigger fire detection release\n");
}

void CProse::Backup(int ldi, int iSubi, bool bTrace)
{
	GETPAPER(pPaper);
	PVOID pCurr = GetInfo(false, ldi, iSubi, LI_CURR);		// send frame, sdr
	PVOID pBkup = GetInfo(false, ldi, iSubi, LI_BKUP);
	memcpy(pBkup, pCurr, pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_REQ] : c_wFlapLength1[ldi][LL_REQ]);
	memset(pCurr, 0, pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_REQ] : c_wFlapLength2[ldi][LL_REQ]);
	if (bTrace)	return;									// 트레이스는 백업하지 않는다.
	pCurr = GetInfo(true, ldi, iSubi, LI_CURR);			// receive frame, sda
	pBkup = GetInfo(true, ldi, iSubi, LI_BKUP);
	memcpy(pBkup, pCurr, pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_ANS] : c_wFlapLength1[ldi][LL_ANS]);
	memset(pCurr, 0, pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_ANS] : c_wFlapLength1[ldi][LL_ANS]);
}

void CProse::Closure(int ldi, int iSubi)
{
	GETPAPER(pPaper);
	c_mtx.Lock();
	PVOID pCurr = GetInfo(false, ldi, iSubi, LI_CURR);
	PVOID pWork = GetInfo(false, ldi, iSubi, LI_WORK);
	memcpy(pWork, pCurr, pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_REQ] : c_wFlapLength1[ldi][LL_REQ]);
	pCurr = GetInfo(true, ldi, iSubi, LI_CURR);
	pWork = GetInfo(true, ldi, iSubi, LI_WORK);
	memcpy(pWork, pCurr, pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_ANS] : c_wFlapLength1[ldi][LL_ANS]);
	c_mtx.Unlock();
}

//void CProse::Success(int ldi, int iSubi)
//{
//	Closure(ldi, iSubi);
//	int id = GetSerialID(ldi, iSubi);
//	c_wLineState |= (1 << id);
//	c_wDebs[id] = 0;
//	if (ldi == DEVID_SIV) {
//		PSIVAINFOTRC pSivta = (PSIVAINFOTRC)GetInfo(true, DEVID_SIV, 0, LI_WORK);
//		if (pSivta->tt.s.cFlow != DEVTRACE_ANS)	c_cSivTraceLength = pSivta->tt.s.trc.b.leng;
//	}
//	else if (ldi == DEVID_V3F) {
//		PV3FAINFOTRC pV3fta = (PV3FAINFOTRC)GetInfo(true, DEVID_V3F, 0, LI_WORK);
//		if (pV3fta->tt.s.cFlow != DEVTRACE_ANS)	c_cV3fTraceLength = pV3fta->tt.s.trc.b.leng;
//	}
//	else if (ldi == DEVID_ECU) {
//		PECUAINFO pEcua = (PECUAINFO)GetInfo(true, DEVID_ECU, 0, LI_WORK);
//		c_cEcuTraceLength = pEcua->t.s.trc.b.leng;
//	}
//}
//
//bool CProse::Repair(int ldi, int iSubi, int recf)
//{
//	PVOID pc = GetInfo(true, ldi, iSubi, LI_CURR);
//	PVOID pb = GetInfo(true, ldi, iSubi, LI_BKUP);
//
//	int id = GetSerialID(ldi, iSubi);
//	bool bRes;
//	if (++ c_wDebs[id] >= c_wFlapLength[ldi][LL_DEB]) {
//		c_wDebs[id] = c_wFlapLength[ldi][LL_DEB];
//		*((BYTE*)pc + 2) = (BYTE)((0 - recf) & 0xff);	// cCtrl
//		if (GetWarmup(ldi) == 0) {
//			c_mtxRed.Lock();
//			INCWORD(c_wRed[id]);
//			c_mtxRed.Unlock();
//			c_wLineState &= ~(1 << id);
//		}
//		bRes = false;
//	}
//	else {
//		PVOID ps = GetInfo(false, ldi, iSubi, LI_CURR);
//		if (*((BYTE*)ps + 3) != DEVTRACE_REQ)	memcpy(pc, pb, c_wFlapLength[ldi][LL_ANS]);
//		c_wLineState |= (1 << id);
//		bRes = true;
//	}
//	Closure(ldi, iSubi);
//
//	return bRes;
//}
//
bool CProse::Repair(int ldi, int iSubi, int recf)
{
	PVOID pCurr = GetInfo(true, ldi, iSubi, LI_CURR);
	PVOID pBkup = GetInfo(true, ldi, iSubi, LI_BKUP);
	int id = GetSerialID(ldi, iSubi);
	bool bRes;
	//WORD wDebCnt = GetDebounce(ldi);
	//WORD wDebCnt = c_wFlapLength[ldi][LL_DEB];
	// 201116
	//WORD wDebCnt = 10;
	GETPAPER(pPaper);
	//if (pPaper->GetDeviceExFromRecip())	wDebCnt = c_wFlapLength2[ldi][LL_DEB];
	//else	wDebCnt = c_wFlapLength1[ldi][LL_DEB];
	WORD wDebCnt = pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_DEB] : c_wFlapLength1[ldi][LL_DEB];

	if (++ c_wDebs[id] >= wDebCnt) {
		c_wDebs[id] = wDebCnt;
		if (wDebCnt > 1)	c_wDebs[id] += LINEDEB_RECOVER;		// without ecu
		*((BYTE*)pCurr + 2) = (BYTE)((0 - recf) & 0xff);		// error code at cCtrl position
		if (GetWarmup(ldi) == 0) {
			c_mtxRed.Lock();
			INCWORD(c_wRed[id]);
			c_mtxRed.Unlock();
			c_wLineState &= ~(1 << id);
		}
		bRes = false;
		//if (ldi < DEVID_DCUL)	TRACK("%p REPAIR OUT %d %d\n", this, ldi, recf);
		//if (ldi == DEVID_ECU)	c_bEcuTimeout = true;
	}
	else {
		PVOID pSdr = GetInfo(false, ldi, iSubi, LI_CURR);
		if (*((BYTE*)pSdr + 3) != DEVTRACE_REQ)
			//memcpy(pCurr, pBkup, c_wFlapLength[ldi][LL_ANS]);
			memcpy(pCurr, pBkup, pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_ANS] : c_wFlapLength1[ldi][LL_ANS]);
		c_wLineState |= (1 << id);
		bRes = true;
	}
	Closure(ldi, iSubi);

	// 171019
	if (ldi == DEVID_ECU && c_wEcuBlindCnt < 0xffff)	++ c_wEcuBlindCnt;

	return bRes;
}

void CProse::Success(int ldi, int iSubi)
{
	bool bEnsure = true;
	int id = GetSerialID(ldi, iSubi);
	//WORD wDebCnt = GetDebounce(ldi);
	GETPAPER(pPaper);
	WORD wDebCnt = pPaper->GetDeviceExFromRecip() ? c_wFlapLength2[ldi][LL_DEB] : c_wFlapLength1[ldi][LL_DEB];
	if (c_wDebs[id] > wDebCnt && -- c_wDebs[id] > wDebCnt) {
		PVOID pCurr = GetInfo(true, ldi, iSubi, LI_CURR);
		*((BYTE*)pCurr + 2) |= 0x70;	// recover code at cCtrl position
		c_wLineState &= ~(1 << id);
		bEnsure = false;
	}
	else	c_wDebs[id] = 0;

	Closure(ldi, iSubi);
	// 171226
	if (ldi == DEVID_ECU)	c_eculr.bSync = false;

	if (bEnsure) {
		c_wLineState |= (1 << id);
		if (ldi == DEVID_SIV) {
			PSIVAINFOTRCEX pSivta = (PSIVAINFOTRCEX)GetInfo(true, DEVID_SIV, 0, LI_WORK);
			if (pSivta->tt.s.cFlow != DEVTRACE_ANS)	c_cSivTraceLength = pSivta->tt.s.trc.b.leng;
		}
		else if (ldi == DEVID_V3F) {
			PV3FAINFOTRC pV3fta = (PV3FAINFOTRC)GetInfo(true, DEVID_V3F, 0, LI_WORK);
			if (pV3fta->tt.s.cFlow != DEVTRACE_ANS)	c_cV3fTraceLength = pV3fta->tt.s.trc.b.leng;
		}
		else if (ldi == DEVID_ECU) {
			PECUAINFO pEcua = (PECUAINFO)GetInfo(true, DEVID_ECU, 0, LI_WORK);
			c_cEcuTraceLength = pEcua->t.s.trc.b.leng;
			//c_bEcuTimeout = false;
		}
	}
}

void CProse::Reform(PRECIP pRecipWork, PRECIP pRecipHead)
{
	c_mtx.Lock();

	// 181012
	//PRECIP pRecipHead;
	GETPAPER(pPaper);
	//PDOZEN pDoz = pPaper->GetDozen();

	//if ((pPaper->GetOblige() & ((1 << OBLIGE_MAIN) | (1 << OBLIGE_HEAD))) == ((1 << OBLIGE_MAIN) | (1 << OBLIGE_HEAD)))
	//if (CAR_FSELF() == FID_HEAD)
	//	pRecipHead = pRecip;		// 마스터는 수신받은 프레임이 아니다.
	//else	pRecipHead = &pDoz->recip[FID_HEAD];

	if (c_bSiv) {
		if (!pPaper->GetDeviceExFromRecip())
			memcpy(&pRecipWork->real.svf.s, &c_sivfltEx[LI_WORK], sizeof(SIVFLAPEX));
		else	memcpy(&pRecipWork->real.svf.s2, &c_sivflt2[LI_WORK], sizeof(SIVFLAP2));
		if (pRecipHead->real.cst.cID == CSTID_TRACE &&
			(pRecipHead->real.cst.roll.trc.req.wDevIDMap & (1 << TRACEITEM_SIV)) &&
			(pRecipHead->real.cst.roll.trc.req.wDevIDMap & (1 << CAR_PSELF()))) {
			pRecipWork->real.cst.cID = CSTID_TRACE;
			pRecipWork->real.cst.roll.trc.ans.wDevIDMap = ((1 << CAR_PSELF()) | (1 << TRACEITEM_SIV));
			// 200218
			//if (c_sivfltEx[LI_WORK].ta.tt.s.cFlow == DEVTRACE_ANS) {
			if ((!pPaper->GetDeviceExFromRecip() && c_sivfltEx[LI_WORK].ta.tt.s.cFlow == DEVTRACE_ANS) ||
				(pPaper->GetDeviceExFromRecip() && c_sivflt2[LI_WORK].ta.tt.s.cFlow == DEVTRACE_ANS)) {
				// * TRACE 6
				// 각 디바이스가 응답하는 트레이스 데이터들을 마스터로 보낸다.
				pRecipWork->real.cst.roll.trc.ans.wChapID = (WORD)c_sivfltEx[LI_WORK].ta.tt.t.cChapID;
				pRecipWork->real.cst.roll.trc.ans.wPageID = XWORD(c_sivfltEx[LI_WORK].ta.tt.t.wPageID);
				memcpy(pRecipWork->real.cst.roll.trc.c, c_sivfltEx[LI_WORK].ta.tt.t.c, SIZE_TRACETEXT);
			}
			else	pRecipWork->real.cst.roll.trc.ans.wChapID = 0xffff;	// 완료.
			PSIVAINFOTRCEX pSivta = (PSIVAINFOTRCEX)GetInfo(true, DEVID_SIV, 0, LI_BKUP);
			if (!pPaper->GetDeviceExFromRecip())
				memcpy(&pRecipWork->real.svf.s.a, pSivta, sizeof(SIVAINFOEX));
			else	memcpy(&pRecipWork->real.svf.s2.a, pSivta, sizeof(SIVAINFO2));
		}
	}
	else if (c_bV3f) {
		memcpy(&pRecipWork->real.svf.v, &c_v3fflt[LI_WORK], sizeof(V3FFLAP));
		if (pRecipHead->real.cst.cID == CSTID_TRACE &&
			(pRecipHead->real.cst.roll.trc.req.wDevIDMap & (1 << TRACEITEM_V3F)) &&
			(pRecipHead->real.cst.roll.trc.req.wDevIDMap & (1 << CAR_PSELF()))) {
			pRecipWork->real.cst.cID = CSTID_TRACE;
			pRecipWork->real.cst.roll.trc.ans.wDevIDMap = ((1 << CAR_PSELF()) | (1 << TRACEITEM_V3F));
			if (c_v3fflt[LI_WORK].ta.tt.s.cFlow == DEVTRACE_ANS) {
				pRecipWork->real.cst.roll.trc.ans.wChapID = (WORD)c_v3fflt[LI_WORK].ta.tt.t.cChapID;
				pRecipWork->real.cst.roll.trc.ans.wPageID = XWORD(c_v3fflt[LI_WORK].ta.tt.t.wPageID);
				memcpy(pRecipWork->real.cst.roll.trc.c, c_v3fflt[LI_WORK].ta.tt.t.c, SIZE_TRACETEXT);
			}
			else	pRecipWork->real.cst.roll.trc.ans.wChapID = 0xffff;	// 완료.
			PV3FAINFOTRC pV3fta = (PV3FAINFOTRC)GetInfo(true, DEVID_V3F, 0, LI_BKUP);
			memcpy(&pRecipWork->real.svf.v.a, pV3fta, sizeof(V3FAINFO));
		}
	}

	memcpy(&pRecipWork->real.ecu, &c_ecufl[LI_WORK], sizeof(ECUFLAP));
	// 200218
	// 171019
	//pRecipWork->real.ecu.a.t.s.wBlindCnt = c_wEcuBlindCnt;
	if (pPaper->GetDeviceExFromRecip())	pRecipWork->real.ecu.ct.a.t.s.wBlindCnt = c_wEcuBlindCnt;
	else	pRecipWork->real.ecu.co.a.t.s.wBlindCnt = c_wEcuBlindCnt;

	GETARCHIVE(pArch);
	PBULKPACK pBulk = pArch->GetBulk();
	// 200218
	//pRecipWork->real.ecu.a.t.s.trc.b.leng = pBulk->cEcuTraceLength;
	if (pPaper->GetDeviceExFromRecip())	pRecipWork->real.ecu.ct.a.t.s.trc.b.leng = pBulk->cEcuTraceLength;
	else	pRecipWork->real.ecu.co.a.t.s.trc.b.leng = pBulk->cEcuTraceLength;
	if (pRecipHead->real.cst.cID == CSTID_TRACE && (pRecipHead->real.cst.roll.trc.req.wDevIDMap & ((1 << TRACEITEM_ECU) | (1 << TRACEITEM_ALIGN)))) {
		pRecipWork->real.cst.cID = CSTID_TRACE;
		GETARCHIVE(pArch);
		PBULKPACK pBulk = pArch->GetBulk();
		if ((pRecipHead->real.cst.roll.trc.req.wDevIDMap & (1 << TRACEITEM_ECU))) {
			// * TRACE 6-2 :
			if (pBulk->cEcuTraceLength > 0) {
				pRecipWork->real.cst.roll.trc.ans.wDevIDMap = ((1 << CAR_PSELF()) | (1 << TRACEITEM_ECU));
				c_bEcuTraceUpload = true;		// 끝나고 트레이스 버퍼를 지운다.
				if (pArch->TakeoutEcuTrace(pRecipHead->real.cst.roll.trc.req.wChapID, pRecipHead->real.cst.roll.trc.req.wPageID, pRecipWork->real.cst.roll.trc.c)) {
					pRecipWork->real.cst.roll.trc.ans.wChapID = pRecipHead->real.cst.roll.trc.req.wChapID;
					pRecipWork->real.cst.roll.trc.ans.wPageID = pRecipHead->real.cst.roll.trc.req.wPageID;
				}
				else	pRecipWork->real.cst.roll.trc.ans.wChapID = 0xffff;	// 완료.
			}
			else	pRecipWork->real.cst.roll.trc.ans.wChapID = 0xffff;
		}
		else {
			// * TRACE 6-1 :
			// align 명령에서 트레이스 데이터들을 정렬하여 temp.로 옮긴다 -> 완료되면 wChapID = AETSTEP_END로 응답.
			// ArrangeEcuTrace() -> LoadEcuTraceFromTemporary()
			pRecipWork->real.cst.roll.trc.ans.wDevIDMap = ((1 << CAR_PSELF()) | (1 << TRACEITEM_ALIGN));
			if (pBulk->cEcuTraceLength > 0) {
				int step = pArch->GetEcuTraceStep();
				switch (step) {
				case AETSTEP_EMPTY :
					pArch->ArrangeEcuTrace();
					pArch->SetEcuTraceStep(AETSTEP_WAITARRANGE);
					break;
				case AETSTEP_WAITARRANGE :
					if (pBulk->wState & (1 << BULKSTATE_ARRANGEECUTRACE)) {
						pArch->LoadEcuTraceFromTemporary();
						pArch->SetEcuTraceStep(AETSTEP_WAITLOAD);
					}
					break;
				case AETSTEP_WAITLOAD :
					if (pBulk->wState & (1 << BULKSTATE_ARRANGEECUTRACE))	pBulk->wState &= ~(1 << BULKSTATE_ARRANGEECUTRACE);
					break;
				default :	break;
				}
				pRecipWork->real.cst.roll.trc.ans.wChapID = (WORD)step;
			}
			else {
				pArch->ClearEcuTrace();		// ecu trace buffer clear
				pRecipWork->real.cst.roll.trc.ans.wChapID = 0xffff;
			}
		}
	}
	else {
		if (c_bEcuTraceUpload) {
			c_bEcuTraceUpload = false;
			pArch->ClearEcuTrace();
		}
	}

	// 200218		c_bFinalAlter = 0: hvac, 1: cmsb
	//memcpy(&pRecipWork->real.hvac, &c_hvacfl[LI_WORK], sizeof(HVACFLAP));
	if (!c_bFinalAlter) {
		memcpy(&pRecipWork->real.hcb.hv, &c_hvacfl[LI_WORK], sizeof(HVACFLAP));
	}
	else {
		pRecipWork->real.hcb.cb.wSign = SIGN_CMSBFLAP;
		memcpy(&pRecipWork->real.hcb.cb.c, &c_cmsbfl[LI_WORK], sizeof(CMSBFLAP));
	}

	if (c_iFinalDcuID[0] < 4)	memcpy(&pRecipWork->real.dcul, &c_dcufl[c_iFinalDcuID[0]][LI_WORK], sizeof(DCUFLAP));
	if (c_iFinalDcuID[1] < 4)	memcpy(&pRecipWork->real.dcur, &c_dcufl[c_iFinalDcuID[1] + 4][LI_WORK], sizeof(DCUFLAP));

	// 200218
	//for (int n = 0; n < DEVID_DCUL; n ++)	pRecipWork->real.wLocRed[n] = c_wRed[n];
	pRecipWork->real.localf.cDevID = c_cLocalRedID;
	pRecipWork->real.localf.wCnt = c_wRed[c_cLocalRedID];
	if (++ c_cLocalRedID >= REALDEV_MAX)	c_cLocalRedID = 0;	// untill cmsb

	pRecipWork->real.sign.insp.b.cancel = c_lsv.wInspectCancelTime > 0 ? true : false;
	// 210720
	pRecipWork->real.sign.scmdEx.b.hicmd = c_inspCmdBuf.b.hvac;

	c_mtx.Unlock();
}

PVOID CProse::Decorate(int ldi, int iSubi)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	bool bTrace = false;
	if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_TRACE &&
		((ldi == DEVID_SIV &&
		(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << TRACEITEM_SIV)) &&
		(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << CAR_PSELF())) &&
		pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wChapID < (WORD)c_cSivTraceLength) ||
		(ldi == DEVID_V3F &&
		(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << TRACEITEM_V3F)) &&
		(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << CAR_PSELF())) &&
		(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wChapID < (WORD)c_cV3fTraceLength))))
		bTrace = true;

	// 170419:Backup()과 Fill4Check()에서 iSubi를 넣지않았더니 DCU SDR에서 id1~3의 버퍼가 완전히 지워지지 않고 이전의 것이 나갔다.
	Backup(ldi, iSubi, bTrace);
	//Fill4Check(ldi, iSubi);
	PVOID pSdr = GetInfo(false, ldi, iSubi, LI_CURR);
	GETSCHEME(pSch);
	int fid = pSch->C2F(pPaper->GetTenor(), CAR_PSELF());
	GETARCHIVE(pArch);

	switch (ldi) {
	case DEVID_SIV :
		{
			PSIVRINFO pSivr = (PSIVRINFO)pSdr;
			pSivr->wAddr = LADD_SIV;
			pSivr->cCtrl = DEVCHAR_CTL;

			//if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_TRACE &&
			//	(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << DEVID_SIV)) &&
			//	(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << CAR_PSELF())) &&
			//	pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wChapID < (WORD)c_cSivTraceLength) {
			if (bTrace) {
				// * TRACE 5 :
				// master가 요구하는 트레이스 데이터 내용을 각 디바이스에 전달한다.
				pSivr->t.t.cFlow = DEVTRACE_REQ;
				pSivr->t.t.cChapID = (BYTE)pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wChapID;
				pSivr->t.t.wPageID = XWORD(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wPageID);
				pSivr->t.t.cSpr = 0;
			}
			else {
				pSivr->t.s.cFlow = DEVFLOW_REQ;
				pSivr->t.s.cmd.b.tsr = false;
				if ((pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NDETECT_ZEROSPEED)) &&
					pDoz->recip[FID_HEAD].real.sign.insp.b.siv && !c_inspCmdBuf.b.siv) {
					// 200218
					//if (pDoz->recip[fid].real.svf.s.a.t.s.tst.b.st == 0)	pSivr->t.s.cmd.b.tsr = true;	// st가 0일 때만...!!!
					if ((!pPaper->GetDeviceExFromRecip() && pDoz->recip[fid].real.svf.s.a.t.s.tst.b.st == 0) ||
						(pPaper->GetDeviceExFromRecip() && pDoz->recip[fid].real.svf.s2.a.t.s.tst.b.st == 0))
						pSivr->t.s.cmd.b.tsr = true;
					else	c_inspCmdBuf.b.siv = true;
					// 기기의 st가 시험 완료 되었다고 0 이 되면 tsr이 다시 설정되어버린다.
					// 각 기기마다 반응 시간이 틀리므로 기기가 시험 상태로 들어가면 c_inspCmdBuf을 설정하여 tsr이 다시 설정 되는 일이 없어야 한다.
				}
				if (!pDoz->recip[FID_HEAD].real.sign.insp.b.siv)	c_inspCmdBuf.b.siv = false;

				pSivr->t.s.cmd.b.esk =
						pDoz->recip[FID_HEAD].real.sign.scmd.b.esk0 || pDoz->recip[FID_HEAD].real.sign.scmd.b.esk1 ? true : false;
				// 200218
				//if (pDoz->recip[FID_HEAD].real.sign.ecmd.b.tdc && c_sivfltEx[LI_WORK].ta.tt.s.trc.b.leng > 0)	pSivr->t.s.cmd.b.tdc = true;
				if (pDoz->recip[FID_HEAD].real.sign.ecmd.b.tdc) {
					if (!pPaper->GetDeviceExFromRecip()) {
						if (c_sivfltEx[LI_WORK].ta.tt.s.trc.b.leng > 0)	pSivr->t.s.cmd.b.tdc = true;
					}
					else {
						if (c_sivflt2[LI_WORK].ta.tt.s.trc.b.leng > 0)	pSivr->t.s.cmd.b.tdc = true;
					}
				}

				pSivr->t.s.cCid = CAR_PSELF();
				// 170726,
				//pArch->GetBcdDevTime(&pSivr->t.s.dt);
				// 170801
				//memcpy(&pSivr->t.s.dt, &pDoz->recip[FID_HEAD].real.prol.dtm, sizeof(DEVTIME));
				pArch->Dbft2BcdDevt(&pSivr->t.s.devt, pDoz->recip[FID_HEAD].real.prol.dbft);
			}
		}
		break;
	case DEVID_V3F :
		{
			PV3FRINFO pV3fr = (PV3FRINFO)pSdr;
			pV3fr->wAddr = LADD_V3F;
			pV3fr->cCtrl = DEVCHAR_CTL;

			//if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_TRACE &&
			//	(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << DEVID_V3F)) &&
			//	(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << CAR_PSELF())) &&
			//	pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wChapID < (WORD)c_cV3fTraceLength) {
			if (bTrace) {
				pV3fr->t.t.cFlow = DEVTRACE_REQ;
				pV3fr->t.t.cChapID = (BYTE)pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wChapID;
				pV3fr->t.t.wPageID = XWORD(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wPageID);
				pV3fr->t.t.cSpr = 0;
			}
			else {
				pV3fr->t.s.cFlow = DEVFLOW_REQ;
				// 170726,
				//pArch->GetBcdDevTime(&pV3fr->t.s.dt);
				// 170801
				//memcpy(&pV3fr->t.s.dt, &pDoz->recip[FID_HEAD].real.prol.dtm, sizeof(DEVTIME));
				pArch->Dbft2BcdDevt(&pV3fr->t.s.devt, pDoz->recip[FID_HEAD].real.prol.dbft);
				pV3fr->t.s.cmd.b.f = pDoz->recip[FID_HEAD].real.motiv.lead.b.f;
				pV3fr->t.s.cmd.b.r = pDoz->recip[FID_HEAD].real.motiv.lead.b.r;

				pV3fr->t.s.cst.b.atom = pDoz->recip[FID_HEAD].real.motiv.lead.b.atom;
				if (pDoz->recip[FID_HEAD].real.motiv.cMode == MOTIVMODE_YARD)	pV3fr->t.s.cmd.b.y = true;
				else if (pDoz->recip[FID_HEAD].real.motiv.cMode == MOTIVMODE_EMERGENCY)	pV3fr->t.s.cmd.b.fms = true;

				pV3fr->t.s.cmd.b.tsr = false;
				if ((pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NDETECT_ZEROSPEED)) &&
					pDoz->recip[FID_HEAD].real.sign.insp.b.v3f && !c_inspCmdBuf.b.v3f) {
					if (pDoz->recip[fid].real.svf.v.a.t.s.tst.b.st == 0)	pV3fr->t.s.cmd.b.tsr = true;
					else	c_inspCmdBuf.b.v3f = true;
				}
				if (!pDoz->recip[FID_HEAD].real.sign.insp.b.v3f)	c_inspCmdBuf.b.v3f = false;
				// 180108
				if (pDoz->recip[FID_HEAD].real.motiv.cEffType == EFFORT_BRAKING)	pV3fr->t.s.cmd.b.b = true;
				if (!pDoz->recip[FID_HEAD].real.sign.insp.b.v3f)
					pV3fr->t.s.cmd.b.p = (pDoz->recip[FID_HEAD].real.motiv.cEffType == EFFORT_POWERING) ? true : false;
				else {
					// 210630
					if (pDoz->recip[FID_HEAD].real.motiv.cEffType != EFFORT_BRAKING && !pV3fr->t.s.cmd.b.b &&
						pDoz->recip[FID_HEAD].real.motiv.cEffort >= PWMRATE_MINP1)	pV3fr->t.s.cmd.b.p = true;
					//if (pDoz->recip[FID_HEAD].real.motiv.cEffType != EFFORT_BRAKING && !pV3fr->t.s.cmd.b.b &&
					//	pDoz->recip[FID_HEAD].real.motiv.cEffort >= PWMRATE_MINP1 &&
					//	pPaper->GetDeviceExFromRecip() &&
					//	(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_PARKING)) &&
					//	(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)))
					//		pV3fr->t.s.cmd.b.p = true;
				}

				if (pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY))	pV3fr->t.s.cst.b.sb = true;
				if (pDoz->recip[FID_HEAD].real.drift.cEbrk != 0)	pV3fr->t.s.cst.b.eb = true;
				pV3fr->t.s.ref.b.cn = CAR_PSELF();
				if (pDoz->recip[FID_HEAD].real.sign.ecmd.b.tdc && c_v3fflt[LI_WORK].ta.tt.s.trc.b.leng > 0)	pV3fr->t.s.ref.b.tdc = true;
				int v3fid = pSch->GetV3fID(pV3fr->t.s.ref.b.cn);
				if (v3fid < 0xff) {
					// 180626
					//WORD w = ((WORD)pDoz->recip[FID_HEAD].real.cDiai[v3fid] + MIN_WHEELDIA) * 10;
					WORD w = ((WORD)c_lsv.cDiai[v3fid] + MIN_WHEELDIA) * 10;
					pV3fr->t.s.wWheel = XWORD(w);
					pV3fr->t.s.ref.b.vwc = true;
				}
				else	pV3fr->t.s.ref.b.vwc = false;
			}
		}
		break;
	case DEVID_ECU :
		{
			PECURINFO pEcur = (PECURINFO)pSdr;
			pEcur->wAddr = LADD_ECU;
			pEcur->cCtrl = DEVCHAR_CTL;
			pEcur->t.s.cFlow = DEVFLOW_REQ;
			// 170726,
			//pArch->GetBcdDevTime(&pEcur->t.s.dt);
			// 170801
			//memcpy(&pEcur->t.s.dt, &pDoz->recip[FID_HEAD].real.prol.dtm, sizeof(DEVTIME));
			pArch->Dbft2BcdDevt(&pEcur->t.s.devt, pDoz->recip[FID_HEAD].real.prol.dbft);
			if (pDoz->recip[FID_HEAD].real.motiv.cEffType == EFFORT_BRAKING)	pEcur->t.s.cmd.b.b = true;
			else if (pDoz->recip[FID_HEAD].real.motiv.cEffType == EFFORT_POWERING)	pEcur->t.s.cmd.b.p = true;
			// 171226
			//if (GETTI(FID_HEAD, TUDIB_DIR))	pEcur->t.s.cmd.b.dc = true;
			if (c_eculr.wTimer > 0)	pEcur->t.s.cmd.b.dc = false;
			else if (GETTI(FID_HEAD, TUDIB_DIR))	pEcur->t.s.cmd.b.dc = true;
			pEcur->t.s.cmd.b.atom = pDoz->recip[FID_HEAD].real.motiv.lead.b.atom;
			if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.unifTr ||
				(pPaper->GetRouteState(FID_TAIL) && pDoz->recip[FID_TAIL].real.sign.gcmd.b.unifTr))		// 180911, 통신이 안되면 1이 안될테니.. 확인
				pEcur->t.s.cmd.b.tset = true;
			else	pEcur->t.s.cmd.b.tset = false;
			// 171221
			if (GETTI(FID_HEAD, TUDIB_CPRS))	pEcur->t.s.cmd.b.cprs = true;
			pEcur->t.s.cmd.b.btr = false;
			pEcur->t.s.cmd.b.astr = false;
			if (pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NDETECT_ZEROSPEED)) {
				if (pDoz->recip[FID_HEAD].real.sign.insp.b.ecub && !c_inspCmdBuf.b.ecub) {
					// 200218
					// 171101
					//if (!pDoz->recip[fid].real.ecu.a.t.s.evt.b.bt)	pEcur->t.s.cmd.b.btr = true;
					//if (!pDoz->recip[fid].real.ecu.a.t.s.evt.b.btr)	pEcur->t.s.cmd.b.btr = true;	-- 보류
					//else	c_inspCmdBuf.b.ecub = true;
					if (pPaper->GetDeviceExFromRecip()) {
						if (!pDoz->recip[fid].real.ecu.ct.a.t.s.evt.b.bt)	pEcur->t.s.cmd.b.btr = true;
						else	c_inspCmdBuf.b.ecub = true;
					}
					else {
						if (!pDoz->recip[fid].real.ecu.co.a.t.s.evt.b.bt)	pEcur->t.s.cmd.b.btr = true;
						else	c_inspCmdBuf.b.ecub = true;
					}
				}
				else if (pDoz->recip[FID_HEAD].real.sign.insp.b.ecua && !c_inspCmdBuf.b.ecua) {
					// 200218
					//if (!pDoz->recip[fid].real.ecu.a.t.s.evt.b.ast)	pEcur->t.s.cmd.b.astr = true;
					//else	c_inspCmdBuf.b.ecua = true;
					if (pPaper->GetDeviceExFromRecip()) {
						if (!pDoz->recip[fid].real.ecu.ct.a.t.s.evt.b.ast)	pEcur->t.s.cmd.b.astr = true;
						else	c_inspCmdBuf.b.ecua = true;
					}
					else {
						if (!pDoz->recip[fid].real.ecu.co.a.t.s.evt.b.ast)	pEcur->t.s.cmd.b.astr = true;
						else	c_inspCmdBuf.b.ecua = true;
					}
				}
			}
			if (!pDoz->recip[FID_HEAD].real.sign.insp.b.ecub)	c_inspCmdBuf.b.ecub = false;
			if (!pDoz->recip[FID_HEAD].real.sign.insp.b.ecua)	c_inspCmdBuf.b.ecua = false;

			if (pDoz->recip[FID_HEAD].real.sign.ecmd.b.tdc) {
				PBULKPACK pBulk = pArch->GetBulk();
				if (pBulk->cEcuTraceLength > 0 && !(pBulk->wState & (1 << BULKSTATE_ERASEECUTRACE)))
					pArch->EraseEcuTrace();
			}
		}
		break;
	case DEVID_HVAC :
		{
			PHVACRINFO pHvacr = (PHVACRINFO)pSdr;
			pHvacr->wAddr = LADD_HVAC;
			pHvacr->cCtrl = DEVCHAR_CTL;
			pHvacr->t.s.cFlow = DEVFLOW_REQ;
			// 18/11/28
			//int cid = (int)CAR_PSELF();
			//if (GETCI(cid, CUDIB_FD))	pHvacr->t.s.req.b.fire = true;
			int fid = pPaper->GetFloatID();
			// 210806
			//if (GETCI(fid, CUDIB_FD))	pHvacr->t.s.req.b.fire = true;
			if (pPaper->GetDeviceExFromRecip()) {
				if (c_lsv.fire.wFromMaster & (1 << CAR_PSELF()))
					pHvacr->t.s.req.b.fire = true;
			}
			else {
				if (GETCI(fid, CUDIB_FD))	pHvacr->t.s.req.b.fire = true;
			}
			//pHvacr->t.s.req.b.ground
			//pHvacr->t.s.req.b.full
			//bool bTenor = pPaper->GetTenor();
			//if ((!bTenor && cid == 0) || (bTenor && cid == 9))	pHvacr->t.s.req.b.head = true;
			// 18/11/28
			// 21/08/06
			//if (fid == 0)	pHvacr->t.s.req.b.head = true;
			pHvacr->t.s.req.b.head = GETTI(FID_HEAD, TUDIB_HCR) ? true : false;
			// 170727
			//pHvacr->t.s.req.b.lor = pDoz->recip[FID_HEAD].real.sign.scmd.b.lor;
			//pHvacr->t.s.req.b.lrr = pDoz->recip[FID_HEAD].real.sign.scmd.b.lrr;
			// 170729
			//pHvacr->t.s.req.b.lor = pDoz->recip[FID_HEAD].real.sign.scmd.b.lor;
			//if (!pHvacr->t.s.req.b.lor) {
			//	if (pDoz->recip[FID_HEAD].real.sign.scmd.b.lrr) {
			//		if (pDoz->recip[FID_HEAD].real.sign.scmd.b.esk0 && !pDoz->recip[FID_HEAD].real.sign.scmd.b.esk1)
			//			pHvacr->t.s.req.b.lrr = (cid >= 0 && cid <= 5) ? true : false;
			//		else if (!pDoz->recip[FID_HEAD].real.sign.scmd.b.esk0 && pDoz->recip[FID_HEAD].real.sign.scmd.b.esk1)
			//			pHvacr->t.s.req.b.lrr = (cid >= 3 && cid <= 9) ? true : false;
			//		else	pHvacr->t.s.req.b.lrr = true;		// 여기는 실행되지 말아야할 부분인데...
			//	}
			//}
			if (GETTI(FID_HEAD, TUDIB_HCR))	pHvacr->t.s.req.b.head = true;  //220117

			int cid = (int)CAR_PSELF();
			BYTE cLength = pDoz->recip[FID_HEAD].real.prol.cCarLength & 0x3f;
			if (pDoz->recip[FID_HEAD].real.sign.scmd.b.lor || pDoz->recip[FID_HEAD].real.sign.scmd.b.lrr) {
				if (cLength == 10) {
					if (pDoz->recip[FID_HEAD].real.sign.scmd.b.esk0 && !pDoz->recip[FID_HEAD].real.sign.scmd.b.esk1) {
						if (cid <= 5) {		// 0 ~ 5
							pHvacr->t.s.req.b.lrr = pDoz->recip[FID_HEAD].real.sign.scmd.b.lrr;
							pHvacr->t.s.req.b.lor = pDoz->recip[FID_HEAD].real.sign.scmd.b.lor;
							pHvacr->t.s.req.b.esk = pDoz->recip[FID_HEAD].real.sign.scmd.b.eskp;
						}
						else	pHvacr->t.s.req.b.lrr = pHvacr->t.s.req.b.lor = pHvacr->t.s.req.b.esk = false;
					}
					else if (!pDoz->recip[FID_HEAD].real.sign.scmd.b.esk0 && pDoz->recip[FID_HEAD].real.sign.scmd.b.esk1) {
						if (cid >= 3) {		// 3 ~ 9
							pHvacr->t.s.req.b.lrr = pDoz->recip[FID_HEAD].real.sign.scmd.b.lrr;
							pHvacr->t.s.req.b.lor = pDoz->recip[FID_HEAD].real.sign.scmd.b.lor;
							pHvacr->t.s.req.b.esk = pDoz->recip[FID_HEAD].real.sign.scmd.b.eskp;
						}
						else	pHvacr->t.s.req.b.lrr = pHvacr->t.s.req.b.lor = pHvacr->t.s.req.b.esk = false;
					}
					else if (pDoz->recip[FID_HEAD].real.sign.scmd.b.esk0 && pDoz->recip[FID_HEAD].real.sign.scmd.b.esk1) {
						pHvacr->t.s.req.b.lor = true;
						pHvacr->t.s.req.b.lrr = false;
						pHvacr->t.s.req.b.esk = pDoz->recip[FID_HEAD].real.sign.scmd.b.eskp;
					}
				}
				else {
					pHvacr->t.s.req.b.lrr = pDoz->recip[FID_HEAD].real.sign.scmd.b.lrr;
					pHvacr->t.s.req.b.lor = pDoz->recip[FID_HEAD].real.sign.scmd.b.lor;
					pHvacr->t.s.req.b.esk = pDoz->recip[FID_HEAD].real.sign.scmd.b.eskp;
				}
			}

			//if ((pDoz->recip[FID_HEAD].real.sign.scmd.b.esk0 || pDoz->recip[FID_HEAD].real.sign.scmd.b.esk1) && pDoz->recip[FID_HEAD].real.sign.scmd.b.eskp)
			//	pHvacr->t.s.req.b.esk = true;
			// 171130
			//GETVERSE(pVerse);
			//PPISAINFO pPisa = (PPISAINFO)pVerse->GetInfo(true, PDEVID_PIS, LI_CURR);
			//if (pPisa->t.s.ref.b.dow)	pHvacr->t.s.req.b.dow = true;
			pHvacr->t.s.req.b.dow = pDoz->recip[FID_HEAD].real.sign.pcmd.b.dow;

			if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin)
				pHvacr->t.s.ctrl.a = pDoz->recip[FID_HEAD].real.sign.hcmd.a;

			pHvacr->t.s.ref.b.cno = CAR_PSELF() & 0xf;	//CAR_PSELF();
			//170726, modify
			//pHvacr->t.s.ref.b.stv = pDoz->recip[FID_HEAD].real.sign.hcmdEx.b.hts;
			pHvacr->t.s.ref.b.stv = true;
			pHvacr->t.s.ref.b.vnt2 = pDoz->recip[FID_HEAD].real.sign.hcmdEx.b.vnt2;
			pHvacr->t.s.ref.b.vnt1 = pDoz->recip[FID_HEAD].real.sign.hcmdEx.b.vnt1;
			pHvacr->t.s.cOutDoorTemp = pDoz->recip[FID_HEAD].real.cExtTemp;
			// 180626
			//pHvacr->t.s.cInDoorTemp = pDoz->recip[FID_HEAD].real.cTempSv[CAR_PSELF()];
			pHvacr->t.s.cInDoorTemp = c_lsv.cTempSv[CAR_PSELF()];
			pHvacr->t.s.cNextSt = pDoz->recip[FID_HEAD].real.ato.a.t.s.cNextSt;
			pHvacr->t.s.cDestSt = pDoz->recip[FID_HEAD].real.ato.a.t.s.cDestSt;
			// 170726,
			//pArch->GetBcdDevTime(&pHvacr->t.s.dt);
			// 170801
			//memcpy(&pHvacr->t.s.dt, &pDoz->recip[FID_HEAD].real.prol.dtm, sizeof(DEVTIME));
			pArch->Dbft2BcdDevt(&pHvacr->t.s.devt, pDoz->recip[FID_HEAD].real.prol.dbft);

			pHvacr->t.s.tst.a = 0;
			if ((pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NDETECT_ZEROSPEED)) &&
				pDoz->recip[FID_HEAD].real.sign.insp.b.hvac && !c_inspCmdBuf.b.hvac) {
				// 21/07/20
				//switch (pDoz->recip[fid].real.hvac.a.t.s.test.b.seq) {
				//case 0 :
				//	pHvacr->t.s.tst.b.tsr = true;
				//	pHvacr->t.s.tst.b.item = pDoz->recip[FID_HEAD].real.sign.insp.b.horc ? 1 : 2;
				//	break;
				//case 1 :
				//	pHvacr->t.s.tst.b.tsr = false;
				//	pHvacr->t.s.tst.b.item = pDoz->recip[FID_HEAD].real.sign.insp.b.horc ? 1 : 2;
				//	break;
				//default :
				//	c_inspCmdBuf.b.hvac = true;
				//	break;
				//}
				if (c_hvacfl[LI_WORK].a.t.s.test.b.seq == 0 || c_hvacfl[LI_WORK].a.t.s.test.b.seq == 1) {
					pHvacr->t.s.tst.b.tsr = true;
					pHvacr->t.s.tst.b.item = pDoz->recip[FID_HEAD].real.sign.insp.b.horc ? 1 : 2;
				}
				else {
					pHvacr->t.s.tst.b.tsr = false;
					pHvacr->t.s.tst.b.item = 0;
					c_inspCmdBuf.b.hvac = true;
				}
			}
			if (!pDoz->recip[FID_HEAD].real.sign.insp.b.hvac)	c_inspCmdBuf.b.hvac = false;

			if (pDoz->recip[FID_HEAD].real.sign.insp.b.cancel) {
				if (!c_inspCmdBuf.b.cancel) {
					c_inspCmdBuf.b.cancel = true;
					c_bHvacInspectCancel = true;
				}
			}
			else	c_inspCmdBuf.b.cancel = false;
			if (c_bHvacInspectCancel) {
				if ((pPaper->GetDeviceExFromRecip() &&
					c_hvacfl[LI_WORK].a.t.s.test.b.seq) ||
					(!pPaper->GetDeviceExFromRecip() &&
					pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.seq != 0))
					pHvacr->t.s.tst.b.tsp = true;
				else	c_bHvacInspectCancel = false;
			}
			// 201028
			BYTE cl = pPaper->GetLength();
			//---hroy 20210717
			switch (cl) {
			case 4 :	pHvacr->t.s.tst.b.cl1 = pHvacr->t.s.tst.b.cl0 = true;			break;
			case 6 :	pHvacr->t.s.tst.b.cl1 = true;	pHvacr->t.s.tst.b.cl0 = false;	break;
			case 8  :	pHvacr->t.s.tst.b.cl1 = false;	pHvacr->t.s.tst.b.cl0 = true;	break;
			default :	pHvacr->t.s.tst.b.cl1 = pHvacr->t.s.tst.b.cl0 = false;			break;
			}
			if (pPaper->GetDeviceExFromRecip()) {
				if (pDoz->recip[FID_HEAD].real.sign.hcmdEx.b.apdk2) {
					pHvacr->t.s.ctrl.b.apdk2 = true;
					pHvacr->t.s.req.b.apdk1 = false;
				}
				else if (pDoz->recip[FID_HEAD].real.sign.hcmd.b.apdk) {
					pHvacr->t.s.ctrl.b.apdk2 = false;
					pHvacr->t.s.req.b.apdk1 = true;
				}
				else {
					pHvacr->t.s.ctrl.b.apdk2 = false;
					pHvacr->t.s.req.b.apdk1 = false;
				}
			}
			else {
				// 210803, 그동안 200칸의 경우에는 어떻게 apdk를 동작시켰을까?
				pHvacr->t.s.ctrl.b.apdk2 = pDoz->recip[FID_HEAD].real.sign.hcmd.b.apdk;
			}
		}
		break;
	case DEVID_CMSB :
		{
			PCMSBRINFO pCmsbr = (PCMSBRINFO)pSdr;
			pCmsbr->wAddr = LADD_CMSB;
			pCmsbr->cCtrl = DEVCHAR_CTL;
			pCmsbr->t.s.cFlow = DEVFLOW_REQ;
			pCmsbr->t.s.cmd.a = 0;
			pCmsbr->t.s.cmd.b.cmg1 = pDoz->recip[FID_HEAD].real.sign.scmd.b.cm0;
			pCmsbr->t.s.cmd.b.cmg2 = pDoz->recip[FID_HEAD].real.sign.scmd.b.cm1;
			if (pPaper->GetLength() >= CID_MAX)
				pCmsbr->t.s.cmd.b.cmg3 = pDoz->recip[FID_HEAD].real.sign.scmd.b.cm2;
			if (pDoz->recip[FID_HEAD].real.sign.scmd.b.cm0 ||
				pDoz->recip[FID_HEAD].real.sign.scmd.b.cm1 ||
				pDoz->recip[FID_HEAD].real.sign.scmd.b.cm2)
				pCmsbr->t.s.cmd.b.cmsb = true;

			PDEVCONF pConf = pSch->GetDevConf();
			int cid = 0;
			for ( ; cid < (int)pConf->siv.cLength; cid ++)
				if (pConf->siv.cPos[cid] == CAR_PSELF())	break;
			pCmsbr->t.s.cDelay = (BYTE)(cid * 3);	// second
			pArch->Dbft2BcdDevt(&pCmsbr->t.s.devt, pDoz->recip[FID_HEAD].real.prol.dbft);
		}
		break;
	case DEVID_DCUL :
	case DEVID_DCUR :
		{
			PDCURINFO pDcur = (PDCURINFO)pSdr;
			pDcur->wAddr = LADD_DCUL;
			pDcur->cCtrl = DEVCHAR_CTL;
			pDcur->t.s.cFlow = DEVFLOW_REQ;
			BYTE cCid = CAR_PSELF();
			pDcur->t.s.cCid = cCid;
			//pDcur->t.s.ref.b.tst
			//pDcur->t.s.ref.b.dlt
			if (pDoz->recip[FID_HEAD].real.ato.a.t.s.valid.b.dcw &&
				pDoz->recip[FID_HEAD].real.ato.a.t.s.door.b.dcw)
				pDcur->t.s.ref.b.dcw = true;
			//GETVERSE(pVerse);
			//PPISAINFO pPisa = (PPISAINFO)pVerse->GetInfo(true, PDEVID_PIS, LI_CURR);
			//if (pPisa->t.s.ref.b.dow) {
			bool bLeft = ldi == DEVID_DCUL ? true : false;
			if (cCid == pPaper->GetLength() - 1)	bLeft ^= true;
			if (pDoz->recip[FID_HEAD].real.sign.pcmd.b.dow) {
				// Modified 2017/05/18
				if (pPaper->GetTenor())	bLeft ^= true;
				if (pDoz->recip[FID_HEAD].real.ato.a.t.s.valid.b.ndo) {
					if (pDoz->recip[FID_HEAD].real.ato.a.t.s.door.b.ln && bLeft)	pDcur->t.s.ref.b.dow = true;
					else if (pDoz->recip[FID_HEAD].real.ato.a.t.s.door.b.rn && !bLeft)	pDcur->t.s.ref.b.dow = true;
				}
			}
			if (pDoz->recip[FID_HEAD].real.sign.pcmd.b.dtest) {
				WORD w;
				if (cCid < 8)
					w = pDoz->recip[FID_HEAD].real.sign.wDoorTest & (3 << (cCid << 1));
				else	w = (WORD)(pDoz->recip[FID_HEAD].real.sign.pcmd.a & (3 << ((cCid - 8) << 1)));
				if (((w & 0xaaaa) && bLeft) || ((w & 0x5555) && !bLeft))	pDcur->t.s.ref.b.dlt = true;
			}
			// 170726,
			//memcpy(&pDcur->t.s.dt, &pDoz->recip[FID_HEAD].real.prol.dtm, sizeof(DEVTIME));
			// 170801
			pArch->Dbft2BcdDevt(&pDcur->t.s.devt, pDoz->recip[FID_HEAD].real.prol.dbft);
		}
		break;
	default :	break;
	}
	return pSdr;
}

void CProse::Monitor()
{
	// 181012
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	//if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_TEMPDIA) {
	//	for (int n = 0; n < LENGTH_WHEELDIA; n ++)
	//		c_lsv.cDiai[n] = pDoz->recip[FID_HEAD].real.cst.roll.tds.cDiai[n];		// use to calculate distance in CSteer
	//	for (int n = 0; n < CID_MAX; n ++)
	//		c_lsv.cTempSv[n] = pDoz->recip[FID_HEAD].real.cst.roll.tds.cTempSv[n];
	//}
	if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_ENVIRONS) {
		for (int n = 0; n < LENGTH_WHEELDIA; n ++)
			c_lsv.cDiai[n] = pDoz->recip[FID_HEAD].real.cst.roll.ecm.env.real.cWheelDiai[n];
		for (int n = 0; n < CID_MAX; n ++)
			c_lsv.cTempSv[n] = pDoz->recip[FID_HEAD].real.cst.roll.ecm.env.real.cTempSv[n];
		if (pPaper->GetDeviceExFromRecip())
			c_lsv.fire.wFromMaster = pDoz->recip[FID_HEAD].real.cst.roll.ecm.wFire;
	}
}

WORD CProse::GetControlSide()
{
	// 180511
	WORD w = 0;
	if (c_hvacMode.IsInit() && c_hvacMode.GetAct()) {
		w |= c_hvacMode.GetSide() ? HVACCTRL_RIGHT : HVACCTRL_LEFT;
	}
	if (c_hlfsSw.IsInit() && c_hlfsSw.GetAct()) {
		w |= c_hlfsSw.GetSide() ? HLFSCTRL_RIGHT : HLFSCTRL_LEFT;
	}
	if (c_elfsSw.IsInit() && c_elfsSw.GetAct()) {
		w |= c_elfsSw.GetSide() ? ELFSCTRL_RIGHT : ELFSCTRL_LEFT;
	}
	if (c_vfsSw.IsInit() && c_vfsSw.GetAct()) {
		w |= c_vfsSw.GetSide() ? VFSCTRL_RIGHT : VFSCTRL_LEFT;
	}
	if (c_apdsSw.IsInit() && c_apdsSw.GetAct()) {
		w |= c_apdsSw.GetSide() ? APDSCTRL_RIGHT : APDSCTRL_LEFT;
	}
	if (c_acSw.IsInit() && c_acSw.GetAct()) {
		w |= c_acSw.GetSide() ? ACLPCTRL_RIGHT : ACLPCTRL_LEFT;
	}
	if (c_dcSw.IsInit() && c_dcSw.GetAct()) {
		w |= c_dcSw.GetSide() ? DCLPCTRL_RIGHT : DCLPCTRL_LEFT;
	}

	return w;
}

// 201016
WORD CProse::GetControlExSide()
{
	WORD w = 0;
	GETPAPER(pPaper);
	if (pPaper->GetDeviceExFromRecip() && c_apdsASw.IsInit()
		&& c_apdsASw.GetAct()) {
		w |= c_apdsASw.GetSide() ? APDSACTRL_RIGHT : APDSACTRL_LEFT;
	}
	return w;
}

void CProse::ClearAllRed()
{
	c_mtxRed.Lock();
	for (int n = 0; n < REALDEV_MAX; n ++)	c_wRed[n] = 0;
	c_mtxRed.Unlock();
}

void CProse::CorralEcuTrace()
{
	// 200218
	//memcpy(&c_ecuNote.cura, c_ecufl[LI_WORK].a.t.c, sizeof(SAECU));
	GETPAPER(pPaper);
	if (pPaper->GetDeviceExFromRecip())	memcpy(&c_ecuNote.cura.ct, c_ecufl[LI_WORK].ct.a.t.c, sizeof(SAECU2));
	else	memcpy(&c_ecuNote.cura.co, c_ecufl[LI_WORK].co.a.t.c, sizeof(SAECU));
	GETARCHIVE(pArch);
	// 200218
	//pArch->GetSysTimeToDbf(&c_ecuNote.cura.dbft);	// 170801, dozen에서 시간을 가져오는 것이 타당할까???
	if (pPaper->GetDeviceExFromRecip())	pArch->GetSysTimeToDbf(&c_ecuNote.cura.ct.dbft);
	else	pArch->GetSysTimeToDbf(&c_ecuNote.cura.co.dbft);
	if (c_ecuNote.wSufLength > 0) {	// 고장이 발생하여 기록 중에 있다.
		if (c_ecuNote.wSufLength < SIZE_ECUTRACE)
			memcpy(&c_ecuNote.cBuf[c_ecuNote.wSufLength ++][0], &c_ecuNote.cura, sizeof(SAECU));
		else {
			WORD id = c_ecuNote.wPreLength < SIZE_PREECUTRACE ? 0 : c_ecuNote.wPreID;	// nPreLength가 50보다 작으면 아직 덮어쓰기를 하지 않은 것이다.
			for (WORD w = 0; w < c_ecuNote.wPreLength; w ++) {
				memcpy(&c_ecuNote.cBuf[w][0], &c_ecuNote.cPreBuf[id][0], sizeof(SAECU));
				if (++ id >= SIZE_PREECUTRACE)	id = 0;
			}
			c_ecuNote.wPreID = c_ecuNote.wPreLength = c_ecuNote.wSufLength = 0;
			GETARCHIVE(pArch);
			pArch->SaveEcuTrace((BYTE*)c_ecuNote.cBuf);
			//if (c_ecuNote.cTotalLength < LIMIT_TRACE)	++ c_ecuNote.cTotalLength;
			//TRACK("PROSE:ecu trace %d\n", c_ecuNote.cTotalLength);
		}
	}
	else {
		BYTE majf = 0;
		for (WORD w = 0; w < (WORD)sizeof(SAECU); w ++) {
			// 200218
			//majf |= (c_ecufl[LI_WORK].a.t.c[w] & c_cEcuMajfs[w]);
			if (pPaper->GetDeviceExFromRecip())
				majf |= (c_ecufl[LI_WORK].ct.a.t.c[w] & c_cEcuMajfs2[w]);
			else	majf |= (c_ecufl[LI_WORK].co.a.t.c[w] & c_cEcuMajfs1[w]);
		}

		if (majf != 0) {	// 중 고장 감지 순간...
			memcpy(&c_ecuNote.cBuf[c_ecuNote.wPreLength][0], &c_ecuNote.cura, sizeof(SAECU));
			c_ecuNote.wSufLength = c_ecuNote.wPreLength + 1;
			TRACK("PROSE:begin ecut.\n");
		}
		else {				// 고장 발생 전이라서 라운드 로빈으로 저장한다.
			memcpy(&c_ecuNote.cPreBuf[c_ecuNote.wPreID][0], &c_ecuNote.cura, sizeof(SAECU));
			if (++ c_ecuNote.wPreID >= SIZE_PREECUTRACE)	c_ecuNote.wPreID = 0;
			if (c_ecuNote.wPreLength < SIZE_PREECUTRACE)	++ c_ecuNote.wPreLength;
		}
	}
}

void CProse::CorralEcuTrace(BYTE* pSiva, WORD wStatus)
{
	// 200218
	//memcpy(&c_ecuNote.cura, c_ecufl[LI_WORK].a.t.c, sizeof(SAECU));
	GETPAPER(pPaper);
	if (pPaper->GetDeviceExFromRecip())	memcpy(&c_ecuNote.cura.ct, c_ecufl[LI_WORK].ct.a.t.c, sizeof(SAECU2));
	else	memcpy(&c_ecuNote.cura.co, c_ecufl[LI_WORK].co.a.t.c, sizeof(SAECU));
	GETARCHIVE(pArch);
	// 200218
	//pArch->GetSysTimeToDbf(&c_ecuNote.cura.dbft);	// 170801, dozen에서 시간을 가져오는 것이 타당할까???
	if (pPaper->GetDeviceExFromRecip())
		pArch->GetSysTimeToDbf(&c_ecuNote.cura.ct.dbft);
	else	pArch->GetSysTimeToDbf(&c_ecuNote.cura.co.dbft);
	if (c_ecuNote.wSufLength > 0) {	// 고장이 발생하여 기록 중에 있다.
		if (c_ecuNote.wSufLength < SIZE_ECUTRACE)
			memcpy(&c_ecuNote.cBuf[c_ecuNote.wSufLength ++][0], &c_ecuNote.cura, sizeof(SAECU));
		else {
			WORD id = c_ecuNote.wPreLength < SIZE_PREECUTRACE ? 0 : c_ecuNote.wPreID;	// nPreLength가 50보다 작으면 아직 덮어쓰기를 하지 않은 것이다.
			for (WORD w = 0; w < c_ecuNote.wPreLength; w ++) {
				memcpy(&c_ecuNote.cBuf[w][0], &c_ecuNote.cPreBuf[id][0], sizeof(SAECU));
				if (++ id >= SIZE_PREECUTRACE)	id = 0;
			}
			c_ecuNote.wPreID = c_ecuNote.wPreLength = c_ecuNote.wSufLength = 0;
			GETARCHIVE(pArch);
			pArch->SaveEcuTrace((BYTE*)c_ecuNote.cBuf);
			//if (c_ecuNote.cTotalLength < LIMIT_TRACE)	++ c_ecuNote.cTotalLength;
			//TRACK("PROSE:ecu trace %d\n", c_ecuNote.cTotalLength);
		}
	}
	else {
		BYTE majf = 0;
		for (WORD w = 0; w < (WORD)sizeof(SAECU); w ++) {
			// 200218
			//majf |= (c_ecufl[LI_WORK].a.t.c[w] & c_cEcuMajfs[w]);
			if (pPaper->GetDeviceExFromRecip())
				majf |= (c_ecufl[LI_WORK].ct.a.t.c[w] & c_cEcuMajfs2[w]);
			else	majf |= (c_ecufl[LI_WORK].co.a.t.c[w] & c_cEcuMajfs1[w]);
		}

		if (majf != 0) {	// 중 고장 감지 순간...
			memcpy(&c_ecuNote.cBuf[c_ecuNote.wPreLength][0], &c_ecuNote.cura, sizeof(SAECU));
			c_ecuNote.wSufLength = c_ecuNote.wPreLength + 1;
			TRACK("PROSE:begin ecut.\n");
		}
		else {				// 고장 발생 전이라서 라운드 로빈으로 저장한다.
			memcpy(&c_ecuNote.cPreBuf[c_ecuNote.wPreID][0], &c_ecuNote.cura, sizeof(SAECU));
			if (++ c_ecuNote.wPreID >= SIZE_PREECUTRACE)	c_ecuNote.wPreID = 0;
			if (c_ecuNote.wPreLength < SIZE_PREECUTRACE)	++ c_ecuNote.wPreLength;
		}
	}
}

// not used
void CProse::CorralSivDetect()
{
	memcpy(&c_sivNote.cura.d, &c_sivfltEx[LI_WORK].ta, sizeof(SIVAINFOEX));
	c_sivNote.cura.wStatus = 0;
	GETARCHIVE(pArch);
	pArch->GetSysTimeToDbf(&c_sivNote.cura.dbft);
	if (c_sivNote.wSufLength > 0) {
		if (c_sivNote.wSufLength < SIZE_SIVDETECT)
			memcpy(&c_sivNote.cBuf[c_sivNote.wSufLength ++][0], &c_sivNote.cura, sizeof(SIVADETEX));
		else {
			WORD id = c_sivNote.wPreLength < SIZE_PRESIVDETECT ? 0 : c_sivNote.wPreID;
			for (WORD w = 0; w < c_sivNote.wPreLength; w ++) {
				memcpy(&c_sivNote.cBuf[w][0], &c_sivNote.cPreBuf[id][0], sizeof(SIVADETEX));
				if (++ id >= SIZE_PRESIVDETECT)	id = 0;
			}
			c_sivNote.wPreID = c_sivNote.wPreLength = c_sivNote.wSufLength = 0;
			GETARCHIVE(pArch);
			pArch->SaveSivDetect((BYTE*)c_sivNote.cBuf);
		}
	}
	else {
		BYTE detf = 0;
		for (WORD w = 0; w < (WORD)sizeof(SASIVEX); w ++)
			detf |= (c_sivNote.cura.d.t.c[w] & c_cSivDets1[w]);

		if (detf != 0) {
			memcpy(&c_sivNote.cBuf[c_sivNote.wPreLength][0], &c_sivNote.cura, sizeof(SIVADETEX));
			c_sivNote.wSufLength = c_sivNote.wPreLength + 1;
			TRACK("PROSE:begin sivt.\n");
		}
		else {
			memcpy(&c_sivNote.cPreBuf[c_sivNote.wPreID][0], &c_sivNote.cura, sizeof(SIVADETEX));
			if (++ c_sivNote.wPreID >= SIZE_PRESIVDETECT)	c_sivNote.wPreID = 0;
			if (c_sivNote.wPreLength < SIZE_PRESIVDETECT)	++ c_sivNote.wPreLength;
		}
	}
}

// not used
void CProse::CorralSivDetect(BYTE* pSiva, WORD wStatus)
{
	memcpy(&c_sivNote.cura.d, pSiva, sizeof(SIVAINFOEX));
	c_sivNote.cura.wStatus = wStatus;
	GETARCHIVE(pArch);
	pArch->GetSysTimeToDbf(&c_sivNote.cura.dbft);
	if (c_sivNote.wSufLength > 0) {
		if (c_sivNote.wSufLength < SIZE_SIVDETECT)
			memcpy(&c_sivNote.cBuf[c_sivNote.wSufLength ++][0], &c_sivNote.cura, sizeof(SIVADETEX));
		else {
			WORD id = c_sivNote.wPreLength < SIZE_PRESIVDETECT ? 0 : c_sivNote.wPreID;
			for (WORD w = 0; w < c_sivNote.wPreLength; w ++) {
				memcpy(&c_sivNote.cBuf[w][0], &c_sivNote.cPreBuf[id][0], sizeof(SIVADETEX));
				if (++ id >= SIZE_PRESIVDETECT)	id = 0;
			}
			c_sivNote.wPreID = c_sivNote.wPreLength = c_sivNote.wSufLength = 0;
			GETARCHIVE(pArch);
			pArch->SaveSivDetect((BYTE*)c_sivNote.cBuf);
		}
	}
	else {
		BYTE detf = 0;
		for (WORD w = 0; w < (WORD)sizeof(SASIV); w ++) {
			//detf |= (c_sivNote.cura.d.t.c[w] & c_cSivDets[w]);
			BYTE f = (c_sivNote.cura.d.t.c[w] & c_cSivDets1[w]);
			if (detf == 0 && f != 0)
				TRACK("PROSE:cap sivt %d(0x%02x)\n", w, c_sivNote.cura.d.t.c[w]);
			detf |= f;
		}

		if (detf != 0) {
			memcpy(&c_sivNote.cBuf[c_sivNote.wPreLength][0], &c_sivNote.cura, sizeof(SIVADETEX));
			c_sivNote.wSufLength = c_sivNote.wPreLength + 1;
			TRACK("PROSE:begin sivt.\n");
		}
		else {
			memcpy(&c_sivNote.cPreBuf[c_sivNote.wPreID][0], &c_sivNote.cura, sizeof(SIVADETEX));
			if (++ c_sivNote.wPreID >= SIZE_PRESIVDETECT)	c_sivNote.wPreID = 0;
			if (c_sivNote.wPreLength < SIZE_PRESIVDETECT)	++ c_sivNote.wPreLength;
		}
	}
}

// not used
void CProse::CorralSivDetect2()
{
	memcpy(&c_sivNote2.cura.d, &c_sivflt2[LI_WORK].ta, sizeof(SIVAINFO2));
	c_sivNote2.cura.wStatus = 0;
	GETARCHIVE(pArch);
	pArch->GetSysTimeToDbf(&c_sivNote2.cura.dbft);
	if (c_sivNote2.wSufLength > 0) {
		if (c_sivNote2.wSufLength < SIZE_SIVDETECT)
			memcpy(&c_sivNote2.cBuf[c_sivNote2.wSufLength ++][0], &c_sivNote2.cura, sizeof(SIVADET2));
		else {
			WORD id = c_sivNote2.wPreLength < SIZE_PRESIVDETECT ? 0 : c_sivNote2.wPreID;
			for (WORD w = 0; w < c_sivNote2.wPreLength; w ++) {
				memcpy(&c_sivNote2.cBuf[w][0], &c_sivNote2.cPreBuf[id][0], sizeof(SIVADET2));
				if (++ id >= SIZE_PRESIVDETECT)	id = 0;
			}
			c_sivNote2.wPreID = c_sivNote2.wPreLength = c_sivNote2.wSufLength = 0;
			GETARCHIVE(pArch);
			pArch->SaveSivDetect2((BYTE*)c_sivNote2.cBuf);
		}
	}
	else {
		BYTE detf = 0;
		for (WORD w = 0; w < (WORD)sizeof(SASIV2); w ++)
			detf |= (c_sivNote2.cura.d.t.c[w] & c_cSivDets2[w]);

		if (detf != 0) {
			memcpy(&c_sivNote2.cBuf[c_sivNote2.wPreLength][0], &c_sivNote2.cura, sizeof(SIVADET2));
			c_sivNote2.wSufLength = c_sivNote2.wPreLength + 1;
			TRACK("PROSE:begin sivt.\n");
		}
		else {
			memcpy(&c_sivNote2.cPreBuf[c_sivNote2.wPreID][0], &c_sivNote2.cura, sizeof(SIVADET2));
			if (++ c_sivNote2.wPreID >= SIZE_PRESIVDETECT)	c_sivNote2.wPreID = 0;
			if (c_sivNote2.wPreLength < SIZE_PRESIVDETECT)	++ c_sivNote2.wPreLength;
		}
	}
}

void CProse::CorralSivDetect2(BYTE* pSiva, WORD wStatus)
{
	memcpy(&c_sivNote2.cura.d, pSiva, sizeof(SIVAINFO2));
	c_sivNote2.cura.wStatus = wStatus;
	GETARCHIVE(pArch);
	pArch->GetSysTimeToDbf(&c_sivNote2.cura.dbft);
	if (c_sivNote2.wSufLength > 0) {
		if (c_sivNote2.wSufLength < SIZE_SIVDETECT)
			memcpy(&c_sivNote2.cBuf[c_sivNote2.wSufLength ++][0], &c_sivNote2.cura, sizeof(SIVADET2));
		else {
			WORD id = c_sivNote2.wPreLength < SIZE_PRESIVDETECT ? 0 : c_sivNote2.wPreID;
			for (WORD w = 0; w < c_sivNote2.wPreLength; w ++) {
				memcpy(&c_sivNote2.cBuf[w][0], &c_sivNote2.cPreBuf[id][0], sizeof(SIVADET2));
				if (++ id >= SIZE_PRESIVDETECT)	id = 0;
			}
			c_sivNote2.wPreID = c_sivNote2.wPreLength = c_sivNote2.wSufLength = 0;
			GETARCHIVE(pArch);
			pArch->SaveSivDetect2((BYTE*)c_sivNote2.cBuf);
		}
	}
	else {
		BYTE detf = 0;
		for (WORD w = 0; w < (WORD)sizeof(SASIV2); w ++) {
			//detf |= (c_sivNote.cura.d.t.c[w] & c_cSivDets[w]);
			BYTE f = (c_sivNote2.cura.d.t.c[w] & c_cSivDets2[w]);
			if (detf == 0 && f != 0)
				TRACK("PROSE:cap sivt %d(0x%02x)\n", w, c_sivNote2.cura.d.t.c[w]);
			detf |= f;
		}

		if (detf != 0) {
			memcpy(&c_sivNote2.cBuf[c_sivNote2.wPreLength][0], &c_sivNote2.cura, sizeof(SIVADET2));
			c_sivNote2.wSufLength = c_sivNote2.wPreLength + 1;
			TRACK("PROSE:begin sivt.\n");
		}
		else {
			memcpy(&c_sivNote2.cPreBuf[c_sivNote2.wPreID][0], &c_sivNote2.cura, sizeof(SIVADET2));
			if (++ c_sivNote2.wPreID >= SIZE_PRESIVDETECT)	c_sivNote2.wPreID = 0;
			if (c_sivNote2.wPreLength < SIZE_PRESIVDETECT)	++ c_sivNote2.wPreLength;
		}
	}
}

int CProse::Bale(bool bType, BYTE* pMesh)
{
	int leng = 0;
	//if (bType) {
	//	sprintf((char*)pMesh, "$PRS, %d", sizeof(HVACFLAP) * HVAC_MAX + sizeof(CMSBFLAP) * CMSB_MAX);
	//	while (*pMesh != '\0') {
	//		++ pMesh;
	//		++ leng;
	//	}
	//}
	//else {
	//	memcpy(pMesh, &c_lsv.hvaccf.fl, sizeof(HVACFLAP) * HVAC_MAX);
	//	pMesh += (sizeof(HVACFLAP) * HVAC_MAX);
	//	leng += (sizeof(HVACFLAP) * HVAC_MAX);
	//	memcpy(pMesh, &c_lsv.cmsbcf.fl, sizeof(CMSBFLAP) * CMSB_MAX);
	//	pMesh += (sizeof(CMSBFLAP) * CMSB_MAX);
	//	leng += (sizeof(CMSBFLAP) * CMSB_MAX);
	//}
	//if (bType) {
	//	sprintf((char*)pMesh, "$PRS, %d", sizeof(WORD) * REALDEV_MAX * CID_MAX);
	//	while (*pMesh != '\0') {
	//		++ pMesh;
	//		++ leng;
	//	}
	//}
	//else {
	//	memcpy(pMesh, &c_lsv.wReceivedRed, sizeof(WORD) * REALDEV_MAX * CID_MAX);
	//	pMesh += (sizeof(WORD) * REALDEV_MAX * CID_MAX);
	//	leng += (sizeof(WORD) * REALDEV_MAX * CID_MAX);
	//}
	if (bType) {
		sprintf((char*)pMesh, "$PRS, %d", sizeof(WORD) * REALDEV_MAX * CID_MAX + sizeof(FDUFLAPAB) * 2);
		while (*pMesh != '\0') {
			++ pMesh;
			++ leng;
		}
	}
	else {
		memcpy(pMesh, &c_lsv.wReceivedRed, sizeof(WORD) * REALDEV_MAX * CID_MAX);
		pMesh += (sizeof(WORD) * REALDEV_MAX * CID_MAX);
		leng += (sizeof(WORD) * REALDEV_MAX * CID_MAX);
		GETVERSE(pVerse);
		CVerse::PPSHARE pPsv = pVerse->GetShareVars();
		memcpy(pMesh, &pPsv->fducf.fl[0], sizeof(FDUFLAPAB));
		pMesh += sizeof(FDUFLAPAB);
		leng += sizeof(FDUFLAPAB);
		memcpy(pMesh, &pPsv->fducf.fl[1], sizeof(FDUFLAPAB));
		pMesh += sizeof(FDUFLAPAB);
		leng += sizeof(FDUFLAPAB);
	}
	return leng;
}

void CProse::Arteriam()
{
	if (c_lsv.sivcf.wWarmUpTimer > 0)	-- c_lsv.sivcf.wWarmUpTimer;
	if (c_lsv.v3fcf.wWarmUpTimer > 0)	-- c_lsv.v3fcf.wWarmUpTimer;
	if (c_lsv.ecucf.wWarmUpTimer > 0)	-- c_lsv.ecucf.wWarmUpTimer;
	if (c_lsv.hvaccf.wWarmUpTimer > 0)	-- c_lsv.hvaccf.wWarmUpTimer;
	// 200218
	if (c_lsv.cmsbcf.wWarmUpTimer > 0)	-- c_lsv.cmsbcf.wWarmUpTimer;
	if (c_lsv.dcucf.wWarmUpTimer > 0)	-- c_lsv.dcucf.wWarmUpTimer;
	if (c_lsv.wInspectCancelTime > 0)	-- c_lsv.wInspectCancelTime;
	for (int n = 0; n < ECU_MAX; n ++) {
		if (c_lsv.ecucf.wNrbdDetectTimer[n] > 0)
			-- c_lsv.ecucf.wNrbdDetectTimer[n];
	}

	// 171226
	if (!c_eculr.bSync && c_eculr.wTimer > 0)	-- c_eculr.wTimer;
	//if (c_lsv.hvaccf.wInspectCancelTime > 0)	-- c_lsv.hvaccf.wInspectCancelTime;
	// 200218
	if (c_lsv.fire.wReleaseTimer > 0)	-- c_lsv.fire.wReleaseTimer;
}

ENTRY_CONTAINER(CProse)
	SCOOP(&c_wLineState,				sizeof(WORD),		"Prs")
	SCOOP(&c_lsv.sivcf.cLines[0],		sizeof(BYTE) * 3,	"sivc")
	SCOOP(&c_lsv.sivcf.wCorrectMap,		sizeof(WORD),		"")
	SCOOP(&c_lsv.sivcf.wDefectMap,		sizeof(WORD),		"")
	SCOOP(&c_lsv.sivcf.wManualDefectMap,	sizeof(WORD),	"")
	SCOOP(&c_lsv.wCmgBitmap,			sizeof(WORD),		"cmg")
	SCOOP(&c_lsv.bManualCmg,			sizeof(bool),		"")
	SCOOP(&c_lsv.hvaccf.cLines[0],		sizeof(BYTE),		"hvac")
	SCOOP(&c_lsv.hvaccf.cLines[1],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.hvaccf.cLines[2],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.hvaccf.cLines[3],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.hvaccf.cLines[4],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.hvaccf.cLines[5],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.hvaccf.cLines[6],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.hvaccf.cLines[7],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.hvaccf.cLines[8],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.hvaccf.cLines[9],		sizeof(BYTE),		"")
	SCOOP(&c_lsv.v3fcf.cLines[0],		sizeof(BYTE) * 5,	"v3fc")
	SCOOP(&c_lsv.ecucf.cLines[0],		sizeof(BYTE) * 10,	"ecuc")
	//SCOOP(&c_ecuNote.cTotalLength,		sizeof(BYTE),		"etr")
	SCOOP(&c_ecuNote.wPreID,			sizeof(WORD),		"etd")
	SCOOP(&c_ecuNote.wPreLength,		sizeof(WORD),		"")
	SCOOP(&c_ecuNote.wSufLength,		sizeof(WORD),		"")
	SCOOP(&c_lsv.sivcf.bTrace,			sizeof(bool),		"trc")
	SCOOP(&c_cSivTraceLength,			sizeof(BYTE),		"")
	SCOOP(&c_lsv.v3fcf.bTrace,			sizeof(bool),		"")
	SCOOP(&c_cV3fTraceLength,			sizeof(BYTE),		"")
	SCOOP(&c_lsv.ecucf.bTrace,			sizeof(bool),		"")
	SCOOP(&c_cEcuTraceLength,			sizeof(BYTE),		"")
	//SCOOP(&c_wDebs[0],					sizeof(WORD),		"Deb0")	// DEVID_SIV
	//SCOOP(&c_wDebs[1],					sizeof(WORD),		"")		// DEVID_V3F
	//SCOOP(&c_wDebs[2],					sizeof(WORD),		"")		// DEVID_ECU
	//SCOOP(&c_wDebs[3],					sizeof(WORD),		"")		// DEVID_HVAC
	//SCOOP(&c_wDebs[4],					sizeof(WORD),		"Deb4")	// DEVID_DCUL
	//SCOOP(&c_wDebs[5],					sizeof(WORD),		"")
	//SCOOP(&c_wDebs[6],					sizeof(WORD),		"")
	//SCOOP(&c_wDebs[7],					sizeof(WORD),		"")
	//SCOOP(&c_wDebs[8],					sizeof(WORD),		"Deb8")	// DEVID_DCUR
	//SCOOP(&c_wDebs[9],					sizeof(WORD),		"")
	//SCOOP(&c_wDebs[10],					sizeof(WORD),		"")
	//SCOOP(&c_wDebs[11],					sizeof(WORD),		"")
	SCOOP(&c_lsv.wFailedCCs,			sizeof(WORD),		"Fail")
	SCOOP(&c_lsv.wNrbds,				sizeof(WORD),		"")
	//SCOOP(&c_lsv.dcucf.cLines[8][0],	sizeof(BYTE),		"dcf")
	//SCOOP(&c_lsv.dcucf.cLines[8][4],	sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.cLines[9][0],	sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.cLines[9][4],	sizeof(BYTE),		"")
	SCOOP(&c_lsv.fire.wDefectMap,		sizeof(WORD),		"fire")
	SCOOP(&c_lsv.fire.wDefectSync,		sizeof(WORD),		"")
	SCOOP(&c_lsv.fire.wRecoverMap,		sizeof(WORD),		"")
	//SCOOP(&c_lsv.dcucf.mon[0][0].stat.a, sizeof(BYTE),		"dcu")
	//SCOOP(&c_lsv.dcucf.mon[0][0].wXchTimer, sizeof(WORD),	"")
	//SCOOP(&c_lsv.dcucf.mon[0][1].stat.a, sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.mon[0][1].wXchTimer, sizeof(WORD),	"")
	//SCOOP(&c_lsv.dcucf.mon[0][2].stat.a, sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.mon[0][2].wXchTimer, sizeof(WORD),	"")
	//SCOOP(&c_lsv.dcucf.mon[0][3].stat.a, sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.mon[0][3].wXchTimer, sizeof(WORD),	"")
	//SCOOP(&c_lsv.dcucf.mon[0][4].stat.a, sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.mon[0][4].wXchTimer, sizeof(WORD),	"")
	//SCOOP(&c_lsv.dcucf.mon[0][5].stat.a, sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.mon[0][5].wXchTimer, sizeof(WORD),	"")
	//SCOOP(&c_lsv.dcucf.mon[0][6].stat.a, sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.mon[0][6].wXchTimer, sizeof(WORD),	"")
	//SCOOP(&c_lsv.dcucf.mon[0][7].stat.a, sizeof(BYTE),		"")
	//SCOOP(&c_lsv.dcucf.mon[0][7].wXchTimer, sizeof(WORD),	"")
	//SCOOP(&c_lsv.hvaccf.cLines[0],		sizeof(BYTE) * 10,	"hvcs")
	//SCOOP(&c_lsv.dcucf.cLines[0][0],	sizeof(BYTE) * 8,	"dss0")
	//SCOOP(&c_lsv.dcucf.cLines[1][0],	sizeof(BYTE) * 8,	"dss1")
	//SCOOP(&c_lsv.dcucf.cLines[2][0],	sizeof(BYTE) * 8,	"dss2")
	//SCOOP(&c_lsv.dcucf.cLines[3][0],	sizeof(BYTE) * 8,	"dss3")
	//SCOOP(&c_lsv.dcucf.cLines[4][0],	sizeof(BYTE) * 8,	"dss4")
	//SCOOP(&c_lsv.dcucf.cLines[5][0],	sizeof(BYTE) * 8,	"dss5")
	//SCOOP(&c_lsv.dcucf.cLines[6][0],	sizeof(BYTE) * 8,	"dss6")
	//SCOOP(&c_lsv.dcucf.cLines[7][0],	sizeof(BYTE) * 8,	"dss7")
	//SCOOP(&c_lsv.dcucf.cLines[8][0],	sizeof(BYTE) * 8,	"dss8")
	//SCOOP(&c_lsv.dcucf.cLines[9][0],	sizeof(BYTE) * 8,	"dss9")
	//SCOOP(&c_lsv.wRed[0][0],	sizeof(WORD),		"red")
	//SCOOP(&c_lsv.wRed[1][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[2][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[3][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[4][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[5][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[6][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[7][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[8][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[9][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[10][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wRed[11][0],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.wManualCmgCounter,		sizeof(WORD),	"")
	//SCOOP(&c_lsv.fire.wSeq[0],	sizeof(WORD),		"fseq")
	//SCOOP(&c_lsv.fire.wSeq[1],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.fire.wSeq[2],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.fire.wSeq[3],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.fire.wSeq[4],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.fire.wSeq[5],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.fire.wSeq[6],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.fire.wSeq[7],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.fire.wSeq[8],	sizeof(WORD),		"")
	//SCOOP(&c_lsv.fire.wSeq[9],	sizeof(WORD),		"")
EXIT_CONTAINER()
