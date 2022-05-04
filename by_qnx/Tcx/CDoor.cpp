/*
 * CDoor.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include "CDoor.h"
#include "CTcx.h"

#define StampDir(cmd)\
	do {\
			if (cmd == DOORCMD_RIGHT)\
				c_pTcRef->real.door.nState |= DOORSTATE_RIGHT;\
			else	c_pTcRef->real.door.nState &= ~DOORSTATE_RIGHT;\
	} while(0)

#define	IsRight()		(c_pTcRef->real.door.nState & DOORSTATE_RIGHT)
#define	TimerTrigger(time)	pTcx->InterTrigger(IsRight() ? &c_pTcRef->real.door.wTimerR : &c_pTcRef->real.door.wTimerL, time)
#define	IsLeftOpenSw()	(GETTBIT(OWN_SPACEA, TCDIB_DOSL) || GETTBIT(OWN_SPACEA, TCDIB_LDOS))
#define	IsRightOpenSw()	(GETTBIT(OWN_SPACEA, TCDIB_DOSR) || GETTBIT(OWN_SPACEA, TCDIB_RDOS))
#define	IsOpenSw()\
	(GETTBIT(OWN_SPACEA, TCDIB_DOSL) ||	GETTBIT(OWN_SPACEA, TCDIB_DOSR) ||\
	GETTBIT(OWN_SPACEA, TCDIB_LDOS) ||	GETTBIT(OWN_SPACEA, TCDIB_RDOS))
#define	IsReopenSw()	(GETTBIT(OWN_SPACEA, TCDIB_DROS) || GETTBIT(OWN_SPACEA, TCDIB_LRDROS))
#define	IsCloseSw()		(GETTBIT(OWN_SPACEA, TCDIB_DCS) || GETTBIT(OWN_SPACEA, TCDIB_LRDCS))

CDoor::CDoor()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pDoz = NULL;
}

CDoor::~CDoor()
{
}

void CDoor::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pDoz = pTcx->GetTcDoz();
}

// Modified 2012/11/12 ... begin
WORD CDoor::XchAtZero(BYTE nCid, WORD wMsg)
{
	if (nCid != (c_pTcRef->real.cf.nLength - 1))	return wMsg;
	if (wMsg >= 703 && wMsg <= 710)
		return (((wMsg - 703) ^ 7) + 703);
	else if (wMsg >= 713 && wMsg <= 720)
		return (((wMsg - 713) ^ 7) + 713);
	else if (wMsg >= 729 && wMsg <= 736)
		return (((wMsg - 729) ^ 7) + 729);
	else if (wMsg >= 756 && wMsg <= 899) {
		WORD base = ((wMsg - 756) & 0xfff8) + 756;	// x/8*8
		return (((wMsg - base) ^ 7) + base);
	}
	return wMsg;
}

// Modified 2012/01/16 ... begin
void CDoor::ShotL(BYTE nCid, WORD wMsg)
{
	if (c_pTcRef->real.door.wInhibitAlarmTimer == 0) {
		CTcx* pTcx = (CTcx*)c_pParent;
		pTcx->Shot(nCid, XchAtZero(nCid, wMsg));
	}
}

// Modified 2013/01/10
WORD* CDoor::GetTimer()
{
	if (IsRight())	return &c_pTcRef->real.door.wTimerR;
	else	return &c_pTcRef->real.door.wTimerL;
}

void CDoor::Mode()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BYTE lamp = 0;
	switch (c_pTcRef->real.door.nMode) {
	case DOORMODE_AOAC :	lamp = TCDOM_DOORAOAC;	break;
	case DOORMODE_AOMC :	lamp = TCDOM_DOORAOMC;	break;
	case DOORMODE_MOMC :	lamp = TCDOM_DOORMOMC;	break;
	default :				lamp = 0;				break;
	}
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))
		return;
	if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY) {
		c_pTcRef->real.door.nMode = DOORMODE_MOMC;
		lamp = TCDOM_DOORMOMC;
	}
	else if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS) {
		c_pTcRef->real.door.nMode = DOORMODE_AOAC;
		lamp = TCDOM_DOORAOAC;
	}
	else {
		BYTE dm = READTBYTE(OWN_SPACEA, TCDI_DOORMODE) & TCDIM_DOORONLY;
		if (c_pTcRef->real.door.nCuri != dm) {
			c_pTcRef->real.door.nCuri = dm;
			c_pTcRef->real.door.nDeb = DEB_DOORMODE;
		}
		else if (c_pTcRef->real.door.nDeb != 0 && -- c_pTcRef->real.door.nDeb == 0) {
			dm = c_pTcRef->real.door.nCuri;
			switch (dm) {
			case 1:
				c_pTcRef->real.door.nMode = DOORMODE_AOAC;
				lamp = TCDOM_DOORAOAC;
				break;
			case 2:
				c_pTcRef->real.door.nMode = DOORMODE_AOMC;
				lamp = TCDOM_DOORAOMC;
				break;
			case 4:
				c_pTcRef->real.door.nMode = DOORMODE_MOMC;
				lamp = TCDOM_DOORMOMC;
				break;
			default:
				if (dm == 0)	ShotL(SCENE_ID_HEADCAR, 701);
				else	ShotL(SCENE_ID_HEADCAR, 702);
				c_pTcRef->real.door.nMode = DOORMODE_UNKNOWN;
				break;
			}
		}
	}
	if (lamp != 0) {
		CUTS(SCENE_ID_HEADCAR, 701, 702);
	}
	Lamp(lamp, LP_ON);
	if (c_pTcRef->real.door.nMode == DOORMODE_MOMC) {
		CUTS(SCENE_ID_HEADCAR, 915, 917);
	}
}

void CDoor::Lamp(BYTE lp, BYTE state)
{
	BYTE lamp = FEEDTBYTE(OWN_SPACEA, TCDO_DOORMODELAMP);
	if (state) {
		lamp &= ~TCDOM_DOORLAMPES;
		lamp |= lp;
	}
	else	lamp &= ~lp;
	WRITETBYTE(OWN_SPACEA, TCDO_DOORMODELAMP, lamp);
}

BYTE CDoor::BitIndex(BYTE bits, BOOL state)
{
	for (UCURV n = 0; n < 8; n++) {
		BOOL cur = bits & (1 << n) ? TRUE : FALSE;
		if (cur == state)	return (n + 1);
	}
	return 0;
}

void CDoor::Shut()
{
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		c_pDoz->tcs[OWN_SPACEA].real.nOrder[n][0] &= ~0x1f;
		c_pDoz->tcs[OWN_SPACEA].real.nOrder[n][1] &= ~0x1f;
	}
	c_pTcRef->real.door.nState &= ~(DOORSTATE_ONLY | DOORSTATE_ENABLEREOPEN);
	c_pTcRef->real.door.nState |= DOORSTATE_CLOSED; // reset state!!!
	// Modified 2013/11/02
	c_pTcRef->real.door.nRefer = DOORREF_CLOSE;
}

void CDoor::Move(BOOL bDir, BOOL bCmd)
{
	BYTE dir = bDir ? 0 : 1; // bDir:right, !bDir:left;
	CTcx* pTcx = (CTcx*)c_pParent;
	WORD bus = pTcx->GetBusState();
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		c_pDoz->tcs[OWN_SPACEA].real.nOrder[n][dir] &= ~0x1f;
		if (bCmd == DOORCMD_OPEN) {
			c_pDoz->tcs[OWN_SPACEA].real.nOrder[n][dir] |= 0x1e;	// CCDOB_DOR1-4, CCDOB_DOL1-4
			if (bus & (1 << (LENGTH_TC + (n ^ 1)))) {
				c_pDoz->tcs[OWN_SPACEA].real.nOrder[n][dir] |= 1;	// CCDOB_OCRDO, CCDOB_OCLDO
				c_pTcRef->real.door.nBackup |= (1 << n);
				ShotL(n, 737);
			}
			else {
				c_pTcRef->real.door.nBackup &= ~(1 << n);
				CUT(n, 737);
			}
			CUTS(SCENE_ID_ALLCAR, 703, 710);
		}
		else {
			// closing not need reference
			CUTS(SCENE_ID_ALLCAR, 729, 736);
		}
	}
	if (bCmd == DOORCMD_OPEN) {
		if (bDir) {
			// Modified 2013/01/10
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY)
				c_pTcRef->real.door.nRefer &= ~(DOORREF_LEFT | DOORREF_LEFTMON);
			c_pTcRef->real.door.nRefer |= (DOORREF_RIGHT | DOORREF_RIGHTMON);
		}
		else {
			// Modified 2013/01/10
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY)
				c_pTcRef->real.door.nRefer &= ~(DOORREF_RIGHT | DOORREF_RIGHTMON);
			c_pTcRef->real.door.nRefer |= (DOORREF_LEFT | DOORREF_LEFTMON);
		}
	}
	else {
		if (bDir) {
			c_pTcRef->real.door.nRefer &= ~DOORREF_RIGHT;
			c_pTcRef->real.door.nRefer |= DOORREF_RIGHTMON;
		}
		else {
			c_pTcRef->real.door.nRefer &= ~DOORREF_LEFT;
			c_pTcRef->real.door.nRefer |= DOORREF_LEFTMON;
		}
	}
}

void CDoor::ReOpen(BOOL cmd)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->ObsCtrl(cmd, CCDOB_DROR, ALLCAR_FLAGS);
	pTcx->ObsCtrl(cmd, CCDOB_DROL, ALLCAR_FLAGS);

	if (c_pTcRef->real.door.nState & DOORSTATE_ENABLEREOPEN) {
		BYTE dir = IsRight() ? 0 : 1;
		WORD bus = pTcx->GetBusState();
		for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			c_pDoz->tcs[OWN_SPACEA].real.nOrder[n][dir] &= ~1;
			if (cmd && (bus & (1 << (LENGTH_TC + (n ^ 1)))))
				c_pDoz->tcs[OWN_SPACEA].real.nOrder[n][dir] |= 1;
		}
	}
}

BOOL CDoor::OpenCmd(BOOL bDir)
{
	// bDir=right, !bDir=left
	CTcx* pTcx = (CTcx*)c_pParent;
	if (!(c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) ||
			(GETTBIT(OWN_SPACEA, TCDIB_P))) {
		if (c_pTcRef->real.door.nMode == DOORMODE_MOMC)
			ShotL(SCENE_ID_HEADCAR, 744);
		else	ShotL(SCENE_ID_HEADCAR, 912);
		return FALSE;
	}
	else if (c_pTcRef->real.door.nMode == DOORMODE_AOAC || c_pTcRef->real.door.nMode == DOORMODE_AOMC) {
		c_pTcRef->real.door.nState |= DOORSTATE_WAITODNEGATE;
		c_pTcRef->real.door.nAtcOdDeb = DEBOUNCE_ATCODNEGATE;
	}
	CUT(SCENE_ID_HEADCAR, 912);
	CUT(SCENE_ID_HEADCAR, 744);
	Move(bDir, DOORCMD_OPEN);
	// Modified 2012/11/12
	c_pTcRef->real.door.nState |= DOORSTATE_ENABLEREOPEN;
	return TRUE;
}

void CDoor::CloseCmd(BOOL bDir)
{
	// bDir=right, !bDir=left
	Move(bDir, DOORCMD_CLOSE);
}

UCURV CDoor::ScanOpenCmd(BOOL bClosing)	// at emergency, bClosing must be 0
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
	BOOL bXch = c_pTcRef->real.nAddr > 2 ? TRUE : FALSE;
	WORD* pwTimer = GetTimer();
	if (c_pTcRef->real.door.nMode == DOORMODE_AOAC || c_pTcRef->real.door.nMode == DOORMODE_AOMC) {
		if (c_pTcRef->real.insp.nMode > INSPECTMODE_NON &&
				 c_pTcRef->real.insp.nMode < INSPECTMODE_VIEW &&
				 c_pTcRef->real.insp.nStep == 1 &&
				(c_pTcRef->real.insp.nAuxStep == SNDINSPECTSTEP_TESTATCA ||
				 c_pTcRef->real.insp.nAuxStep == SNDINSPECTSTEP_TESTATCB) &&
				(c_pTcRef->real.insp.nSubStep == INSPECTSND_SPEEDCODEL ||
				 c_pTcRef->real.insp.nSubStep == INSPECTSND_SPEEDCODEH ||
				 c_pTcRef->real.insp.nSubStep == INSPECTSND_OVERSPEEDL ||
				 c_pTcRef->real.insp.nSubStep == INSPECTSND_OVERSPEEDH))
			return DOORCMD_NON;
		if (paAtc != NULL && paAtc->d.st.mode.b.odl && paAtc->d.st.sph.b.edls &&
				!(c_pTcRef->real.door.nState & DOORSTATE_WAITODNEGATE)) { // open door left
			if (paAtc->d.st.mode.b.odr) { // open door right
				if (!bClosing)	ShotL(SCENE_ID_HEADCAR, 915);
				return DOORCMD_ERROR;
			}
			if ((!bXch && IsRightOpenSw()) || (bXch && IsLeftOpenSw())) {
				if (!bClosing) {
					ShotL(SCENE_ID_HEADCAR, 916);
					return DOORCMD_ERROR;
				}
			}
			// Modified 2012/11/29
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY) {	// can not excute emergency must be MOMC
				BYTE oppo = ScanOppositeOpenedDoor(FALSE);
				if (oppo < 64) {
					ShotL((BYTE)(oppo / 8), (WORD)((oppo % 8) + 721));
					return DOORCMD_ERROR;
				}
			}
			CUTS(SCENE_ID_ALLCAR, 721, 728);
			CUTS(SCENE_ID_ALLCAR, 729, 736);
			CUTS(SCENE_ID_HEADCAR, 915, 917);
			if (bClosing)	ShotL(SCENE_ID_HEADCAR, 908);
			return (bXch ? DOORCMD_RIGHT : DOORCMD_LEFT);
		}
		else if (paAtc != NULL && paAtc->d.st.mode.b.odr && paAtc->d.st.sph.b.edrs &&
				!(c_pTcRef->real.door.nState & DOORSTATE_WAITODNEGATE)) { // open door right
			if (paAtc != NULL && paAtc->d.st.mode.b.odl) { // open door left
				if (!bClosing)	ShotL(SCENE_ID_HEADCAR, 915);
				return DOORCMD_ERROR;
			}
			if ((!bXch && IsLeftOpenSw()) || (bXch && IsRightOpenSw())) {
				if (!bClosing) {
					ShotL(SCENE_ID_HEADCAR, 917);
					return DOORCMD_ERROR;
				}
			}
			// Modified 2012/11/29
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY) {	// can not excute emergency must be MOMC
				BYTE oppo = ScanOppositeOpenedDoor(TRUE);
				if (oppo < 64) {
					ShotL((BYTE)(oppo / 8), (WORD)((oppo % 8) + 721));
					return DOORCMD_ERROR;
				}
			}
			CUTS(SCENE_ID_ALLCAR, 721, 728);
			// Inserted 2012/01/17
			CUTS(SCENE_ID_ALLCAR, 729, 736);
			CUTS(SCENE_ID_HEADCAR, 915, 917);
			if (bClosing)	ShotL(SCENE_ID_HEADCAR, 908);
			return (bXch ? DOORCMD_LEFT : DOORCMD_RIGHT);
		}
	}
	else if (c_pTcRef->real.door.nMode == DOORMODE_MOMC) {
		// scan manual open
		if (IsLeftOpenSw()) {
			// Modified 2013/01/10
			//if (IsRightOpenSw()) {
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && IsRightOpenSw()) {
				if (!bClosing)	ShotL(SCENE_ID_HEADCAR, 918);
				return DOORCMD_ERROR;
			}
			CUT(SCENE_ID_HEADCAR, 918);
			if (IsCloseSw()) {
				ShotL(SCENE_ID_HEADCAR, 913);
				return DOORCMD_ERROR;
			}
			// Modified 2012/11/29
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY) {
				BYTE oppo = ScanOppositeOpenedDoor(FALSE);
				if (oppo < 64) {
					ShotL((BYTE)(oppo / 8), (WORD)((oppo % 8) + 721));
					return DOORCMD_ERROR;
				}
			}
			CUTS(SCENE_ID_ALLCAR, 721, 728);
			// Modified 2012/01/30 ... begin
			if ((paAtc != NULL && paAtc->d.st.sph.b.edls) || c_pTcRef->real.op.nMode == OPMODE_EMERGENCY) {
				CUT(SCENE_ID_HEADCAR, 924);
			// ... end
				if (!bClosing) {
					if (IsReopenSw())	ShotL(SCENE_ID_HEADCAR, 914);
					else	CUT(SCENE_ID_HEADCAR, 914);
					CUTS(SCENE_ID_ALLCAR, 729, 736);
					//CUTS(SCENE_ID_HEADCAR, 742, 743);
					CUT(SCENE_ID_HEADCAR, 913);
					return (bXch ? DOORCMD_RIGHT : DOORCMD_LEFT);
				}
				else {	// closing now
					if (*pwTimer > ((TIME_WAITDOOR - 3000) / TIME_INTERVAL) && !pTcx->CheckShot(SCENE_ID_HEADCAR, 900))
						ShotL(SCENE_ID_HEADCAR, 902);
					return (bXch ? DOORCMD_RIGHT : DOORCMD_LEFT);
				}
			// Modified 2012/01/30 ... begin
			}
			else {
				ShotL(SCENE_ID_HEADCAR, 924);
				return DOORCMD_ERROR;
			}
			// ... end
		}
		else if (IsRightOpenSw()) {
			// Modified 2013/01/10
			//if (IsLeftOpenSw()) {
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && IsLeftOpenSw()) {
				if (!bClosing)	ShotL(SCENE_ID_HEADCAR, 918);
				return DOORCMD_ERROR;
			}
			CUT(SCENE_ID_HEADCAR, 918);
			if (IsCloseSw()) {
				ShotL(SCENE_ID_HEADCAR, 913);
				return DOORCMD_ERROR;
			}
			// Modified 2012/11/29
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY) {
				BYTE oppo = ScanOppositeOpenedDoor(TRUE);
				if (oppo < 64) {
					ShotL((BYTE)(oppo / 8), (WORD)((oppo % 8) + 721));
					return DOORCMD_ERROR;
				}
			}
			CUTS(SCENE_ID_ALLCAR, 721, 728);
			// Modified 2012/01/30 ... begin
			if ((paAtc != NULL && paAtc->d.st.sph.b.edrs) || c_pTcRef->real.op.nMode == OPMODE_EMERGENCY) {
				CUT(SCENE_ID_HEADCAR, 924);
			// ... end
				if (!bClosing) {
					if (IsReopenSw())	ShotL(SCENE_ID_HEADCAR, 914);
					else	CUT(SCENE_ID_HEADCAR, 914);
					CUTS(SCENE_ID_ALLCAR, 729, 736);
					//CUTS(SCENE_ID_HEADCAR, 742, 743);
					CUT(SCENE_ID_HEADCAR, 913);
					return (bXch ? DOORCMD_LEFT : DOORCMD_RIGHT);
				}
				else {	// closing now
					if (*pwTimer > ((TIME_WAITDOOR - 3000) / TIME_INTERVAL) && !pTcx->CheckShot(SCENE_ID_HEADCAR, 900))
						ShotL(SCENE_ID_HEADCAR, 902);
					return (bXch ? DOORCMD_LEFT : DOORCMD_RIGHT);
				}
			// Modified 2012/01/30 ... begin
			}
			else {
				ShotL(SCENE_ID_HEADCAR, 924);
				return DOORCMD_ERROR;
			}
			// ... end
		}
		if (bClosing && IsReopenSw() && !pTcx->CheckShot(SCENE_ID_HEADCAR, 901) &&
				*pwTimer > ((TIME_WAITDOOR - 3000) / TIME_INTERVAL)) {
			ShotL(SCENE_ID_HEADCAR, 903);
			return DOORCMD_ERROR;
		}
	}
	return DOORCMD_NON; // unknown mode
}

UCURV CDoor::ScanCloseCmd()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
	BOOL bXch = c_pTcRef->real.nAddr > 2 ? TRUE : FALSE;
	if (c_pTcRef->real.door.nMode == DOORMODE_AOAC) {
		// scan auto close
		if (!IsRight() &&
				((!bXch && paAtc != NULL && (!paAtc->d.st.mode.b.odl || !paAtc->d.st.sph.b.edls)) ||
				(bXch && paAtc != NULL && (!paAtc->d.st.mode.b.odr || !paAtc->d.st.sph.b.edrs))))
			return DOORCMD_LEFT;
		else if (IsRight() &&
				((!bXch && paAtc != NULL && (!paAtc->d.st.mode.b.odr || !paAtc->d.st.sph.b.edrs)) ||
				(bXch && paAtc != NULL && (!paAtc->d.st.mode.b.odl || !paAtc->d.st.sph.b.edls))))
			return DOORCMD_RIGHT;
		// ... end
	}
	else if (c_pTcRef->real.door.nMode == DOORMODE_AOMC
			|| c_pTcRef->real.door.nMode == DOORMODE_MOMC) {
		// scan manual close
		if (IsCloseSw()) {
			if (IsOpenSw())
				ShotL(SCENE_ID_HEADCAR, 900);
			else	CUT(SCENE_ID_HEADCAR, 900);
			if (IsReopenSw()) {
				ShotL(SCENE_ID_HEADCAR, 901);
				return DOORCMD_ERROR;
			}
			else	CUT(SCENE_ID_HEADCAR, 901);
			return (IsRight() ? DOORCMD_RIGHT : DOORCMD_LEFT);
		}
	}
	return DOORCMD_NON;
}

void CDoor::CollectDoorState()
{
	// Modified 2011/12/21 - begin
	if (IsRight()) {
		c_pTcRef->real.door.nOppo = 0xf;
		c_pTcRef->real.door.nOwn = 0xf0;
	}
	else {
		c_pTcRef->real.door.nOppo = 0xf0;
		c_pTcRef->real.door.nOwn = 0xf;
	}
	// - end
	// Modified 2013/01/10
	//BOOL state = FALSE;	// close state
	//BYTE step = c_pTcRef->real.door.nState & DOORSTATE_ONLY;
	//if (step == DOORSTATE_OPENING || step == DOORSTATE_OPENED)	state = TRUE;
	c_pTcRef->real.door.nSum = 0;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		if (!(c_pTcRef->real.door.nBackup & (1 << (n ^ 1)))) {
			BYTE door = READCBYTE(n, CCDI_DOOR);
			// Modified 2013/01/10
			//DoorBypassMap(n, &door, state);
			DoorBypassMap(n, &door);
			c_pTcRef->real.door.nCur[n] = door;
			c_pTcRef->real.door.nSum |= door;
			if ((c_pTcRef->real.door.nBackup & (1 << n)) && !GETCBIT(n, CCDIB_OCCR))
				c_pTcRef->real.door.nSum |= 1;
		}
	}
}

// Modified 2013/01/10
// Only emergency mode
BOOL CDoor::ScanDoorState()		// after CollectDoorState()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL right = FALSE;
	BYTE expect, oppo;
	switch (c_pTcRef->real.door.nRefer & DOORREF_ALL) {
	case DOORREF_LEFT :
		expect = 0xf;
		oppo = 0xf0;
		break;
	case DOORREF_RIGHT :
		expect = 0xf0;
		oppo = 0xf;
		right = TRUE;
		break;
	case DOORREF_ALL :
		expect = 0xff;
		oppo = 0;
		break;
	default :
		expect = 0;
		oppo = 0;
		break;
	}
	BOOL aopen, aclose;
	aopen = aclose = TRUE;
	// reference timerR at right opening, else timerL(left open, close)
	WORD timer = c_pTcRef->real.door.wTimerL;
	WORD timerR = c_pTcRef->real.door.wTimerR;
	if (timer < timerR)	timer = timerR;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		if (c_pTcRef->real.door.nBackup & (1 << (n ^ 1)))	continue;
		BYTE door = c_pTcRef->real.door.nCur[n];
		if (expect != 0) {		// open sequence
			if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY) {	// can not excute this line
				if (door & oppo) {
					aopen = FALSE;
					ShotL(n, right ? 909 : 910);
				}
				else	CUT(n, right ? 909 : 910);
			}
			door &= expect;
			if (door != expect) {
				aopen = FALSE;
				if (timer == 0) {
					if (door == 0) {
						CUTS(n, 729, 736);
						ShotL(n, 741);
					}
					else {
						CUT(n, 741);
						BYTE di;
						door |= oppo;
						while ((di = BitIndex(door, FALSE)) != 0) {
							ShotL(n, 729 + di - 1);
							door |= (1 << (di - 1));
						}
					}
					if (!GETTBIT(OWN_SPACEA, TCDIB_ADSCR))	ShotL(n, 712);
					else	CUT(n, 712);
				}
			}
			else {
				CUT(n, 712);
				CUTS(n, 729, 736);
				CUT(n, 741);
			}
			if ((c_pTcRef->real.door.nBackup & (1 << n)) && GETCBIT(n, CCDIB_OCCR)) {
				aopen = FALSE;
				if (timer == 0)	ShotL(n ^ 1, 738);
			}
			else	CUT(n ^ 1, 738);
		}
		else {			// close sequence, can not occur 906-907
			if (door != expect) {
				aclose = FALSE;
				if (timer == 0) {
					if ((door & 0xf) == 0xf || (door & 0xf0) == 0xf0) {
						CUTS(n, 703, 710);
						if (c_pTcRef->real.door.nMode == DOORMODE_AOAC)	ShotL(n, 905);
						else	ShotL(n, 904);
					}
					else {
						CUTS(n, 904, 905);
						BYTE di;
						while ((di = BitIndex(door, TRUE)) != 0) {
							ShotL(n, 703 + di - 1);
							door &= ~(1 << (di - 1));
						}
					}
					if (GETTBIT(OWN_SPACEA, TCDIB_ADSCR))	ShotL(n, 711);
					else	CUT(n, 711);
				}
			}
			else {
				CUT(n, 711);
				CUTS(n, 703, 710);
				CUTS(n, 906, 907);
			}
			if ((c_pTcRef->real.door.nBackup & (1 << n)) && !GETCBIT(n, CCDIB_OCCR)) {
				aclose = FALSE;
				if (timer == 0)	ShotL(n ^ 1, 739);
			}
			else	CUT(n ^ 1, 739);
		}
	}
	if (expect != 0) {
		if (aopen || timer == 0)	return TRUE;
	}
	else {
		if (aclose && !GETTBIT(OWN_SPACEA, TCDIB_ADSCR))	aclose = FALSE;
		if (aclose || timer == 0)	return TRUE;
	}
	return FALSE;
}

/* for CCDIB_ANNBS
 * if this input is 1 then door operation is normal.
 * but this input is 0, door is always close state.
 */
// Only normal state
BOOL CDoor::ScanDoorAtOpen()		// after CollectDoorState()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL allopen = TRUE;
	WORD* pwTimer = GetTimer();
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		if (c_pTcRef->real.door.nBackup & (1 << (n ^ 1)))	continue;
		BYTE door = c_pTcRef->real.door.nCur[n];
		// Modified 2013/01/10
		//if (door & c_nOppo) {
		// Modified 2013/11/02
		//if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && (door & c_nOppo)) {
		if (door & c_pTcRef->real.door.nOppo) {
			ShotL(n, IsRight() ? 909 : 910);
			allopen = FALSE;
		}
		else	CUT(n, IsRight() ? 909 : 910);
		door &= c_pTcRef->real.door.nOwn;
		if (door != c_pTcRef->real.door.nOwn) {
			allopen = FALSE;
			if (*pwTimer == 0) {
				if (door == 0) {
					CUTS(n, 729, 736);
					ShotL(n, 741);
				}
				else {
					CUT(n, 741);
					BYTE di;
					door |= c_pTcRef->real.door.nOppo;
					while ((di = BitIndex(door, FALSE)) != 0) {
						ShotL(n, 729 + di - 1);
						door |= (1 << (di - 1));
					}
				}
				// Modified 2012/11/21
				//if (!GETTBIT(OWN_SPACEA, TCDIB_ADSCR))	ShotL(n, 712);
				//else	CUT(n, 712);
				if (!GETTBIT(OWN_SPACEA, TCDIB_ADSCR))	ShotL(n, 712);
				else	CUT(n, 712);
			}
		}
		else {
			CUT(n, 712);
			CUTS(n, 729, 736);
			CUT(n, 741);
		}
		if ((c_pTcRef->real.door.nBackup & (1 << n)) && GETCBIT(n, CCDIB_OCCR)) {
			allopen = FALSE;
			if (*pwTimer == 0)	ShotL(n ^ 1, 738);
		}
		else	CUT(n ^ 1, 738);
	}
	if (allopen || *pwTimer == 0)	return TRUE;
	return FALSE;
}

// Only normal state
BOOL CDoor::ScanDoorAtClose(BOOL bClosed)		// after CollectDoorState()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL allclose = TRUE;
	WORD* pwTimer = GetTimer();
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		if (c_pTcRef->real.door.nBackup & (1 << (n ^ 1)))	continue;
		BYTE door = c_pTcRef->real.door.nCur[n];
		// Modified 2013/01/10
		//if (!bClosed) {		// closing...
		// Modified 2013/11/02
		//if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && !bClosed) {		// closing...
		if (!bClosed) {		// closing...
			if (door & c_pTcRef->real.door.nOppo) {
				ShotL(n, IsRight() ? 906 : 907);
				allclose = FALSE;
			}
			else	CUT(n, IsRight() ? 906 : 907);
			door &= c_pTcRef->real.door.nOwn;
		}
		if (door != 0) {
			allclose = FALSE;
			if (*pwTimer == 0) {
				// Modified 2012/11/12 ... begin
				//if (door == c_nOwn && !bClosed) {
				// Modified 2013/01/10
				//if ((door & c_nOwn) == c_nOwn) {
				// Modified 2013/11/02
				//BYTE mask = c_pTcRef->real.op.nMode == OPMODE_EMERGENCY ? 0xff : c_nOwn;
				//if ((door & mask) == mask) {
				if ((door & c_pTcRef->real.door.nOwn) == c_pTcRef->real.door.nOwn) {
					CUTS(n, 703, 710);
					if (c_pTcRef->real.door.nMode != DOORMODE_AOAC)	ShotL(n, 905);
					else	ShotL(n, 904);
				}
				else {
					CUTS(n, 904, 905);
					BYTE di;
					// Modified 2013/01/10
					//if (!bClosed)	door &= c_nOwn;
					// Modified 2013/11/02
					//if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && !bClosed)
					//	door &= c_nOwn;
					if (!bClosed)	door &= c_pTcRef->real.door.nOwn;
					while ((di = BitIndex(door, TRUE)) != 0) {
						ShotL(n, 703 + di - 1);
						door &= ~(1 << (di - 1));
					}
				}
				if (GETTBIT(OWN_SPACEA, TCDIB_ADSCR))	ShotL(n, 711);
				else	CUT(n, 711);
			}
		}
		else {
			CUT(n, 711);
			CUTS(n, 703, 710);
			CUTS(n, 906, 907);
		}
		if ((c_pTcRef->real.door.nBackup & (1 << n)) && !GETCBIT(n, CCDIB_OCCR)) {
			allclose = FALSE;
			if (*pwTimer == 0)	ShotL(n ^ 1, 739);
		}
		else	CUT(n ^ 1, 739);
	}
	// Modified 2011/10/17-8)
	if (allclose && !GETTBIT(OWN_SPACEA, TCDIB_ADSCR))	allclose = FALSE;
	if (allclose || *pwTimer == 0)	return TRUE;
	return FALSE;
}

// Only normal state
void CDoor::ScanOppositeDoorOpenCmd(BOOL bRight)
{
	/*	bRight	xch		scan...
		1		0		left(odl)
		1		1		right(odr)
		0		0		right(odr)
		0		1		left(odl)
	 */
	CTcx* pTcx = (CTcx*)c_pParent;
	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
	BOOL bXch = c_pTcRef->real.nAddr > 2 ? TRUE : FALSE;
	BOOL bScan = bRight ^ bXch;
	if (bScan) {	// scan left cmd
		// Modified 2011/12/28 ... begin
		if (c_pTcRef->real.door.nMode == DOORMODE_AOAC || c_pTcRef->real.door.nMode == DOORMODE_AOMC) {
			if (paAtc != NULL && paAtc->d.st.mode.b.odl)
				ShotL(SCENE_ID_HEADCAR, bRight ? 743 : 742);
			else	CUTS(SCENE_ID_HEADCAR, 742, 743);
		}
		// ... end
		if (IsLeftOpenSw())
			ShotL(SCENE_ID_HEADCAR, bRight ? 923 : 922);
		else	CUTS(SCENE_ID_HEADCAR, 922, 923);
	}
	else {			// scan right cmd
		// Modified 2011/12/28 ... begin
		if (c_pTcRef->real.door.nMode == DOORMODE_AOAC || c_pTcRef->real.door.nMode == DOORMODE_AOMC) {
			if (paAtc != NULL && paAtc->d.st.mode.b.odr)
				ShotL(SCENE_ID_HEADCAR, bRight ? 743 : 742);
			else	CUTS(SCENE_ID_HEADCAR, 742, 743);
		}
		// ... end
		if (IsRightOpenSw())
			ShotL(SCENE_ID_HEADCAR, bRight ? 923 : 922);
		else	CUTS(SCENE_ID_HEADCAR, 922, 923);
	}
}

BYTE CDoor::ScanOppositeOpenedDoor(BOOL bRight)		// can not excute at emergency mode
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL bXch = c_pTcRef->real.nAddr > 2 ? TRUE : FALSE;
	BOOL bScan = bRight ^ bXch;
	BYTE scan = bScan ? 0xf : 0xf0;	// bRight->scan left

	BYTE did = 0;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		if (!(c_pTcRef->real.door.nBackup & (1 << (n ^ 1)))) {
			BYTE door = READCBYTE(n, CCDI_DOOR) & scan;
			if (!GETCBIT(n, CCDIB_DNBS)) {
				BYTE map = pTcx->GetDoorBypass(n) & scan;	// bypass door is 1
				if (map != 0) {
					map = (map ^ 0xff) & scan;
					door &= ((map ^ 0xff) & scan);
				}
			}
			if (door != 0) {
				for (BYTE n = 0; n < 8; n ++) {
					if (door & (1 << n))	return did + n;
				}
			}
		}
		did += 8;
	}
	return 64;
}

// Modified 2013/01/10
void CDoor::DoorBypassMap(UCURV nCcid, BYTE* pDoor)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (!GETCBIT(nCcid, CCDIB_DNBS)) {
		BYTE map = pTcx->GetDoorBypass(nCcid);	// bypass door is 1
		if (map != 0) {
			BYTE add = 0;
			BYTE rem = 0xff;
			switch (c_pTcRef->real.door.nRefer & DOORREF_ALL) {
			case DOORREF_LEFT :
				add = map & 0xf;
				rem = ~(map & 0xf0);
				break;
			case DOORREF_RIGHT :
				add = map & 0xf0;
				rem = ~(map & 0xf);
				break;
			case DOORREF_ALL :
				add = map;
				rem = 0xff;
				break;
			default :
				add = 0;
				rem = ~map;
				break;
			}
			*pDoor |= add;
			*pDoor &= rem;
		}
		ShotL(nCcid, 746);
	}
	else	CUT(nCcid, 746);
}

/*=================================================================
 *	if Doorbypass is 1...
 *	stat			bOwn		exec
 *	1(at open)		1(side)		OR	 flag 	(make open)
 *	1(at open)		0(oppo)		AND	~flag	(make close)
 *	0(at close)		1(side)		AND	~flag	(make close)
 *	---0(at close)		0(oppo)		OR 	 flag	(make open)
 *	0(at close)		0(oppo)		AND ~flag	(make close)
 =================================================================*/
void CDoor::DoorBypassMap(UCURV nCcid, BYTE* pDoor, BOOL state)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (!GETCBIT(nCcid, CCDIB_DNBS)) {
		WORD map = pTcx->GetDoorBypass(nCcid);	// bypass door is 1
		if (map != 0) {
			for (UCURV n = 0; n < 8; n ++) {
				BYTE flag = 1 << n;
				if (map & flag) {
					// found bypass door
					BOOL bSide = c_pTcRef->real.door.nOwn & flag ? TRUE : FALSE;
					if (state && bSide)	*pDoor |= flag;
					else	*pDoor &= ~flag;
				}
			}
		}
		ShotL(nCcid, 746);
	}
	else	CUT(nCcid, 746);
}

void CDoor::DoorAnnals()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n++) {
		for (UCURV m = 0; m < 4; m ++) {
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].state.b.diso, n, XchAtZero(n, m + 756));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].state.b.diso, n, XchAtZero(n, m + 760));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].state.b.eeh, n, XchAtZero(n, m + 772));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].state.b.eeh, n, XchAtZero(n, m + 776));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].state.b.ods, n, XchAtZero(n, m + 780));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].state.b.ods, n, XchAtZero(n, m + 784));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].state.b.eha, n, XchAtZero(n, m + 788));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].state.b.eha, n, XchAtZero(n, m + 792));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].fail.b.dcuf1, n, XchAtZero(n, m + 796));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].fail.b.dcuf1, n, XchAtZero(n, m + 800));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].fail.b.dcuf2, n, XchAtZero(n, m + 804));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].fail.b.dcuf2, n, XchAtZero(n, m + 808));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].fail.b.dmcf, n, XchAtZero(n, m + 812));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].fail.b.dmcf, n, XchAtZero(n, m + 816));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].fail.b.cdlsf, n, XchAtZero(n, m + 820));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].fail.b.cdlsf, n, XchAtZero(n, m + 824));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].fail.b.cdcsf, n, XchAtZero(n, m + 828));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].fail.b.cdcsf, n, XchAtZero(n, m + 832));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].err.b.odlsf, n, XchAtZero(n, m + 836));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].err.b.odlsf, n, XchAtZero(n, m + 840));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].err.b.odcsf, n, XchAtZero(n, m + 844));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].err.b.odcsf, n, XchAtZero(n, m + 848));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].err.b.dfco, n, XchAtZero(n, m + 852));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].err.b.dfco, n, XchAtZero(n, m + 856));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].err.b.dfoo, n, XchAtZero(n, m + 860));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].err.b.dfoo, n, XchAtZero(n, m + 864));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].err.b.svl, n, XchAtZero(n, m + 868));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].err.b.svl, n, XchAtZero(n, m + 872));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].err.b.svh, n, XchAtZero(n, m + 876));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].err.b.svh, n, XchAtZero(n, m + 880));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].err.b.dcc, n, XchAtZero(n, m + 884));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].err.b.dcc, n, XchAtZero(n, m + 888));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[0][m].err.b.doc, n, XchAtZero(n, m + 892));
			pTcx->Annals(c_pDoz->ccs[n].real.aDcu[1][m].err.b.doc, n, XchAtZero(n, m + 896));
		}
	}
}

UCURV CDoor::DecisionDoorByDir(UCURV ddir)
{
	if (c_pTcRef->real.nAddr > 2) {
		if (ddir == DOORCMD_LEFT)	return DOORCMD_RIGHT;
		return DOORCMD_LEFT;
	}
	return ddir;
}

// Modified 2013/01/10
void CDoor::ByEmergency()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	// Scan open button
	UCURV cmd = ScanOpenCmd(FALSE);
	if ((cmd == DOORCMD_LEFT && !(c_pTcRef->real.door.nRefer & DOORREF_LEFT)) ||
			(cmd == DOORCMD_RIGHT && !(c_pTcRef->real.door.nRefer & DOORREF_RIGHT))) {
		if (OpenCmd(cmd == DOORCMD_RIGHT ? TRUE : FALSE)) {
			StampDir(cmd);
			TimerTrigger(TIME_WAITDOOR);
			c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;
			c_pTcRef->real.door.nState |= DOORSTATE_OPENING;
			CUTS(SCENE_ID_ALLCAR, 721, 728);
			CUT(SCENE_ID_ALLCAR, 739);
			CUTS(SCENE_ID_ALLCAR, 904, 905);
		}
	}
	// Scan close button
	cmd = ScanCloseCmd();
	if (cmd > DOORCMD_ERROR && (c_pTcRef->real.door.nRefer & DOORREF_ALL)) {
		if (c_pTcRef->real.door.nRefer & DOORREF_RIGHT) {
			cmd = DOORCMD_RIGHT;
			StampDir(cmd);
			CloseCmd(TRUE);
		}
		if (c_pTcRef->real.door.nRefer & DOORREF_LEFT) {
			cmd = DOORCMD_LEFT;
			StampDir(cmd);
			CloseCmd(FALSE);
		}
		TimerTrigger(TIME_WAITDOOR);
		c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;
		c_pTcRef->real.door.nState |= DOORSTATE_CLOSING;
		CUT(SCENE_ID_HEADCAR, 908);
		CUTS(SCENE_ID_ALLCAR, 713, 720);
		CUT(SCENE_ID_ALLCAR, 738);
		CUT(SCENE_ID_ALLCAR, 741);
		CUTS(SCENE_ID_ALLCAR, 729, 736);
	}
	// monit
	if (c_pTcRef->real.door.nRefer & (DOORREF_LEFTMON | DOORREF_RIGHTMON)) {
		if (ScanDoorState()) {
			c_pTcRef->real.door.nRefer &= ~(DOORREF_LEFTMON | DOORREF_RIGHTMON);
			pTcx->InterTrigger(&c_pTcRef->real.door.wTimerL, 0);
			pTcx->InterTrigger(&c_pTcRef->real.door.wTimerR, 0);
			c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;
			if (c_pTcRef->real.door.nRefer & (DOORREF_LEFT | DOORREF_RIGHT))
				c_pTcRef->real.door.nState |= DOORSTATE_OPENED;
			else	c_pTcRef->real.door.nState |= DOORSTATE_CLOSED;
		}
	}
}

void CDoor::ByNormal()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	UCURV cmd;
	switch (c_pTcRef->real.door.nState & DOORSTATE_ONLY) {
	case DOORSTATE_CLOSED :
		cmd = ScanOpenCmd(FALSE);
		if (cmd == DOORCMD_LEFT || cmd == DOORCMD_RIGHT) {
			if (OpenCmd(cmd == DOORCMD_RIGHT ? TRUE : FALSE)) {
				StampDir(cmd);
				TimerTrigger(TIME_WAITDOOR);
				c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;
				c_pTcRef->real.door.nState |= DOORSTATE_OPENING;	// | DOORSTATE_REOPENENABLE);
				CUTS(SCENE_ID_ALLCAR, 721, 728);
				CUT(SCENE_ID_ALLCAR, 739);
				CUTS(SCENE_ID_ALLCAR, 904, 905);
			}
		}
		else {
			if (!(c_pTcRef->real.door.nState & DOORSTATE_REOPENING))
				ScanDoorAtClose(TRUE);
		}
		break;

	case DOORSTATE_OPENED :
		ScanOppositeDoorOpenCmd(IsRight() ? TRUE : FALSE);
		// take on close command
		if (!(c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF))
			ShotL(SCENE_ID_HEADCAR, 745);
		else	CUT(SCENE_ID_HEADCAR, 745);
		cmd = ScanCloseCmd();
		if (cmd == DOORCMD_LEFT || cmd == DOORCMD_RIGHT) {
			// Modified 2013/11/02
			//StampDir(cmd);
			//CloseCmd(IsRight() ? TRUE : FALSE);
			if (c_pTcRef->real.door.nRefer & DOORREF_RIGHT) {
				cmd = DOORCMD_RIGHT;
				StampDir(cmd);
				CloseCmd(TRUE);
			}
			if (c_pTcRef->real.door.nRefer & DOORREF_LEFT) {
				cmd = DOORCMD_LEFT;
				StampDir(cmd);
				CloseCmd(FALSE);
			}
			TimerTrigger(TIME_WAITDOOR);
			c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
			c_pTcRef->real.door.nState |= DOORSTATE_CLOSING;
			CUT(SCENE_ID_HEADCAR, 908);
			CUTS(SCENE_ID_ALLCAR, 713, 720);
			CUT(SCENE_ID_ALLCAR, 738);
			CUT(SCENE_ID_ALLCAR, 741);
			CUTS(SCENE_ID_ALLCAR, 729, 736);
		}
		else	ScanDoorAtOpen();
		break;

	case DOORSTATE_OPENING :
		ScanOppositeDoorOpenCmd(IsRight() ? TRUE : FALSE);
		if (ScanDoorAtOpen()) {
			TimerTrigger(0);
			c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
			c_pTcRef->real.door.nState |= DOORSTATE_OPENED;
			c_pTcRef->real.door.nRefer &= ~(DOORREF_LEFTMON | DOORREF_RIGHTMON);
		}
		else {
			cmd = ScanCloseCmd();
			if (cmd == DOORCMD_LEFT || cmd == DOORCMD_RIGHT) {
				// Modified 2013/11/02
				//StampDir(cmd);
				//CloseCmd(IsRight() ? TRUE : FALSE);
				if (c_pTcRef->real.door.nRefer & DOORREF_RIGHT) {
					cmd = DOORCMD_RIGHT;
					StampDir(cmd);
					CloseCmd(TRUE);
				}
				if (c_pTcRef->real.door.nRefer & DOORREF_LEFT) {
					cmd = DOORCMD_LEFT;
					StampDir(cmd);
					CloseCmd(FALSE);
				}
				TimerTrigger(TIME_WAITDOOR);
				c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
				c_pTcRef->real.door.nState |= DOORSTATE_CLOSING;
				CUT(SCENE_ID_HEADCAR, 908);
			}
		}
		break;

	case DOORSTATE_CLOSING :
		// Modified 2012/11/12
		if (!(c_pTcRef->real.door.nState & DOORSTATE_REOPENING)) {
			if (ScanDoorAtClose(FALSE)) {
				TimerTrigger(0);
				c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
				c_pTcRef->real.door.nState |= DOORSTATE_CLOSED; // reset state!!!
				c_pTcRef->real.door.nRefer &= ~(DOORREF_LEFTMON | DOORREF_RIGHTMON);
			}
			else {
				if (c_pTcRef->real.door.nMode != DOORMODE_AOMC) {
					// not close door closing cycle
					cmd = ScanOpenCmd(TRUE);
					if (cmd == DOORCMD_LEFT || cmd == DOORCMD_RIGHT) {
						if (OpenCmd(cmd == DOORCMD_RIGHT ? TRUE : FALSE)) {
							StampDir(cmd);
							TimerTrigger(TIME_WAITDOOR);
							c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
							c_pTcRef->real.door.nState |= DOORSTATE_OPENING;
						}
					}
				}
			}
		}
		break;

	default:
		break;
	}
}

void CDoor::Control()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	// Modified 2012/01/17 ... begin
	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
	if (paAtc != NULL) {
		if (c_pTcRef->real.door.nState & DOORSTATE_WAITODNEGATE) {
			if (!paAtc->d.st.mode.b.odl && !paAtc->d.st.mode.b.odr && (c_pTcRef->real.door.nRefer & DOORREF_ALL) == DOORREF_CLOSE) {
				if (-- c_pTcRef->real.door.nAtcOdDeb == 0)
					c_pTcRef->real.door.nState &= ~DOORSTATE_WAITODNEGATE;
			}
			else {
				c_pTcRef->real.door.nState |= DOORSTATE_WAITODNEGATE;
				c_pTcRef->real.door.nAtcOdDeb = DEBOUNCE_ATCODNEGATE;
			}
		}
	}
	else if (c_pTcRef->real.door.nState & DOORSTATE_WAITODNEGATE) {
		c_pTcRef->real.door.nState &= ~DOORSTATE_WAITODNEGATE;
		c_pTcRef->real.door.nAtcOdDeb = 0;
	}
	// ... end

	Mode();
	CollectDoorState();
	if ((GETTBIT(OWN_SPACEA, TCDIB_ADSCR) && c_pTcRef->real.door.nSum != 0) ||
			(!GETTBIT(OWN_SPACEA, TCDIB_ADSCR) && c_pTcRef->real.door.nSum == 0)) {
		if (FBTBIT(OWN_SPACEA, TCDOB_HCR)) {		// ??????????????????
		if (c_pTcRef->real.door.wAdscrDeb > 0) {
			if (-- c_pTcRef->real.door.wAdscrDeb == 1)
				ShotL(SCENE_ID_HEADCAR, 740);
		}
		}
	}
	else {
		CUT(SCENE_ID_HEADCAR, 740);
		c_pTcRef->real.door.wAdscrDeb = DEB_ADSCR;
	}

	if (!IsOpenSw()) {
		CUT(SCENE_ID_HEADCAR, 900);
		CUT(SCENE_ID_HEADCAR, 902);
		CUT(SCENE_ID_HEADCAR, 918);
	}
	if (!IsReopenSw()) {
		CUT(SCENE_ID_HEADCAR, 901);
		CUT(SCENE_ID_HEADCAR, 903);
		CUT(SCENE_ID_HEADCAR, 914);
	}
	if (!IsCloseSw()) {
		CUT(SCENE_ID_HEADCAR, 913);
	}

	if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY)	ByEmergency();
	else	ByNormal();

	// scan reopen...
	if ((c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) &&
			!GETTBIT(OWN_SPACEA, TCDIB_P) &&
			!(c_pTcRef->real.door.nState & DOORSTATE_REOPENING) &&
			(c_pTcRef->real.door.nState & DOORSTATE_ONLY) != DOORSTATE_OPENED &&
			(c_pTcRef->real.door.nState & DOORSTATE_ONLY) != DOORSTATE_OPENING &&
			IsReopenSw()) {
		ReOpen(TRUE);
		c_pTcRef->real.door.nState |= DOORSTATE_REOPENING;
		TimerTrigger(0);
		// Inserted 2012/01/16
		pTcx->InterTrigger(&c_pTcRef->real.door.wInhibitAlarmTimer, TIME_INHIBITALARM);
		if (c_pTcRef->real.door.nMode == DOORMODE_AOAC)	ShotL(SCENE_ID_HEADCAR, 920);
		else	ShotL(SCENE_ID_HEADCAR, 921);
	}
	else if (c_pTcRef->real.door.nState & DOORSTATE_REOPENING) {
		if (!IsReopenSw()) {
			ReOpen(FALSE);
			// Modified 2012/11/12 ... begin
			TimerTrigger(TIME_WAITDOOR);
			c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
			c_pTcRef->real.door.nState |= DOORSTATE_CLOSING;
			// ... end
			c_pTcRef->real.door.nState &= ~DOORSTATE_REOPENING;
			// Inserted 2012/01/16
			pTcx->InterTrigger(&c_pTcRef->real.door.wInhibitAlarmTimer, 0);
			CUTS(SCENE_ID_HEADCAR, 920, 921);
		}
	}

	if (GETTBIT(OWN_SPACEA, TCDIB_ADSCR)) {
		SETTBIT(OWN_SPACEA, TCDOB_DCL);
		//SETTBIT(OWN_SPACEA, OPPO_TCDO, TCDOB_DCL);
		if (GETTBIT(OWN_SPACEA, TCDIB_ADSBS))
			ShotL(SCENE_ID_HEADCAR, 747);
		else	CUT(SCENE_ID_HEADCAR, 747);
	}
	else {
		CLRTBIT(OWN_SPACEA, TCDOB_DCL);
		//CLRTBIT(OWN_SPACEA, OPPO_TCDO, TCDOB_DCL);
	}

	if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	// Ummm... this process has many time(17ms) only slave
		DoorAnnals();
}

//void CDoor::Control()
//{
//	CTcx* pTcx = (CTcx*)c_pParent;
//	// Modified 2012/01/17 ... begin
//	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
//	if (paAtc != NULL) {
//		if (c_pTcRef->real.door.nState & DOORSTATE_WAITODNEGATE) {
//			if (!paAtc->d.st.mode.b.odl && !paAtc->d.st.mode.b.odr) {
//				if (-- c_pTcRef->real.door.nAtcOdDeb == 0)
//					c_pTcRef->real.door.nState &= ~DOORSTATE_WAITODNEGATE;
//			}
//			else {
//				c_pTcRef->real.door.nState |= DOORSTATE_WAITODNEGATE;
//				c_pTcRef->real.door.nAtcOdDeb = DEBOUNCE_ATCODNEGATE;
//			}
//		}
//	}
//	else if (c_pTcRef->real.door.nState & DOORSTATE_WAITODNEGATE) {
//		c_pTcRef->real.door.nState &= ~DOORSTATE_WAITODNEGATE;
//		c_pTcRef->real.door.nAtcOdDeb = 0;
//	}
//	// ... end
//
//	Mode();
//	CollectDoorState();
//	if ((GETTBIT(OWN_SPACEA, TCDIB_ADSCR) && c_pTcRef->real.door.nSum != 0) ||
//			(!GETTBIT(OWN_SPACEA, TCDIB_ADSCR) && c_pTcRef->real.door.nSum == 0)) {
//		if (FBTBIT(OWN_SPACEA, TCDOB_HCR)) {		// ??????????????????
//		if (c_pTcRef->real.door.wAdscrDeb > 0) {
//			if (-- c_pTcRef->real.door.wAdscrDeb == 1)
//				ShotL(SCENE_ID_HEADCAR, 740);
//		}
//		}
//	}
//	else {
//		CUT(SCENE_ID_HEADCAR, 740);
//		c_pTcRef->real.door.wAdscrDeb = DEB_ADSCR;
//	}
//
//	if (!IsOpenSw()) {
//		CUT(SCENE_ID_HEADCAR, 900);
//		CUT(SCENE_ID_HEADCAR, 902);
//		CUT(SCENE_ID_HEADCAR, 918);
//	}
//	if (!IsReopenSw()) {
//		CUT(SCENE_ID_HEADCAR, 901);
//		CUT(SCENE_ID_HEADCAR, 903);
//		CUT(SCENE_ID_HEADCAR, 914);
//	}
//	if (!IsCloseSw()) {
//		CUT(SCENE_ID_HEADCAR, 913);
//	}
//
//	UCURV cmd;
//	switch (c_pTcRef->real.door.nState & DOORSTATE_ONLY) {
//	case DOORSTATE_CLOSED :
//		// Modified 2012/11/12 ... delete 2011/10/17
//		// Modified 2011/10/17-8)
//		//if (c_pTcRef->real.door.nMode != DOORMODE_AOMC || c_pTcRef->real.door.wOpenRescanTimer == 0) {
//			// take on open command
//			cmd = ScanOpenCmd(FALSE);
//			if (cmd == DOORCMD_LEFT || cmd == DOORCMD_RIGHT) {
//				if (OpenCmd(cmd == DOORCMD_RIGHT ? TRUE : FALSE)) {
//					pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, TIME_WAITDOOR);
//					c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;
//					c_pTcRef->real.door.nState |= DOORSTATE_OPENING;	// | DOORSTATE_REOPENENABLE);
//					if (cmd == DOORCMD_RIGHT)
//						c_pTcRef->real.door.nState |= DOORSTATE_RIGHT;
//					else	c_pTcRef->real.door.nState &= ~DOORSTATE_RIGHT;
//				}
//				CUTS(SCENE_ID_ALLCAR, 721, 728);
//				CUT(SCENE_ID_ALLCAR, 739);
//				CUTS(SCENE_ID_ALLCAR, 904, 905);
//			}
//			else {
//				// Modified 2012/11/12
//				if (!(c_pTcRef->real.door.nState & DOORSTATE_REOPENING))
//					ScanDoorAtClose(TRUE);
//			}
//		//}
//		break;
//
//	case DOORSTATE_OPENED :
//		ScanOppositeDoorOpenCmd(c_pTcRef->real.door.nState & DOORSTATE_RIGHT ? TRUE : FALSE);
//		// take on close command
//		if (!(c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF)) {
//			//cmd = c_pTcRef->real.door.nState & DOORSTATE_RIGHT ? DOORCMD_RIGHT
//			//		: DOORCMD_LEFT;
//			ShotL(SCENE_ID_HEADCAR, 745);
//		}
//		else	CUT(SCENE_ID_HEADCAR, 745);
//		//c_pTcRef->real.speed.dbDist = 0.f;
//		cmd = ScanCloseCmd();
//		if (cmd == DOORCMD_LEFT || cmd == DOORCMD_RIGHT) {
//			CloseCmd(cmd == DOORCMD_RIGHT ? TRUE : FALSE);
//			pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, TIME_WAITDOOR);
//			c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
//			c_pTcRef->real.door.nState |= DOORSTATE_CLOSING;
//			if (cmd == DOORCMD_RIGHT)
//				c_pTcRef->real.door.nState |= DOORSTATE_RIGHT;
//			else	c_pTcRef->real.door.nState &= ~DOORSTATE_RIGHT;
//			CUT(SCENE_ID_HEADCAR, 908);
//			CUTS(SCENE_ID_ALLCAR, 713, 720);
//			CUT(SCENE_ID_ALLCAR, 738);
//			CUT(SCENE_ID_ALLCAR, 741);
//			CUTS(SCENE_ID_ALLCAR, 729, 736);
//		}
//		else	ScanDoorAtOpen();
//		break;
//
//	case DOORSTATE_OPENING :
//		ScanOppositeDoorOpenCmd(c_pTcRef->real.door.nState & DOORSTATE_RIGHT ? TRUE : FALSE);
//		if (ScanDoorAtOpen()) {
//			pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, 0);
//			c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
//			c_pTcRef->real.door.nState |= DOORSTATE_OPENED;
//		}
//		else {
//			cmd = ScanCloseCmd();
//			if (cmd == DOORCMD_LEFT || cmd == DOORCMD_RIGHT) {
//				CloseCmd(cmd == DOORCMD_RIGHT ? TRUE : FALSE);
//				pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, TIME_WAITDOOR);
//				c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
//				c_pTcRef->real.door.nState |= DOORSTATE_CLOSING;
//				if (cmd == DOORCMD_RIGHT)
//					c_pTcRef->real.door.nState |= DOORSTATE_RIGHT;
//				else	c_pTcRef->real.door.nState &= ~DOORSTATE_RIGHT;
//				CUT(SCENE_ID_HEADCAR, 908);
//			}
//		}
//		break;
//
//	case DOORSTATE_CLOSING :
//		// Modified 2012/11/12
//		if (!(c_pTcRef->real.door.nState & DOORSTATE_REOPENING)) {
//			if (ScanDoorAtClose(FALSE)) {
//				pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, 0);
//				// Modified 2013/01/10
//				// Modified 2011/10/17-8)
//				//if (c_pTcRef->real.door.nMode == DOORMODE_AOMC)
//				//	pTcx->InterTrigger(&c_pTcRef->real.door.wOpenRescanTimer, TIME_OPENRESCAN);
//				c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
//				c_pTcRef->real.door.nState |= DOORSTATE_CLOSED; // reset state!!!
//			}
//			else {
//				// Modified 2011/10/17-8)
//				if (c_pTcRef->real.door.nMode != DOORMODE_AOMC) {
//					// not close door closing cycle
//					cmd = ScanOpenCmd(TRUE);
//					if (cmd == DOORCMD_LEFT || cmd == DOORCMD_RIGHT) {
//						if (OpenCmd(cmd == DOORCMD_RIGHT ? TRUE : FALSE)) {
//							pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, TIME_WAITDOOR);
//							c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
//							c_pTcRef->real.door.nState |= DOORSTATE_OPENING;
//							if (cmd == DOORCMD_RIGHT)
//								c_pTcRef->real.door.nState |= DOORSTATE_RIGHT;
//							else	c_pTcRef->real.door.nState &= ~DOORSTATE_RIGHT;
//						}
//					}
//				}
//			}
//		}
//		break;
//
//	case DOORSTATE_NOTCLOSED : // not execute!!!
//		/*//if (IsReopenSw()) {
//			for (n = 0; n < c_pTcRef->real.cf.nLength; n++) {
//				BYTE door = READCBYTE(n, CCDI_DOOR);
//				if (!(c_pTcRef->real.door.nState & DOORSTATE_RIGHT) && (door & 0xf) != 0)
//					SETOBIT(OWN_SPACEA, n, CCDOB_DROL);
//				else if ((c_pTcRef->real.door.nState & DOORSTATE_RIGHT) && (door & 0xf0) != 0)
//					SETOBIT(OWN_SPACEA, n, CCDOB_DROR);
//			}
//			pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, TIME_WAITDOOR);
//			c_pTcRef->real.door.nState |= DOORSTATE_REOPEN;
//			c_pTcRef->real.door.nState &= ~(DOORSTATE_ONLY | DOORSTATE_REOPENENABLE);
//			c_pTcRef->real.door.nState |= DOORSTATE_CLOSING;
//		}*/
//		break;
//
//	default:
//		break;
//	}
//
//	// scan reopen...
//	if ((c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) &&
//			!GETTBIT(OWN_SPACEA, TCDIB_P) &&
//			!(c_pTcRef->real.door.nState & DOORSTATE_REOPENING) &&
//			(c_pTcRef->real.door.nState & DOORSTATE_ONLY) != DOORSTATE_OPENED &&
//			(c_pTcRef->real.door.nState & DOORSTATE_ONLY) != DOORSTATE_OPENING &&
//			IsReopenSw()) {
//		ReOpen(TRUE);
//		c_pTcRef->real.door.nState |= DOORSTATE_REOPENING;
//		pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, 0);
//		// Inserted 2012/01/16
//		pTcx->InterTrigger(&c_pTcRef->real.door.wInhibitAlarmTimer, TIME_INHIBITALARM);
//		if (c_pTcRef->real.door.nMode == DOORMODE_AOAC)	ShotL(SCENE_ID_HEADCAR, 920);
//		else	ShotL(SCENE_ID_HEADCAR, 921);
//	}
//	else if (c_pTcRef->real.door.nState & DOORSTATE_REOPENING) {
//		if (!IsReopenSw()) {
//			ReOpen(FALSE);
//			// Modified 2012/11/12 ... begin
//			pTcx->InterTrigger(&c_pTcRef->real.door.wTimer, TIME_WAITDOOR);
//			c_pTcRef->real.door.nState &= ~DOORSTATE_ONLY;	// | DOORSTATE_REOPENENABLE);
//			c_pTcRef->real.door.nState |= DOORSTATE_CLOSING;
//			// ... end
//			c_pTcRef->real.door.nState &= ~DOORSTATE_REOPENING;
//			// Inserted 2012/01/16
//			pTcx->InterTrigger(&c_pTcRef->real.door.wInhibitAlarmTimer, 0);
//			CUTS(SCENE_ID_HEADCAR, 920, 921);
//		}
//	}
//
//	if (GETTBIT(OWN_SPACEA, TCDIB_ADSCR)) {
//		SETTBIT(OWN_SPACEA, TCDOB_DCL);
//		//SETTBIT(OWN_SPACEA, OPPO_TCDO, TCDOB_DCL);
//		if (GETTBIT(OWN_SPACEA, TCDIB_ADSBS))
//			ShotL(SCENE_ID_HEADCAR, 747);
//		else	CUT(SCENE_ID_HEADCAR, 747);
//	}
//	else {
//		CLRTBIT(OWN_SPACEA, TCDOB_DCL);
//		//CLRTBIT(OWN_SPACEA, OPPO_TCDO, TCDOB_DCL);
//	}
//
//	if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	// Ummm... this process has many time(17ms) only slave
//		DoorAnnals();
//}
