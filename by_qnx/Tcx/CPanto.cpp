/*
 * CPanto.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include "CPanto.h"
#include "CTcx.h"

#define	PANTOSTEP()	(c_pTcRef->real.ptflow.cur.nState & PTFLOW_STEP)

CPanto::CPanto()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pDoz = NULL;
	c_nSwPrev =
	c_nSwBuf =
	c_nSwDeb = 0;
	//c_bMf = FALSE;
	//c_bMr = FALSE;
}

CPanto::~CPanto()
{
}

void CPanto::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pDoz = pTcx->GetTcDoz();
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.cur.wTimer);
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.cur.wRunTimer);
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.auxf.wTimer);
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.auxf.wRunTimer);
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.auxf.wMonit);
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.auxr.wTimer);
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.auxr.wRunTimer);
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.auxr.wMonit);
	pTcx->TimeRegister(&c_pTcRef->real.ptflow.wMonitCatenary);
	//pTcx->TimeRegister(&c_pTcRef->real.ptflow.mon.wRunTimer);
}

BOOL CPanto::Drive()
{
	// if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return FALSE;

	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {
		// Modified 2013/11/02
		c_pTcRef->real.ptflow.nState &= ~PS_LINES;
		if (_ISOCC(__AUXA) && pTcx->GetBusCondition(_REALCC(__AUXA)))
			c_pTcRef->real.ptflow.nState |= PS_PANFRONTLINE;
		if (_ISOCC(__AUXB) && pTcx->GetBusCondition(_REALCC(__AUXB)))
			c_pTcRef->real.ptflow.nState |= PS_PANREARLINE;
		if (_ISOCC(__SIVA) && pTcx->GetBusCondition(_REALCC(__SIVA)))
			c_pTcRef->real.ptflow.nState |= PS_SIVFRONTLINE;
		if (_ISOCC(__SIVB) && pTcx->GetBusCondition(_REALCC(__SIVB)))
			c_pTcRef->real.ptflow.nState |= PS_SIVREARLINE;
		//c_bMf = c_bMr = c_bSf = c_bSr = FALSE;
		//if (_ISOCC(__AUXA))
		//	c_bMf = pTcx->GetBusCondition(_REALCC(__AUXA));
		//if (_ISOCC(__AUXB))
		//	c_bMr = pTcx->GetBusCondition(_REALCC(__AUXB));
		//if (_ISOCC(__SIVA))
		//	c_bSf = pTcx->GetBusCondition(_REALCC(__SIVA));
		//if (_ISOCC(__SIVB))
		//	c_bSr = pTcx->GetBusCondition(_REALCC(__SIVB));

		//if (c_bMf && !GETCBIT(__AUXA, CCDIB_ACMG)) {
		if (IsPanFront() && !GETCBIT(__AUXA, CCDIB_ACMG)) {
			CUT(__AUXA, 446);
			CUT(__AUXA, 449);
		}
		//if (c_bMr && !GETCBIT(__AUXB, CCDIB_ACMG)) {
		if (IsPanRear() && !GETCBIT(__AUXB, CCDIB_ACMG)) {
			CUT(__AUXB, 446);
			CUT(__AUXB, 449);
		}
	}
	Sheet();
	// Modified 2013/11/02
	if ((c_pTcRef->real.ptflow.sheet.state & 0xf) != c_pTcRef->real.ptflow.nIn) {
		c_pTcRef->real.ptflow.nIn = c_pTcRef->real.ptflow.sheet.state & 0xf;
		c_pTcRef->real.ptflow.nDeb = DEB_PANIN;
	}
	else if (c_pTcRef->real.ptflow.nDeb != 0 && -- c_pTcRef->real.ptflow.nDeb == 0) {
		c_pTcRef->real.ptflow.nPos = c_pTcRef->real.ptflow.nIn;
	}

	if (c_pTcRef->real.ptflow.sheet.state != 0) {
		if (c_pTcRef->real.ptflow.auxf.nState == AUXSTATE_PTNON)
			c_pTcRef->real.ptflow.auxf.nState = AUXSTATE_PTREADY;
		if (c_pTcRef->real.ptflow.auxr.nState == AUXSTATE_PTNON)
			c_pTcRef->real.ptflow.auxr.nState = AUXSTATE_PTREADY;
	}
	// else {
	else if (c_pTcRef->real.ptflow.sheet.state == 0) {
		if (c_pTcRef->real.ptflow.auxf.nState != AUXSTATE_PTNON ||
				c_pTcRef->real.ptflow.auxr.nState != AUXSTATE_PTNON) {
			if ((c_pTcRef->real.ptflow.cur.nState & PTFLOW_UP) &&
					((PANTOSTEP() == PTSTEP_MOVE) || (PANTOSTEP() == PTSTEP_MONIT))) {
			}
			else	AuxCompReset();
		}
	}
	if (IsPanFront())	AuxCompStart(0);
	if (IsPanRear())		AuxCompStart(1);
	//if (c_bMf)	AuxCompStart(0);
	//if (c_bMr)	AuxCompStart(1);
	if (c_pTcRef->real.ptflow.auxf.nState)	AuxCompCtrl(0);
	if (c_pTcRef->real.ptflow.auxr.nState)	AuxCompCtrl(1);

	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return FALSE;

	CheckSW();
	BOOL res = TRUE;
	BYTE cur;
	WORD w;
	if ((PANTOSTEP() != PTSTEP_NON) && c_pTcRef->real.ptflow.cur.wTimer == 0) {
		switch (PANTOSTEP()) {
		case PTSTEP_READY :
			// Modified 2013/11/02
			//c_pTcRef->real.ptflow.mon.nState = 0;
			c_pTcRef->real.ptflow.nState &= ~PS_MONITCATENARY;
			pTcx->InterTrigger(&c_pTcRef->real.ptflow.wMonitCatenary, 0);
			if (c_pTcRef->real.ptflow.cur.nState & PTFLOW_DOWN)
				Down();
			break;
		case PTSTEP_MOVE :
			// Modified 2013/11/02
			//c_pTcRef->real.ptflow.mon.nState = 0;
			c_pTcRef->real.ptflow.nState &= ~PS_MONITCATENARY;
			pTcx->InterTrigger(&c_pTcRef->real.ptflow.wMonitCatenary, 0);
			Off();			// turn to monitor step
			break;
		case PTSTEP_MONIT :
			Sheet();
			cur = c_pTcRef->real.ptflow.sheet.enable & (c_pTcRef->real.ptflow.sheet.cmd ^ c_pTcRef->real.ptflow.sheet.state);	// 1->different command and state
			c_pTcRef->real.ptflow.cur.nCtrl = cur ^ 0xf;
			w = c_pTcRef->real.ptflow.cur.wRunTimer;
			if (w == 0) {
				if (cur & 1)	SHOT(__AUXA, c_pTcRef->real.ptflow.sheet.cmd & 1 ? 429 : 481);
				else	CUT(__AUXA, c_pTcRef->real.ptflow.sheet.cmd & 1 ? 429 : 481);
				if (cur & 2)	SHOT(__AUXA, c_pTcRef->real.ptflow.sheet.cmd & 2 ? 430 : 482);
				else	CUT(__AUXA, c_pTcRef->real.ptflow.sheet.cmd & 2 ? 430 : 482);
				if (cur & 4)	SHOT(__AUXB, c_pTcRef->real.ptflow.sheet.cmd & 4 ? 429 : 481);
				else	CUT(__AUXB, c_pTcRef->real.ptflow.sheet.cmd & 4 ? 429 : 481);
				if (cur & 8)	SHOT(__AUXB, c_pTcRef->real.ptflow.sheet.cmd & 8 ? 430 : 482);
				else	CUT(__AUXB, c_pTcRef->real.ptflow.sheet.cmd & 8 ? 430 : 482);
			}
			if (w == 0 || cur == 0) {
				c_pTcRef->real.ptflow.cur.nState &= ~(PTFLOW_STEP);
				pTcx->InterTrigger(&c_pTcRef->real.ptflow.cur.wRunTimer, 0);
				// Modified 2013/11/02
				//c_pTcRef->real.ptflow.mon.nState = 1;
				c_pTcRef->real.ptflow.nState |= PS_MONITCATENARY;
				pTcx->InterTrigger(&c_pTcRef->real.ptflow.wMonitCatenary,
						(c_pTcRef->real.ptflow.cur.nState & PTFLOW_UP) ?
								TIME_PTCATENARYMONITUP : TIME_PTCATENARYMONITDOWN);
			}
			break;
		default :
			break;
		}
	}

	if (PANTOSTEP() == PTSTEP_NON) {
		Sheet();
		BYTE cur = c_pTcRef->real.ptflow.cur.nCtrl & (c_pTcRef->real.ptflow.sheet.cmd ^ c_pTcRef->real.ptflow.sheet.state);	// ctrl = 1 then complete up/dowm
		//if (c_bMf && IsWakeup(__AUXA)) {
		if (IsPanFront() && IsWakeup(__AUXA)) {
			if (cur & 1)
				SHOT(__AUXA, c_pTcRef->real.ptflow.sheet.cmd & 1 ? 431 : 483);
			else	CUT(__AUXA, c_pTcRef->real.ptflow.sheet.cmd & 1 ? 431 : 483);
			if (cur & 2)
				SHOT(__AUXA, c_pTcRef->real.ptflow.sheet.cmd & 2 ? 432 : 484);
			else	CUT(__AUXA, c_pTcRef->real.ptflow.sheet.cmd & 2 ? 432 : 484);
		}
		//if (c_bMr && IsWakeup(__AUXB)) {
		if (IsPanRear() && IsWakeup(__AUXB)) {
			if (cur & 4)
				SHOT(__AUXB, c_pTcRef->real.ptflow.sheet.cmd & 4 ? 431 : 483);
			else	CUT(__AUXB, c_pTcRef->real.ptflow.sheet.cmd & 4 ? 431 : 483);
			if (cur & 8)
				SHOT(__AUXB, c_pTcRef->real.ptflow.sheet.cmd & 8 ? 432 : 484);
			else	CUT(__AUXB, c_pTcRef->real.ptflow.sheet.cmd & 8 ? 432 : 484);
		}

		// Modified 2013/11/02
		//if (c_pTcRef->real.ptflow.mon.nState != 0) {
		if (c_pTcRef->real.ptflow.nState & PS_MONITCATENARY) {
			BYTE diff = 0;
			// need CheckSiv !!!!!
			//if (c_pTcRef->real.ptflow.cur.nCtrl & 3) {	// front ?
			//if ((c_pTcRef->real.ptflow.cur.nCtrl & 3) && c_bSf && IsWakeup(__SIVA)) {	// front ?
			if ((c_pTcRef->real.ptflow.cur.nCtrl & 3) && IsSivFront() && IsWakeup(__SIVA)) {	// front ?
				BYTE sv = c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.nEd;
				if (c_pTcRef->real.ptflow.sheet.cmd & 3) {	// up
					if (sv < CATENARY_PTON) {
						// Modified 2013/11/02
						//diff |= 1;
						if (!GETCBIT(__SIVA, CCDIB_IVF) && GETCBIT(__SIVA, CCDIB_IVS) && !GETCBIT(__SIVA, CCDIB_SIVFR))
							diff |= 1;
						if (c_pTcRef->real.ptflow.wMonitCatenary == 0)
							SHOT(__SIVA, 227);
					}
					else {
						if (c_pTcRef->real.ptflow.wMonitCatenary == 0)
							CUT(__SIVA, 227);
					}
				}
				else {					// down
					if (sv > CATENARY_PTOFF) {
						diff |= 4;
						if (c_pTcRef->real.ptflow.wMonitCatenary == 0)
							SHOT(__SIVA, 234);
					}
					else {
						if (c_pTcRef->real.ptflow.wMonitCatenary == 0)
							CUT(__SIVA, 234);
					}
				}
			}
			//if (c_pTcRef->real.ptflow.cur.nCtrl & 0xc) {	// rear ?
			//if ((c_pTcRef->real.ptflow.cur.nCtrl & 0xc) && c_bSr && IsWakeup(__SIVB)) {	// rear ?
			if ((c_pTcRef->real.ptflow.cur.nCtrl & 0xc) && IsSivRear() && IsWakeup(__SIVB)) {	// rear ?
				BYTE sv = c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.nEd;
				if (c_pTcRef->real.ptflow.sheet.cmd & 0xc) {	// up
					if (sv < CATENARY_PTON) {
						// Modified 2013/11/02
						//diff |= 2;
						if (!GETCBIT(__SIVB, CCDIB_IVF) && GETCBIT(__SIVB, CCDIB_IVS) && !GETCBIT(__SIVB, CCDIB_SIVFR))
							diff |= 2;
						if (c_pTcRef->real.ptflow.wMonitCatenary == 0)
							SHOT(__SIVB, 227);
					}
					else {
						if (c_pTcRef->real.ptflow.wMonitCatenary == 0)
							CUT(__SIVB, 227);
					}
				}
				else {						// down
					if (sv > CATENARY_PTOFF) {
						diff |= 8;
						if (c_pTcRef->real.ptflow.wMonitCatenary == 0)
							SHOT(__SIVB, 234);
					}
					else {
						if (c_pTcRef->real.ptflow.wMonitCatenary == 0)
							CUT(__SIVB, 234);
					}
				}
			}
			// Modified 2013/11/02
			//if (diff & 3) {		// not capture voltage at up, so down panto!!!
			//	if (c_pTcRef->real.ptflow.wMonitCatenary == 0) {
			//		if (!(c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY)) {
			//			c_pTcRef->real.ptflow.cur.nState |= PTFLOW_ABSDOWN;
			//			res = FALSE;
			//		}
			//	}
			//}
			if (diff == 0 && c_pTcRef->real.ptflow.wMonitCatenary != 0)
				pTcx->InterTrigger(&c_pTcRef->real.ptflow.wMonitCatenary, 0);
		}
	}

	if (IsPanFront() && !ScanEarthing(__AUXA))	res = FALSE;
	if (IsPanRear() && !ScanEarthing(__AUXB))	res = FALSE;
	//if (c_bMf && !ScanEarthing(__AUXA))	res = FALSE;
	//if (c_bMr && !ScanEarthing(__AUXB))	res = FALSE;
	return res;
}

void CPanto::Sheet()
{
	c_pTcRef->real.ptflow.sheet.enable = c_pTcRef->real.ptflow.sheet.cmd = c_pTcRef->real.ptflow.sheet.state = 0;
	// Modified 2013/11/02
	if (c_pTcRef->real.ptflow.cur.nState & PTFLOW_UP)	c_pTcRef->real.ptflow.sheet.cmd = 0xf;
	if (IsPanFront()) {
		c_pTcRef->real.ptflow.sheet.enable |= (FRONT_PANPS1 | FRONT_PANPS2);
		if (GETCBIT(__AUXA, CCDIB_PANPS1))	c_pTcRef->real.ptflow.sheet.state |= FRONT_PANPS1;
		if (GETCBIT(__AUXA, CCDIB_PANPS2))	c_pTcRef->real.ptflow.sheet.state |= FRONT_PANPS2;
	}
	if (IsPanRear()) {
		c_pTcRef->real.ptflow.sheet.enable |= (REAR_PANPS1 | REAR_PANPS2);
		if (GETCBIT(__AUXB, CCDIB_PANPS1))	c_pTcRef->real.ptflow.sheet.state |= REAR_PANPS1;
		if (GETCBIT(__AUXB, CCDIB_PANPS2))	c_pTcRef->real.ptflow.sheet.state |= REAR_PANPS2;
	}
	//if (c_bMf)	c_sheet.enable |= 3;
	//if (c_bMr)	c_sheet.enable |= 0xc;
	//if (c_pTcRef->real.ptflow.cur.nState & PTFLOW_UP)	c_sheet.cmd = 0xf;
	//if (c_bMf && GETCBIT(__AUXA, CCDIB_PANPS1))	c_sheet.state |= 1;
	//if (c_bMf && GETCBIT(__AUXA, CCDIB_PANPS2))	c_sheet.state |= 2;
	//if (c_bMr && GETCBIT(__AUXB, CCDIB_PANPS1))	c_sheet.state |= 4;
	//if (c_bMr && GETCBIT(__AUXB, CCDIB_PANPS2))	c_sheet.state |= 8;
}

void CPanto::CheckSW()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	// Modified 2012/02/20 ... begin
	//BYTE sw = !GETTBIT(OWN_SPACEA, TCDIB_EPANDS) ? PANTOSW_EDN : 0;
	//pTcx->TbsCtrl(sw ? TRUE : FALSE, TCDOB_HSCBPD);
	//if (GETTBIT(OWN_SPACEA, TCDIB_PANDS))	sw |= PANTOSW_DN;
	//if (GETTBIT(OWN_SPACEA, TCDIB_PANUS))	sw |= PANTOSW_UP;
	// !!!!! This action should be the same at tail !!!!!.
	BYTE sw = !GETTBIT(CAR_HEAD, TCDIB_EPANDS) ? PANTOSW_EDN : 0;
	if (c_pTcRef->real.nObligation & OBLIGATION_HEAD) {
		if (sw)	SETTBIT(OWN_SPACEA, TCDOB_EPANDNLP);
		else	CLRTBIT(OWN_SPACEA, TCDOB_EPANDNLP);
	}
	if (GETTBIT(CAR_HEAD, TCDIB_PANDS))	sw |= PANTOSW_DN;
	if (GETTBIT(CAR_HEAD, TCDIB_PANUS))	sw |= PANTOSW_UP;
	// ... end
	if (sw != c_nSwBuf) {
		c_nSwBuf = sw;
		c_nSwDeb = PANTOSW_DEB;
	}
	else if (c_nSwDeb != 0 && -- c_nSwDeb == 0) {
		BYTE psw = c_nSwBuf & ~c_nSwPrev;		// peekup only change bit
		c_nSwPrev = c_nSwBuf;
		if (!(c_pTcRef->real.ptflow.cur.nState & PTFLOW_DOWN) ||
				(PANTOSTEP() == PTSTEP_NON)) {	// not down...
			if ((psw & PANTOSW_EDN) || (psw & PANTOSW_DN) ||
					((c_pTcRef->real.ptflow.cur.nState & PTFLOW_ABSDOWN) &&
					!(c_pTcRef->real.ptflow.cur.nState & PTFLOW_DOWN))) {
				if (psw & PANTOSW_EDN) {
					SHOT(SCENE_ID_HEADCAR, 439);
					// Modified 2013/11/02
					c_pTcRef->real.ptflow.cur.nState |= PTFLOW_MDOWN;
				}
				DownPreparation();
			}
			else if ((psw & PANTOSW_UP) && !(c_nSwBuf & PANTOSW_EDN) && !(c_nSwBuf & PANTOSW_DN)) {
				int us = 0;
				// Modified 2013/11/02
				//if (c_bMf && (!GETCBIT(__AUXA, CCDIB_PANPS2) || !GETCBIT(__AUXA, CCDIB_PANPS2))) {
				if (IsPanFront() && (!(c_pTcRef->real.ptflow.nPos & FRONT_PANPS1) || !(c_pTcRef->real.ptflow.nPos & FRONT_PANPS2))) {
					if (GETCBIT(__AUXA, CCDIB_IESPE))	SHOT(__AUXA, 440);
					else {
						// Modified 2013/11/02
						//if (_SHIFTV(__SIVA) && (!pTcx->GetBusCondition(_REALCC(__SIVA)) ||
						//		!GETCBIT(__SIVA, CCDIB_IVS)))
						if (!IsSivFront() || !GETCBIT(__SIVA, CCDIB_IVS))
							SHOT(__SIVA, 441);
						else	us |= PANTOUP_FRONT;
					}
				}
				else {
					CUT(__AUXA, 440);
					CUT(__SIVA, 441);
				}
				// Modified 2013/11/02
				//if (c_bMr && (!GETCBIT(__AUXB, CCDIB_PANPS1) || !GETCBIT(__AUXB, CCDIB_PANPS2))) {
				if (IsPanRear() && (!(c_pTcRef->real.ptflow.nPos & REAR_PANPS1) || !(c_pTcRef->real.ptflow.nPos & REAR_PANPS2))) {
					if (GETCBIT(__AUXB, CCDIB_IESPE))	SHOT(__AUXB, 440);
					else {
						// Modified 2013/11/02
						//if (_SHIFTV(__SIVB) && (!pTcx->GetBusCondition(_REALCC(__SIVB)) ||
						//		!GETCBIT(__SIVB, CCDIB_IVS)))
						if (!IsSivRear() || !GETCBIT(__SIVB, CCDIB_IVS))
							SHOT(__SIVB, 441);
						else	us |= PANTOUP_REAR;
					}
				}
				else {
					CUT(__AUXB, 440);
					CUT(__SIVB, 441);
				}
				if (us != 0) {
					PrevUp(us);
					CUT(SCENE_ID_HEADCAR, 40);
					CUT(SCENE_ID_HEADCAR, 439);
				}
				else	SHOT(SCENE_ID_HEADCAR, 40);
				AuxCompReadyA();
			}
		}
		else {
		}
	}
	// Modified 2013/11/02
	//else if (c_pTcRef->real.ptflow.cur.nState & PTFLOW_RUP) {
	//	// this lines are skiped by prevup()
	//	if ((!(c_nUp & PANTOUP_FRONT) || (_ISOCC(__AUXA) && !GETCBIT(__AUXA, CCDIB_ACMG))) &&
	//		(!(c_nUp & PANTOUP_REAR) || (_ISOCC(__AUXB) && !GETCBIT(__AUXB, CCDIB_ACMG)))) {
	//		c_pTcRef->real.ptflow.mon.nState = 0;
	//		Up(c_nUp);
	//	}
	//}
	//else {
	//}
	CheckAcmcs();
}

void CPanto::CheckAcmcs()
{
	CTcx* pTcx = (CTcx*)c_pParent;

	// Modified 2013/11/02
	//if (c_bMf && GETCBIT(__AUXA, CCDIB_ACMCS)) {
	if (IsPanFront() && GETCBIT(__AUXA, CCDIB_ACMCS)) {
		if (!(c_pTcRef->real.ptflow.auxf.nCtrl & COMPCTRLBY_ACMCS)) {
			CompCtrl(0, COMPONBY_ACMCS);
			pTcx->InterTrigger(&c_pTcRef->real.ptflow.auxf.wMonit, TIME_PTAUXMONITSW);
			SHOT(__AUXA, 447);
		}
	}
	else {
		if (c_pTcRef->real.ptflow.auxf.nCtrl & COMPCTRLBY_ACMCS) {
			CompCtrl(0, COMPOFFBY_ACMCS);
			pTcx->InterTrigger(&c_pTcRef->real.ptflow.auxf.wMonit, 0);
			CUT(__AUXA, 447);
			CUT(__AUXA, 448);
		}
	}

	// Modified 2013/11/02
	//if (c_bMr && GETCBIT(__AUXB, CCDIB_ACMCS)) {
	if (IsPanRear() && GETCBIT(__AUXB, CCDIB_ACMCS)) {
		if (!(c_pTcRef->real.ptflow.auxr.nCtrl & COMPCTRLBY_ACMCS)) {
			CompCtrl(1, COMPONBY_ACMCS);
			pTcx->InterTrigger(&c_pTcRef->real.ptflow.auxr.wMonit, TIME_PTAUXMONITSW);
			SHOT(__AUXB, 447);
		}
	}
	else {
		if (c_pTcRef->real.ptflow.auxr.nCtrl & COMPCTRLBY_ACMCS) {
			CompCtrl(1, COMPOFFBY_ACMCS);
			pTcx->InterTrigger(&c_pTcRef->real.ptflow.auxr.wMonit, 0);
			CUT(__AUXB, 447);
			CUT(__AUXB, 448);
		}
	}

	if (c_pTcRef->real.ptflow.auxf.nCtrl & COMPCTRLBY_ACMCS) {
		if (c_pTcRef->real.ptflow.auxf.wMonit == 0) {
			pTcx->InterTrigger(&c_pTcRef->real.ptflow.auxf.wMonit, TIME_PTAUXMONITSW);
			SHOT(__AUXA, 448);
		}
	}
	if (c_pTcRef->real.ptflow.auxr.nCtrl & COMPCTRLBY_ACMCS) {
		if (c_pTcRef->real.ptflow.auxr.wMonit == 0) {
			pTcx->InterTrigger(&c_pTcRef->real.ptflow.auxf.wMonit, TIME_PTAUXMONITSW);
			SHOT(__AUXB, 448);
		}
	}
}

BOOL CPanto::PrevUp(BYTE state)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	// Modified 2013/11/02
	//c_nUp = state;
	//c_pTcRef->real.ptflow.mon.nState = 0;
	//Up(c_nUp);
	c_pTcRef->real.ptflow.nState &= ~PS_MONITCATENARY;
	Up(state);
	//BYTE who = c_nUp;
	//BOOL bRes = TRUE;
	//if ((c_nUp & PANTOUP_FRONT) && c_bMf && (GETCBIT(__AUXA, CCDIB_ACMG))) {
	//	if (!AuxCompStart(0))	bRes = FALSE;
	//	else	c_pTcRef->real.ptflow.cur.nState |= PTFLOW_RUP;
	//	who &= ~PANTOUP_FRONT;
	//}
	//if ((c_nUp & PANTOUP_REAR) && c_bMr && (GETCBIT(__AUXB, CCDIB_ACMG))) {
	//	if (!AuxCompStart(1))	bRes = FALSE;
	//	else	c_pTcRef->real.ptflow.cur.nState |= PTFLOW_RUP;
	//	who &= ~PANTOUP_REAR;
	//}
	//if (who == c_nUp) {
	//	c_pTcRef->real.ptflow.mon.nState = 0;
	//	Up(who);
	//}
	CUT(__AUXA, 227);
	CUT(__AUXB, 227);
	return TRUE;
}

void CPanto::DownPreparation()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (_ISOCC(__AUXA))
		CLROBIT(OWN_SPACEA, __AUXA, CCDOB_PANUR);
	if (_ISOCC(__AUXB))
		CLROBIT(OWN_SPACEA, __AUXB, CCDOB_PANUR);
	c_pDoz->tcs[OWN_SPACEA].real.cV3f.wPanto = _SHIFTV(__V3FA) | _SHIFTV(__V3FB) | _SHIFTV(__V3FC) | _SHIFTV(__V3FD);
	c_pDoz->tcs[OWN_SPACEA].real.cSiv.wPanto = _SHIFTV(__SIVA) | _SHIFTV(__SIVB);
	c_pTcRef->real.ptflow.cur.nState &= ~(PTFLOW_STEP | PTFLOW_UP | PTFLOW_ABSDOWN);	// | PTFLOW_RUP); never ptflow_mdown!!!
	c_pTcRef->real.ptflow.cur.nState |= (PTSTEP_READY | PTFLOW_DOWN);
	pTcx->InterTrigger(&c_pTcRef->real.ptflow.cur.wTimer, TIME_PTDOWNREADY);
}

void CPanto::Down()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDoz->tcs[OWN_SPACEA].real.cV3f.wPanto = 0;
	c_pDoz->tcs[OWN_SPACEA].real.cSiv.wPanto = 0;
	if (_ISOCC(__AUXA)) {
		CLROBIT(OWN_SPACEA, __AUXA, CCDOB_PANUR);
		// Modified 2013/11/02
		if (c_pTcRef->real.ptflow.cur.nState & PTFLOW_MDOWN)
			CLROBIT(OWN_SPACEA, __AUXA, CCDOB_PANUR);
		else	SETOBIT(OWN_SPACEA, __AUXA, CCDOB_PANDR);
	}
	if (_ISOCC(__AUXB)) {
		CLROBIT(OWN_SPACEA, __AUXB, CCDOB_PANUR);
		// Modified 2013/11/02
		if (c_pTcRef->real.ptflow.cur.nState & PTFLOW_MDOWN)
			CLROBIT(OWN_SPACEA, __AUXB, CCDOB_PANUR);
		else	SETOBIT(OWN_SPACEA, __AUXB, CCDOB_PANDR);
	}
	c_pTcRef->real.ptflow.cur.nState &= ~(PTFLOW_STEP | PTFLOW_UP | PTFLOW_ABSDOWN | PTFLOW_MDOWN);	// | PTFLOW_RUP);
	c_pTcRef->real.ptflow.cur.nState |= (PTSTEP_MOVE | PTFLOW_DOWN);
	pTcx->InterTrigger(&c_pTcRef->real.ptflow.cur.wTimer, TIME_PTDOWNHOLD);
}

void CPanto::Up(BYTE state)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (_ISOCC(__AUXA))
		CLROBIT(OWN_SPACEA, __AUXA, CCDOB_PANDR);
	if (_ISOCC(__AUXB))
		CLROBIT(OWN_SPACEA, __AUXB, CCDOB_PANDR);
	if ((state & PANTOUP_FRONT) && _ISOCC(__AUXA))
		SETOBIT(OWN_SPACEA, __AUXA, CCDOB_PANUR);
	if ((state & PANTOUP_REAR) && _ISOCC(__AUXB))
		SETOBIT(OWN_SPACEA, __AUXB, CCDOB_PANUR);
	c_pTcRef->real.ptflow.cur.nState &= ~(PTFLOW_STEP | PTFLOW_DOWN | PTFLOW_ABSDOWN | PTFLOW_MDOWN);	// | PTFLOW_RUP);
	c_pTcRef->real.ptflow.cur.nState |= (PTSTEP_MOVE | PTFLOW_UP);
	pTcx->InterTrigger(&c_pTcRef->real.ptflow.cur.wTimer, TIME_PTUPHOLD);
	CUT(__SIVA, 227);
	CUT(__SIVB, 227);
}

void CPanto::Off()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (_ISOCC(__AUXA)) {
		CLROBIT(OWN_SPACEA, __AUXA, CCDOB_PANDR);
		CLROBIT(OWN_SPACEA, __AUXA, CCDOB_PANUR);
	}
	if (_ISOCC(__AUXB)) {
		CLROBIT(OWN_SPACEA, __AUXB, CCDOB_PANDR);
		CLROBIT(OWN_SPACEA, __AUXB, CCDOB_PANUR);
	}
	c_pTcRef->real.ptflow.cur.nState &= ~(PTFLOW_STEP | PTFLOW_ABSDOWN | PTFLOW_MDOWN);	// | PTFLOW_RUP);
	c_pTcRef->real.ptflow.cur.nState |= PTSTEP_MONIT;	// ?????VERY IMPORTANT?????
	pTcx->InterTrigger(&c_pTcRef->real.ptflow.cur.wTimer, TIME_PTMONIT);
	if (c_pTcRef->real.ptflow.cur.nState & PTFLOW_UP)
		pTcx->InterTrigger(&c_pTcRef->real.ptflow.cur.wRunTimer, TIME_PTUPMONIT);
	else	pTcx->InterTrigger(&c_pTcRef->real.ptflow.cur.wRunTimer, 0);
}

void CPanto::AuxCompReady()
{
	if (c_pTcRef->real.ptflow.auxf.nState != AUXSTATE_PTWAITACMK &&
			c_pTcRef->real.ptflow.auxf.nState != AUXSTATE_PTWAITUNACMG &&
			c_pTcRef->real.ptflow.auxf.nState != AUXSTATE_PTWAITUNACMK &&
			c_pTcRef->real.ptflow.auxf.nState != AUXSTATE_PTWAITUNACMKFAIL)
		c_pTcRef->real.ptflow.auxf.nState = AUXSTATE_PTREADY;

	if (c_pTcRef->real.ptflow.auxr.nState != AUXSTATE_PTWAITACMK &&
			c_pTcRef->real.ptflow.auxr.nState != AUXSTATE_PTWAITUNACMG &&
			c_pTcRef->real.ptflow.auxr.nState != AUXSTATE_PTWAITUNACMK &&
			c_pTcRef->real.ptflow.auxr.nState != AUXSTATE_PTWAITUNACMKFAIL)
		c_pTcRef->real.ptflow.auxr.nState = AUXSTATE_PTREADY;
}

void CPanto::AuxCompReadyA()
{
	if (c_pTcRef->real.ptflow.auxf.nState != AUXSTATE_PTWAITACMK &&
			c_pTcRef->real.ptflow.auxf.nState != AUXSTATE_PTWAITUNACMG &&
			c_pTcRef->real.ptflow.auxf.nState != AUXSTATE_PTWAITUNACMK)
		c_pTcRef->real.ptflow.auxf.nState = AUXSTATE_PTREADY;

	if (c_pTcRef->real.ptflow.auxr.nState != AUXSTATE_PTWAITACMK &&
			c_pTcRef->real.ptflow.auxr.nState != AUXSTATE_PTWAITUNACMG &&
			c_pTcRef->real.ptflow.auxr.nState != AUXSTATE_PTWAITUNACMK)
		c_pTcRef->real.ptflow.auxr.nState = AUXSTATE_PTREADY;
}

BOOL CPanto::AuxCompStart(UCURV id)
{
	PPTTYPE2 pState = id ? &c_pTcRef->real.ptflow.auxr : &c_pTcRef->real.ptflow.auxf;
	// Modified 2013/11/02
	UCURV car;
	BOOL line;
	if (id) {
		car = __AUXB;
		line = IsPanRear() ? TRUE : FALSE;
	}
	else {
		car = __AUXA;
		line = IsPanFront() ? TRUE : FALSE;
	}
	//UCURV car = id ? __AUXB : __AUXA;
	//BOOL line = id ? c_bMr : c_bMf;
	CTcx* pTcx = (CTcx*)c_pParent;
	if (line && car < 0x10 && pState->nState == AUXSTATE_PTREADY && GETCBIT(car, CCDIB_ACMG)) {
		// ACMG=1, low pressure
		CompCtrl(id, COMPONBY_PANTO);
		CUT(car, 444);
		pState->nState = AUXSTATE_PTWAITACMK;
		CTcx* pTcx = (CTcx*)c_pParent;
		pTcx->InterTrigger(&pState->wTimer, TIME_PTAUXCOMPCONTACT);
		CUT(car, 493);
		return TRUE;
	}
	SHOT(car, 493);
	return FALSE;
}

BOOL CPanto::AuxCompCtrl(UCURV id)
{
	PPTTYPE2 pState = id ? &c_pTcRef->real.ptflow.auxr : &c_pTcRef->real.ptflow.auxf;
	// Modified 2013/11/02
	UCURV car;
	BOOL line;
	if (id) {
		car = __AUXB;
		line = IsPanRear() ? TRUE : FALSE;
	}
	else {
		car = __AUXA;
		line = IsPanFront() ? TRUE : FALSE;
	}
	//UCURV car = id ? __AUXB : __AUXA;
	//BOOL line = id ? c_bMr : c_bMf;
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (pState->nState) {
	case AUXSTATE_PTWAITACMK :		// 1
		if (line && car < 0x10 && GETCBIT(car, CCDIB_ACMKR)) {
			pState->nState = AUXSTATE_PTWAITUNACMG;
			pTcx->InterTrigger(&pState->wTimer, TIME_PTAUXCOMPPRESSURE);
			pTcx->InterTrigger(&pState->wRunTimer, TIME_PTAUXCOMPONMAX);
			CUT(car, 445);
			return TRUE;
		}
		break;
	case AUXSTATE_PTWAITUNACMG :	// 2
		if (line && car < 0x10 && !GETCBIT(car, CCDIB_ACMG)) {
			// ACMG=0, full pressure
			CompCtrl(id, COMPOFFBY_PANTO);
			CUT(car, 445);
			CUT(car, 446);
			CUT(car, 449);
			pState->nState = AUXSTATE_PTWAITUNACMK;
			pTcx->InterTrigger(&pState->wRunTimer, 0);
			pTcx->InterTrigger(&pState->wTimer, TIME_PTAUXCOMPRELEASE);
			return TRUE;
		}
		else if (pState->wRunTimer == 0) {
			if (car < 0x10) {
				CompCtrl(id, COMPOFFBY_PANTO);
				CUT(car, 445);
			}
			CompCtrl(id, COMPOFFBY_PANTO);
			pState->nState = AUXSTATE_PTWAITUNACMKFAIL;
			pTcx->InterTrigger(&pState->wTimer, TIME_PTAUXCOMPRELEASE);
			SHOT(car, 449);
			return TRUE;
		}
		break;
	case AUXSTATE_PTWAITUNACMK :		// 3
	case AUXSTATE_PTWAITUNACMKFAIL :	// 4
		if (line && car < 0x10 && !GETCBIT(car, CCDIB_ACMKR)) {
			pState->nState = pState->nState == AUXSTATE_PTWAITUNACMK ?
					AUXSTATE_PTREADY : AUXSTATE_PTFAILED;
			pTcx->InterTrigger(&pState->wTimer, 0);
			CUT(car, 444);
			return TRUE;
		}
		break;
	case AUXSTATE_PTFAILED :
		// wait panto up s/w
		return TRUE;
		break;
	default :
		return TRUE;
		break;
	}
	if (pState->wTimer != 0)	return TRUE;

	switch (pState->nState) {
	case AUXSTATE_PTWAITUNACMK :
		SHOT(car, 444);
		pTcx->InterTrigger(&pState->wTimer, TIME_PTAUXCOMPRELEASE);
		break;
	case AUXSTATE_PTWAITACMK :
		SHOT(car, 445);
		pTcx->InterTrigger(&pState->wTimer, TIME_PTAUXCOMPCONTACT);
		break;
	case AUXSTATE_PTWAITUNACMG :
		SHOT(car, 446);
		pTcx->InterTrigger(&pState->wTimer, TIME_PTAUXCOMPPRESSURE);
		break;
	default :
		return TRUE;
		break;
	}
	return FALSE;
}

void CPanto::AuxCompReset()
{
	CompCtrl(0, COMPOFFBY_XCHG);
	CompCtrl(1, COMPOFFBY_XCHG);
	c_pTcRef->real.ptflow.auxf.nState =
	c_pTcRef->real.ptflow.auxr.nState = AUXSTATE_PTNON;
	c_pTcRef->real.ptflow.auxf.nCtrl =
	c_pTcRef->real.ptflow.auxr.nCtrl = 0;
}

void CPanto::CompCtrl(UCURV id, UCURV cmd)
{
	PPTTYPE2 pState = id ? &c_pTcRef->real.ptflow.auxr : &c_pTcRef->real.ptflow.auxf;
	UCURV car = id ?__AUXB : __AUXA;
	switch (cmd) {
	case COMPONBY_PANTO :
		if (!(pState->nCtrl & COMPCTRLBY_PANTO)) {
			pState->nCtrl |= COMPCTRLBY_PANTO;
			if (!(pState->nCtrl & COMPCTRLBY_ACMCS))
				SETOBIT(OWN_SPACEA, car, CCDOB_ACMKR);
		}
		break;
	case COMPONBY_ACMCS :
		if (!(pState->nCtrl & COMPCTRLBY_ACMCS)) {
			pState->nCtrl |= COMPCTRLBY_ACMCS;
			if (!(pState->nCtrl & COMPCTRLBY_PANTO))
				SETOBIT(OWN_SPACEA, car, CCDOB_ACMKR);
		}
		break;
	case COMPOFFBY_PANTO :
		pState->nCtrl &= ~COMPCTRLBY_PANTO;
		if (!(pState->nCtrl & COMPCTRLBY_ACMCS))
			CLROBIT(OWN_SPACEA, car, CCDOB_ACMKR);
		break;
	case COMPOFFBY_ACMCS :
		pState->nCtrl &= ~COMPCTRLBY_ACMCS;
		if (!(pState->nCtrl & COMPCTRLBY_PANTO))
			CLROBIT(OWN_SPACEA, car, CCDOB_ACMKR);
		break;
	case COMPOFFBY_XCHG :
		CLROBIT(OWN_SPACEA, car, CCDOB_ACMKR);
		break;
	default :
		break;
	}
}

BOOL CPanto::ScanEarthing(UCURV cid)
{
	UCURV flag = GETCBIT(cid, CCDIB_IESPS) ? 1 : 0;
	if (GETCBIT(cid, CCDIB_IESPE))	flag |= 2;
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (flag) {
	case 3 :
		CUTS(cid, 332, 333);
		SHOT(cid, 331);
		return FALSE;
		break;
	case 2 :
		CUT(cid, 331);
		CUT(cid, 333);
		SHOT(cid, 332);
		return FALSE;
		break;
	case 0 :
		CUTS(cid, 331, 332);
		SHOT(cid, 333);
		return FALSE;
		break;
	default :
		CUT(cid, 333);
		CUT(cid, 332);
		CUT(cid, 331);
		return TRUE;
		break;
	}
}

void CPanto::Primary()
{
	if (!(c_pTcRef->real.ptflow.nState & PS_ENABLEALARM)) {
		if (IsPantoUp())
			c_pTcRef->real.ptflow.nState |= PS_ENABLEALARM;
	}
}
