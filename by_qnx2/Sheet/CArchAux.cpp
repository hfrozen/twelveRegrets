/*
 * CArchAux.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

#include "Track.h"
#include "../Component2/CTool.h"
#include "../Component2/CBand.h"
#include "CArch.h"
#include "CSch.h"
#include "CPaper.h"
#include "CVerse.h"
#include "CProse.h"
#include "../ts/CLand.h"

#include "CArchAux.h"

#define	CONVW32Q(q, a)\
	do {\
		_OCTET oc;\
		oc.w[0] = XWORD(a[2]);\
		oc.w[1] = XWORD(a[1]);\
		oc.w[2] = XWORD(a[0]);\
		oc.w[3] = 0;\
		q = oc.qw;\
	} while (0)

#define	CONVW22D(d, a)\
	do {\
		_QUARTET qu;\
		qu.w[0] = XWORD(a[1]);\
		qu.w[1] = XWORD(a[0]);\
		d = qu.dw;\
	} while (0)

CArchAux::CArchAux()
{
	c_pParent = NULL;
	c_hMm = INVALID_HANDLE;
	c_hShared = INVALID_HANDLE;
	c_pBulk = NULL;
	c_nChannel = INVALID_HANDLE;
	c_nConnect = INVALID_HANDLE;
	c_hThread = INVALID_HANDLE;
	//memset(&c_hts, 0, sizeof(HTEMPSYNC));
	c_wBulkCmd = 0;
	c_cMmState = 0;
	c_bEcuTrace = false;
	c_bSivDetect = false;
	c_bRtdNewSheet = false;
	c_dwMakeCdmaTime = 0;
	Initial(INTROFROM_KEY);
	InitialEnvirons();
	InitialCareers();
	memset(&c_cmjog, 0, sizeof(CMJOG));
	c_cMs10 = 0;
	Launch();
}

CArchAux::~CArchAux()
{
	if (Validity(c_hMm)) {
		mq_close(c_hMm);
		c_hMm = INVALID_HANDLE;
	}
	if (Validity(c_hShared)) {
		close(c_hShared);
		c_hShared = INVALID_HANDLE;
		c_pBulk = NULL;
	}
	if (Validity(c_hThread)) {
		MsgSendPulse(c_nConnect, PRIORITY_POST, TEMPCMD_EXIT, c_nChannel);
		if (pthread_cancel(c_hThread) == EOK) {
			PVOID pResult;
			pthread_join(c_hThread, &pResult);
		}
		c_hThread = INVALID_HANDLE;
	}
	if (Validity(c_nConnect)) {
		ConnectDetach(c_nConnect);
		c_nConnect = INVALID_HANDLE;
	}
	if (Validity(c_nChannel)) {
		ChannelDestroy(c_nChannel);
		c_nChannel = INVALID_HANDLE;
	}
}

PVOID CArchAux::TemporaryManagerEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CArchAux* pAux = (CArchAux*)pVoid;
	if (pAux != NULL)	pAux->TemporaryManager();
	return NULL;
}

void CArchAux::TemporaryManager()
{
	TRACK("AUX:begin temp.man.\n");
	struct _pulse pulse;
	while (true) {
		int res = MsgReceivePulse(c_nChannel, &pulse, sizeof(pulse), NULL);
		if (res == 0) {
			switch (pulse.code) {
			case TEMPCMD_LOADECUTRACE :
				{
					_TWATCHCLICK();
					int size = (int)GetFileSize(ECUTRACEALL_PATHTMP);
					FILE* fp;
					if ((fp = fopen(ECUTRACEALL_PATHTMP, "r")) == NULL) {
						TRACK("AUX>ERR:fopen %s\n", ECUTRACEALL_PATHTMP);
						c_etu.wLength = 0;
					}
					else {
						fread(c_etu.tds.c1, 1, size, fp);
						fclose(fp);
						c_etu.wLength = (WORD)(size / (ECUTRACEPAGE_MAX * SIZE_TRACETEXT));
						if (size % (ECUTRACEPAGE_MAX * SIZE_TRACETEXT))	TRACK("AUX:ERR>trace length! %d\n", size);
						_TWATCHMEASURED();
						TRACK("AUX:load ecu. %f sec\n", __sec_);
					}
					c_etu.iStep = AETSTEP_END;
				}
				break;
			case TEMPCMD_SAVESIVTRACESPHERE :
				{
					_TWATCHCLICK();
					char path[256];
					// 190909
					//GETPROSE(pProse);
					//WORD wSivType = pProse->GetSivType();
					for (int n = 0; n < SIV_MAX; n ++) {
						//if (wSivType == 1) {
						//	for (int m = 0; m < (int)c_sivsp.cb[n].wLeng; m ++) {
						//		sprintf(path, "%s/%s%01d_%02d", PATHTMP, SIVTRACESPHERE_NAME, n, m);
						//		if (!SaveFileA(path, c_sivsp.cb[n].ds.c2[m], SIVTRACEREAL_SIZE))	// 8048 bytes
						//			TRACK("AUX>ERR:save siv trace failure! %s\n", path);
						//	}
						//}
						//else {
							for (int m = 0; m < (int)c_sivsp.cbEx[n].wLeng; m ++) {
								sprintf(path, "%s/%s%01d_%02d", PATHTMP, SIVTRACESPHERE_NAME, n, m);
								if (!SaveFileA(path, c_sivsp.cbEx[n].ds.c2[m], SIVTRACEREAL_SIZE))	// 180730 8048 bytes,	SIVTRACEREALEX_SIZE))	// 1616 byte
									TRACK("AUX>ERR:save siv trace failure! %s\n", path);
							}
						//}
					}
					c_sivsp.bExist = true;
					_TWATCHMEASURED();
					TRACK("AUX:save siv trace sphere. %f sec\n", __sec_);
				}
				break;
			case TEMPCMD_SAVEV3FTRACESPHERE :
				{
					_TWATCHCLICK();
					char path[256];
					for (int n = 0; n < V3F_MAX; n ++) {
						for (int m = 0; m < (int)c_v3fsp.cb[n].wLeng; m ++) {
							sprintf(path, "%s/%s%01d_%02d", PATHTMP, V3FTRACESPHERE_NAME, n, m);
							if (!SaveFileA(path, c_v3fsp.cb[n].ds.c2[m], V3FTRACEREAL_SIZE))	// 35008 byte
								TRACK("AUX>ERR:save v3f trace failure! %s\n", path);
						}
					}
					c_v3fsp.bExist = true;
					_TWATCHMEASURED();
					TRACK("AUX:save v3f trace sphere. %f sec\n", __sec_);
				}
				break;
			case TEMPCMD_SAVEECUTRACESPHERE :
				{
					_TWATCHCLICK();
					char path[256];
					for (int n = 0; n < ECU_MAX; n ++) {
						for (int m = 0; m < (int)c_ecusp.cb[n].wLeng; m ++) {
							sprintf(path, "%s/%s%01d_%02d", PATHTMP, ECUTRACESPHERE_NAME, n, m);
							if (!SaveFileA(path, c_ecusp.cb[n].ds.c2[m], ECUTRACEREAL_SIZE))	// 3030 byte
								TRACK("AUX>ERR:save ecu trace failure! %s\n", path);
						}
					}
					c_ecusp.bExist = true;					// 다음번 마감에서 mm에게 ECU TRACE SPHERE를 저장할 필요가 있다는 것을 알리기위해...
					_TWATCHMEASURED();
					TRACK("AUX:save ecu trace sphere. %f sec\n", __sec_);
				}
				break;
			case TEMPCMD_EXIT :
				TRACK("AUX:temporary manager end...\n");
				return;	break;
			default :	break;
			}
		}
	}
	TRACK("AUX:end temp.man.\n");
}

void CArchAux::Initial(int iFrom)
{
	if (iFrom != INTROFROM_OTRN) {
		for (int n = 0; n < V3F_MAX; n ++)	c_vbs.bHb[n] = c_vbs.bLb[n] = false;
		c_vbs.u.v.wAptness = BCAPTNESS_WORD;

		memset(&c_cdmar, 0, sizeof(CDMAREPORT));
		ClearEcuTrace();
		memset(&c_sivsp, 0, sizeof(SIVTRACESPHERE));
		memset(&c_v3fsp, 0, sizeof(V3FTRACESPHERE));
		memset(&c_ecusp, 0, sizeof(ECUTRACESPHERE));
	}
	InitialTable();
}

void CArchAux::InitialTable()
{
	c_dbSegment = c_dbPrevDist = 0.f;
	SetLogbookhead(false);
}

int64_t CArchAux::GetFileSize(const char* pPath)
{
	struct stat st;
	if (stat(pPath, &st) >= 0)	return ((int64_t)st.st_size);
	return 0;
}

int CArchAux::GetFileLength(const char* pPath, char cSign)
{
	int leng = 0;
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			if (pEntry->d_name[0] == cSign)	++ leng;
		}
		closedir(pDir);
	}
	return leng;
}

bool CArchAux::SaveFile(const char* pPath, PVOID pVoid, int iLength)
{
	FILE* fp;
	if ((fp = fopen(pPath, "w")) != NULL) {
		size_t leng = fwrite(pVoid, 1, (size_t)iLength, fp);
		fclose(fp);
		if (leng == (size_t)iLength)	return true;
	}
	return false;
}

bool CArchAux::SaveFileA(const char* pPath, PVOID pVoid, int iLength)
{
	int fs = open(pPath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fs < 0) {
		close(fs);
		TRACK("AUX>ERR:open %s\n", pPath);
		return false;
	}
	write(fs, pVoid, iLength);
	close(fs);
	return true;
}

bool CArchAux::MirrorEnvirons(BYTE* pSrc)
{
	ENVIRONS env;
	memcpy(&env, pSrc, sizeof(ENVIRONS));
	if (env.real.wVersion == ENV_VERSION) {
		memcpy(&c_environs, &env, sizeof(ENVIRONS));
		if (CheckEnvirons() != MERROR_NON) {
			InitialEnvirons();
			return false;
		}
		return true;
	}
	TRACK("ENV VERSIONA %d%d\n", env.c[0], env.c[1]);
	return false;
}

void CArchAux::AccordEnvirons(int eid)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	if (eid >= FID_PAS || pDoz->recip[eid].real.cst.cID != CSTID_ENVIRONS) {
		TRACK("AUX:can not accord env!\n");
		return;
	}

	BYTE sum = 0;
	for (int n = 0; n < (int)(sizeof(ENVIRONS) - 1); n ++)
		sum += pDoz->recip[eid].real.cst.roll.ecm.env.c[n];

	if (pDoz->recip[eid].real.cst.roll.ecm.env.real.wVersion != ENV_VERSION) {	// SaveEnv는 주기적인 것이 아니므로 쓰기 도중에 전원 차단으로 깨어질 염려는 없다.
		TRACK("AUX:invalid env. version! %d - 0x%04X\n", eid, pDoz->recip[eid].real.cst.roll.ecm.env.real.wVersion);
		return;
	}
	if (pDoz->recip[eid].real.cst.roll.ecm.env.real.cCSum != sum) {				// sum이 맞지않으면 fid_head를 선택하게되니.. 별다른 문제는 없다.
		TRACK("AUX:invalid env. checksum!\n");
		return;
	}

	if (memcmp(&c_environs.real.cWheelDiai[0], &pDoz->recip[eid].real.cst.roll.ecm.env.real.cWheelDiai[0], SIZE_ENVITEMS)) {
		//pPaper->Shoot(&pDoz->recip[eid], SIZE_RECIP);
		memcpy(&c_environs.real.cWheelDiai[0], &pDoz->recip[eid].real.cst.roll.ecm.env.real.cWheelDiai[0], SIZE_ENVITEMS);
		SaveEnvirons();
	}
	TRACK("AUX:accord env. from %d.\n", eid);

	AccordCmjog();
	AccordV3fBreaker();
}

// Environ은 logbook을 통해 저장할 수가 없으므로 요구가 있을 때(주로 DU를 통한 편집) 아래와 같이 tmp에 저장하였다가 mm을 통해 sd로 저장한다.
void CArchAux::SaveEnvirons()
{
	CArch* pArch = (CArch*)c_pParent;

	PDTSTIME pDtst = pArch->GetDtsTime();
	memcpy(&c_environs.real.dt, pDtst, sizeof(DTSTIME));
	c_environs.real.wVersion = ENV_VERSION;

	BYTE sum = 0;
	for (int n = 0; n < (int)(sizeof(ENVIRONS) - 1); n ++)	sum += c_environs.c[n];
	c_environs.real.cCSum = sum;

	SaveFile(ENVIRONS_PATHTMP, (PVOID)&c_environs, sizeof(ENVIRONS));
	pArch->ReserveSaveEnvirons();		// mm에 저장 명령을...
}

void CArchAux::InitialEnvirons()
{
	c_environs.real.wVersion = ENV_VERSION;

	CArch* pArch = (CArch*)c_pParent;
	if (pArch != NULL) {						// 생성자에서도 호출하므로...
		PDTSTIME pDtst = pArch->GetDtsTime();
		memcpy(&c_environs.real.dt, pDtst, sizeof(DTSTIME));
	}
	else	memset(&c_environs, 0, sizeof(ENVIRONS));

	for (int n = 0; n < LENGTH_WHEELDIA; n ++)
		c_environs.real.cWheelDiai[n] = DEF_WHEELDIA;
	for (int n = 0; n < CID_MAX; n ++)
		c_environs.real.cTempSv[n] = DEF_TEMPSV;
	c_environs.real.cVentTime = DEF_VENTTIME;
}

int CArchAux::CheckEnvirons()
{
	int res = CheckTimeScope(c_environs.real.dt, true);
	if (res != MERROR_NON)	return res;

	if (!CheckEnvironsTime(c_environs.real.dt, true))	return MERROR_UNMATCHTIME;
	if ((res = CheckDiaScope(c_environs.real.cWheelDiai, true)) != MERROR_NON)	return res;
	if ((res = CheckTempScope(c_environs.real.cTempSv, c_environs.real.cVentTime, true)) != MERROR_NON)	return res;
	return MERROR_NON;
}

// 지금 시간과 파일 저장 시간을 비교한다.
// 지금 시간이 파일 저장 시간보다 앞선다면 문제가 없으나(true를 리턴)
// 늦다면 전원 투입 때 rtc에 문제가 있는 것이므로 false를 리턴한다.
bool CArchAux::CheckEnvironsTime(DTSTIME dt, bool bTrack)
{
	CArch* pArch = (CArch*)c_pParent;
	if (pArch == NULL)	return false;

	PDTSTIME pDtst = pArch->GetDtsTime();
	if (pDtst->year < dt.year) {
		if (bTrack)	TRACK("AUX>TMACH:year! %d %d\n", pDtst->year, dt.year);
		return false;
	}
	else if (pDtst->year > dt.year)	return true;

	if (pDtst->mon < dt.mon) {
		if (bTrack)	TRACK("AUX>TMACH:month! %d %d\n", pDtst->mon, dt.mon);
		return false;
	}
	else if (pDtst->mon > dt.mon)	return true;

	if (pDtst->day < dt.day) {
		if (bTrack)	TRACK("AUX>TMACH:day! %d %d\n", pDtst->day, dt.day);
		return false;
	}
	else if (pDtst->day > dt.day)	return true;

	if (pDtst->hour < dt.hour) {
		if (bTrack)	TRACK("AUX>TMACH:hour! %d %d\n", pDtst->hour, dt.hour);
		return false;
	}
	else if (pDtst->hour > dt.hour)	return true;

	if (pDtst->min < dt.min) {
		if (bTrack)	TRACK("AUX>TMACH:minute! %d %d\n", pDtst->min, dt.min);
		return false;
	}
	else if (pDtst->min > dt.min)	return true;

	if (pDtst->sec < dt.sec) {
		if (bTrack)	TRACK("AUX>TMACH:second! %d %d\n", pDtst->sec, dt.sec);
		return false;
	}
	return true;
}

int CArchAux::CheckTimeScope(DTSTIME dt, bool bTrack)
{
	if (dt.year < 2000 || dt.year > 2060) {
		if (bTrack)	TRACK("ENV>ERR:YEAR-out of range!(%d)\n", dt.year);
		return MERROR_OUTRANGEYEAR;
	}
	if (dt.mon < 1 || dt.mon > 12) {
		if (bTrack)	TRACK("ENV>ERR:MONTH-out of range!(%d)\n", dt.mon);
		return MERROR_OUTRANGEMONTH;
	}
	if (dt.day < 1 || dt.day > 31) {
		if (bTrack)	TRACK("ENV>ERR:DAY-out of range!(%d)\n", dt.day);
		return MERROR_OUTRANGEDAY;
	}
	if (dt.hour > 23) {
		if (bTrack)	TRACK("ENV>ERR:HOUR-out of range!(%d)\n", dt.hour);
		return MERROR_OUTRANGEHOUR;
	}
	if (dt.min > 59) {
		if (bTrack)	TRACK("ENV>ERR:MINUTE-out of range!(%d)\n", dt.min);
		return MERROR_OUTRANGEMINUTE;
	}
	if (dt.sec > 59) {
		if (bTrack)	TRACK("ENV>ERR:SECOND-out of range!(%d)\n", dt.sec);
		return MERROR_OUTRANGESECOND;
	}
	return MERROR_NON;
}

int CArchAux::CheckDiaScope(BYTE* pDia, bool bTrack)
{
	for (int n = 0; n < LENGTH_WHEELDIA; n ++) {
		if (*pDia > (MAX_WHEELDIA - MIN_WHEELDIA)) {
			if (bTrack)	TRACK("ENV>ERR:DIAMETER #%d-out of range!(%d)\n", n, *pDia + MIN_WHEELDIA);
			return MERROR_OUTRANGEWHEELD0 + n;
		}
		++ pDia;
	}
	return MERROR_NON;
}

int CArchAux::CheckTempScope(BYTE* pTemp, BYTE cVentTime, bool bTrack)
{
	for (int n = 0; n < CID_MAX; n ++) {
		if (*pTemp < MIN_TEMPSV || *pTemp > MAX_TEMPSV) {
			if (bTrack)	TRACK("ENV>ERR:TEMPS #%d-out of range!(%d)\n", n, *pTemp);
			return MERROR_OUTRANGETEMPSV0 + n;
		}
		++ pTemp;
	}
	if (cVentTime < MIN_VENTTIME || cVentTime > MAX_VENTTIME) {
		if (bTrack)	TRACK("ENV>ERR:VENT TIME-out of range!(%d)\n", cVentTime);
		return MERROR_OUTRANGEVENTTIME;
	}
	return MERROR_NON;
}

// qword -> double
bool CArchAux::MirrorCareers(BYTE* pSrc)
{
	CAREERSB crr;
	memcpy(&crr, pSrc, sizeof(CAREERSB));
	if (crr.real.wVersion == CAREERS_VERSION) {
		c_careers.real.dbDistance = (double)crr.real.qwDistance;
		for (int n = 0; n < SIV_MAX; n ++)
			c_careers.real.dbSivPower[n] = (double)crr.real.dps.qwSivPower[n];
		for (int n = 0; n < V3F_MAX; n ++) {
			c_careers.real.dbV3fPower[n] = (double)crr.real.dps.qwV3fPower[n];
			c_careers.real.dbV3fReviv[n] = (double)crr.real.dps.qwV3fReviv[n];
		}
		c_careers.real.wVersion = CAREERS_VERSION;
		return true;
	}
	return false;
}

// double -> double
void CArchAux::AccordCareers(int fid)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[fid].real.cst.cID != CSTID_CAREERS)	return;

	// 181012
	//memcpy(&c_careers, &pDoz->recip[FID_HEAD].real.cst.roll.crr.c[0], sizeof(CAREERS));
	memcpy(&c_careers, &pDoz->recip[fid].real.cst.roll.crr.c[0], sizeof(CAREERS));
	TRACK("AUX:accord careers from %d.\n", fid);
}

void CArchAux::InitialCareers()
{
	c_careers.real.wVersion = CAREERS_VERSION;
	c_careers.real.dbDistance = 0.f;
	for (int n = 0; n < SIV_MAX; n ++)
		c_careers.real.dbSivPower[n] = 0.f;
	for (int n = 0; n < V3F_MAX; n ++)
		c_careers.real.dbV3fPower[n] = c_careers.real.dbV3fReviv[n] = 0.f;
}

void CArchAux::AddCareers(int nCrri, int di, double db)
{
	switch (nCrri) {
	case CRRITEM_DISTANCE :	c_careers.real.dbDistance += db;	break;
	case CRRITEM_SIVPOWER :	c_careers.real.dbSivPower[di] += db;	break;
	case CRRITEM_V3FPOWER :	c_careers.real.dbV3fPower[di] += db;	break;
	case CRRITEM_V3FREVIV :	c_careers.real.dbV3fReviv[di] += db;	break;
	default :	break;
	}
}

double CArchAux::GetCareers(int nCrri, int di)
{
	switch (nCrri) {
	case CRRITEM_DISTANCE :	return c_careers.real.dbDistance;	break;
	case CRRITEM_SIVPOWER :	return c_careers.real.dbSivPower[di];	break;
	case CRRITEM_V3FPOWER : return c_careers.real.dbV3fPower[di];	break;
	case CRRITEM_V3FREVIV :	return c_careers.real.dbV3fReviv[di];	break;
	default :	break;
	}
	return 0.f;
}

bool CArchAux::MirrorCmjog(BYTE* pSrc)
{
	CMJOGB cmj;
	memcpy(&cmj, pSrc, sizeof(CMJOGB));
	if (cmj.real.wVersion == CMJOG_VERSION) {
		memcpy(&c_cmjog, &cmj, sizeof(CMJOGB));
		CheckCmjog();
		return true;
	}
	return false;
}

void CArchAux::AccordCmjog()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	CMJOGTIMES cmj;
	memset(&cmj, 0, sizeof(CMJOGTIMES));

	for (int fid = FID_HEAD; fid < FID_PAS; fid ++) {
		if (pDoz->recip[fid].real.cst.cID == CSTID_ENVIRONS &&
			pDoz->recip[fid].real.cst.roll.ecm.cmj.real.wVersion == CMJOG_VERSION) {
			if (cmj.srt.a < pDoz->recip[fid].real.cst.roll.ecm.cmj.real.cjt.srt.a)
				cmj.srt.a = pDoz->recip[fid].real.cst.roll.ecm.cmj.real.cjt.srt.a;
			for (int n = 0; n < CM_MAX; n ++) {
				if (cmj.cm[n].a < pDoz->recip[fid].real.cst.roll.ecm.cmj.real.cjt.cm[n].a)
					cmj.cm[n].a = pDoz->recip[fid].real.cst.roll.ecm.cmj.real.cjt.cm[n].a;
			}
		}
	}

	c_cmjog.real.cjt.srt.a = cmj.srt.a;
	for (int n = 0; n < CM_MAX; n ++)
		c_cmjog.real.cjt.cm[n].a = cmj.cm[n].a;

	CheckCmjog();
	TRACK("AUX:accord cmjog.\n");
}

bool CArchAux::CheckCmjog()
{
	bool bRes = true;
	for (int n = 0; n < CM_MAX; n ++) {
		if (c_cmjog.real.cjt.cm[n].b.day > c_cmjog.real.cjt.srt.b.day || (c_cmjog.real.cjt.cm[n].b.day == c_cmjog.real.cjt.srt.b.day && c_cmjog.real.cjt.cm[n].b.sec > c_cmjog.real.cjt.srt.b.sec)) {
			c_cmjog.real.cjt.cm[n].a = c_cmjog.real.cjt.srt.a;
			bRes = false;
			TRACK("AUX:cm%d time over!\n", n);
		}
	}
	return bRes;
}

bool CArchAux::MirrorV3fBreaker(PV3FBRKCNTU pVbc)
{
	if (pVbc->v.wAptness == BCAPTNESS_WORD) {
		for (int n = 0; n < V3F_MAX; n ++) {
			c_vbs.u.v.wHbCnt[n] = pVbc->v.wHbCnt[n];
			c_vbs.u.v.wLbCnt[n] = pVbc->v.wLbCnt[n];
			c_vbs.bHb[n] = c_vbs.bLb[n] = 0;
		}
		c_vbs.u.v.wCSum = pVbc->v.wCSum;
		TRACK("AUX:mirror breaker.\n");
		return true;
	}
	return false;
}

void CArchAux::ClearV3fBreaker()
{
	for (int n = 0; n < V3F_MAX; n ++)
		c_vbs.u.v.wHbCnt[n] = c_vbs.u.v.wLbCnt[n] = 0;
}

void CArchAux::AccordV3fBreaker()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	if (pDoz->recip[FID_HEAD].real.cst.cID != CSTID_ENVIRONS)	return;

	WORD wHb[V3F_MAX];
	WORD wLb[V3F_MAX];
	for (int n = 0; n < V3F_MAX; n ++)	wHb[n] = wLb[n] = 0;

	bool bCSumForm = false;
	int fid;
	// 먼저 체크섬까지 검사한다.
	for (fid = FID_HEAD; fid < FID_PAS; fid ++) {
		if (pDoz->recip[fid].real.cst.cID == CSTID_ENVIRONS) {
			WORD sum = 0;
			for (int n = 0; n < (int)(sizeof(V3FBRKCNTS) - sizeof(WORD)); n ++)
				sum += (WORD)pDoz->recip[fid].real.cst.roll.ecm.bcu.c[n];
			if (pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wAptness == BCAPTNESS_WORD &&
				pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wCSum == sum) {
				bCSumForm = true;
				for (int n = 0; n < V3F_MAX; n ++) {
					if (wHb[n] < pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wHbCnt[n])
						wHb[n] = pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wHbCnt[n];
					if (wLb[n] < pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wLbCnt[n])
						wLb[n] = pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wLbCnt[n];
				}
			}
		}
	}

	if (!bCSumForm) {
		// 체크섬이 없으면 그냥 검사한다.
		for (fid = FID_HEAD; fid < FID_PAS; fid ++) {
			for (int n = 0; n < V3F_MAX; n ++) {
				if (pDoz->recip[fid].real.cst.cID == CSTID_ENVIRONS &&
					pDoz->recip[FID_HEAD].real.cst.roll.ecm.bcu.v.wAptness == BCAPTNESS_WORD) {
					if (wHb[n] < pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wHbCnt[n])
						wHb[n] = pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wHbCnt[n];
					if (wLb[n] < pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wLbCnt[n])
						wLb[n] = pDoz->recip[fid].real.cst.roll.ecm.bcu.v.wLbCnt[n];
				}
			}
		}
	}

	c_vbs.u.v.wAptness = BCAPTNESS_WORD;
	for (int n = 0; n < V3F_MAX; n ++) {
		c_vbs.u.v.wHbCnt[n] = wHb[n];
		c_vbs.u.v.wLbCnt[n] = wLb[n];
	}
	TRACK("AUX:accord v3f breaker.\n");
}

bool CArchAux::CullRequiredDoc()
{
	if (IsValidAid()) {
		AidCmd(AIDCMD_CULLREQUIREDDOC);
		return true;
	}
	return false;
}

bool CArchAux::CloseCurrent()
{
	if (IsValidAid()) {
		SetLogbookhead(true);
		AidCmd(AIDCMD_CLOSECURRENT);
		return true;
	}
	return false;
}

bool CArchAux::NewSheet()
{
	if (IsValidAid()) {
		SetLogbookhead(true);
		AidCmd(AIDCMD_NEWSHEET);
		return true;
	}
	return false;
}

bool CArchAux::SaveInspect(BYTE* pInsp)
{
	FILE* fp;
	if ((fp = fopen(INSPECT_PATHTMP, "w")) != NULL) {
		size_t leng = fwrite(pInsp, 1, sizeof(INSPREPORT), fp);
		fclose(fp);
		if (leng == sizeof(INSPREPORT)) {
			if (c_pBulk != NULL) {
				memcpy(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], pInsp, sizeof(WORD) + sizeof(DBFTIME));	// wItem + time
				AidCmdPeriod(AIDCMD_SAVEINSPECT, (BYTE*)pInsp, (WORD)sizeof(INSPREPORT));
			}
			TRACK("AUX:save insp.\n");
			return true;
		}
		TRACK("AUX>ERR:save insp!(%d:%d:%s)\n", sizeof(INSPREPORT), leng, strerror(errno));
	}
	else	TRACK("AUX>ERR:save insp!(%s)\n", strerror(errno));
	return false;
}

void CArchAux::ClearTraceSphere(int nDevID)
{
	switch (nDevID) {
	case DEVID_SIV :
		memset(&c_sivsp, 0, sizeof(SIVTRACESPHERE));
		break;
	case DEVID_V3F :
		memset(&c_v3fsp, 0, sizeof(V3FTRACESPHERE));
		break;
	case DEVID_ECU :
		memset(&c_ecusp, 0, sizeof(ECUTRACESPHERE));
		break;
	default :	break;
	}
}

bool CArchAux::GetTraceSphereDemand(int nDevID)
{
	switch (nDevID) {
	case DEVID_SIV :	return c_sivsp.bExist;	break;
	case DEVID_V3F :	return c_v3fsp.bExist;	break;
	case DEVID_ECU :	return c_ecusp.bExist;	break;
	default :	break;
	}
	return false;
}

void CArchAux::ClearTraceSphereDemand(int nDevID)
{
	switch (nDevID) {
	case DEVID_SIV :	c_sivsp.bExist = false;	break;
	case DEVID_V3F :	c_v3fsp.bExist = false;	break;
	case DEVID_ECU :	c_ecusp.bExist = false;	break;
	default :	break;
	}
}

void CArchAux::SetTraceLengthSphere(int nDevID, int nCID, WORD wLeng)
{
	int did;
	GETSCHEME(pSch);
	switch (nDevID) {
	case DEVID_SIV :
		did = pSch->GetSivID(nCID);
		if (did < SIV_MAX) {
			// 190909 sivsda has an extended length, but no trace
			//GETPROSE(pProse);
			//if (pProse->GetSivType() == 1)	c_sivsp.cb[did].wLeng = wLeng;
			//else
			c_sivsp.cbEx[did].wLeng = wLeng;
		}
		break;
	case DEVID_V3F :
		did = pSch->GetV3fID(nCID);
		if (did < V3F_MAX)	c_v3fsp.cb[did].wLeng = wLeng;
		break;
	case DEVID_ECU :
		did = nCID;
		if (did < ECU_MAX)	c_ecusp.cb[did].wLeng = wLeng;	break;
	default :	break;
	}
}

void CArchAux::LocateTraceSphere(int nDevID, int nCID, WORD wChapterID, WORD wPageID, BYTE* p)
{
	int did;
	GETSCHEME(pSch);
	//GETPROSE(pProse);
	switch (nDevID) {
	case DEVID_SIV :
		did = pSch->GetSivID(nCID);
		// 190909
		//if (pProse->GetSivType() == 1) {
		//	if (did < SIV_MAX && wChapterID < c_sivsp.cb[did].wLeng && wPageID < SIVTRACEPAGE_MAX)
		//		memcpy(&c_sivsp.cb[did].ds.c3[wChapterID][wPageID][0], p, SIZE_TRACETEXT);
		//}
		//else {
			// 180730
			//if (did < SIV_MAX && wChapterID < c_sivsp.cbEx[did].wLeng && wPageID < SIVTRACEPAGEEX_MAX)
			//	memcpy(&c_sivsp.cbEx[did].ds.c3[wChapterID][wPageID][0], p, SIZE_TRACETEXT);
			if (did < SIV_MAX && wChapterID < c_sivsp.cb[did].wLeng && wPageID < SIVTRACEPAGE_MAX)
				memcpy(&c_sivsp.cb[did].ds.c3[wChapterID][wPageID][0], p, SIZE_TRACETEXT);
		//}
		break;
	case DEVID_V3F :
		did = pSch->GetV3fID(nCID);
		if (did < V3F_MAX && wChapterID < c_v3fsp.cb[did].wLeng && wPageID < V3FTRACEPAGE_MAX)
			memcpy(&c_v3fsp.cb[did].ds.c3[wChapterID][wPageID][0], p, SIZE_TRACETEXT);
		break;
	case DEVID_ECU :
		did = nCID;
		if (did < ECU_MAX && wChapterID < c_ecusp.cb[did].wLeng  && wPageID < ECUTRACEPAGE_MAX)
			memcpy(&c_ecusp.cb[did].ds.c3[wChapterID][wPageID][0], p, SIZE_TRACETEXT);
		break;
	default :	break;
	}
}

void CArchAux::ResultTraceSphere(int nDevID)
{
	switch (nDevID) {
	case DEVID_SIV :
		// 190909
		//{
			//GETPROSE(pProse);
			for (int n = 0; n < SIV_MAX; n ++)
				//TRACK("AUX:siv id %d trace length %d\n", n, pProse->GetSivType() == 1 ? c_sivsp.cb[n].wLeng : c_sivsp.cbEx[n].wLeng);
				TRACK("AUX:siv id %d trace length %d\n", n, c_sivsp.cbEx[n].wLeng);
		//}
		break;
	case DEVID_V3F :
		for (int n = 0; n < V3F_MAX; n ++)
			TRACK("AUX:v3f id %d trace length %d\n", n, c_v3fsp.cb[n].wLeng);
		break;
	case DEVID_ECU :
		for (int n = 0; n < ECU_MAX; n ++)
			TRACK("AUX:ecu id %d trace length %d\n", n, c_ecusp.cb[n].wLeng);
		break;
	default :	break;
	}
}

void CArchAux::SaveTraceSphere(int nDevID)
{
	int iCode = 0;
	switch (nDevID) {
	case DEVID_SIV :	iCode = TEMPCMD_SAVESIVTRACESPHERE;	break;
	case DEVID_V3F :	iCode = TEMPCMD_SAVEV3FTRACESPHERE;	break;
	case DEVID_ECU :	iCode = TEMPCMD_SAVEECUTRACESPHERE;	break;
	default :	break;
	}
	if (iCode != 0)	MsgSendPulse(c_nConnect, PRIORITY_POST, iCode, c_nChannel);
}

void CArchAux::ClearEcuTrace()
{
	memset(&c_etu, 0, sizeof(ECUTRACEUNIT));
	remove(ECUTRACEALL_PATHTMP);
}

void CArchAux::ArrangeEcuTrace()
{
	c_pBulk->wState &= ~(1 << BULKSTATE_ARRANGEECUTRACE);
	c_etu.wLength = 0;
	if (IsValidAid())	AidCmd(AIDCMD_ARRAGEECUTRACE);
}

void CArchAux::LoadEcuTraceFromTemporary()
{
	MsgSendPulse(c_nConnect, PRIORITY_POST, TEMPCMD_LOADECUTRACE, c_nChannel);
}

bool CArchAux::TakeoutEcuTrace(WORD wChapID, WORD wPageID, BYTE* pBuf)
{

	if (wChapID >= c_etu.wLength || wPageID >= ECUTRACEPAGE_MAX)	return false;

	memcpy(pBuf, &c_etu.tds.c3[wChapID][wPageID][0], SIZE_TRACETEXT);
	return true;
}

int CArchAux::SaveEcuTrace(BYTE* pTrace)
{
	_TWATCHCLICK();
	FILE* fp;
	if ((fp = fopen(ECUTRACE_PATHTMP, "w")) != NULL) {
		size_t leng = fwrite(pTrace, 1, sizeof(SAECU) * SIZE_ECUTRACE, fp);
		fclose(fp);
		_TWATCHMEASURED();
		if (leng == (sizeof(SAECU) * SIZE_ECUTRACE)) {
			c_bEcuTrace = true;						// 다음번 마감에서 mm에게 ECU TRACE를 저장할 필요가 있다는 것을 알리기위해...
			TRACK("AUX:save ecu trace.(%f sec)\n", __sec_);
			return MERROR_NON;
		}
		remove(ECUTRACE_PATHTMP);
		TRACK("AUX>ERR:save ecu trace!(%d:%d:%s)\n", sizeof(SAECU) * SIZE_ECUTRACE, leng, strerror(errno));
	}
	else	TRACK("AUX>ERR:save ecu trace!(%s)\n", strerror(errno));
	return -1;
}

void CArchAux::EraseEcuTrace()
{
	if (IsValidAid())	AidCmd(AIDCMD_ERASEECUTRACE);
}

int CArchAux::SaveSivDetect(BYTE* pDetect)
{
	_TWATCHCLICK();
	FILE* fp;
	if ((fp = fopen(SIVDETECT_PATHTMP, "w")) != NULL) {
		size_t leng = fwrite(pDetect, 1, sizeof(SIVADETEX) * SIZE_SIVDETECT, fp);
		fclose(fp);
		_TWATCHMEASURED();
		if (leng == (sizeof(SIVADETEX) * SIZE_SIVDETECT)) {
			c_bSivDetect = true;
			TRACK("AUX:save siv detect.(%f sec)\n", __sec_);
			return MERROR_NON;
		}
		remove(SIVDETECT_PATHTMP);
		TRACK("AUX>ERR:save siv detect!(%d:%d:%s)\n", sizeof(SIVADETEX) * SIZE_SIVDETECT, leng, strerror(errno));
	}
	else	TRACK("AUX>ERR:save siv detect!(%s)\n", strerror(errno));
	return -1;
}

int CArchAux::SaveSivDetect2(BYTE* pDetect)
{
	_TWATCHCLICK();
	FILE* fp;
	if ((fp = fopen(SIVDETECT_PATHTMP, "w")) != NULL) {
		size_t leng = fwrite(pDetect, 1, sizeof(SIVADET2) * SIZE_SIVDETECT, fp);
		fclose(fp);
		_TWATCHMEASURED();
		if (leng == (sizeof(SIVADET2) * SIZE_SIVDETECT)) {
			c_bSivDetect = true;
			TRACK("AUX:save siv detect.(%f sec)\n", __sec_);
			return MERROR_NON;
		}
		remove(SIVDETECT_PATHTMP);
		TRACK("AUX>ERR:save siv detect!(%d:%d:%s)\n", sizeof(SIVADET2) * SIZE_SIVDETECT, leng, strerror(errno));
	}
	else	TRACK("AUX>ERR:save siv detect!(%s)\n", strerror(errno));
	return -1;
}

void CArchAux::InitialBulkSyncro()
{
	c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex = 0;
	c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal = 0;
	c_pBulk->m[BPID_OCCASN].bEnd = false;
	c_pBulk->m[BPID_OCCASN].bBegin = true;
}

void CArchAux::GrowBulkSyncro()
{
	++ c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex;
	c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal = 0;
	c_pBulk->m[BPID_OCCASN].bEnd = false;
	c_pBulk->m[BPID_OCCASN].bBegin = true;
}

void CArchAux::GetLogbookEntry(DWORD dwIndex, DWORD dwTotal)
{
	c_pBulk->m[BPID_OCCASN].bEnd = false;
	c_pBulk->m[BPID_OCCASN].bBegin = true;
	AidCmdOccasion(AIDCMD_EXTRACTLOGBOOKENTRY, dwIndex, dwTotal);
}

void CArchAux::GetInspectEntry(DWORD dwIndex, DWORD dwTotal)
{
	c_pBulk->m[BPID_OCCASN].bEnd = false;
	c_pBulk->m[BPID_OCCASN].bBegin = true;
	AidCmdOccasion(AIDCMD_EXTRACTINSPECTENTRY, dwIndex, dwTotal);
}

void CArchAux::GetLogbookList(char* pszLogbook, DWORD dwIndex, DWORD dwTotal)
{
	c_pBulk->m[BPID_OCCASN].bEnd = false;
	c_pBulk->m[BPID_OCCASN].bBegin = true;
	if (pszLogbook != NULL)
		strncpy(c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.h.f.szName, pszLogbook, SIZE_ENTRYNAME);
	AidCmdOccasion(AIDCMD_EXTRACTLOGBOOKLIST, dwIndex, dwTotal);
}

void CArchAux::CopyLogbook(char* pszLogbook)
{
	if (IsValidAid() && pszLogbook != NULL) {
		strncpy(c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.h.f.szName, pszLogbook, SIZE_ENTRYNAME);
		InitialBulkSyncro();
		AidCmd(AIDCMD_COPYLOGBOOK);
	}
}

void CArchAux::GetLogbook(char* pszLogbook)
{
	if (IsValidAid()) {
		if (pszLogbook != NULL) {
			strncpy(c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.h.f.szName, pszLogbook, SIZE_ENTRYNAME);
			InitialBulkSyncro();
		}
		else	GrowBulkSyncro();
		AidCmd(AIDCMD_EXTRACTLOGBOOK);
	}
}

void CArchAux::GetInspect(char* pszInspect)
{
	if (IsValidAid()) {
		if (pszInspect != NULL) {
			strncpy(c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.h.f.szName, pszInspect, SIZE_ENTRYNAME);
			InitialBulkSyncro();
		}
		else	GrowBulkSyncro();
		AidCmd(AIDCMD_EXTRACTINSPECT);
	}
}

void CArchAux::MakeDestination(char* pszIndex, bool bLog)
{
	// pszIndex는 이니셜을 포함하고 있다.
	if (IsValidAid()) {
		if (bLog) {
			c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.ucmd.cItem = USBDIRITEM_DAILY;
			DWORD dw = (DWORD)atoi(pszIndex + 1);
			c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.ucmd.dwIndex = dw;
		}
		else	c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.ucmd.cItem = USBDIRITEM_INSPECT;
		c_pBulk->m[BPID_OCCASN].bEnd = false;
		c_pBulk->m[BPID_OCCASN].bBegin = true;
		AidCmd(AIDCMD_DESTINATIONONUSB);
	}
}

void CArchAux::PutLogbook(PRTDSPADE pSpd)
{
	if (pSpd != NULL)	memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad, pSpd, sizeof(RTDSPADE));
	if (IsValidAid()) {
		c_pBulk->m[BPID_OCCASN].bEnd = false;
		c_pBulk->m[BPID_OCCASN].bBegin = true;
		AidCmd(AIDCMD_INSERTIONLOGBOOK);
	}
}

void CArchAux::PutInspect(PRTDSPADE pSpd)
{
	if (pSpd != NULL)	memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad, pSpd, sizeof(RTDSPADE));
	if (IsValidAid()) {
		c_pBulk->m[BPID_OCCASN].bEnd = false;
		c_pBulk->m[BPID_OCCASN].bBegin = true;
		TRACK("AUX:put insp.\n");
		AidCmd(AIDCMD_INSERTIONINSPECT);
	}
}

void CArchAux::BuildCdmaReport()
{
	_TWATCHCLICK();
	memset(&c_cdmar, 0, sizeof(CDMAREPORT));

	CArch* pArch = (CArch*)c_pParent;
	if (pArch == NULL)	return;
	pArch->GetSysTimeToDbf(&c_cdmar.real.dbft);

	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	c_cdmar.real.cProperNo = GMSB(hdlb.wProperNo);
	c_cdmar.real.cab.a = GLSB(hdlb.cabEx.a);
	// 171127
	//c_cdmar.real.lamp.a = GLSB(hdlb.lamp.a) & 0xf;
	c_cdmar.real.lamp.a = 0;
	c_cdmar.real.lamp.b.pb = hdlb.lampEx.b.pb;
	c_cdmar.real.lamp.b.sb = hdlb.lampEx.b.sb;
	c_cdmar.real.lamp.b.eb = hdlb.lampEx.b.eb;

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	c_cdmar.real.lamp.b.dir = GETTI(FID_HEAD, TUDIB_DIR) ? true : false;

	c_cdmar.real.lamp.b.psdc = hdlb.lampEx.b.psdc;
	c_cdmar.real.lamp.b.adbs = hdlb.lampEx.b.adbs;
	c_cdmar.real.lamp.b.fire = hdlb.lampEx.b.fire;
	c_cdmar.real.lamp.b.err = hdlb.lampEx.b.trb;
	c_cdmar.real.cMode = GMSB(hdlb.wMode);
	c_cdmar.real.cPowerRate = GMSB(hdlb.wPowerRate);
	c_cdmar.real.cBrakeRate = GMSB(hdlb.wBrakeRate);
	c_cdmar.real.cNextStation = GMSB(hdlb.wNextStation);
	c_cdmar.real.cDestStation = GMSB(hdlb.wDestStation);
	c_cdmar.real.wCurSpeed = XWORD(hdlb.wSpeed);
	_QUARTET quar;
	quar.dw = hdlb.dwOdoMeter;
	c_cdmar.real.dwDistance = ((DWORD)quar.c[0] << 24) | ((DWORD)quar.c[1] << 16) | ((DWORD)quar.c[2] << 8) | (DWORD)quar.c[3];
	c_cdmar.real.wOtrn = XWORD(hdlb.wOtrn);

	//GETPAPER(pPaper);
	DWORD dw0, dw1;
	CONVW22D(dw0, unab.wBypass);
	CONVW22D(dw1, unab.wEmhand);
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		WORD wd = XWORD(unab.wDoor[n][0]);
		for (int m = 0; m < 4; m ++) {
			if ((wd & 0xf) >= 8)	c_cdmar.real.cDoor[n] |= (1 << m);
			wd >>= 4;
		}
		wd = XWORD(unab.wDoor[n][1]);
		for (int m = 0; m < 4; m ++) {
			if ((wd & 0xf) >= 8)	c_cdmar.real.cDoor[n] |= (1 << (m + 4));
			wd >>= 4;
		}
		if ((dw0 & 3) != 0) {
			if (n < 8)	c_cdmar.real.cDbp |= (1 << n);
			else	c_cdmar.real.dbeh.a |= (1 << (n - 8));
		}
		dw0 >>= 2;
		if ((dw1 & 3) != 0) {
			if (n < 8)	c_cdmar.real.cDeh |= (1 << n);
			else	c_cdmar.real.dbeh.a |= (1 << (n - 6));
		}
		dw1 >>= 2;
	}

	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	CLand* pLand = (CLand*)pArch->GetCarrier();
	ASSERTP(pLand);
	for (int n = 0; n < pConf->pan.cLength; n ++) {
		BYTE pan = pLand->GetPantoState(n);
		c_cdmar.real.panto.a |= ((pan & 3) << (n * 2));
	}

	//PDOZEN pDoz = pPaper->GetDozen();
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();

	WORD w0 = XWORD(unab.wSiv);
	WORD w1 = XWORD(unab.wCmsb);
	for (int n = 0; n < pConf->siv.cLength; n ++) {
		c_cdmar.real.svf.a |= (ConvHdcToCdcA((BYTE)(w0 & 0xf)) << (n * 2));
		w0 >>= 4;
		c_cdmar.real.etc.a |= (ConvHdcToCdcB((BYTE)(w1 & 0xf)) << ((n + 2) * 2));
		w1 >>= 4;
		if (!IsFreeze(pLsv->sivcf.cLines[n])) {
			int fid = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[n]);
			// 200218
			//c_cdmar.real.cSivVo[n] = pDoz->recip[fid].real.svf.s.a.t.s.cVo;
			c_cdmar.real.cSivVo[n] = !pPaper->GetDeviceExFromRecip() ? pDoz->recip[fid].real.svf.s.a.t.s.cVo : pDoz->recip[fid].real.svf.s2.a.t.s.cVo;
			// 171114
			//c_cdmar.real.cSivIo[n] = pDoz->recip[fid].real.svf.s.a.t.s.cIo;
			//c_cdmar.real.wSivIo[n] = (WORD)pDoz->recip[fid].real.svf.s.a.t.s.ste.b.iom << 8 | (WORD)pDoz->recip[fid].real.svf.s.a.t.s.cIo;
			c_cdmar.real.wSivIo[n] = !pPaper->GetDeviceExFromRecip() ?
									(WORD)pDoz->recip[fid].real.svf.s.a.t.s.ste.b.iom << 8 | (WORD)pDoz->recip[fid].real.svf.s.a.t.s.cIo :
									XWORD(pDoz->recip[fid].real.svf.s2.a.t.s.wIo);
			c_cdmar.real.cSivNull[n] = 0;
			//c_cdmar.real.cSivFo[n] = pDoz->recip[fid].real.svf.s.a.t.s.cFo;
			c_cdmar.real.cSivFo[n] = !pPaper->GetDeviceExFromRecip() ? pDoz->recip[fid].real.svf.s.a.t.s.cFo : pDoz->recip[fid].real.svf.s2.a.t.s.cFo;
			c_cdmar.real.wBatV[n] = pSch->GetProportion((WORD)pDoz->recip[fid].real.avs.wCurve[ADCONVCH_BATTERY], (WORD)0, (WORD)4095, (WORD)0, (WORD)MAX_BATTERY);
			c_cdmar.real.wBatV[n] += OFFSETV_BATTERY;
			// 170929
			//c_cdmar.real.cSivEs[n] = pDoz->recip[fid].real.svf.s.a.t.s.cEs;
			c_cdmar.real.cSivEs[n] = !pPaper->GetDeviceExFromRecip() ? pDoz->recip[fid].real.svf.s.a.t.s.cEs : pDoz->recip[fid].real.svf.s2.a.t.s.cEs;
		}
	}

	CONVW22D(dw0, unab.wV3f);
	w0 = XWORD(unab.wHb);
	w1 = XWORD(unab.wLb);
	WORD w2 = 0;
	for (int n = 0; n < pConf->v3f.cLength; n ++) {
		c_cdmar.real.svf.a |= (ConvHdcToCdcA((BYTE)(dw0 & 0xf)) << ((n + 3) * 2));
		dw0 >>= 4;
		if ((w0 & 3) == LPS_ON)	c_cdmar.real.hbc.a |= (1 << n);
		if ((w1 & 3) == LPS_ON)	c_cdmar.real.lba.a |= (1 << n);
		w0 >>= 2;
		w1 >>= 2;
		if (!IsFreeze(pLsv->v3fcf.cLines[n])) {
			int fid = pSch->C2F(pPaper->GetTenor(), pConf->v3f.cPos[n]);
			c_cdmar.real.cV3fIq[n] = pDoz->recip[fid].real.svf.v.a.t.s.cIq;
			c_cdmar.real.wV3fFc[n] = XWORD(pDoz->recip[fid].real.svf.v.a.t.s.wFc);
			c_cdmar.real.cV3fTbe[n] = pDoz->recip[fid].real.svf.v.a.t.s.cTbe;
			WORD w = XWORD(pDoz->recip[fid].real.svf.v.a.t.s.wApbr);
			SHORT v = (SHORT)w;
			if (v >= 0)	c_cdmar.real.wV3fAbr[n] = 0;
			else	c_cdmar.real.wV3fAbr[n] = (WORD)abs(v);
			w = 0;
			if (pDoz->recip[fid].real.svf.v.a.t.s.cmd.b.p)	w = 2;
			else if (pDoz->recip[fid].real.svf.v.a.t.s.cmd.b.b)	w = 3;
			w2 |= (w << (n << 1));
		}
	}
	c_cdmar.real.v3fd.a = w2;

	QWORD qw0, qw1, qw2, qw3, qw4, qw5, qw6, qw7, qw8, qw9, qw10;
	CONVW32Q(qw0, unbb.wEcu);
	CONVW32Q(qw1, unbb.wFire);
	CONVW32Q(qw2, unbb.wPhone);
	CONVW32Q(qw3, unbb.wBcs);
	CONVW32Q(qw4, unbb.wAss);
	CONVW32Q(qw5, unbb.wHeat);
	CONVW32Q(qw6, unbb.wCool1);
	CONVW32Q(qw7, unbb.wCool2);
	CONVW32Q(qw8, unbb.wLampAc1);
	CONVW32Q(qw9, unbb.wLampAc2);
	CONVW32Q(qw10, unbb.wLampDc);
	CONVW22D(dw0, unbb.wFan1);
	CONVW22D(dw1, unbb.wVent);
	DWORD dw2;
	CONVW22D(dw2, unbb.wClean);
	DWORD dw3;
	CONVW22D(dw3, unbb.wFan2);
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		BYTE cv = ConvHdcToCdcA((BYTE)(qw0 & 0xf));
		if (n < 8)	c_cdmar.real.wEcu |= (cv << (n * 2));
		else	c_cdmar.real.etc.a |= (cv << ((n - 8) * 2));
		qw0 >>= 4;

		cv = ConvHdcToCdcC((BYTE)(qw1 & 0xf));
		if (n < 8)	c_cdmar.real.wFire |= (cv << (n * 2));
		else	c_cdmar.real.fph.a |= (cv << ((n - 8) * 2));
		qw1 >>= 4;

		if ((qw2 & 0xf) != LPS_NORMAL)	c_cdmar.real.fph.a |= (1 << (n + 4));
		qw2 >>= 4;

		if ((qw3 & 0xf) == LPS_ON) {
			if (n < 8)	c_cdmar.real.cBc |= (1 << n);
			else	c_cdmar.real.hbc.a |= (1 << (n - 3));
		}
		qw3 >>= 4;

		if ((qw4 & 0xf) == LPS_ON) {
			if (n < 8)	c_cdmar.real.cAs |= (1 << n);
			else	c_cdmar.real.lba.a |= (1 << (n - 3));
		}
		qw4 >>= 4;

		if (n < 8) {
			c_cdmar.real.wHeat |= (WORD)(((WORD)qw5 & 3) << (n * 2));
			c_cdmar.real.wCool1 |= (WORD)(((WORD)qw6 & 3) << (n * 2));
			c_cdmar.real.wCool2 |= (WORD)(((WORD)qw7 & 3) << (n * 2));
		}
		else {
			c_cdmar.real.hf.a |= (WORD)(((WORD)qw5 & 3) << ((n - 8) * 2));
			c_cdmar.real.cov.a |= (WORD)(((WORD)qw6 & 3) << ((n - 8) * 2));
			c_cdmar.real.ctr.a |= (WORD)(((WORD)qw7 & 3) << ((n - 8) * 2));
		}
		qw5 >>= 4;
		qw6 >>= 4;
		qw7 >>= 4;

		if ((qw8 & 0xf) == LPS_ON) {
			if (n < 8)	c_cdmar.real.cAc1 |= (1 << n);
			else	c_cdmar.real.rem.a |= (1 << (n - 8));
		}
		qw8 >>= 4;
		if ((qw9 & 0xf) == LPS_ON) {
			if (n < 8)	c_cdmar.real.cAc2 |= (1 << n);
			else	c_cdmar.real.rem.a |= (1 << (n - 6));
		}
		qw9 >>= 14;
		if ((qw10 & 0xf) == LPS_ON) {
			if (n < 8)	c_cdmar.real.cDc |= (1 << n);
			else	c_cdmar.real.rem.a |= (1 << (n - 4));
		}
		qw10 >>= 4;

		if ((dw0 & 3) != 0)	c_cdmar.real.hf.a |= (1 << (n + 4));
		if ((dw1 & 3) != 0)	c_cdmar.real.cov.a |= (1 << (n + 4));
		if ((dw2 & 3) != 0)	c_cdmar.real.ctr.a |= (1 << (n + 4));
		if ((dw3 * 3) != 0)	c_cdmar.real.fan2.a |= (1 << n);
		dw0 >>= 2;
		dw1 >>= 2;
		dw2 >>= 2;
		dw3 >>= 2;

		if (!IsFreeze(pLsv->ecucf.cLines[n])) {
			int fid = pSch->C2F(pPaper->GetTenor(), n);
			// 200218
			//c_cdmar.real.cEcuBcp[n] = pDoz->recip[fid].real.ecu.a.t.s.cBcp;
			//c_cdmar.real.cEcuAsp[n] = pDoz->recip[fid].real.ecu.a.t.s.cAsp;
			//c_cdmar.real.wBcv[n] = pSch->GetProportion((WORD)pDoz->recip[fid].real.avs.wCurve[ADCONVCH_PRESSURE], (WORD)819, (WORD)4096, (WORD)0, (WORD)100);
			// 170929
			//c_cdmar.real.cEcuLw[n] = pDoz->recip[fid].real.ecu.a.t.s.cLw;
			if (pPaper->GetDeviceExFromRecip()) {
				c_cdmar.real.cEcuBcp[n] = pDoz->recip[fid].real.ecu.ct.a.t.s.cBcp;
				c_cdmar.real.cEcuAsp[n] = (pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[0] +
											pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1] +
											pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1] +
											pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1]) / 4;
				c_cdmar.real.wBcv[n] = pSch->GetProportion((WORD)pDoz->recip[fid].real.avs.wCurve[ADCONVCH_PRESSURE], (WORD)819, (WORD)4096, (WORD)0, (WORD)100);
				c_cdmar.real.cEcuLw[n] = pDoz->recip[fid].real.ecu.ct.a.t.s.cLw;
			}
			else {
				c_cdmar.real.cEcuBcp[n] = pDoz->recip[fid].real.ecu.co.a.t.s.cBcp;
				c_cdmar.real.cEcuAsp[n] = pDoz->recip[fid].real.ecu.co.a.t.s.cAsp;
				c_cdmar.real.wBcv[n] = pSch->GetProportion((WORD)pDoz->recip[fid].real.avs.wCurve[ADCONVCH_PRESSURE], (WORD)819, (WORD)4096, (WORD)0, (WORD)100);
				// 170929
				c_cdmar.real.cEcuLw[n] = pDoz->recip[fid].real.ecu.co.a.t.s.cLw;
			}
		}
		if (!IsFreeze(pLsv->hvaccf.cLines[n])) {
			// 200218
			//c_cdmar.real.cIntTemp[n] = pDoz->recip[fid].real.hvac.a.t.s.cTemp;
			c_cdmar.real.cIntTemp[n] = pLsv->hvaccf.fl[n].a.t.s.cTemp;
		}
	}
	w0 = XWORD(unab.wEsk);
	if ((w0 & 0xf) != 0)	c_cdmar.real.etc.b.esk3 = true;
	if ((w0 & 0xf0) != 0)	c_cdmar.real.etc.b.esk6 = true;

	w0 = XWORD(unab.wAto);
	w1 = XWORD(unab.wPisPau);
	w2 = XWORD(unab.wTrsRtd);
	for (int n = 0; n < 2; n ++) {
		c_cdmar.real.etc.a |= (ConvHdcToCdcA((BYTE)(w0 & 0xf)) << ((n * 2) + 10));		// ato
		c_cdmar.real.svc.a |= (ConvHdcToCdcA((BYTE)((w1 >> 8) & 0xf)) << (n * 2));		// pau
		c_cdmar.real.svc.a |= (ConvHdcToCdcA((BYTE)(w1 & 0xf)) << ((n * 2) + 4));		// pis
		c_cdmar.real.svc.a |= (ConvHdcToCdcA((BYTE)((w2 >> 8) & 0xf)) << ((n * 2) + 8));	// rtd
		c_cdmar.real.svc.a |= (ConvHdcToCdcA((BYTE)(w2 & 0xf)) << ((n * 2) + 12));		// trs
		w0 >>= 4;
		w1 >>= 4;
		w2 >>= 4;
		c_cdmar.real.wExtTemp[n] = XWORD(unbb.wExtTemp[n]);
	}

	w0 = XWORD(unab.wTu);
	for (int n = 0; n < 4; n ++) {
		if ((w0 & 0xf) != LPS_NORMAL)	c_cdmar.real.tus.a |= (1 << n);
		w0 >>= 4;
	}
	dw0 = XDWORD(unab.dwCu);
	for (int n = 0; n < 8; n ++) {
		if ((dw0 & 0xf) != LPS_NORMAL)	c_cdmar.real.cCus |= (1 << n);		// 171120,	|= 1;
		dw0 >>= 4;
	}
	c_cdmar.real.cLength = pPaper->GetLength();
	c_cdmar.real.wCode = CDMA_VERIFYCODE;
	_TWATCHMEASURED();
	c_dwMakeCdmaTime = (DWORD)(__sec_ * 1e6);
}

BYTE CArchAux::ConvHdcToCdcA(BYTE ch)
{
	switch (ch & 0xf) {
	case LPS_FAIL :		return 3;	break;
	case LPS_LERR :		return 2;	break;
	case LPS_NORMAL :	return 1;	break;
	default :	break;
	}
	return 0;
}

BYTE CArchAux::ConvHdcToCdcB(BYTE ch)
{
	switch (ch & 0xf) {
	case LPS_BYPASS :	return 3;	break;
	case LPS_RUN :		return 2;	break;
	case LPS_STOP :		return 1;	break;
	default :	break;
	}
	return 0;
}

BYTE CArchAux::ConvHdcToCdcC(BYTE ch)
{
	switch (ch & 0xf) {
	case LPS_FAIL :		return 2;	break;
	case LPS_FIREF :	return 1;	break;
	default :	break;
	}
	return 0;
}

bool CArchAux::OpenAid()
{
	c_hMm = mq_open(MTQ_STR, O_WRONLY);
	if (!Validity(c_hMm)) {
		TRACK("ARCH>ERR:can not open aid queue!(%s)\n", strerror(errno));
		return false;
	}
	c_hShared = shm_open(BULK_STR, O_RDWR, 0777);
	if (!Validity(c_hShared)) {
		TRACK("ARCH>ERR:can not open shared memory!(%s)\n", strerror(errno));
		mq_close(c_hMm);
		c_hMm = INVALID_HANDLE;
		return false;
	}
	c_pBulk = (PBULKPACK)mmap(NULL, sizeof(BULKPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hShared, 0);
	if (c_pBulk == MAP_FAILED) {
		TRACK("ARCH>ERR:can not access shared memory!(%s)\n", strerror(errno));
		close(c_hShared);
		c_hShared = INVALID_HANDLE;
		mq_close(c_hMm);
		c_hMm = INVALID_HANDLE;
		return false;
	}
	c_pBulk->wState |= (1 << BULKSTATE_INTCMS);
	SetLogbookhead(false);
	return true;
}

// 로그 헤드도 저장할 때 사용하므로 BPID_PERIOD 쪽으로 한다.
// bFrom = true일 때 bool로 리턴이 되어야...
void CArchAux::SetLogbookhead(bool bFrom)
{
	if (c_pBulk == NULL)	return;

	memcpy(&c_pBulk->lbHead.title, TITLE_LOGBOOK, strlen(TITLE_LOGBOOK));
	c_pBulk->lbHead.wVer = LOGBOOK_VERSION;
	GETPAPER(pPaper);
	c_pBulk->lbHead.wPermNo = pPaper->GetPermNo();
	c_pBulk->lbHead.cLength = pPaper->GetLength();
	c_pBulk->lbHead.cRate = 5;

	CArch* pArch = (CArch*)c_pParent;
	CLand* pLand = (CLand*)pArch->GetCarrier();
	ASSERTP(pLand);
	c_pBulk->lbHead.cAux = (pLand->GetProperID() & 1) ? 1 : 0;
	//for (int n = 0; n < 14; n ++)	c_pBulk->lbHead.cDummy[n] = 0xff;

	bool bTimeSet = false;
	if (bFrom) {
		if (pPaper->GetRouteState(FID_HEAD)) {		// 180911, 날짜 맞추는 조건, 확인
			PDOZEN pDoz = pPaper->GetDozen();
			if (pDoz->recip[FID_HEAD].real.logMatch.t.year != 0 &&
				pDoz->recip[FID_HEAD].real.logMatch.t.mon > 0 && pDoz->recip[FID_HEAD].real.logMatch.t.mon < 13 &&
				pDoz->recip[FID_HEAD].real.logMatch.t.day > 0 && pDoz->recip[FID_HEAD].real.logMatch.t.day < 32 &&
				pDoz->recip[FID_HEAD].real.logMatch.t.hour < 24 && pDoz->recip[FID_HEAD].real.logMatch.t.min < 60 &&
				pDoz->recip[FID_HEAD].real.logMatch.t.sec < 60) {
				c_pBulk->lbHead.devt.year = pDoz->recip[FID_HEAD].real.logMatch.t.year + 10;
				c_pBulk->lbHead.devt.mon = pDoz->recip[FID_HEAD].real.logMatch.t.mon;
				c_pBulk->lbHead.devt.day = pDoz->recip[FID_HEAD].real.logMatch.t.day;
				c_pBulk->lbHead.devt.hour = pDoz->recip[FID_HEAD].real.logMatch.t.hour;
				c_pBulk->lbHead.devt.min = pDoz->recip[FID_HEAD].real.logMatch.t.min;
				c_pBulk->lbHead.devt.sec = pDoz->recip[FID_HEAD].real.logMatch.t.sec;
				TRACK("AUX:make logbook head %02d/%02d/%02d %02d:%02d:%02d\n",
						c_pBulk->lbHead.devt.year, c_pBulk->lbHead.devt.mon, c_pBulk->lbHead.devt.day,
						c_pBulk->lbHead.devt.hour, c_pBulk->lbHead.devt.min, c_pBulk->lbHead.devt.sec);
				bTimeSet = true;
				c_bRtdNewSheet = true;	// RTD에 로그북 헤드를 적용해야 된다는 것을 나타낸다...
			}
			else	TRACK("ARCH:logbook head fail cause time format!(%02d/%02d/%02d %02d:%02d:%02d)\n",
					pDoz->recip[FID_HEAD].real.logMatch.t.year, pDoz->recip[FID_HEAD].real.logMatch.t.mon,
					pDoz->recip[FID_HEAD].real.logMatch.t.day, pDoz->recip[FID_HEAD].real.logMatch.t.hour,
					pDoz->recip[FID_HEAD].real.logMatch.t.min, pDoz->recip[FID_HEAD].real.logMatch.t.sec);
		}
		else	TRACK("AUX:logbook head fail cause DTB head!\n");
	}

	if (!bTimeSet) {
		CArch* pArch = (CArch*)c_pParent;
		PDEVTIME pDevt = pArch->GetDevTime();
		memcpy(&c_pBulk->lbHead.devt, pDevt, sizeof(DEVTIME));
	}
	for (int n = 0; n < 7; n ++)	c_pBulk->lbHead.cDummy[n] = 0xff;
}

bool CArchAux::IsValidAid()
{
	// 180105
	if (!Validity(c_hMm)) {
		c_cMmState |= (1 << MMSTATE_INVALIDQUEUE);
		return false;
	}
	c_cMmState &= ~(1 << MMSTATE_INVALIDQUEUE);
	if (!Validity(c_hShared)) {
		c_cMmState |= (1 << MMSTATE_INVALIDSHAREMEM);
		return false;
	}
	c_cMmState &= ~(1 << MMSTATE_INVALIDSHAREMEM);
	if (c_pBulk == NULL) {
		c_cMmState |= (1 << MMSTATE_INVALIDMAPPING);
		return false;
	}
	c_cMmState &= ~(1 << MMSTATE_INVALIDMAPPING);
	if (!(c_pBulk->wState & (1 << BULKSTATE_LOGAREA)))
		c_cMmState |= (1 << MMSTATE_INVALIDLOGAREA);
	else	c_cMmState &= ~(1 << MMSTATE_INVALIDLOGAREA);

	return true;
	// 171206
	//GETPAPER(pPaper);
	//int cid = CAR_FSELF() < FID_TAIL ? CID_BOW : CID_STERN;
	//
	//CArch* pArch = (CArch*)c_pParent;
	//
	//if (!Validity(c_hMm)) {
	//	pArch->Shot(cid, 80);
	//	return false;
	//}
	//else	pArch->Cut(cid, 80);
	//
	//if (!Validity(c_hShared)) {
	//	pArch->Shot(cid, 81);
	//	return false;
	//}
	//else	pArch->Cut(cid, 81);
	//
	//if (c_pBulk == NULL) {
	//	pArch->Shot(cid, 82);
	//	return false;
	//}
	//else	pArch->Cut(cid, 82);
	//
	//return true;
}

void CArchAux::AidCmd(WORD wCmd)
{
	if (IsValidAid()) {
		c_wAidCmd = wCmd;
		struct mq_attr attr;
		if (Validity(mq_getattr(c_hMm, &attr))) {
			if (attr.mq_curmsgs < attr.mq_maxmsg) {
				AIDMSG msg;
				msg.wCmd = wCmd;
				mq_send(c_hMm, (char*)&msg, sizeof(AIDMSG), 0);
			}
			else	TRACK("AUX>ERR:aid queue full!\n");
		}
	}
}

// RTDSPADE 형식을 이용하여 저장된 운행 기록이나 시험 기록을 가져온다.
// AIDCMD_EXTRACTLOGBOOKENTRY, AIDCMD_EXTRACTINSPECTENTRY, AIDCMD_EXTRACTLOGBOOKLIST,
bool CArchAux::AidCmdOccasion(WORD cmd, DWORD dwIndex, DWORD dwTotal)
{
	if (IsValidAid()) {	// && dwTotal > 0) {
		c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex = dwIndex;
		c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal = dwTotal;
		c_pBulk->m[BPID_OCCASN].iResult = 0;
		if (cmd == AIDCMD_EXTRACTLOGBOOKENTRY || cmd == AIDCMD_EXTRACTINSPECTENTRY ||
			cmd == AIDCMD_EXTRACTLOGBOOKLIST/* || cmd == AIDCMD_EXTRACTOLDTROUBLE*/) {
			c_pBulk->m[BPID_OCCASN].bEnd = false;		// 10ms마다 scan을 하므로 이 항이 먼저 실행되어야 한다!!!
			c_pBulk->m[BPID_OCCASN].bBegin = true;
		}
		AidCmd(cmd);
		return true;
	}
	return false;
}

// RTDTYPIC 형식을 이용하여 운행 기록이나 고장 정보를 저장한다.
// AIDCMD_APPENDLOGBOOK, AIDCMD_APPENDTROUBLE, AIDCMD_SAVEINSPECT
bool CArchAux::AidCmdPeriod(WORD cmd, BYTE* p, WORD wLength)
{
	if (IsValidAid()) {
		memcpy(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], p, wLength);
		c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength = wLength;
		c_pBulk->m[BPID_PERIOD].iResult = 0;
		// 170731, PERIOD에서는 bBegin과 bEnd를 쓰지 않아야 한다.
		// 170804, 다시 쓴다..
		c_pBulk->m[BPID_PERIOD].bBegin = true;
		AidCmd(cmd);
		return true;
	}
	return false;
}

void CArchAux::Arteriam()
{
	if (c_pBulk != NULL) {
		c_wBulkState = c_pBulk->wState;
		if (c_pBulk->m[BPID_PERIOD].bBegin)	c_wBulkCmd |= 1;
		else	c_wBulkCmd &= ~1;
		if (c_pBulk->m[BPID_PERIOD].bEnd)	c_wBulkCmd |= 2;
		else	c_wBulkCmd &= ~2;
		if (c_pBulk->m[BPID_OCCASN].bBegin)	c_wBulkCmd |= 0x10;
		else	c_wBulkCmd &= ~0x10;
		if (c_pBulk->m[BPID_OCCASN].bEnd)	c_wBulkCmd |= 0x20;
		else	c_wBulkCmd &= ~0x20;
		memcpy(&c_fts, &c_pBulk->fts, sizeof(FILEATS));
		c_cEcuTraceLength = c_pBulk->cEcuTraceLength;
	}

	// 170708
	if (++ c_cMs10 >= 100) {
		c_cMs10 = 0;
		if (++ c_cmjog.real.cjt.srt.b.sec >= SECONDONEDAY) {
			c_cmjog.real.cjt.srt.b.sec = 0;
			++ c_cmjog.real.cjt.srt.b.day;
		}
		GETPAPER(pPaper);
		PDOZEN pDoz = pPaper->GetDozen();
		for (int n = 0; n < CM_MAX; n ++) {
			if (pDoz->recip[FID_HEAD].real.sign.scmd.a & (1 << n)) {
				if (++ c_cmjog.real.cjt.cm[n].b.sec >= SECONDONEDAY) {
					c_cmjog.real.cjt.cm[n].b.sec = 0;
					++ c_cmjog.real.cjt.cm[n].b.day;
				}
				if (c_cmjog.real.cjt.cm[n].b.day > c_cmjog.real.cjt.srt.b.day ||
					/*(c_cmjog.real.cjt.cm[n].b.day == c_cmjog.real.cjt.srt.b.day && */c_cmjog.real.cjt.cm[n].b.sec > c_cmjog.real.cjt.srt.b.sec) {
					c_cmjog.real.cjt.cm[n].b.day = c_cmjog.real.cjt.srt.b.day;
					c_cmjog.real.cjt.cm[n].b.sec = c_cmjog.real.cjt.srt.b.sec;
				}
			}
		}
	}
}

bool CArchAux::Launch()
{
	if (!Validity(c_nChannel = ChannelCreate(0))) {
		TRACK("AUX>ERR:can not channel create!(%s)\n", strerror(errno));
		return false;
	}

	if (!Validity(c_nConnect = ConnectAttach(ND_LOCAL_NODE, 0, c_nChannel, 0, 0))) {
		TRACK("AUX>ERR:can not connect attach!(%s)\n", strerror(errno));
		return false;
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("AUX>ERR:can not set inherit!(%s)\n", strerror(errno));
		return false;
	}

	struct sched_param param;
	param.sched_priority = PRIORITY_POST;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("AUX>ERR:can not set sch param!(%s)\n", strerror(errno));
		return false;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("AUX>ERR:can not set sch policy!(%s)\n", strerror(errno));
		return false;
	}

	res = pthread_create(&c_hThread, &attr, &CArchAux::TemporaryManagerEntry, (PVOID)this);
	if (!Validity(res)) {
		TRACK("AUX>ERR:can not create thread!(%s)\n", strerror(errno));
		return false;
	}
	return true;
}

int CArchAux::Bale(bool bType, BYTE* pMesh)
{
	int leng = 0;
	if (bType) {
		sprintf((char*)pMesh, "$CDM, %d,", sizeof(CDMAREPORT));
		while (*pMesh != '\0') {
			++ pMesh;
			++ leng;
		}
	}
	else {
		memcpy(pMesh, &c_cdmar, sizeof(CDMAREPORT));
		pMesh += sizeof(CDMAREPORT);
		leng += sizeof(CDMAREPORT);
	}
	return leng;
}

ENTRY_CONTAINER(CArchAux)
	SCOOP(&c_wAidCmd,				sizeof(WORD),		"Aux")
	SCOOP(&c_wBulkState,			sizeof(WORD),		"")
	SCOOP(&c_wBulkCmd,				sizeof(WORD),		"")
	SCOOP(&c_dwMakeCdmaTime,		sizeof(DWORD),		"mc")
	SCOOP(&c_careers.real.dbDistance, sizeof(double),	"DIST")		// BAS, SPD, DIST등은 대문자로 쓰고 Watcher에서 알아보도록 한다
	//SCOOP(&c_cmjog.sysrun.wDay,		sizeof(WORD),		"cmj")
	//SCOOP(&c_cmjog.sysrun.dwSec,	sizeof(DWORD),		"")
	//SCOOP(&c_cmjog.cm[0].wDay,		sizeof(WORD),		"")
	//SCOOP(&c_cmjog.cm[0].dwSec,		sizeof(DWORD),		"")
	//SCOOP(&c_cmjog.cm[1].wDay,		sizeof(WORD),		"")
	//SCOOP(&c_cmjog.cm[1].dwSec,		sizeof(DWORD),		"")
	//SCOOP(&c_cmjog.cm[2].wDay,		sizeof(WORD),		"")
	//SCOOP(&c_cmjog.cm[2].dwSec,		sizeof(DWORD),		"")
	SCOOP(&c_fts,					sizeof(FILEATS),	"fts")
	SCOOP(&c_cEcuTraceLength,		sizeof(BYTE),		"el")
EXIT_CONTAINER()
