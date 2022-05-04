/*
 * CSteer.cpp
 *
 *  Created on: 2016. 9. 9.
 *      Author: SWCho
 */
#include "Track.h"
#include "../Inform2/DevInfo2/AtoInfo.h"
#include "../Component2/CTool.h"
#include "CVerse.h"
#include "CProse.h"
#include "CPaper.h"
#include "CSch.h"
#include "CArch.h"
#include "CLand.h"

#include "CSteer.h"

// *** PWM		DEV						ENCODER
//	Manual		STICK(x) ->	 5 ~ 95% -> compare ->	10 ~ 90%	Logical has typial value for du,	physical has 10 ~ 90%
//	Auto		ATO(x) ->	10 ~ 90% -> convert ->	0 ~ 100%	Logical & physical has 0 ~ 100%

/* ===== 180219 ENCODER PWM by "엔코더 부품 규격서-REV01_20160927(수정).pdf"
	C	10%	+_5%	 5 ~ 19	->	12.0	13.5
	B1	28%	+_5%	20 ~ 33	->	25.5	13.0
	B2	38%	+_5%	34 ~ 43	->	38.5	10.5
	B3	49%	+_5%	44 ~ 54	->	49.0	10.5
	B4	59%	+_5%	55 ~ 64	->	59.5	10.5
	B5	70%	+_5%	65 ~ 75	->	70.0	10.5
	B6	80%	+_5%	76 ~ 85	->	80.5	10.0
	B7	90%	+_5%	86 ~ 95	->	90.5

	C	10%	+_5%	 5 ~ 27	->	16.0	20.5
	P1	36%	+_5%	28 ~ 45	->	36.5	19.0
	P2	54%	+_5%	46 ~ 65	->	55.5	18.0
	P3	72%	+_5%	66 ~ 81	->	73.5	15.0
	P4	90%	+_5%	82 ~ 95	->	88.5
*/

const CSteer::NOTCHSECTION	CSteer::c_notchSpectrum[2][10] = {
	{
		{	0,	4,	0,		NOTCHID_ERR	},
		{	5,	19,	0,		NOTCHID_C	},
		{	20,	33,	22,		NOTCHID_B1	},
		{	34,	43,	34,		NOTCHID_B2	},
		{	44,	54,	47,		NOTCHID_B3	},
		{	55,	64,	60,		NOTCHID_B4	},
		{	65,	75,	73,		NOTCHID_B5	},
		{	76,	85,	86,		NOTCHID_B6	},
		{	86,	95,	100,	NOTCHID_B7	},
		{	0,	0,	0,		NOTCHID_ERR	}
	},
	{
		{	0,	4,	0,		NOTCHID_ERR	},
		{	5,	27,	0,		NOTCHID_C	},
		{	28,	45,	28,		NOTCHID_P1	},
		{	46,	65,	52,		NOTCHID_P2	},
		{	66,	81,	75,		NOTCHID_P3	},
		{	82,	95,	100,	NOTCHID_P4	},
		{	0,	0,	0,		NOTCHID_ERR	},
		{	0,	0,	0,		NOTCHID_ERR	},
		{	0,	0,	0,		NOTCHID_ERR	},
		{	0,	0,	0,		NOTCHID_ERR	}
	}
};

//const CSteer::NOTCHSECTION	CSteer::c_notchSpectrum[2][11] = {
//	{
//		{	0,					PWMPERCENTAGE_MIN,	0,		NOTCHID_ERR	},
//		{	PWMPERCENTAGE_MIN,	23,					0,		NOTCHID_C	},	//			// 10 + 8
//		{	24,					33,					22,		NOTCHID_B1	},	// 28.9		// 10 + 18, | TUDIB_BRAKING
//		{	34,					44,					34,		NOTCHID_B2	},	// 39		// 10 + 28
//		{	45,					54,					47,		NOTCHID_B3	},	// 49.5		// 10 + 38
//		{	55,					65,					60,		NOTCHID_B4	},	// 60		// 10 + 48
//		{	66,					75,					73,		NOTCHID_B5	},	// 70.5		// 10 + 58
//		{	76,					85,					86,		NOTCHID_B6	},	// 80.6		// 10 + 68
//		{	86,					92,					100,	NOTCHID_B7	},	// 89		// 10 + 75
//		{	93,					PWMPERCENTAGE_MAX,	100,	NOTCHID_EB	},	// 94		// 10 + 75
//		{	0,					0,					0,		NOTCHID_ERR	}
//	},
//	{
//		{	0,					PWMPERCENTAGE_MIN,	0,		NOTCHID_ERR	},
//		{	PWMPERCENTAGE_MIN,	23,					0,		NOTCHID_C	},
//		{	PWMPERCENTAGE_MINP1,44,					28,		NOTCHID_P1	},	// 10 + 22, | TUDIB_POWERING
//		{	45,					62,					52,		NOTCHID_P2	},	// 10 + 41
//		{	63,					80,					75,		NOTCHID_P3	},	// 10 + 60
//		{	81,					PWMPERCENTAGE_MAX,	100,	NOTCHID_P4	},	// 10 + 75
//		{	0,					0,					0,		NOTCHID_ERR	},
//		{	0,					0,					0,		NOTCHID_ERR	},
//		{	0,					0,					0,		NOTCHID_ERR	},
//		{	0,					0,					0,		NOTCHID_ERR	},
//		{	0,					0,					0,		NOTCHID_ERR	}
//	}
//};
//
CSteer::CSteer()
{
	c_pParent = NULL;
	c_dcCmd.b.SetDebounceTime(DEB_DCS);
	//c_mode.d.buf = 0xff;
	//c_mode.d.SetDebounceTime(DEB_STEER);
	//c_mode.cDetermined = 0;
	//c_pwm.d[0].SetDebounceTime(DEB_STEER);
	//c_pwm.d[1].SetDebounceTime(DEB_STEER);
	//c_pwm.wCur = 0;
	//c_pad.d.SetDebounceTime(DEB_STEER);
	//c_pad.notch = NOTCHID_ERR;
	Initial();
}

CSteer::~CSteer()
{
}

void CSteer::Initial()
{
	c_bWakeup = true;
	c_bAfsMode = false;
	c_speedCalc.clk = ClockCycles();
	c_speedCalc.dbSum = c_speedCalc.dbCur = 0.f;
	c_speedCalc.wSamplingLength = 0;
	c_speedCalc.dist.dbCur = c_speedCalc.dist.dbPrev = 0.f;
	c_speedCalc.wPulseWidthTimer = 0;
	c_cMode = MOTIVMODE_NON;
	memset(&c_mam, 0, sizeof(_MAMIX));
	memset(&c_zsCalc, 0, sizeof(ZSCALC));
	memset(&c_tber, 0, sizeof(TBEREF));
	memset(&c_brake, 0, sizeof(BRAKES));
	memset(&c_moment, 0, sizeof(MOMENTUMINFO));
	c_bTest = false;
	c_wPsdoCounter = 0;	// 201124
	c_wEncfUnDetectTimer = TIME_ENCFUNDETECT;	// 210614
}

// Monitor에서는 domin을 보고 shot과 cut을 하는데 Control에서는 그럴 수가 없다. 그래서 Control의 shot과 cut을 담당하도록 아래 함수를 둔다.
void CSteer::Shot(int iCID, WORD wCode)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		GETARCHIVE(pArch);
		pArch->Shot(iCID, wCode);
	}
}

void CSteer::Cut(int iCID, WORD wBegin, WORD wEnd, WORD wFlesh)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		GETARCHIVE(pArch);
		pArch->Cut(iCID, wBegin, wEnd, wFlesh);
	}
}

void CSteer::Closeup(int iCID, WORD wShot, WORD wCutBegin, WORD wCutEnd)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		GETARCHIVE(pArch);
		pArch->Closeup(iCID, wShot, wCutBegin, wCutEnd);
	}
}

//=== Control function ===
void CSteer::CalcTachoSpeed()
{
	c_mts.Lock();
	GETARCHIVE(pArch);
	uint64_t clk = ClockCycles();
	double sec = (double)(clk - c_speedCalc.clk) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	if (sec > 0.08f) {		// tacho 샘플링은 0.1초 단위.
		GETPAPER(pPaper);
		PDOZEN pDoz = pPaper->GetDozen();
		double dist;
		if (pDoz->recip[FID_HEAD].real.avs.wTacho == 0)	dist = c_speedCalc.dbCur = 0.f;
		else {
			// 17/07/06, Modify
			//double diam = (double)(pDoz->recip[FID_HEAD].real.cDiai[CAR_PSELF == 0 ? 5 : 6]);
			BYTE cHeadID = pDoz->recip[FID_HEAD].real.prol.cProperID;
			// 180626
			//double diam = (double)(pDoz->recip[FID_HEAD].real.cDiai[cHeadID < PID_RIGHT ? 5 : 6]);
			// 181003
			//GETPROSE(pProse);
			//CProse::PLSHARE pLsv = pProse->GetShareVars();
			//double diam = (double)pLsv->cDiai[cHeadID < PID_RIGHT ? 5 : 6];
			PENVIRONS pEnv = pArch->GetEnvirons();
			double diam = (double)pEnv->real.cWheelDiai[cHeadID < PID_RIGHT ? 5 : 6];
			diam += (double)MIN_WHEELDIA;
			dist = (double)pDoz->recip[FID_HEAD].real.avs.wTacho * diam / 1000.f * M_PI / TACHO_PULSE;	// m
			c_speedCalc.dbCur = dist * 36000.f;			// 0.1초를 1시간, m/0.1s -> m/h, 2.78m ~= 100000m/h
		}
		++ c_speedCalc.wSamplingLength;
		c_speedCalc.dbSum += c_speedCalc.dbCur;
		c_speedCalc.clk = clk;

		GETARCHIVE(pArch);
		pArch->AddCareers(CRRITEM_DISTANCE, 0, dist);
		pArch->AddSegment(dist);
		c_speedCalc.dist.dbCur += dist;
		if ((c_speedCalc.dist.dbCur - c_speedCalc.dist.dbPrev) >= DISTANCE_PULSE) {
			c_speedCalc.dist.dbPrev = c_speedCalc.dist.dbCur;

			CLand* pLand = (CLand*)c_pParent;
			ASSERTP(pLand);
			CFio* pFio = pLand->GetFio();
			pFio->SetOutput(UDOB_DISTP, true);

			c_mtx.Lock();
			c_speedCalc.wPulseWidthTimer = PERIOD_PULSE;
			c_mtx.Unlock();
		}
	}
	c_mts.Unlock();
}

void CSteer::CaptureZeroSpeed()
{
	// 181011
	double db[LENGTH_TU];

	if (c_speedCalc.dbCur < SPEED_ZERO) {
		GETPAPER(pPaper);
		PDOZEN pDoz = pPaper->GetDozen();
		GETARCHIVE(pArch);
		PCAREERS pCrr = pArch->GetCareers();
		GETTOOL(pTool);

		// 210809
		int iLive = 0;
		for (int fid = FID_HEAD; fid < FID_PAS; fid ++) {
			if (pPaper->GetRouteState(fid))	++ iLive;
		}

		switch (c_zsCalc.cSeq) {
		case CALCZEROSSEQ_NON :
			c_mtx.Lock();
			c_zsCalc.wTimer = TIME_DETECTZV;
			c_mtx.Unlock();
			c_zsCalc.cSeq = CALCZEROSSEQ_DETECT;
			break;
		case CALCZEROSSEQ_DETECT :
			if (c_zsCalc.wTimer == 0) {
				c_brake.cNorm |= (1 << NDETECT_ZEROSPEED);
				//if (!pDoz->recip[FID_HEAD].real.sign.scmdEx.b.lab || c_bWakeup) {
				//	//c_bWakeup = false;
				// 210809
				//	c_zsCalc.cSeq = CALCZEROSSEQ_CMPMOVE;
				//}
				//else	c_zsCalc.cSeq = CALCZEROSSEQ_END;
				if (iLive > 1)	c_zsCalc.cSeq = CALCZEROSSEQ_CMPMOVE;
				else	c_zsCalc.cSeq = CALCZEROSSEQ_END;
			}
			break;
		case CALCZEROSSEQ_CMPMOVE :
			if (!pTool->WithinCriterion(pCrr->real.dbDistance, pArch->GetPrevDistance(), 100.f))	// 100m 차이
				pArch->SetPrevDistance(pCrr->real.dbDistance);
			c_zsCalc.cSeq = CALCZEROSSEQ_CMPDISTANCE;
			memset(c_bufMon, 0, 4096);
			c_pMon = c_bufMon;
			break;
		case CALCZEROSSEQ_CMPDISTANCE :
			// 181003
			//if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_CAREERS && pDoz->recip[FID_HEAD].real.cst.roll.crr.real.wVersion == ENV_VERSION) {
			if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_CAREERS && pDoz->recip[FID_HEADBK].real.cst.cID == CSTID_CAREERS &&
				pDoz->recip[FID_TAIL].real.cst.cID == CSTID_CAREERS && pDoz->recip[FID_TAILBK].real.cst.cID == CSTID_CAREERS) {
				if (pDoz->recip[FID_HEAD].real.cst.roll.crr.real.wVersion == ENV_VERSION) {
					if (pPaper->GetLabFromRecip())	TRACK("STER:cmp distance a.\n");
#if	0
					// 181011
					double dbMax = 0.f;
					int mid = FID_HEAD;
					for (int fid = FID_HEAD; fid < FID_PAS; fid ++) {
						if (pDoz->recip[fid].real.cst.roll.crr.real.dbDistance > dbMax) {
							dbMax = pDoz->recip[fid].real.cst.roll.crr.real.dbDistance;
							mid = fid;
						}
					}
					if (!c_bTest) {
						c_bTest = true;
						TRACK("TEST: %3.f %.3f %.3f %.3f\n",
								pDoz->recip[0].real.cst.roll.crr.real.dbDistance, pDoz->recip[1].real.cst.roll.crr.real.dbDistance,
								pDoz->recip[2].real.cst.roll.crr.real.dbDistance, pDoz->recip[3].real.cst.roll.crr.real.dbDistance);
					}
					if (mid != (int)CAR_FSELF() && !pTool->WithinCriterion(dbMax, pCrr->real.dbDistance, 5.f)) {
						pCrr->real.dbDistance = dbMax;
						//c_zsCalc.bModifyCareers = true;
						pArch->Shot(CAR_PSELF(), 184);
						if (pPaper->GetLabFromRecip())	TRACK("STER:cmp distance b.\n");
					}
					else {
						if (pPaper->GetLabFromRecip())	TRACK("STER:cmp distance c.\n");
					}
#endif
					// 190826
					//for (int n = 0; n < LENGTH_TU; n ++)
					//	db[n] = pDoz->recip[n].real.cst.roll.crr.real.dbDistance;
					for (int fid = FID_HEAD; fid < FID_PAS; fid ++)
						db[fid] = pDoz->recip[fid].real.cst.roll.crr.real.dbDistance;
#if	defined(__STEER_MON__)
					sprintf(c_pMon, "cmp %f %f %f %f, ", db[0], db[1], db[2], db[3]);
					while (*c_pMon != '\0')	++ c_pMon;
#endif
					qsort((void*)db, LENGTH_TU, sizeof(double), CmpDouble);
					if (!pTool->WithinCriterion(pCrr->real.dbDistance, db[2], 5.f)) {
						pCrr->real.dbDistance = db[2];
						pArch->Shot(CAR_PSELF(), 184);
						if (pPaper->GetLabFromRecip())	TRACK("STER:cmp distance b. %f %f %f %f\n", db[0], db[1], db[2], db[3]);
					}
					else if (pPaper->GetLabFromRecip())	TRACK("STER:cmp distance c.\n");

					c_zsCalc.cSeq = CALCZEROSSEQ_CMPPOWER;
				}
				else {
					TRACK("STER>ERR:not match env. version1!\n");
					c_zsCalc.cSeq = CALCZEROSSEQ_END;
				}
			}
			else {
				/*if (pPaper->GetLabFromRecip()) {
					//BYTE ch = 0;
					bool bCatch = false;
					char buf[128];
					memset(buf, 0, 128);
					strncpy(buf, "Can not represent careers:", 128);
					if (pPaper->GetRouteState(FID_HEAD) && pDoz->recip[FID_HEAD].real.cst.cID != CSTID_CAREERS) {
						strncat(buf, " HEAD", 8);	//ch |= 1;
						bCatch = true;
					}
					if (pPaper->GetRouteState(FID_HEADBK) && pDoz->recip[FID_HEADBK].real.cst.cID != CSTID_CAREERS) {
						strncat(buf, " HBK", 8);	//ch |= 2;
						bCatch = true;
					}
					if (pPaper->GetRouteState(FID_TAIL) && pDoz->recip[FID_TAIL].real.cst.cID != CSTID_CAREERS) {
						strncat(buf, " TAIL", 8);	//ch |= 4;
						bCatch = true;
					}
					if (pPaper->GetRouteState(FID_TAILBK) && pDoz->recip[FID_TAILBK].real.cst.cID != CSTID_CAREERS) {
						strncat(buf, " TBK", 8);	//ch |= 8;
						bCatch = true;
					}
					if (bCatch)	TRACK("STER:%s\n", buf);	//ch);
				}*/
				c_zsCalc.cSeq = CALCZEROSSEQ_CMPDISTANCE;
			}
			//c_zsCalc.cSeq = CALCZEROSSEQ_CMPPOWER;
			break;
		case CALCZEROSSEQ_CMPPOWER :
			// 181003
			//if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_CAREERS && pDoz->recip[FID_HEAD].real.cst.roll.crr.real.wVersion == ENV_VERSION) {
			if (pDoz->recip[FID_HEAD].real.cst.cID == CSTID_CAREERS && pDoz->recip[FID_HEADBK].real.cst.cID == CSTID_CAREERS &&
				pDoz->recip[FID_TAIL].real.cst.cID == CSTID_CAREERS && pDoz->recip[FID_TAILBK].real.cst.cID == CSTID_CAREERS) {
				if (pDoz->recip[FID_HEAD].real.cst.roll.crr.real.wVersion == ENV_VERSION) {
					for (int n = 0; n < SIV_MAX; n ++) {
#if	0
						// 181011
						double dbMax = 0.f;
						int mid = FID_HEAD;
						for (int fid = FID_HEAD; fid < FID_PAS; fid ++) {
							if (pDoz->recip[fid].real.cst.roll.crr.real.dbSivPower[n] > dbMax) {
								dbMax = pDoz->recip[fid].real.cst.roll.crr.real.dbSivPower[n];
								mid = fid;
							}
						}
						if (mid != (int)CAR_FSELF() && !pTool->WithinCriterion(dbMax, pCrr->real.dbSivPower[n], 100.f)) {
							pCrr->real.dbSivPower[n] = dbMax;
							//c_zsCalc.bModifyCareers = true;
							pArch->Shot(CAR_PSELF(), 185 + n);
						}
#endif
						// 190826
						//for (int n = 0; n < LENGTH_TU; n ++)
						//	db[n] = pDoz->recip[n].real.cst.roll.crr.real.dbSivPower[n];
						for (int fid = FID_HEAD; fid < FID_PAS; fid ++)
							db[fid] = pDoz->recip[fid].real.cst.roll.crr.real.dbSivPower[n];
#if	defined(__STEER_MON__)
						sprintf(c_pMon, "%f %f %f %f, ", db[0], db[1], db[2], db[3]);
						while (*c_pMon != '\0')	++ c_pMon;
#endif
						qsort((void*)db, LENGTH_TU, sizeof(double), CmpDouble);
						if (!pTool->WithinCriterion(pCrr->real.dbSivPower[n], db[2], 100.f)) {
							pCrr->real.dbSivPower[n] = db[2];
							pArch->Shot(CAR_PSELF(), 185 + n);
						}
					}
					for (int n = 0; n < V3F_MAX; n ++) {
#if	0
						// 181011
						double dbMax = 0.f;
						int mid = FID_HEAD;
						for (int fid = FID_HEAD; fid < FID_PAS; fid ++) {
							if (pDoz->recip[fid].real.cst.roll.crr.real.dbV3fPower[n] > dbMax) {
								dbMax = pDoz->recip[fid].real.cst.roll.crr.real.dbV3fPower[n];
								mid = fid;
							}
						}
						if (mid != (int)CAR_FSELF() && !pTool->WithinCriterion(dbMax, pCrr->real.dbV3fPower[n], 100.f)) {
							pCrr->real.dbV3fPower[n] = dbMax;
							//c_zsCalc.bModifyCareers = true;
							pArch->Shot(CAR_PSELF(), 188 + n);
						}
#endif
						// 190826
						//for (int n = 0; n < LENGTH_TU; n ++)
						//	db[n] = pDoz->recip[n].real.cst.roll.crr.real.dbV3fPower[n];
						for (int fid = FID_HEAD; fid < FID_PAS; fid ++)
							db[fid] = pDoz->recip[fid].real.cst.roll.crr.real.dbV3fPower[n];
#if	defined(__STEER_MON__)
						sprintf(c_pMon, "%f %f %f %f, ", db[0], db[1], db[2], db[3]);
						while (*c_pMon != '\0')	++ c_pMon;
#endif
						qsort((void*)db, LENGTH_TU, sizeof(double), CmpDouble);
						if (!pTool->WithinCriterion(pCrr->real.dbV3fPower[n], db[2], 100.f)) {
							pCrr->real.dbV3fPower[n] = db[2];
							pArch->Shot(CAR_PSELF(), 188 + n);
						}
#if	0
						// 181011
						dbMax = 0.f;
						mid = FID_HEAD;
						for (int fid = FID_HEAD; fid < FID_PAS; fid ++) {
							if (pDoz->recip[fid].real.cst.roll.crr.real.dbV3fReviv[n] > dbMax) {
								dbMax = pDoz->recip[fid].real.cst.roll.crr.real.dbV3fReviv[n];
								mid = fid;
							}
						}
						if (mid != (int)CAR_FSELF() && !pTool->WithinCriterion(dbMax, pCrr->real.dbV3fReviv[n], 100.f)) {
							pCrr->real.dbV3fReviv[n] = dbMax;
							//c_zsCalc.bModifyCareers = true;
							pArch->Shot(CAR_PSELF(), 193 + n);
						}
#endif
						// 190826
						//for (int n = 0; n < LENGTH_TU; n ++)
						//	db[n] = pDoz->recip[n].real.cst.roll.crr.real.dbV3fReviv[n];
						for (int fid = FID_HEAD; fid < FID_PAS; fid ++)
							db[fid] = pDoz->recip[fid].real.cst.roll.crr.real.dbV3fReviv[n];
#if	defined(__STEER_MON__)
						sprintf(c_pMon, "%f %f %f %f, ", db[0], db[1], db[2], db[3]);
						while (*c_pMon != '\0')	++ c_pMon;
#endif
						qsort((void*)db, LENGTH_TU, sizeof(double), CmpDouble);
						if (!pTool->WithinCriterion(pCrr->real.dbV3fReviv[n], db[2], 100.f)) {
							pCrr->real.dbV3fReviv[n] = db[2];
							pArch->Shot(CAR_PSELF(), 193 + n);
						}
					}
#if	defined(__STEER_MON__)
					TRACK("STER:%s\n", c_bufMon);
#endif
					c_zsCalc.cSeq = CALCZEROSSEQ_SAVECAREERS;
				}
				else {
					TRACK("STER>ERR:not match env. version2!\n");
					c_zsCalc.cSeq = CALCZEROSSEQ_END;
				}
			}
			else {
				if (pPaper->GetLabFromRecip()) {
					BYTE ch = 0;
					if (pDoz->recip[FID_HEAD].real.cst.cID != CSTID_CAREERS)	ch |= 1;
					if (pDoz->recip[FID_HEADBK].real.cst.cID != CSTID_CAREERS)	ch |= 2;
					if (pDoz->recip[FID_TAIL].real.cst.cID != CSTID_CAREERS)	ch |= 4;
					if (pDoz->recip[FID_TAILBK].real.cst.cID != CSTID_CAREERS)	ch |= 8;
					TRACK("STER:df2 %d\n", ch);
				}
				c_zsCalc.cSeq = CALCZEROSSEQ_CMPPOWER;
			}
			//c_zsCalc.cSeq = CALCZEROSSEQ_SAVECAREERS;
			break;
		case CALCZEROSSEQ_SAVECAREERS :
			c_zsCalc.cSeq = CALCZEROSSEQ_END;
			break;
		default :	break;
		}
	}
	else {
		c_mtx.Lock();
		c_zsCalc.wTimer = 0;
		c_mtx.Unlock();
		//c_zsCalc.bModifyCareers = false;
		c_brake.cNorm &= ~(1 << NDETECT_ZEROSPEED);
		c_zsCalc.cSeq = CALCZEROSSEQ_NON;
	}
}

// 180105
//void CSteer::CapturePwm()
//{
//	GETPAPER(pPaper);
//	PDOZEN pDoz = pPaper->GetDozen();
//	WORD period = LOWWORD(pDoz->recip[FID_HEAD].real.avs.dwPwm);
//	WORD high = HIGHWORD(pDoz->recip[FID_HEAD].real.avs.dwPwm);
//
//	if (period < PWMPERIOD_MIN || period > PWMPERIOD_MAX)	c_mam.cPwm = 0;
//	else if (high > period)	c_mam.cPwm = 0;
//	else {
//		// 180105, ATO 모드에서도 TEBE는 P,B,PWM을 통해서 들어온다.
//		c_mam.cPwm = (BYTE)((DWORD)high * 100L / (DWORD)period);
//		if (c_mam.cPwm > PWMPERCENT_MAX)	c_mam.cPwm = PWMPERCENT_MAX;
//		else if (c_mam.cPwm < PWMPERCENT_MIN)	c_mam.cPwm = PWMPERCENT_MIN;
//	}
//}
///

// 180219
//void CSteer::CaptureNotch()
void CSteer::CapturePwm()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	WORD period = LOWWORD(pDoz->recip[FID_HEAD].real.avs.dwPwm);
	WORD high = HIGHWORD(pDoz->recip[FID_HEAD].real.avs.dwPwm);
	// 181003
	if (GETTI(FID_HEAD, TUDIB_FORWARD) || GETTI(FID_HEAD, TUDIB_REVERSE)) {
		if (high > PWMHIGH_MIN && high < PWMHIGH_MAX) {
			// 180222
			if (period < PWMPERIOD_MIN || period > PWMPERIOD_MAX)	period = 2000;		//for 210
			// 180219
			if (period < PWMPERIOD_MIN || period > PWMPERIOD_MAX)	c_mam.cRealPwm = 0;	//c_mam.cPhysicalPwm = 0;
			else if (high > period)	c_mam.cRealPwm = 0;									//c_mam.cPhysicalPwm = 0;
			else	c_mam.cRealPwm = (BYTE)((DWORD)high * 100L / (DWORD)period);		//c_mam.cPhysicalPwm = (BYTE)((DWORD)high * 100L / (DWORD)period);
		}
	}
	else	c_mam.cRealPwm = 0;

	c_mam.cConvPwm = c_mam.cRealPwm;
	if (c_mam.cConvPwm < PWMRATE_MIN)	c_mam.cConvPwm = PWMRATE_MIN;
	else if (c_mam.cConvPwm > PWMRATE_MAX)	c_mam.cConvPwm = PWMRATE_MAX;
	// at Contro(), c_tber.cEffort = c_mam.cConvPwm;

	c_mam.cNotch = NOTCHID_C;
	c_mam.cLogicPwm = 0;

	if (GETTI(FID_HEAD, TUDIB_POWERING) || GETTI(FID_HEAD, TUDIB_BRAKING)) {
		GETSCHEME(pSch);
		if (IsAutoMotive(c_cMode))
			c_mam.cLogicPwm = c_mam.cConvPwm = (BYTE)pSch->GetProportion((WORD)c_mam.cConvPwm, (WORD)PWMRATE_MIN, (WORD)PWMRATE_MAX, (WORD)0, (WORD)100);
		else {
			if (c_brake.emer.cCause & (1 << EBCAUSE_MC)) {
				c_mam.cLogicPwm = c_mam.cConvPwm = 100;
				c_mam.cNotch = NOTCHID_EB;
			}
			else {
				int paddle = GETTI(FID_HEAD, TUDIB_POWERING) ? 1 : 0;
				for (int n = 0; c_notchSpectrum[paddle][n].cMax > 0; n ++) {
					if (c_mam.cConvPwm >= c_notchSpectrum[paddle][n].cMin && c_mam.cConvPwm <= c_notchSpectrum[paddle][n].cMax) {
						c_mam.cNotch = c_notchSpectrum[paddle][n].id;
						c_mam.cLogicPwm = c_notchSpectrum[paddle][n].cTypic;		// 이값은 단지 수동 모드에서 HDU에 표시하는 값이다.
						break;
					}
				}
				c_mam.cConvPwm = (BYTE)pSch->GetProportion((WORD)c_mam.cConvPwm, (WORD)PWMRATE_MIN, (WORD)PWMRATE_MAX, (WORD)0, (WORD)100);
			}
		}
		//if (IsAutoMotive(c_cMode)) {
		//	GETSCHEME(pSch);
		//	c_mam.cLogicalPwm = c_mam.cPhysicalPwm = (BYTE)pSch->GetProportion((WORD)c_mam.cPhysicalPwm, (WORD)PWMRATE_MINA, (WORD)PWMRATE_MAXA, (WORD)0, (WORD)100);
		//	// 자동 모드에서 HDU에 표시하는 값은 계산된 PWM값을 그대로 표시한다.
		//}
	}
}

void CSteer::CaptureMode()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	// check ato comm.
	BYTE mode = (pDoz->recip[FID_HEAD].real.ato.a.t.s.mode.a & 0x1f);
	//if (GETTI(FID_HEAD, TUDIB_YMS))	mode |= 0x20;	// 푸시 단추라서 무시한다.
	if (GETTI(FID_HEAD, TUDIB_AFS))	mode |= 0x40;

	if (mode & 0x40) {
		c_cMode = MOTIVMODE_EMERGENCY;
		Closeup(CID_BOW, 47, 41, 48);					// 비상 운전
	}
	else if ((mode & 0x30) != 0) {
		if ((mode & 0x30) == 0x10) {			// 0x30) {
			c_cMode = MOTIVMODE_YARD;
			Closeup(CID_BOW, 46, 41, 48);				// 기지 운전
		}
		else {
			c_cMode = MOTIVMODE_OVERLAP;
			Closeup(CID_BOW, 48, 41, 48);				// 운전 방식 중복
		}
	}
	else if (mode == 8) {
		c_cMode = MOTIVMODE_MANUAL;
		Closeup(CID_BOW, 45, 41, 48);					// 수동 운전
	}
	else if (mode == 4) {
		c_cMode = MOTIVMODE_AUTO;
		Closeup(CID_BOW, 44, 41, 48);					// 자동 운전
	}
	else if (mode == 2) {
		c_cMode = MOTIVMODE_FULLAUTO;
		Closeup(CID_BOW, 43, 41, 48);					// 자동 운전
	}
	else if (mode == 1) {
		c_cMode = MOTIVMODE_AUTORECURRENCE;
		Closeup(CID_BOW, 42, 41, 48);					// 자동 회차
	}
	else if (mode == 0) {
		c_cMode = 0;	// mode 없음.
		Closeup(CID_BOW, 41, 41, 48);					// 운전 방식 없음
	}
	else {
		c_cMode = MOTIVMODE_OVERLAP;			// 운전 방식 중복
		Closeup(CID_BOW, 48, 41, 48);
	}
	// 171219
	if (c_cMode == MOTIVMODE_EMERGENCY)	c_bAfsMode = true;
	else {
		if (c_bAfsMode) {
			c_bAfsMode = false;
			CLand* pLand = (CLand*)c_pParent;
			ASSERTP(pLand);
			pLand->SetAtoLinePrevent();
		}
	}
}

// Control->Monitor	?????
void CSteer::Control(PRECIP pRecip)
{
	GETPAPER(pPaper);
	CUCUTOFF();

	PDOZEN pDoz = pPaper->GetDozen();
	CalcTachoSpeed();
	CaptureZeroSpeed();
	// 180115
	//CaptureNotch();
	//if (GETTI(FID_HEAD, TUDIB_HCR) && (GETTI(FID_HEAD, TUDIB_POWERING) || GETTI(FID_HEAD, TUDIB_BRAKING))) {
	//}
	//else {
	//	//c_pad.notch = NOTCHID_C;
	//	//c_pad.cIndv = 0;
	//	c_mam.cNotch = NOTCHID_C;
	//	c_mam.cTypic = 0;
	//}
	if (GETTI(FID_HEAD, TUDIB_HCR))	CapturePwm();		//Notch();
	CaptureMode();
	// 171121
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		if ((c_brake.cNorm & (1 << NDETECT_ZEROSPEED)) && !GETTI(FID_HEAD, TUDIB_DIR)) {
			GETARCHIVE(pArch);
			pArch->ClearSegment();
		}
	}

	pRecip->real.motiv.lead.b.atom = pDoz->recip[FID_HEAD].real.ato.a.t.s.note.b.atom;

	// 180105
	//if (c_mam.cNotch > NOTCHID_ERR && c_mam.cNotch < NOTCHID_C)	c_tber.cEffType = EFFORT_BRAKING;
	//else if (c_mam.cNotch > NOTCHID_C)	c_tber.cEffType = EFFORT_POWERING;
	//else	c_tber.cEffType = EFFORT_COASTING;
	// 180226
	if (GETTI(FID_HEAD, TUDIB_BRAKING))	c_tber.cEffType = EFFORT_BRAKING;
	else if (GETTI(FID_HEAD, TUDIB_POWERING))	c_tber.cEffType = EFFORT_POWERING;
	else	c_tber.cEffType = EFFORT_COASTING;

	//if (c_tber.cEffType != EFFORT_COASTING)	c_tber.cEffort = c_mam.cPwm;
	//else	c_tber.cEffort = 0;
	c_tber.cEffort = c_mam.cConvPwm;

	//pRecip->real.motiv.cMode = c_mode.cDetermined;		// CaptureMode()
	//pRecip->real.motiv.cNotch = (BYTE)c_pad.notch;		// CaptureNotch()
	pRecip->real.motiv.cMode = c_cMode;
	pRecip->real.motiv.cNotch = c_mam.cNotch;
	pRecip->real.motiv.cEffType = c_tber.cEffType;
	pRecip->real.motiv.cEffort = c_tber.cEffort;
	pRecip->real.drift.dwSpeed = (DWORD)c_speedCalc.dbCur;	// m/h, CalcTachoSpeed()
	// 180405
	c_mtm.Lock();
	if (!c_moment.bInit) {
		memset(&c_moment, 0, sizeof(MOMENTUMINFO));
		c_moment.clk = ClockCycles();
		c_moment.bInit = true;
	}
	else {
		c_moment.speed.dws[c_moment.speed.wIndex ++] = pRecip->real.drift.dwSpeed;	// m/h, CalcTachoSpeed()
		if (c_moment.speed.wIndex >= MOMENTUM_CYCLE) {
			uint64_t clk = ClockCycles();
			double sec = (double)(clk - c_moment.clk) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
			DWORD avr = 0;
			for (int n = 0; n < MOMENTUM_CYCLE; n ++)	avr += c_moment.speed.dws[n];
			avr /= MOMENTUM_CYCLE;

			GETTOOL(pTool);
			c_moment.vMomentum = (WORD)pTool->Deviround10((double)(avr - c_moment.dwSpeed) / sec);	// ex: 123 -> 1.23Km/s2
			// 운행 기록을 보면 약 20초에 60km/h 까지 도달하게 된다. 그럼 초당 3000m/h 가 나오므로 위의 결과 값은 300이 나오게 된다.
			c_moment.dwSpeed = avr;
			c_moment.clk = clk;
			c_moment.speed.wIndex = 0;

			if (!c_moment.power.bTrigger) {
				if (c_tber.cEffType == EFFORT_POWERING) {
					c_moment.power.bTrigger = true;
					c_moment.power.time.begin = clk;
					c_moment.power.distance.dbBegin = c_speedCalc.dist.dbCur;
					c_moment.power.time.dbCurrent = c_moment.power.distance.dbCurrent = 0.f;
				}
			}
			else {
				//if (c_tber.cEffType == EFFORT_POWERING) {
				//	c_moment.power.time.dbCurrent += ((double)(clk - c_moment.power.time.begin) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec);
				//	c_moment.power.distance.dbCurrent += (c_speedCalc.dist.dbCur - c_moment.power.distance.dbBegin);
				//}
				//c_moment.power.time.begin = clk;
				//c_moment.power.distance.dbBegin = c_speedCalc.dist.dbCur;
				//if (c_tber.cEffType == EFFORT_BRAKING)	c_moment.power.bTrigger = false;
				if (c_tber.cEffType == EFFORT_POWERING) {
					c_moment.power.time.dbCurrent += ((double)(clk - c_moment.power.time.begin) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec);
					c_moment.power.distance.dbCurrent += (c_speedCalc.dist.dbCur - c_moment.power.distance.dbBegin);
					c_moment.power.time.begin = clk;
					c_moment.power.distance.dbBegin = c_speedCalc.dist.dbCur;
				}
				else	c_moment.power.bTrigger = false;
			}

			if (!c_moment.brake.bTrigger) {
				if (c_tber.cEffType == EFFORT_BRAKING) {
					c_moment.brake.bTrigger = true;
					c_moment.brake.time.begin = clk;
					c_moment.brake.distance.dbBegin = c_speedCalc.dist.dbCur;
					c_moment.brake.time.dbCurrent = c_moment.brake.distance.dbCurrent = 0.f;
				}
			}
			else {
				//if (c_tber.cEffType == EFFORT_BRAKING) {
				//	c_moment.brake.time.dbCurrent += ((double)(clk - c_moment.brake.time.begin) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec);
				//	c_moment.brake.distance.dbCurrent += (c_speedCalc.dist.dbCur - c_moment.brake.distance.dbBegin);
				//}
				//c_moment.brake.time.begin = clk;
				//c_moment.brake.distance.dbBegin = c_speedCalc.dist.dbCur;
				//if (c_tber.cEffType == EFFORT_POWERING)	c_moment.brake.bTrigger = false;
				if (c_tber.cEffType == EFFORT_BRAKING) {
					c_moment.brake.time.dbCurrent += ((double)(clk - c_moment.brake.time.begin) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec);
					c_moment.brake.distance.dbCurrent += (c_speedCalc.dist.dbCur - c_moment.brake.distance.dbBegin);
					c_moment.brake.time.begin = clk;
					c_moment.brake.distance.dbBegin = c_speedCalc.dist.dbCur;
				}
				else	c_moment.brake.bTrigger = false;
			}
		}
	}
	c_mtm.Unlock();
}

//=== Monitor function ===
bool CSteer::CheckItemSign(WORD tudib, WORD shot)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETARCHIVE(pArch);
	if (GETTI(FID_HEAD, tudib)) {
		pArch->Shot(CID_BOW, shot);
		return true;
	}
	else {
		pArch->Cut(CID_BOW, shot);
		return false;
	}
}

bool CSteer::CheckItemMark(WORD tudib, WORD mbi, WORD shot)
{
	if (CheckItemSign(tudib, shot)) {
		c_brake.cNorm |= (1 << mbi);
		return true;
	}
	else {
		c_brake.cNorm &= ~(1 << mbi);
		return false;
	}
}

bool CSteer::CheckItemCause(WORD tudib, WORD ebi, WORD shot)
{
	//bool bPolarity = (tudib == TUDIB_MCEBN || tudib == TUDIB_ATPEBN) ? true : false;
	// 180704
	//bool bPolarity = tudib == TUDIB_MCEBN || tudib == TUDIB_MRPS ? true : false;
	bool bPolarity = tudib == TUDIB_ATPEB ? false : true;
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETARCHIVE(pArch);
	bool bCur = GETTI(FID_HEAD, tudib) ? true : false;
	bCur ^= bPolarity;
	if (bCur) {
		if (tudib == TUDIB_EBST && !GETTI(FID_HEAD, TUDIB_EBSH))	return false;
		c_brake.emer.cCause |= (1 << ebi);
		c_brake.emer.cCause &= ~(1 << EBCAUSE_OFFRAIL);
		c_brake.emer.cCycle = 0;
		pArch->Shot(CID_BOW, shot);
		pArch->Cut(CID_BOW, 56);
		return true;
	}
	else {
		c_brake.emer.cCause &= ~(1 << ebi);
		pArch->Cut(CID_BOW, shot);
		return false;
	}
}

bool CSteer::CheckEmergency()
{
	GETPAPER(pPaper);
	//if (!pPaper->IsActive())	return false;	// 의미가 있는지 모르겠다..

	PDOZEN pDoz = pPaper->GetDozen();
	GETARCHIVE(pArch);
	if (GETTI(FID_HEAD, TUDIB_EBSLR)) {
		if (GETTI(FID_HEAD, TUDIB_HCR)) {
			CheckItemCause(TUDIB_MCEBN, EBCAUSE_MC, 51);
			CheckItemCause(TUDIB_ATPEB, EBCAUSE_ATP, 52);
			CheckItemCause(TUDIB_EBSH, EBCAUSE_HSW, 53);
			CheckItemCause(TUDIB_EBST, EBCAUSE_TSW, 54);
			CheckItemCause(TUDIB_MRPS, EBCAUSE_MRPS, 55);
			// 171121
			//if (pPaper->GetRouteState(FID_TAIL) && !GETTI(FID_TAIL, TUDIB_MRPS)) {
			// 180911
			if (pPaper->GetRouteState(FID_TAIL) && VALIDRECIP(FID_TAIL) && !GETTI(FID_TAIL, TUDIB_MRPS)) {
				c_brake.emer.cCause |= (1 << EBCAUSE_MRPS);
				c_brake.emer.cCause &= ~(1 << EBCAUSE_OFFRAIL);
				c_brake.emer.cCycle = 0;
				pArch->Shot(CID_STERN, 55);
				pArch->Cut(CID_BOW, 56);
			}
			else	pArch->Cut(CID_STERN, 55);

			// 210803
			//if (c_brake.emer.cCause == 0 && ++ c_brake.emer.cCycle >= CAUSEOFEB_CYCLE) {
			BYTE cLimit = pPaper->GetDeviceExFromRecip() ? CAUSEOFEB_CYCLE2 : CAUSEOFEB_CYCLE1;
			if (c_brake.emer.cCause == 0 && ++ c_brake.emer.cCycle >= cLimit) {
				c_brake.emer.cCause = (1 << EBCAUSE_OFFRAIL);
				pArch->Shot(CID_BOW, 56);
				//c_brake.emer.cCycle = CAUSEOFEB_CYCLE;
				c_brake.emer.cCycle = cLimit;
			}
			// 181003
			//pArch->Cut(CID_BOW, 57);
		}
		else {
			c_brake.emer.cCause |= (1 << EBCAUSE_NONHCR);
			// 181003
			//pArch->Shot(CID_BOW, 57);
		}
	}
	else {
		c_brake.emer.cCause = 0;
		c_brake.emer.cCycle = 0;
		pArch->Cut(CID_BOW, 51, 57);
	}

	if (GETTI(FID_HEAD, TUDIB_EBCOS))	pArch->Shot(CID_BOW, 58);
	else	pArch->Cut(CID_BOW, 58);
	return (c_brake.emer.cCause != 0) ? true : false;
}

bool CSteer::CheckFSB()
{
	//if (!IsManualMotiv(c_mode.cDetermined))	return false;
	if (!IsManualMotive(c_cMode))	return false;

	// FSB는 PWM에 의해 주어지는데 자동에선 PWM의 변화가 심하므로 수동에서만 검사한다.
	GETARCHIVE(pArch);
	//if (c_pad.notch == NOTCHID_B7) {
	if (c_mam.cNotch == NOTCHID_B7) {
		c_brake.cNorm |= (1 << NBRAKE_FSB);
		pArch->Shot(CID_BOW, 58);
		return true;
	}
	else {
		c_brake.cNorm &= ~(1 << NBRAKE_FSB);
		pArch->Cut(CID_BOW, 58);
		return false;
	}
}

bool CSteer::CheckSecurityBrake()
{
	return CheckItemMark(TUDIB_SBR, NBRAKE_SECURITY, 59);
}

bool CSteer::CheckParkingBrake()
{
	//return CheckItemMark(TUDIB_PBR, NBRAKE_PARKING, 60);
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETARCHIVE(pArch);

	bool pb = false;
	if (GETTI(FID_HEAD, TUDIB_PBR)) {
		pb = true;
		pArch->Shot(CID_BOW, 60);
	}
	else	pArch->Cut(CID_BOW, 60);

	// 180911
	//if (pPaper->GetRouteState(FID_TAIL) && GETTI(FID_TAIL, TUDIB_PBR)) {
	if (pPaper->GetRouteState(FID_TAIL) && VALIDRECIP(FID_TAIL) && GETTI(FID_TAIL, TUDIB_PBR)) {
		pb = true;
		pArch->Shot(CID_STERN, 60);
	}
	else	pArch->Cut(CID_STERN, 60);

	if (pb) {
		c_brake.cNorm |= (1 << NBRAKE_PARKING);
		return true;
	}
	else {
		c_brake.cNorm &= ~(1 << NBRAKE_PARKING);
		return false;
	}
}

// 180219
// must monitor(){ if EB } -> control(){ not scan encoder pwm }
void CSteer::Monitor(PRECIP pRecip)
{
	GETPAPER(pPaper);
	CUCUTOFF();

	// 추진,제동 값을 검사. - 추진/제동 값은 운전 방식에 상관없이 Doz에 기록되어있다.
	// b이면 v3f, ecu 값 검사
	// p이면 v3f 값 검사
	// 하기로 했는데 이 부분은 CU에 해당되는 것이라서 다른 클래스에서 하기로 함.
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		CheckEmergency();
		//CheckFSB();
		CheckSecurityBrake();
		CheckParkingBrake();

		GETARCHIVE(pArch);
		// 170717, Remove, 여기서 하지 않고 개별 ECU에서 한다.
		//if (GETTI(FID_HEAD, TUDIB_POWERING) && GETTI(FID_HEAD, TUDIB_HBR)) {
		//	if (++ c_wHbReleaseWatch >= WATCH_HBRELEASE) {
		//		c_wHbReleaseWatch = WATCH_HBRELEASE;
		//		pArch->Shot(CID_BOW, 108);
		//	}
		//}
		//else {
		//	c_wHbReleaseWatch = 0;
		//	pArch->Cut(CID_BOW, 108);
		//}

		// 171221, 171220, 171116
		// 210614
		if (pPaper->GetDeviceExFromRecip()) {
			if (GETTI(FID_HEAD, TUDIB_HCR)) {
				if (c_wEncfUnDetectTimer == 0) {
					if (GETTI(FID_HEAD, TUDIB_ENCF))	Shot(CID_BOW, 37);
					else	Cut(CID_BOW, 37);
				}
				else	-- c_wEncfUnDetectTimer;
			}
			else	c_wEncfUnDetectTimer = TIME_ENCFUNDETECT;
		}
		else {
			if (GETTI(FID_HEAD, TUDIB_ENCF))	Shot(CID_BOW, 37);
			else	Cut(CID_BOW, 37);
		}

		if (!GETTI(FID_HEAD, TUDIB_MRPS))	pArch->Shot(CID_BOW, 211);
		else	pArch->Cut(CID_BOW, 211);
		// 171121
		// 180911
		//if (pPaper->GetRouteState(FID_TAIL) && !GETTI(FID_TAIL, TUDIB_MRPS))	pArch->Shot(CID_STERN, 211);
		if (pPaper->GetRouteState(FID_TAIL) && VALIDRECIP(FID_TAIL) && !GETTI(FID_TAIL, TUDIB_MRPS))
			pArch->Shot(CID_STERN, 211);
		else	pArch->Cut(CID_STERN, 211);

		if (GETTI(FID_HEAD, TUDIB_CPRS))	pArch->Shot(CID_BOW, 305);
		else	pArch->Cut(CID_BOW, 305);
		//20201124
		if (!pPaper->GetDeviceExFromRecip()) {
			if (GETTI(FID_HEAD, TUDIB_DIR) && !GETTI(FID_HEAD, TUDIB_DIAPDC))	pArch->Shot(CID_BOW, 882);
			else	pArch->Cut(CID_BOW, 882);
		}
		else {
			if (GETTI(FID_HEAD, TUDIB_DIR) && !GETTI(FID_HEAD, TUDIB_DIAPDC) && ++ c_wPsdoCounter >= 30) {
				pArch->Shot(CID_BOW, 882);
				c_wPsdoCounter = 30;
			}
			else {
				pArch->Cut(CID_BOW, 882);
				c_wPsdoCounter = 0;
			}
		}

		// monitor deadman
		if (c_mam.cNotch != NOTCHID_C && GETTI(FID_HEAD, TUDIB_DEADMAN))	pArch->Shot(CID_BOW, 66);
		else	pArch->Cut(CID_BOW, 66);

		// 190523
		if (GETTI(FID_HEAD, TUDIB_CUTTRACTION))	pArch->Shot(CID_BOW, 96);
		else	pArch->Cut(CID_BOW, 96);

		// monitor door close
		bool bDcs = GETTI(FID_HEAD, TUDIB_DCS) ? true : false;
		if (c_dcCmd.b.Stabilize(bDcs)) {
			if (c_dcCmd.b.cur && !c_dcCmd.bClsCmd) {
				c_dcCmd.bClsCmd = true;
				c_dcCmd.wWatch = WATCH_DOORINTERLOCK;
			}
		}

		GETVERSE(pVerse);
		CVerse::PPSHARE pPsv = pVerse->GetShareVars();
		if (GETTI(FID_HEAD, TUDIB_DIR)) {	// door close
			c_dcCmd.bClsCmd = false;
			c_dcCmd.wWatch = 0;
			pArch->Cut(CID_BOW, 880);

			if (!pPsv->vwbd.bStb) {
				if (++ pPsv->vwbd.wWatch >= WATCH_VALIDWEIGHTBYDIR)	pPsv->vwbd.bStb = true;
			}
			// 171221
			pArch->Cut(CID_BOW, 883);
		}
		else {								// door open
			if (c_dcCmd.bClsCmd) {
				if (c_dcCmd.wWatch > 0 && -- c_dcCmd.wWatch == 1)	pArch->Shot(CID_BOW, 880);
			}
			else	c_dcCmd.wWatch = 0;

			pPsv->vwbd.bStb = false;
			pPsv->vwbd.wWatch = 0;
			// 171221
			if (GETTI(FID_HEAD, TUDIB_HCR) && GETTI(FID_HEAD, TUDIB_POWERING))	pArch->Shot(CID_BOW, 883);
			else	pArch->Cut(CID_BOW, 883);
		}

		if (pDoz->recip[FID_HEAD].real.motiv.lead.b.r && pDoz->recip[FID_HEAD].real.drift.dwSpeed > REVERSE_MAXSPEED)
			pArch->Shot(CID_BOW, 65);
		else	pArch->Cut(CID_BOW, 65);

		pRecip->real.drift.cEbrk = c_brake.emer.cCause;		// CheckEmergency()
		pRecip->real.drift.cNbrk = c_brake.cNorm;			// CheckSecurityBrake(), CheckParkingBrake()
	}
}

void CSteer::Arteriam()
{
	if (c_zsCalc.wTimer > 0) {
		c_mtx.Lock();
		-- c_zsCalc.wTimer;
		c_mtx.Unlock();
	}
	if (c_speedCalc.wPulseWidthTimer > 0) {
		c_mtx.Lock();
		-- c_speedCalc.wPulseWidthTimer;
		c_mtx.Unlock();
		if (c_speedCalc.wPulseWidthTimer == 0) {
			CLand* pLand = (CLand*)c_pParent;
			ASSERTP(pLand);
			CFio* pFio = pLand->GetFio();
			pFio->SetOutput(UDOB_DISTP, false);
		}
	}
}

void CSteer::GetAvrLog(PLOGBOOK pLog)
{
	c_mts.Lock();
	pLog->real.r.wRealPwm = (WORD)c_mam.cRealPwm;
	//pLog->real.dwTachoSpeed = (DWORD)c_speedCalc.dbCur;		// 170821,	516줄, drift.dwSpeed와 겹친다..
	if (c_speedCalc.wSamplingLength > 0)	pLog->real.r.dwAvrSpeed = (DWORD)(c_speedCalc.dbSum / (double)c_speedCalc.wSamplingLength);
	else	pLog->real.r.dwAvrSpeed = (DWORD)c_speedCalc.dbCur;
	c_speedCalc.dbSum = 0.f;
	c_speedCalc.wSamplingLength = 0;
	c_mts.Unlock();
}

void CSteer::CopyMomentum(PMOMENTUMINFO pMoment)
{
	c_mtm.Lock();
	memcpy(pMoment, &c_moment, sizeof(MOMENTUMINFO));
	c_mtm.Unlock();
}

ENTRY_CONTAINER(CSteer)
	//SCOOP(&c_mode.d.cur,		sizeof(BYTE),	"Steer")
	//SCOOP(&c_mode.d.buf,		sizeof(BYTE),	"")
	SCOOP(&c_cMode,				sizeof(BYTE),	"Steer")
	SCOOP(&c_brake.emer.cCause,	sizeof(BYTE),	"")
	SCOOP(&c_brake.emer.cCycle,	sizeof(BYTE),	"")
	SCOOP(&c_speedCalc.dbCur,	sizeof(double),	"SPD")
	//SCOOP(&c_pad.d.cur,			sizeof(BYTE),	"notc")
	//SCOOP(&c_pad.notch,			sizeof(BYTE),	"")
	//SCOOP(&c_pwm.wCur,			sizeof(WORD),	"")
	SCOOP(&c_mam.cNotch,		sizeof(BYTE),	"mam")
	SCOOP(&c_mam.cRealPwm,		sizeof(BYTE),	"")
	SCOOP(&c_mam.cConvPwm,		sizeof(BYTE),	"")
	SCOOP(&c_mam.cLogicPwm,		sizeof(BYTE),	"")
	SCOOP(&c_dcCmd.bClsCmd,		sizeof(bool),	"dcc")
	SCOOP(&c_dcCmd.wWatch,		sizeof(WORD),	"")
	SCOOP(&c_zsCalc.cSeq,		sizeof(BYTE),	"zs")
	SCOOP(&c_zsCalc.wTimer,		sizeof(WORD),	"")
EXIT_CONTAINER()

