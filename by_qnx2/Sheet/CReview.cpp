/*
 * CReview.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <string.h>

#include "../Inform2/DevInfo2/HduInfo.h"
#include "CPaper.h"
#include "CReview.h"

CReview::CReview()
{
	memset(&c_insp, 0, sizeof(INSPCMD));
}

CReview::~CReview()
{
}

// !!!CAUTION : inspect에 관계되는 시간은 모두 초 단위라야 한다!!!
void CReview::SetInspectTime()
{
	switch (c_insp.wStep) {
	case INSPSTEP_DTBCHECK :	c_insp.wTime = 15;	break;
	case INSPSTEP_POLECHECK :	c_insp.wTime = 5;	break;
	case INSPSTEP_LOCALCHECK :	c_insp.wTime = 15;	break;
	case INSPSTEP_ECUB0CHECK :	c_insp.wTime = 10;	break;
	case INSPSTEP_ECUB7CHECK :	c_insp.wTime = 10;	break;
	case INSPSTEP_ECUEBCHECK :	c_insp.wTime = 10;	break;
	case INSPSTEP_ECUASCHECK :	c_insp.wTime = 90;	break;		//10;	break;	// 170729
	case INSPSTEP_V3FCHECK :	c_insp.wTime = 10;	break;
	case INSPSTEP_SIVCHECK :	c_insp.wTime = 10;	break;
	case INSPSTEP_COOLCHECK :	c_insp.wTime = 120;	break;		//300;	break;	// 170817
	case INSPSTEP_HEATCHECK :	c_insp.wTime = 120;	break;		//300;	break;

	//case INSPSTEP_ECUB0ENTRY :	case INSPSTEP_ECUB7ENTRY :	case INSPSTEP_ECUEBENTRY :	case INSPSTEP_ECUASENTRY :
	//case INSPSTEP_V3FENTRY :	case INSPSTEP_SIVENTRY :
	//case INSPSTEP_COOLENTRY :	case INSPSTEP_HEATENTRY :	c_insp.wTime = 0;	break;

	case INSPSTEP_DTBJUDGE :	case INSPSTEP_POLEJUDGE :	case INSPSTEP_LOCALJUDGE :
	case INSPSTEP_ECUB0JUDGE :	case INSPSTEP_ECUB7JUDGE :	case INSPSTEP_ECUEBJUDGE :	case INSPSTEP_ECUASJUDGE :
	case INSPSTEP_V3FJUDGE :	case INSPSTEP_SIVJUDGE :
	case INSPSTEP_COOLJUDGE :	case INSPSTEP_HEATJUDGE :
		c_insp.wTime = 5;	break;	//REALBYSPREAD(40000);	break;
	default :	c_insp.wTime = 0;	break;
	}
}

int CReview::GetInspectHduPage()
{
	GETPAPER(pPaper);
	switch (c_insp.wStep) {
	case INSPSTEP_RESET :
	case INSPSTEP_DTBREADY :
	case INSPSTEP_DTBCHECK :
	case INSPSTEP_DTBJUDGE :	return DTBCHECK_PAGE;	break;
	case INSPSTEP_POLEREADY :
	case INSPSTEP_POLECHECK :
	case INSPSTEP_POLEJUDGE :
		// 200218
		if (pPaper->GetDeviceExFromRecip())	return POLEEXCHECK_PAGE;
		return POLECHECK_PAGE;
		break;
	case INSPSTEP_LOCALREADY :
	case INSPSTEP_LOCALCHECK :
	case INSPSTEP_LOCALJUDGE :
		// 200218
		if (pPaper->GetDeviceExFromRecip())	return LOCALEXCHECK_PAGE;
		return LOCALCHECK_PAGE;
		break;
	case INSPSTEP_ECUB0READY :
	case INSPSTEP_ECUB0ENTRY :
	case INSPSTEP_ECUB0CHECK :
	case INSPSTEP_ECUB0JUDGE :
	case INSPSTEP_ECUB7READY :
	case INSPSTEP_ECUB7ENTRY :
	case INSPSTEP_ECUB7CHECK :
	case INSPSTEP_ECUB7JUDGE :
	case INSPSTEP_ECUEBREADY :
	case INSPSTEP_ECUEBENTRY :
	case INSPSTEP_ECUEBCHECK :
	case INSPSTEP_ECUEBJUDGE :
	case INSPSTEP_ECUASREADY :
	case INSPSTEP_ECUASENTRY :
	case INSPSTEP_ECUASCHECK :
	case INSPSTEP_ECUASJUDGE :	return ECUCHECK_PAGE;	break;
	case INSPSTEP_V3FREADY :
	case INSPSTEP_V3FIGNITE :
	case INSPSTEP_V3FENTRY :
	case INSPSTEP_V3FCHECK :
	case INSPSTEP_V3FJUDGE :	return V3FCHECK_PAGE;	break;
	case INSPSTEP_SIVREADY :
	case INSPSTEP_SIVENTRY :
	case INSPSTEP_SIVCHECK :
	case INSPSTEP_SIVJUDGE :	return SIVCHECK_PAGE;	break;
	case INSPSTEP_COOLREADY :
	case INSPSTEP_COOLENTRY :
	case INSPSTEP_COOLCHECK :
	case INSPSTEP_COOLJUDGE :	return COOLCHECK_PAGE;	break;
	case INSPSTEP_HEATREADY :
	case INSPSTEP_HEATENTRY :
	case INSPSTEP_HEATCHECK :
	case INSPSTEP_HEATJUDGE :	return HEATCHECK_PAGE;	break;
	default :	break;
	}
	return 0;
}

void CReview::InspectRegister(WORD wItem)
{
	if (c_insp.wItem != 0)	return;

	c_insp.wTime = 0;
	c_insp.w10m = 0;
	c_insp.wHold = 0;
	c_insp.bHold = false;
	c_insp.wStep = INSPSTEP_RESET;
	c_insp.wItem = wItem;
}

void CReview::InspectClear()
{
	memset(&c_insp, 0, sizeof(INSPCMD));
}

int CReview::NextInspectStep()
{
	WORD wExpect = 0;
	switch (c_insp.wStep) {
	case INSPSTEP_RESET :
		wExpect = (1 << INSPITEM_TCL) | (1 << INSPITEM_ECU) | (1 << INSPITEM_V3F) | (1 << INSPITEM_SIV) | (1 << INSPITEM_COOL) | (1 << INSPITEM_HEAT);
		break;
	case INSPSTEP_LOCALJUDGE :
		wExpect =  (1 << INSPITEM_ECU) | (1 << INSPITEM_V3F) | (1 << INSPITEM_SIV) | (1 << INSPITEM_COOL) | (1 << INSPITEM_HEAT);
		break;
	case INSPSTEP_ECUASJUDGE :
		wExpect = (1 << INSPITEM_V3F) | (1 << INSPITEM_SIV) | (1 << INSPITEM_COOL) | (1 << INSPITEM_HEAT);
		break;
	case INSPSTEP_V3FJUDGE :
		wExpect = (1 << INSPITEM_SIV) | (1 << INSPITEM_COOL) | (1 << INSPITEM_HEAT);
		break;
	case INSPSTEP_SIVJUDGE :
		wExpect = (1 << INSPITEM_COOL) | (1 << INSPITEM_HEAT);
		break;
	case INSPSTEP_COOLJUDGE :	case INSPSTEP_HEATJUDGE :
		c_insp.wStep = INSPSTEP_ENDREADY;
		break;
	default :
		if (c_insp.wStep < INSPSTEP_HEATJUDGE)	++ c_insp.wStep;
		else	c_insp.wStep = INSPSTEP_ENDREADY;	//INSPSTEP_NON;
		break;
	}
	if (wExpect != 0) {
		wExpect &= c_insp.wItem;
		if (wExpect & (1 << INSPITEM_TCL))		c_insp.wStep = INSPSTEP_DTBREADY;
		else if (wExpect & (1 << INSPITEM_ECU))	c_insp.wStep = INSPSTEP_ECUB0READY;
		else if (wExpect & (1 << INSPITEM_V3F))	c_insp.wStep = INSPSTEP_V3FREADY;
		else if (wExpect & (1 << INSPITEM_SIV))	c_insp.wStep = INSPSTEP_SIVREADY;
		else if (wExpect & (1 << INSPITEM_COOL))	c_insp.wStep = INSPSTEP_COOLREADY;
		else if (wExpect & (1 << INSPITEM_HEAT))	c_insp.wStep = INSPSTEP_HEATREADY;
		else	c_insp.wStep = INSPSTEP_ENDREADY;
	}
	SetInspectTime();
	return GetInspectHduPage();
}

void CReview::InspectPause()
{
	c_insp.wHold = c_insp.wStep;
	c_insp.bHold = true;
	c_insp.wStep = INSPSTEP_PAUSE;
}

void CReview::InspectRetry()
{
	if (c_insp.wHold <= INSPSTEP_DTBJUDGE)	c_insp.wStep = INSPSTEP_DTBREADY;
	else if (c_insp.wHold <= INSPSTEP_POLEJUDGE)	c_insp.wStep = INSPSTEP_POLEREADY;
	else if (c_insp.wHold <= INSPSTEP_LOCALJUDGE)	c_insp.wStep = INSPSTEP_LOCALREADY;
	else if (c_insp.wHold <= INSPSTEP_ECUB0JUDGE)	c_insp.wStep = INSPSTEP_ECUB0READY;
	else if (c_insp.wHold <= INSPSTEP_ECUB7JUDGE)	c_insp.wStep = INSPSTEP_ECUB7READY;
	else if (c_insp.wHold <= INSPSTEP_ECUEBJUDGE)	c_insp.wStep = INSPSTEP_ECUEBREADY;
	else if (c_insp.wHold <= INSPSTEP_ECUASJUDGE)	c_insp.wStep = INSPSTEP_ECUASREADY;
	else if (c_insp.wHold <= INSPSTEP_V3FJUDGE)		c_insp.wStep = INSPSTEP_V3FREADY;
	else if (c_insp.wHold <= INSPSTEP_SIVJUDGE)		c_insp.wStep = INSPSTEP_SIVREADY;
	else if (c_insp.wHold <= INSPSTEP_COOLJUDGE)	c_insp.wStep = INSPSTEP_COOLREADY;
	else if (c_insp.wHold <= INSPSTEP_HEATJUDGE)	c_insp.wStep = INSPSTEP_HEATREADY;
	else	c_insp.wStep = INSPSTEP_RESET;
	c_insp.wHold = 0;
	c_insp.bHold = false;
}

void CReview::InspectSkip()
{
	if (c_insp.wHold <= INSPSTEP_DTBJUDGE)	c_insp.wStep = INSPSTEP_POLEREADY;
	else if (c_insp.wHold >= INSPSTEP_POLEREADY && c_insp.wHold <= INSPSTEP_POLEJUDGE)	c_insp.wStep = INSPSTEP_LOCALREADY;
	else if (c_insp.wHold >= INSPSTEP_ECUB0READY && c_insp.wHold <= INSPSTEP_ECUB0JUDGE)	c_insp.wStep = INSPSTEP_ECUB7READY;
	else if (c_insp.wHold >= INSPSTEP_ECUB7READY && c_insp.wHold <= INSPSTEP_ECUB7JUDGE)	c_insp.wStep = INSPSTEP_ECUEBREADY;
	else if (c_insp.wHold >= INSPSTEP_ECUEBREADY && c_insp.wHold <= INSPSTEP_ECUEBJUDGE)	c_insp.wStep = INSPSTEP_ECUASREADY;
	else if (c_insp.wHold >= INSPSTEP_COOLREADY && c_insp.wHold <= INSPSTEP_HEATJUDGE)		c_insp.wStep = INSPSTEP_ENDREADY;
	else {
		WORD w = c_insp.wItem;
		if (c_insp.wHold <= INSPSTEP_LOCALJUDGE)	w &= ~(1 << INSPITEM_TCL);
		else if (c_insp.wHold <= INSPSTEP_ECUASJUDGE)	w &= ~((1 << INSPITEM_TCL) | (1 << INSPITEM_ECU));
		else if (c_insp.wHold <= INSPSTEP_V3FJUDGE)		w &= ~((1 << INSPITEM_TCL) | (1 << INSPITEM_ECU) | (1 << INSPITEM_V3F));
		//else if (c_insp.wHold <= INSPSTEP_SIVJUDGE)		w &= ~((1 << INSPITEM_TCL) | (1 << INSPITEM_ECU) | (1 << INSPITEM_V3F) | (1 << INSPITEM_SIV));
		else 	w &= ~((1 << INSPITEM_TCL) | (1 << INSPITEM_ECU) | (1 << INSPITEM_V3F) | (1 << INSPITEM_SIV));

		if (w & (1 << INSPITEM_ECU))	c_insp.wStep = INSPSTEP_ECUB0READY;
		else if (w & (1 << INSPITEM_V3F))	c_insp.wStep = INSPSTEP_V3FREADY;
		else if (w & (1 << INSPITEM_SIV))	c_insp.wStep = INSPSTEP_SIVREADY;
		else if (w & (1 << INSPITEM_COOL))	c_insp.wStep = INSPSTEP_COOLREADY;
		else if (w & (1 << INSPITEM_HEAT))	c_insp.wStep = INSPSTEP_HEATREADY;
		else	c_insp.wStep = INSPSTEP_ENDREADY;
	}
	c_insp.wHold = 0;
	c_insp.bHold = false;
}

bool CReview::GetInspectLapse()
{
	return c_insp.wTime == 0 ? true : false;
}

WORD CReview::ExcerptDuo(WORD w, int shift)
{
	return ((w >> shift) & 3);
}

DWORD CReview::ExcerptDuo(DWORD dw, int shift)
{
	return ((dw >> shift) & 3);
}

void CReview::CoordDuo(WORD* pW, WORD duo, int shift)
{
	*pW &= (WORD)~(3 << shift);
	*pW |= (duo << shift);
}

void CReview::CoordDuo(DWORD* pDw, DWORD duo, int shift)
{
	*pDw &= (DWORD)~(3 << shift);
	*pDw |= (duo << shift);
}

//void CReview::CopyInspectCmd(PINSPCMD pInsp)
//{
//	memcpy(pInsp, &c_insp, sizeof(INSPCMD));
//}
//
void CReview::Arteriam()
{
	// 19/12/05
	//if (c_insp.wItem != 0 && c_insp.wTime > 0)
	if (c_insp.wItem != 0 && c_insp.wTime > 0 && !c_insp.bHold) {
		if (++ c_insp.w10m >= 100) {
			c_insp.w10m = 0;
			-- c_insp.wTime;
		}
	}
}

ENTRY_CONTAINER(CReview)
	SCOOP(&c_insp,					sizeof(INSPCMD),	"Rev")
EXIT_CONTAINER()
