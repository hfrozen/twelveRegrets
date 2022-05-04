/*
 * CScene.cpp
 *
 *  Created on: 2011. 2. 8
 *      Author: Che
 */

#include "CScene.h"
#include "ErrGrade.h"
#include "CTcx.h"

CScene::CScene()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pDoz = NULL;
	c_bInitial = FALSE;
	pthread_mutex_init(&c_mutex, NULL);
}

CScene::~CScene()
{
}

void CScene::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pDoz = pTcx->GetTcDoz();
	c_bInitial = TRUE;
	//pTcx->TimeRegister(&c_pTcRef->real.scArm.wLightTimer);
	//pTcx->TimeRegister(&c_pTcRef->real.scArm.wAlarmTimer);
	pTcx->TimeRegister(&c_pTcRef->real.scArm.wBuzzTimer);
	// Modified 2013/11/02
	pTcx->TimeRegister(&c_pTcRef->real.scArm.wScrollTimer);

	Clear();
}

BYTE CScene::BitIndex(WORD bits)
{
	BYTE n;
	for (n = 0; n < 15; n ++) {
		if (bits & (1 << n))	return n;
	}
	return n;
}

WORD CScene::BitByCid(BYTE cid)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (cid == SCENE_ID_HEADCAR)	cid = pTcx->GetHeadCarID();
	if (cid < 7)	++ cid;
	else	cid = 0;
	return ((WORD)(1 << cid));
}

void CScene::Buzz(WORD wTime)
{
	if ((c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) && (c_pTcRef->real.nObligation & OBLIGATION_HEAD)) {
		if (c_pTcRef->real.scArm.wBuzzTimer < wTime) {
			//c_pTcRef->real.scArm.wBuzzTimer = wTime;
			CTcx* pTcx = (CTcx*)c_pParent;
			pTcx->InterTrigger(&c_pTcRef->real.scArm.wBuzzTimer, wTime);
			c_pTcRef->real.scArm.nState |= ARMSTATE_BUZZON;
			// Modified 2012/12/25
			SETTBIT(OWN_SPACEA, TCDOB_DBZA);
		}
		// Modified 2012/12/25
		c_pTcRef->real.scArm.nState |= ARMSTATE_LAMPON;
		SETTBIT(OWN_SPACEA, TCDOB_FACKLP);
	}
}

// Need? 12/12/03
void CScene::ShutBuzz()
{
	//c_pTcRef->real.scArm.wBuzzTimer = c_pTcRef->real.scArm.wLightTimer = c_pTcRef->real.scArm.wAlarmTimer = 0;
	c_pTcRef->real.scArm.wBuzzTimer = 0;
	// Modified 2012/12/25
	//c_pTcRef->real.scArm.nState &= ~(ARMSTATE_LIGHTON | ARMSTATE_ALARMON | ARMSTATE_BUZZON);
	c_pTcRef->real.scArm.nEmergency = 0;
	c_pTcRef->real.scArm.nState &= ~(ARMSTATE_LIGHTON | ARMSTATE_ALARMON | ARMSTATE_BUZZON | ARMSTATE_LAMPON);
	CLRTBIT(OWN_SPACEA, TCDOB_DBZA);
	CLRTBIT(OWN_SPACEA, TCDOB_FACKLP);
}

void CScene::Revoke(BYTE ci, WORD wMsg, PDEVTIME pDt)
{
	//if (c_pTcRef->real.nProperty & PROPERTY_LOGMASK)	return;
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->RecordTime(TTIME_PRESENT);
	for (DWORD dw = 0; dw < c_pTcRef->real.scArm.dwIndex; dw ++) {
		if (!c_armArch[dw].b.used &&
				c_armArch[dw].b.code == wMsg && c_armArch[dw].b.cid == ci) {
			c_armArch[dw].b.type &= ~ALARMTYPE_HIDDEN;
			c_armArch[dw].b.d_hour = pTcx->ToHex((BYTE)pDt->hour);
			c_armArch[dw].b.d_min = pTcx->ToHex((BYTE)pDt->minute);
			c_armArch[dw].b.d_sec = pTcx->ToHex((BYTE)pDt->sec);
			c_armArch[dw].b.used = 1;
			// Modified 2012/12/25
			if ((c_armArch[dw].b.type & ALARMTYPE_ONLYCODE) == ALARMTYPE_EMERGENCY &&
					c_pTcRef->real.scArm.nEmergency > 0)
				-- c_pTcRef->real.scArm.nEmergency;
			// Modified 2013/01/28
			SetFilm(&c_armArch[dw]);
		}
	}
	if ((c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
			c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT)
		c_pTcRef->real.du.nTerm |= DUTERM_FEED;
	pTcx->RecordTime(TTIME_PRESENTED);
}

void CScene::Add(BYTE ci, WORD wMsg, BYTE nState, PDEVTIME pDt)
{
	//if (c_pTcRef->real.nProperty & PROPERTY_LOGMASK)	return;
	if (c_pTcRef->real.scArm.dwIndex < SIZE_ARMARCH) {
		CTcx* pTcx = (CTcx*)c_pParent;
		c_armArch[c_pTcRef->real.scArm.dwIndex].a = 0;
		//c_armArch[c_ai].b.used = 0;
		// Modified 2012/12/25
		//c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm = (nState == MSGTYPE_HEAVY || nState == MSGTYPE_LIGHT || nState == MSGTYPE_ALARM) ? 1 : 0;
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm =
			(nState == ALARMTYPE_EMERGENCY || nState == ALARMTYPE_SEVERE ||
					nState == ALARMTYPE_LIGHT || nState == ALARMTYPE_WARNING || nState == ALARMTYPE_TEXT) ? 1 : 0;
		// Modified 2012/01/22
		//c_armArch[c_pTcRef->real.scArm.dwIndex].b.type = (MSGTYPE_ADVANCE | (nState & MSGTYPE_ONLYCODE));
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.type = (nState & ALARMTYPE_ONLYCODE);
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.u_mon = pTcx->ToHex((BYTE)pDt->month);
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.u_day = pTcx->ToHex((BYTE)pDt->day);
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.u_hour = pTcx->ToHex((BYTE)pDt->hour);
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.u_min = pTcx->ToHex((BYTE)pDt->minute);
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.u_sec = pTcx->ToHex((BYTE)pDt->sec);
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.code = wMsg;
		c_armArch[c_pTcRef->real.scArm.dwIndex].b.cid = ci;
		// Modified 2013/01/28
		//if (c_pTcRef->real.nObligation & OBLIGATION_HEAD) {
		switch (nState) {
		// Modified 2012/12/25
		case ALARMTYPE_EMERGENCY :
			c_pTcRef->real.scArm.dwAlarm = c_pTcRef->real.scArm.dwIndex;
			c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm = TRUE;
			INCBYTE(c_pTcRef->real.scArm.nEmergency);
			c_pTcRef->real.scArm.nState |= (ARMSTATE_BUZZON | ARMSTATE_LAMPON);
			SETTBIT(OWN_SPACEA, TCDOB_DBZA);
			SETTBIT(OWN_SPACEA, TCDOB_FACKLP);
			break;
		case ALARMTYPE_SEVERE :
			c_pTcRef->real.scArm.dwAlarm = c_pTcRef->real.scArm.dwIndex;
			c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm = TRUE;
			Buzz(TIME_HEAVYBUZZ);
			break;
		case ALARMTYPE_LIGHT :
			c_pTcRef->real.scArm.dwAlarm = c_pTcRef->real.scArm.dwIndex;
			c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm = TRUE;
			// Modified 2012/12/25
			//if (c_pTcRef->real.scArm.wLightTimer < TIME_LIGHTON) {
			//	c_pTcRef->real.scArm.wLightTimer = TIME_LIGHTON;
			//	c_pTcRef->real.scArm.nState |= ARMSTATE_LIGHTON;
			//}
			Buzz(TIME_LIGHTBUZZ);
			break;
		case ALARMTYPE_WARNING :
			c_pTcRef->real.scArm.dwAlarm = c_pTcRef->real.scArm.dwIndex;
			c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm = TRUE;
			// Modified 2012/12/25
			//if (c_pTcRef->real.scArm.wAlarmTimer < TIME_ALARMON) {
			//	c_pTcRef->real.scArm.wAlarmTimer = TIME_ALARMON;
			//	c_pTcRef->real.scArm.nState |= ARMSTATE_ALARMON;
			//}
			// Modified 2012/12/25
			Buzz(TIME_ALARMBUZZ);
			break;
		// Modified 2013/11/02
		case ALARMTYPE_TEXT :
			c_pTcRef->real.scArm.dwAlarm = c_pTcRef->real.scArm.dwIndex;
			c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm = TRUE;
			break;
		case ALARMTYPE_STATE :
			c_pTcRef->real.scArm.dwSign = c_pTcRef->real.scArm.dwIndex;
			c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm = FALSE;
			break;
		default :
			c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm = FALSE;	// event
			break;
		}
		SetFilm(&c_armArch[c_pTcRef->real.scArm.dwIndex]);
		++ c_pTcRef->real.scArm.dwIndex;
		if (c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT)
			c_pTcRef->real.du.nTerm |= DUTERM_FEED;
	}
}

void CScene::SetFilm(PARM_ARCH pArm)
{
	memcpy((PVOID)&c_film.arm[c_film.wi], (PVOID)pArm, sizeof(ARM_ARCH));
	if (++ c_film.wi >= SIZE_FILM)	c_film.wi = 0;
}

PARM_ARCH CScene::GetFilm()
{
	if (c_film.wi == c_film.ri)	return NULL;
	PARM_ARCH pArm = &c_film.arm[c_film.ri];
	if (++ c_film.ri >= SIZE_FILM)	c_film.ri = 0;
	return pArm;
}

void CScene::Hide(BYTE nState)
{
	//if (c_pTcRef->real.nProperty & PROPERTY_LOGMASK)	return;
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->RecordTime(TTIME_PRESENT);
	// Modified 2013/01/22
	//nState |= MSGTYPE_ADVANCE;
	for (DWORD dw = 0; dw < c_pTcRef->real.scArm.dwIndex; dw ++) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.type == nState)
			c_armArch[dw].b.type |= ALARMTYPE_HIDDEN;
	}
}

void CScene::Hide()
{
	for (DWORD dw = 0; dw < c_pTcRef->real.scArm.dwIndex; dw ++) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.alarm && !(c_armArch[dw].b.type & ALARMTYPE_HIDDEN))
			c_armArch[dw].b.type |= ALARMTYPE_HIDDEN;
	}
}

// Modified 2013/11/02
void CScene::Hide(BYTE nCid, WORD wMsg)
{
	if (!c_bInitial)	return;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return;

	if (c_pTcRef->real.scArm.dwAlarm == 0)	return;
	Lock();
	for (DWORD dw = c_pTcRef->real.scArm.dwAlarm; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && !(c_armArch[dw].b.type & ALARMTYPE_HIDDEN) &&
				c_armArch[dw].b.cid == nCid && c_armArch[dw].b.code == wMsg) {
			c_armArch[dw].b.type |= ALARMTYPE_HIDDEN;
			break;
		}
	}
	Unlock();
}

void CScene::Cancel(PDEVTIME pDt)
{
	//if (c_pTcRef->real.nProperty & PROPERTY_LOGMASK)	return;
	CTcx* pTcx = (CTcx*)c_pParent;
	for (DWORD dw = 0; dw < c_pTcRef->real.scArm.dwIndex; dw ++) {
		if (!c_armArch[dw].b.used) {
			c_armArch[dw].b.type &= ~ALARMTYPE_HIDDEN;
			c_armArch[dw].b.d_hour = pTcx->ToHex((BYTE)pDt->hour);
			c_armArch[dw].b.d_min = pTcx->ToHex((BYTE)pDt->minute);
			c_armArch[dw].b.d_sec = pTcx->ToHex((BYTE)pDt->sec);
			c_armArch[dw].b.used = 1;
			// Modified 2012/12/25
			if ((c_armArch[dw].b.type & ALARMTYPE_ONLYCODE) == ALARMTYPE_EMERGENCY &&
					c_pTcRef->real.scArm.nEmergency > 0)
				-- c_pTcRef->real.scArm.nEmergency;
		}
	}
	if ((c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
			c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT)
		c_pTcRef->real.du.nTerm |= DUTERM_FEED;
}

void CScene::Cancel(DWORD dwIndex, PDEVTIME pDt)
{
	//if (c_pTcRef->real.nProperty & PROPERTY_LOGMASK)	return;
	CTcx* pTcx = (CTcx*)c_pParent;
	for (DWORD dw = 0; dw < dwIndex; dw ++) {
		if (!c_armArch[dw].b.used) {
			c_armArch[dw].b.type &= ~ALARMTYPE_HIDDEN;
			c_armArch[dw].b.d_hour = pTcx->ToHex((BYTE)pDt->hour);
			c_armArch[dw].b.d_min = pTcx->ToHex((BYTE)pDt->minute);
			c_armArch[dw].b.d_sec = pTcx->ToHex((BYTE)pDt->sec);
			c_armArch[dw].b.used = 1;
			// Modified 2012/12/25
			if ((c_armArch[dw].b.type & ALARMTYPE_ONLYCODE) == ALARMTYPE_EMERGENCY &&
					c_pTcRef->real.scArm.nEmergency > 0)
				-- c_pTcRef->real.scArm.nEmergency;
		}
	}
	if ((c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
			c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT)
		c_pTcRef->real.du.nTerm |= DUTERM_FEED;
}

void CScene::Clear()
{
	memset((PVOID)&c_wTable, 0, sizeof(WORD) * SIZE_SCENE);
	memset((PVOID)&c_pTcRef->real.scArm, 0, sizeof(SCENEARMARCH));
	memset((PVOID)&c_film, 0, sizeof(FILM));
	//memset((PVOID)&c_wAlarms, 0, sizeof(WORD) * SIZE_SCENE);
	//memset((PVOID)&c_wHeavies, 0, sizeof(WORD) * SIZE_SCENE);
}

void CScene::Shot(BYTE nCid, WORD wMsg, PDEVTIME pDt)
{
	if (!c_bInitial)	return;
	// Modified 2012/11/30
	//if (!(c_pTcRef->real.nObligation & OBLIGATION_MAIN))	return;
	// Modified 2013/01/28
	//if (!(c_pTcRef->real.nObligation & OBLIGATION_HEAD) && wMsg > 30)	return;
	if (OWN_SPACEA != 0)	return;
	if (wMsg >= SIZE_SCENE || nCid > SCENE_ID_HEADCAR)	return;
	// Modified 2012/11/12
	if (c_pTcRef->real.insp.nMode > INSPECTMODE_NON && c_pTcRef->real.insp.nMode < INSPECTMODE_VIEW)	return;

	WORD cf = BitByCid(nCid);
	BYTE nState = nErrGrade[wMsg];
	if (nState != ALARMTYPE_EVENT && (c_wTable[wMsg] & cf))	return;
	Lock();
	if (nState != ALARMTYPE_EVENT)	c_wTable[wMsg] |= cf;
	Add(BitIndex(cf), wMsg, nState, pDt);
	Unlock();
}

void CScene::Cut(BYTE nCid, WORD wBegin, WORD wEnd, PDEVTIME pDt)
{
	if (!c_bInitial)	return;
	Edit(nCid, wBegin, wEnd, 0, pDt);
}

void CScene::Edit(BYTE nCid, WORD wBegin, WORD wEnd, WORD wRest, PDEVTIME pDt)
{
	if (!c_bInitial)	return;
	// Modified 2012/11/30
	//if (!(c_pTcRef->real.nObligation & OBLIGATION_MAIN))	return;
	// Modified 2013/01/28
	//if (!(c_pTcRef->real.nObligation & OBLIGATION_HEAD) && wRest > 30)	return;
	if (OWN_SPACEA != 0)	return;
	if (wBegin >= SIZE_SCENE || wEnd >= SIZE_SCENE ||
			wRest >= SIZE_SCENE || nCid > SCENE_ID_ALLCAR)	return;
	// Modified 2012/11/12
	if (c_pTcRef->real.insp.nMode > INSPECTMODE_NON && c_pTcRef->real.insp.nMode < INSPECTMODE_VIEW)	return;

	WORD cf;
	if (nCid == SCENE_ID_ALLCAR)	cf = 0xff;
	else	cf = BitByCid(nCid);
	BOOL bEdit = FALSE;

	Lock();
	do {
		WORD ccf = cf;
		if (c_wTable[wBegin] & ccf) {
			if (wBegin == wRest) {
				Shot(BitIndex(ccf), wRest, pDt);
				bEdit = TRUE;
			}
			while (c_wTable[wBegin] & ccf) {
				BYTE ci = BitIndex(ccf);		// from B0 ~
				c_wTable[wBegin] &= ~(1 << ci);
				ccf &= ~(1 << ci);
				Revoke(ci, wBegin, pDt);
			}
		}
	} while (++ wBegin <= wEnd);
	Unlock();

	if (!bEdit && wRest != 0 && nCid < SCENE_ID_ALLCAR)	Shot(nCid, wRest, pDt);
}

void CScene::AidShot(PARM_ARCH pArm)
{
	if (pArm == NULL)	return;
	if (pArm->b.used) {
		for (DWORD dw = 0; dw < c_pTcRef->real.scArm.dwIndex; dw ++) {
			if (!c_armArch[dw].b.used &&
					c_armArch[dw].b.code == pArm->b.code && c_armArch[dw].b.cid == pArm->b.cid) {
				c_armArch[dw].b.d_hour = pArm->b.d_hour;
				c_armArch[dw].b.d_min = pArm->b.d_min;
				c_armArch[dw].b.d_sec = pArm->b.d_sec;
				c_armArch[dw].b.used = 1;
			}
		}
	}
	else {
		if (c_pTcRef->real.scArm.dwIndex < SIZE_ARMARCH) {
			memcpy((PVOID)&c_armArch[c_pTcRef->real.scArm.dwIndex], (PVOID)pArm, sizeof(ARM_ARCH));
			if (c_armArch[c_pTcRef->real.scArm.dwIndex].b.alarm)	c_pTcRef->real.scArm.dwAlarm = c_pTcRef->real.scArm.dwIndex;
			else	c_pTcRef->real.scArm.dwSign = c_pTcRef->real.scArm.dwIndex;
			++ c_pTcRef->real.scArm.dwIndex;
		}
	}
	if (c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT)
		c_pTcRef->real.du.nTerm |= DUTERM_FEED;
}

BOOL CScene::CheckShot(BYTE nCid, WORD wMsg)
{
	if (!c_bInitial)	return FALSE;
	if (wMsg >= SIZE_SCENE || nCid > SCENE_ID_HEADCAR)	return FALSE;
	WORD ci = nCid;
	CTcx* pTcx = (CTcx*)c_pParent;
	if (ci == SCENE_ID_HEADCAR)	ci = pTcx->GetHeadCarID();
	if (ci < 7)	++ ci;
	else	ci = 0;
	ci = 1 << ci;
	if (c_wTable[wMsg] & ci)	return TRUE;
	return FALSE;
}

WORD CScene::GetSignCode()
{
	if (!c_bInitial)	return 0;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return 0;

	// Modified 2012/12/25
	if (c_pTcRef->real.scArm.dwSign == 0)	return 0;
	Lock();
	for (DWORD dw = c_pTcRef->real.scArm.dwSign; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.code != 0) {
			BYTE pick = c_armArch[dw].b.type & ALARMTYPE_VALIDCODE;
			if (pick == ALARMTYPE_STATE) {
				c_pTcRef->real.scArm.dwSign = dw;
				Unlock();
				return ((WORD)c_armArch[dw].b.code << 4) | (WORD)c_armArch[dw].b.cid;
			}
		}
	}
	Unlock();
	// Modified 2012/12/25
	c_pTcRef->real.scArm.dwSign = 0;
	return 0;
}

WORD CScene::GetAlarmCode()		// not use
{
	if (!c_bInitial)	return 0;
	// Modified 2013/02/18
	//if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return 0;

	// Modified 2012/12/25
	if (c_pTcRef->real.scArm.dwAlarm == 0)	return 0;
	Lock();
	for (DWORD dw = c_pTcRef->real.scArm.dwAlarm; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.code != 0) {
			BYTE pick = c_armArch[dw].b.type & ALARMTYPE_VALIDCODE;
			if (pick == ALARMTYPE_EMERGENCY || pick == ALARMTYPE_SEVERE ||
					pick == ALARMTYPE_LIGHT || pick == ALARMTYPE_WARNING || pick == ALARMTYPE_TEXT) {
				c_pTcRef->real.scArm.dwAlarm = dw;
				Unlock();
				return ((WORD)c_armArch[dw].b.code << 4) | (WORD)c_armArch[dw].b.cid;
			}
		}
	}
	// Modified 2012/12/25
	c_pTcRef->real.scArm.dwAlarm = 0;
	Unlock();
	return 0;
}

WORD CScene::GetActiveAlarmCode()
{
	if (!c_bInitial)	return 0;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return 0;

	if (c_pTcRef->real.scArm.dwAlarm == 0)	return 0;
	Lock();
	for (DWORD dw = c_pTcRef->real.scArm.dwAlarm; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.code != 0) {
			BYTE pick = c_armArch[dw].b.type & ALARMTYPE_VALIDCODE;
			if (pick == ALARMTYPE_EMERGENCY || pick == ALARMTYPE_SEVERE ||
					pick == ALARMTYPE_LIGHT || pick == ALARMTYPE_WARNING || pick == ALARMTYPE_TEXT) {
				c_pTcRef->real.scArm.dwAlarm = dw;
				Unlock();
				return ((WORD)c_armArch[dw].b.code << 4) | (WORD)c_armArch[dw].b.cid;
			}
		}
	}
	// Modified 2012/12/25
	c_pTcRef->real.scArm.dwAlarm = 0;
	Unlock();
	return 0;
}

// Appended 2013/11/02
WORD CScene::GetActiveAlarmCode(DWORD dwIndex)
{
	if (!c_bInitial)	return 0;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return 0;

	if (c_pTcRef->real.scArm.dwAlarm == 0)	return 0;
	DWORD dwDesc, dwLast;
	dwDesc = dwLast = 0;
	Lock();
	for (DWORD dw = c_pTcRef->real.scArm.dwAlarm; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.code != 0) {
			BYTE pick = c_armArch[dw].b.type & ALARMTYPE_VALIDCODE;
			if (pick == ALARMTYPE_EMERGENCY || pick == ALARMTYPE_SEVERE ||
					pick == ALARMTYPE_LIGHT || pick == ALARMTYPE_WARNING || pick == ALARMTYPE_TEXT) {
				if (dwDesc == 0)	c_pTcRef->real.scArm.dwAlarm = dw;
				if (dwDesc == dwIndex) {
					Unlock();
					return ((WORD)c_armArch[dw].b.code << 4) | (WORD)c_armArch[dw].b.cid;
				}
				++ dwDesc;
				dwLast = dw;
			}
		}
	}
	Unlock();
	if (dwDesc == 0) {
		c_pTcRef->real.scArm.dwAlarm = 0;
		return 0;
	}
	return ((WORD)c_armArch[dwLast].b.code << 4) | (WORD)c_armArch[dwLast].b.cid;
}

// Modified 2013/11/02
WORD CScene::GetSevereCode()
{
	if (!c_bInitial)	return 0;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return 0;

	if (c_pTcRef->real.scArm.dwAlarm == 0)	return 0;
	Lock();
	for (DWORD dw = c_pTcRef->real.scArm.dwAlarm; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.code != 0) {
			BYTE pick = c_armArch[dw].b.type & ALARMTYPE_VALIDCODE;
			if (pick == ALARMTYPE_EMERGENCY || pick == ALARMTYPE_SEVERE) {
				Unlock();
				return ((WORD)c_armArch[dw].b.code << 4) | (WORD)c_armArch[dw].b.cid;
			}
		}
	}
	Unlock();
	return 0;
}

DWORD CScene::GetSevereLength()
{
	if (!c_bInitial)	return 0;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return 0;

	if (c_pTcRef->real.scArm.dwAlarm == 0)	return 0;
	DWORD length = 0;
	for (DWORD dw = c_pTcRef->real.scArm.dwAlarm; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.code != 0) {
			BYTE pick = c_armArch[dw].b.type & ALARMTYPE_VALIDCODE;
			if (pick == ALARMTYPE_EMERGENCY || pick == ALARMTYPE_SEVERE)	++ length;
		}
	}
	return length;
}

DWORD CScene::GetAlarmLength()
{
	if (!c_bInitial)	return 0;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return 0;

	if (c_pTcRef->real.scArm.dwAlarm == 0)	return 0;
	DWORD length = 0;
	for (DWORD dw = c_pTcRef->real.scArm.dwAlarm; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.code != 0) {
			BYTE pick = c_armArch[dw].b.type & ALARMTYPE_VALIDCODE;
			if (pick == ALARMTYPE_EMERGENCY || pick == ALARMTYPE_SEVERE ||
					pick == ALARMTYPE_LIGHT || pick == ALARMTYPE_WARNING || pick == ALARMTYPE_TEXT)	++ length;
		}
	}
	return length;
}

DWORD CScene::GetBellLength()
{
	if (!c_bInitial)	return 0;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return 0;

	if (c_pTcRef->real.scArm.dwAlarm == 0)	return 0;
	DWORD length = 0;
	for (DWORD dw = c_pTcRef->real.scArm.dwAlarm; dw > 0; dw --) {
		if (!c_armArch[dw].b.used && c_armArch[dw].b.code != 0) {
			BYTE pick = c_armArch[dw].b.type & ALARMTYPE_VALIDCODE;
			if (pick == ALARMTYPE_EMERGENCY || pick == ALARMTYPE_SEVERE ||
					pick == ALARMTYPE_LIGHT || pick == ALARMTYPE_WARNING)	++ length;
		}
	}
	return length;
}

WORD CScene::OpenAlarm(WORD page, BYTE nCid)
{
	Lock();
	WORD all = 0;
	memset((PVOID)&c_armList, 0, sizeof(ARM_ARCH) * ALARMLIST_LINES);
	if (c_pTcRef->real.scArm.dwIndex > 0) {
		WORD line = 0;
		for (DWORD dn = 0; dn < c_pTcRef->real.scArm.dwIndex; dn ++) {
			DWORD ai = c_pTcRef->real.scArm.dwIndex - dn - 1;
			if (!c_armArch[ai].b.used && c_armArch[ai].b.alarm == 1 && (nCid > 7 || nCid == c_armArch[ai].b.cid)) {
				if (all == (DWORD)page)
					memcpy((PVOID)&c_armList[line], (PVOID)&c_armArch[ai], sizeof(ARM_ARCH));
				if (++ line >= ALARMLIST_LINES) {
					line = 0;
					++ all;
				}
			}
		}
	}
	Unlock();
	return all + 1;
}

PARM_ARCH CScene::GetAlarmList()
{
	if (c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT)	return c_armList;
	return c_file.GetAlarmList();
}

void CScene::Letter(PDEVTIME pDt)
{
	if (!c_bInitial)	return;
	// Modified 2012/11/30
	//if (!(c_pTcRef->real.nObligation & OBLIGATION_HEAD) ||
	//		!(c_pTcRef->real.nObligation & OBLIGATION_MAIN))	return;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_HEAD))	return;
	if (c_pTcRef->real.nProperty & PROPERTY_LOGMASK)	return;

	if (c_pTcRef->real.scLog.dwIndex < SIZE_DRVARCH) {
		DRV_ARCH drv;
		memset((PVOID)&drv, 0, sizeof(DRV_ARCH));

		CTcx* pTcx = (CTcx*)c_pParent;
		PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
		PATOAINFO paAto = pTcx->GetAto(FALSE);
		// 01
		drv.wId = (WORD)(c_pTcRef->real.scLog.dwIndex & 0xffff);
		// 02
		drv.wFno = MAKEWORD(c_pDoz->tcs[OWN_SPACEA].real.nThumbWheel[1],
								c_pDoz->tcs[OWN_SPACEA].real.nThumbWheel[2]);
		// 03
		drv.wTno = paAto != NULL ? (WORD)(paAto->d.st.wTtn) : 0;
		// 04
		drv.nLength = c_pDoz->tcs[OWN_SPACEA].real.nThumbWheel[0];
		// 05
		drv.nCno = c_pDoz->tcs[OWN_SPACEA].real.nThumbWheel[1] & 0xf;
		BYTE ntc = drv.nCno != 0 ? __SIVA : __SIVB;
		// 06
		drv.tm.year = pTcx->ToHex(pDt->year);
		drv.tm.mon = pTcx->ToHex(pDt->month);
		drv.tm.day = pTcx->ToHex(pDt->day);
		drv.tm.hour = pTcx->ToHex(pDt->hour);
		drv.tm.min = pTcx->ToHex(pDt->minute);
		drv.tm.sec = pTcx->ToHex(pDt->sec);
		// 7-8
		for (UCURV n = 0; n < 4; n ++) {
			SHORT tcid = pTcx->GetPscIDByAddr(n);
			if (tcid >= 0) {
				for (UCURV m = 0; m < 8; m ++)
					drv.nTci[n][m] = c_pDoz->tcs[n].real.nInput[m];
				for (UCURV m = 0; m < 6; m ++)
					drv.nTco[n][m] = c_pDoz->tcs[n].real.nOutput[m];
			}
		}
		// 9-10-38
		BYTE sid, vid;
		sid = vid = 0;
		for (UCURV n = 0; n < 8; n ++) {
			for (UCURV m = 0; m < 6; m ++) {
				drv.nCci[n][m] = c_pDoz->ccs[n].real.nInput[m];
				if (m < 4)
					drv.nCco[n][m] = c_pDoz->ccs[n].real.nOutput[m];
			}
			drv.wWeightRate[n] = (WORD)(pTcx->GetPassengerWeightRate(n) * 10.f);
			// Modified 2012/12/10
			WORD deves = pTcx->GetCarDevices(n);
			if (deves & DEVBEEN_SIV) {
				drv.nHpbpu[sid] = c_pDoz->ccs[n].real.eSiv.d.st.st.nHpbpu;
				drv.nHpbpv[sid] = c_pDoz->ccs[n].real.eSiv.d.st.st.nHpbpv;
				drv.nHpbpw[sid ++] = c_pDoz->ccs[n].real.eSiv.d.st.st.nHpbpw;
			}
			if (deves & DEVBEEN_V3F) {
				drv.nIq[vid] = c_pDoz->ccs[n].real.eV3f.d.st.st.nIq;
				drv.nMf[vid] = c_pDoz->ccs[n].real.eV3f.d.st.st.nMf;
				drv.nBp[vid] = c_pDoz->ccs[n].real.eV3f.d.st.st.nBp;
				drv.nBfc[vid ++] = c_pDoz->ccs[n].real.eV3f.d.st.st.nBfc;
			}
			drv.nAsp[n] = c_pDoz->ccs[n].real.aEcu.d.st.nAsp;
			drv.nBcp[n] = c_pDoz->ccs[n].real.aEcu.d.st.nBcp;
			drv.nMrp[n] = c_pDoz->ccs[n].real.aEcu.d.st.nMrp;
		}
		// 11
		drv.mode.a = paAtc != NULL ? paAtc->d.st.mode.a : 0;
		drv.mode.b.emer = c_pTcRef->real.op.nMode == OPMODE_EMERGENCY ? TRUE : FALSE;
		// 12
		drv.req.a = paAtc != NULL ? paAtc->d.st.req.a : 0;
		if (c_pTcRef->real.ms.wBrake & MSTATE_HOLDINGBRAKE)
			drv.req.b.ahbr = TRUE;
		if (paAtc != NULL)
			drv.req.b.bae = paAtc->d.st.fail.b.bae ? TRUE : FALSE;
		else	drv.req.b.bae = FALSE;
		// 13
		drv.sph.a = paAtc != NULL ? paAtc->d.st.sph.a : 0;
		if (paAto != NULL && paAto->nFlow == LOCALFLOW_NORMAL)
			drv.sph.b.mwb = paAto->d.st.vPwbB > 0 ? TRUE : FALSE;
		// 14
		drv.spl.a = paAtc != NULL ? paAtc->d.st.spl.a : 0;
		if (paAto != NULL && paAto->nFlow == LOCALFLOW_NORMAL)
			drv.spl.b.ator = paAto->d.st.state.b.ator ? TRUE : FALSE;
		// 15
		drv.oper.a = 0;
		// Modified 2011/12/29 ... begin
		//if ((c_pTcRef->real.op.nMode == OPMODE_AUTO ||
		//		c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS) &&
		//		// not need this line, because can not inspect in auto mode
		//		paAto->nFlow == LOCALFLOW_NORMAL) {
		//	if (paAto->d.st.vTbeB > 0)	drv.oper.b.p = TRUE;
		//	else if (paAto->d.st.vTbeB < 0)	drv.oper.b.b = TRUE;
		//}
		//else {
		//	drv.oper.b.p = GETTBIT(OWN_SPACEA, TCDIB_P) ? TRUE : FALSE;
		//	drv.oper.b.b = GETTBIT(OWN_SPACEA, TCDIB_B) ? TRUE : FALSE;
		//}
		//drv.oper.b.f = GETTBIT(OWN_SPACEA, TCDIB_F) ? TRUE : FALSE;
		//drv.oper.b.r = GETTBIT(OWN_SPACEA, TCDIB_R) ? TRUE : FALSE;
		drv.oper.b.f = c_pTcRef->real.power.inst.b.f;
		drv.oper.b.r = c_pTcRef->real.power.inst.b.r;
		drv.oper.b.p = c_pTcRef->real.power.inst.b.p;
		drv.oper.b.b = c_pTcRef->real.power.inst.b.b;
		// ... end
		drv.oper.b.hcr = FBTBIT(OWN_SPACEA, TCDOB_HCR) ? TRUE : FALSE;
		drv.oper.b.ebm = GETTBIT(OWN_SPACEA, TCDIB_EBRM) ? TRUE : FALSE;
		drv.oper.b.ros = GETTBIT(OWN_SPACEA, TCDIB_ROS) ? TRUE : FALSE;
		drv.oper.b.sbs = GETTBIT(OWN_SPACEA, TCDIB_SBS) ? TRUE : FALSE;
		// 16
		drv.stat.a = 0;
		drv.stat.b.ebcos = GETTBIT(OWN_SPACEA, TCDIB_EBCOS) ? TRUE : FALSE;
		drv.stat.b.cprs = GETTBIT(OWN_SPACEA, TCDIB_CPRLS) ? TRUE : FALSE;
		drv.stat.b.ebs = GETTBIT(OWN_SPACEA, TCDIB_EBS) ? TRUE : FALSE;
		drv.stat.b.dbz = FBTBIT(OWN_SPACEA, TCDOB_DBZA) ? TRUE : FALSE;
		drv.stat.b.mrps = GETCBIT(ntc, CCDIB_MRPS) ? TRUE : FALSE;
		drv.stat.b.ebrs = GETTBIT(OWN_SPACEA, TCDIB_EBBR) ? TRUE : FALSE;
		// 17
		drv.door.b.dir = GETTBIT(OWN_SPACEA, TCDIB_ADSCR) ? TRUE : FALSE;
		drv.door.b.adsbs = GETTBIT(OWN_SPACEA, TCDIB_ADSBS) ? TRUE : FALSE;
		drv.door.b.dosl = GETTBIT(OWN_SPACEA, TCDIB_DOSL) ? TRUE : FALSE;
		drv.door.b.dosr = GETTBIT(OWN_SPACEA, TCDIB_DOSR) ? TRUE : FALSE;
		drv.door.b.ldos = GETTBIT(OWN_SPACEA, TCDIB_LDOS) ? TRUE : FALSE;
		drv.door.b.rdos = GETTBIT(OWN_SPACEA, TCDIB_RDOS) ? TRUE : FALSE;
		drv.door.b.dcs = GETTBIT(OWN_SPACEA, TCDIB_DCS) ? TRUE : FALSE;
		drv.door.b.lrdcs = GETTBIT(OWN_SPACEA, TCDIB_LRDCS) ? TRUE : FALSE;
		// 18
		// Modified 2012/01/17 ... begin
		//drv.nNull = 0;
		drv.cmsb.a = 0;
		if (_ISOCC(__CMSBA)) {
			drv.cmsb.b.invona = c_pDoz->ccs[__CMSBA].real.aCmsb.state.b.invon;
			drv.cmsb.b.timera = c_pTcRef->real.op.wCmsbTimer[0] != 0 ? TRUE : FALSE;
			drv.cmsb.b.powera = c_pTcRef->real.op.nCmsbMonitor & CMSBMON_POWERA ? TRUE : FALSE;
			drv.cmsb.b.readya = c_pTcRef->real.op.nCmsbMonitor & CMSBMON_READYA ? TRUE : FALSE;
		}
		if (_ISOCC(__CMSBB)) {
			drv.cmsb.b.invonb = c_pDoz->ccs[__CMSBB].real.aCmsb.state.b.invon;
			drv.cmsb.b.timerb = c_pTcRef->real.op.wCmsbTimer[1] != 0 ? TRUE : FALSE;
			drv.cmsb.b.powerb = c_pTcRef->real.op.nCmsbMonitor & CMSBMON_POWERB ? TRUE : FALSE;
			drv.cmsb.b.readyb = c_pTcRef->real.op.nCmsbMonitor & CMSBMON_READYB ? TRUE : FALSE;
		}
		// ... end
		// 19
		drv.wSivV[0] = pTcx->Conv(c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.nEd, 0, 250, (WORD)0, (WORD)3000);
		drv.wSivV[1] = pTcx->Conv(c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.nEd, 0, 250, (WORD)0, (WORD)3000);
		// insert
		DWORD dwFairDist;
		double fairSpeed;
		pTcx->GetDistanceFair(&dwFairDist, &fairSpeed);
		// 20
		drv.dwFairDist = dwFairDist;
		// 21
		drv.dwFairSpeed = (DWORD)fairSpeed;
		// 22
		drv.wDistHcr = (WORD)(c_pTcRef->real.intg.dbDistLive / 100.f);	// 100m unit
		// 23
		PENVARCHEXTENSION pEnv = pTcx->GetEnv();
		drv.dwDist = (DWORD)(pEnv->real.entire.elem.dbDist / 1000.f);
		// 24
		drv.wDist = (WORD)((DWORD)pEnv->real.entire.elem.dbDist % 1000);
		// 39-40-41
		drv.wSivPowerHcr = (WORD)(c_pTcRef->real.intg.dbSivPowerLive / 1000.f);
		drv.dwSivPower = (DWORD)(pEnv->real.entire.elem.dbSivPower / 1000.f);
		drv.wSivPower = (DWORD)((DWORD)pEnv->real.entire.elem.dbSivPower % 1000);
		// 25
		drv.wV3fPowerHcr = (WORD)(c_pTcRef->real.intg.dbV3fPowerLive / 1000.f);
		drv.dwV3fPower = (DWORD)(pEnv->real.entire.elem.dbV3fPower / 1000.f);
		drv.wV3fPower = (DWORD)((DWORD)pEnv->real.entire.elem.dbV3fPower % 1000);

		//drv.wPowerHcr = (WORD)(c_pTcRef->real.intg.dbPowerLive / 1000.f);
		// 26
		//drv.dwPower = (DWORD)(pEnv->real.entire.elem.dbPower / 1000.f);
		// 17
		//drv.wPower = (WORD)((DWORD)pEnv->real.entire.elem.dbPower % 1000);
		// 28
		drv.dwRevival = (DWORD)(pEnv->real.entire.elem.dbRevival / 1000.f);
		// 29
		drv.wRevival = (WORD)((DWORD)pEnv->real.entire.elem.dbRevival % 1000);
		// 30
		drv.wMrp = pTcx->Conv(c_pDoz->ccs[ntc].real.aEcu.d.st.nMrp, 0, 0x64, (WORD)0, (WORD)10000);
		// 31
		// Modified 2011/12/29 ... begin
		//if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY)	drv.dwSpeed = c_pTcRef->real.speed.dwV3f;
		//else	drv.dwSpeed = c_pTcRef->real.speed.dwAtc;
		drv.dwSpeed = c_pTcRef->real.speed.dwV3fs;
		// ... end
		// 32
		drv.wDistSect = c_pDoz->tcs[OWN_SPACEA].real.rCmm.wStsd;
		// 33
		drv.nCurSt = c_pDoz->tcs[OWN_SPACEA].real.rCmm.nPrst;
		// 34
		drv.nNextSt = c_pDoz->tcs[OWN_SPACEA].real.rCmm.nNext;
		// 35
		drv.vTbeAto = c_pTcRef->real.atoCmd.vTbeB;
		// 36
		drv.vTbeMc = c_pTcRef->real.mcDrv.inst.vTbeB;
		// 37
		drv.vTbeB = GETLWORD(c_pTcRef->real.power.vTbeL);

		//
		if (paAtc != NULL) {
			drv.failCE = paAtc->d.st.fail.a;	// 43
			drv.errCE = paAtc->d.st.err.a;		// 44
			drv.modeCE = paAtc->d.st.mode.a;	// 45
			drv.reqCE = paAtc->d.st.req.a;		// 46
			drv.sphCE = paAtc->d.st.sph.a;		// 47
			drv.splCE = paAtc->d.st.spl.a;		// 48
			drv.posCE = paAtc->d.st.pos.a;		// 49
			drv.swiCE = paAtc->d.st.swi.a;		// 50
		}
		if (paAto != NULL) {
			drv.infoOE = paAto->d.st.info.a;	// 51
			drv.armOE = paAto->d.st.arm.a;		// 52
			drv.stateOE = paAto->d.st.state.a;	// 53
			drv.failOE = paAto->d.st.fail.a;	// 54
			drv.psmOE = paAto->d.st.psm.a;		// 55
			drv.pssOE = paAto->d.st.pss.a;		// 56
		}

		pTcx->CopyEachDevError(&drv.nBusFaultCounter[0]);	// 42
		// Modified 2013/02/20
		drv.wBrake = c_pTcRef->real.ms.wBrake;
		// Modified 2013/03/11
		drv.nIssue = c_pDoz->tcs[OWN_SPACEA].real.nIssue;
		drv.nSlaveCtrl = c_pDoz->tcs[OWN_SPACEA].real.nSlaveCtrl;

		pTcx->SaveDrv(&drv);
		//c_file.AppendPower(&pwr);
		//c_file.AppendDist(&dist);
		++ c_pTcRef->real.scLog.dwIndex;
	}
}

void CScene::Tag(PDEVTIME pDt)
{
	if (!c_bInitial)	return;
	// Modified 2012/11/30
	//if (!(c_pTcRef->real.nObligation & OBLIGATION_HEAD) ||
	//		!(c_pTcRef->real.nObligation & OBLIGATION_MAIN))	return;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_HEAD))	return;

	TRIAL_ARCH trial;
	CTcx* pTcx = (CTcx*)c_pParent;
	// 01
	trial.wId = (WORD)(c_pTcRef->real.scLog.dwTag & 0xffff);
	// 02
	trial.tm.year = pTcx->ToHex(pDt->year);
	trial.tm.mon = pTcx->ToHex(pDt->month);
	trial.tm.day = pTcx->ToHex(pDt->day);
	trial.tm.hour = pTcx->ToHex(pDt->hour);
	trial.tm.min = pTcx->ToHex(pDt->minute);
	trial.tm.sec = pTcx->ToHex(pDt->sec);
	// 03
	trial.vTbeB = GETLWORD(c_pTcRef->real.power.vTbeL);
	// 04
	PTRIALINFO pTrial = pTcx->GetTrial();
	trial.vAccel = pTrial->vAccel;
	// 05
	trial.dwSpeed = pTrial->dwSpeed;
	// 06
	trial.dwDist = pTrial->dwDist;
	// 07-11
	UCURV vi = 0;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		WORD dev = pTcx->GetCarDevices(n);
		if (dev & DEVBEEN_V3F) {
			trial.nTrqRef[vi] = c_pDoz->ccs[n].real.eV3f.d.st.st.nTrqRef;
			trial.nIq[vi] = c_pDoz->ccs[n].real.eV3f.d.st.st.nIq;
			trial.nIm[vi] = c_pDoz->ccs[n].real.eV3f.d.st.st.nIm;
			++ vi;
		}
		trial.nAsp[n] = c_pDoz->ccs[n].real.aEcu.d.st.nAsp;
		trial.nBcp[n] = c_pDoz->ccs[n].real.aEcu.d.st.nBcp;
	}
	// 12
	trial.wPtime = pTrial->pwr.t.wCurrent;
	// 13
	trial.wPdist = pTrial->pwr.d.wCurrent;
	// 14
	trial.wBtime = pTrial->brk.t.wCurrent;
	// 15
	trial.wBdist = pTrial->brk.d.wCurrent;
	// 16
	trial.dwSampleSpeed = pTrial->dwSampleSpeed;
	// 17
	//trial.wSampleLength = pTrial->wSampleIndex;

	c_file.AppendTrial(&trial);
	++ c_pTcRef->real.scLog.dwTag;
}

void CScene::ResetAlarmArch()
{
	if (!c_bInitial)	return;
	Clear();
	//Shot(SCENE_ID_HEADER, 1, MSGTYPE_EVENT);
}

void CScene::ResetDrvArch()
{
	if (!c_bInitial)	return;
	c_pTcRef->real.scLog.dwIndex = 0;
	c_file.DeleteDrv();
}
