/*
 * CPaper.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>

#include "Track.h"
#include "CPaper.h"

CPaper::CPaper()
{
	c_cFloatID = 0xff;
	c_uOblige = 0;
	c_cLength = DEFAULT_CARLENGTH;
	c_cCarID = 10;
	c_wPermNo = 0;
	for (int n = 0; n < PI_MAX; n ++) {
		c_pallets[n].dwRoute = 0;
		c_pallets[n].pDoz = NULL;
	}
	//c_dwCurRoute = 0;
	c_bInLab = false;
	c_bDevEx = false;
	c_bRecipDevEx = false;
	Initial();
}

CPaper::~CPaper()
{
	for (int n = 0; n < PI_MAX; n ++) {
		if (c_pallets[n].pDoz != NULL) {
			delete c_pallets[n].pDoz;
			c_pallets[n].pDoz = NULL;
		}
	}
}

void CPaper::Initial()
{
	c_wWakeupDtb = CYCLE_DTBWAKEUP;
	for (int n = 0; n < PI_MAX; n ++)
		c_pallets[n].dwRoute = 0;
	for (int n = 0; n < FID_MAX; n ++) {
		c_wRed[n] = 0;
		c_iDebs[n] = 0;
		c_wWakeupEach[n] = CYCLE_EACHWAKEUP;
	}
}

void CPaper::Setup(BYTE cOblige, BYTE cLength)
{
	c_uOblige = cOblige;
	c_cFloatID = (BYTE)(~c_uOblige & OBLIGEBF_ONLYPOS);

	SetLength(cLength);
	for (int n = 0; n < PI_MAX; n ++) {
		if (c_pallets[n].pDoz == NULL)
			c_pallets[n].pDoz = new DOZEN;
		memset(c_pallets[n].pDoz, 0, sizeof(DOZEN));
		c_pallets[n].dwRoute = 0;
	}
	TRACK("PAPR:setup to %d (0x%02x)\n", c_cFloatID, c_uOblige);
}

void CPaper::SetOblige(BYTE cObID, bool bAdd)
{
	if (bAdd)	c_uOblige |= (1 << cObID);
	else	c_uOblige &= ~(1 << cObID);
	c_cFloatID = (BYTE)(~c_uOblige & OBLIGEBF_ONLYPOS);
	TRACK("PAPR:set to %d (0x%02x)\n", c_cFloatID, c_uOblige);
}

// 170817
//void CPaper::ChangeOblige(BYTE cOblige)
//{
//	c_uOblige = cOblige;
//	c_cFloatID = ~c_uOblige & OBLIGEBF_ONLYPOS;
//	TRACK("PAPR:chg to %d\n", c_cFloatID);
//}
//
BYTE CPaper::GetHeadCarOffset()
{
	switch (c_cLength) {
	case 4 :	return 3;	break;
	case 6 :	return 2;	break;
	case 8 :	return 1;	break;
	default :	return 0;	break;
	}
	return 0;
}

void CPaper::Freshly()
{
	memcpy(c_pallets[PI_BKUP].pDoz, c_pallets[PI_CURR].pDoz, sizeof(DOZEN));
	c_pallets[PI_BKUP].dwRoute = c_pallets[PI_CURR].dwRoute;
	//memcpy(c_pallets[PI_WORK].pDoz, c_pallets[PI_CURR].pDoz, sizeof(DOZEN));
	memcpy(c_pallets[PI_WORKED].pDoz, c_pallets[PI_CURR].pDoz, sizeof(DOZEN));
	//c_pallets[PI_WORK].dwRoute = c_pallets[PI_CURR].dwRoute;
	c_pallets[PI_WORKED].dwRoute = c_pallets[PI_CURR].dwRoute;
	memset(c_pallets[PI_CURR].pDoz, 0, sizeof(DOZEN));
	c_pallets[PI_CURR].dwRoute = 0;
}

void CPaper::Routing(int iFID, BYTE cRoute)
{
	c_pallets[PI_CURR].dwRoute &= ~(ROUTE_ALL << (iFID << 1 ));
	c_pallets[PI_CURR].dwRoute |= (cRoute << (iFID << 1));
}

void CPaper::Repair()
{
	WORD wDecentMap = 0;
	for (int n = 0; n < (int)(c_cLength + 2); n ++) {
		// FOR SAFETY
		//if (c_cFloatID < LENGTH_TU || n == 0 || n == c_cFloatID) {
			if ((c_pallets[PI_CURR].dwRoute & (ROUTE_ALL << (n << 1))) == 0) {
				// ng
				if (++ c_iDebs[n] >= LINEDEB_DTB) {
					c_iDebs[n] = LINEDEB_DTB;
					c_pallets[PI_CURR].pDoz->recip[n].real.prol.wAddr |= 0xff00;
					//INCWORD(c_wRed[n]);		// 2017/07/06, Modify
					c_wWakeupEach[n] = CYCLE_EACHWAKEUP;			// 171019,
					// 통신이 재개될 때 지연 시간을 주어 각 디바이스들의 상태를 준비시킨다.
				}
				else {
					// 2017/07/06, Modify
					//memcpy(c_pallets[PI_CURR].pDoz, c_pallets[PI_BKUP].pDoz, sizeof(RECIP));
					memcpy(&c_pallets[PI_CURR].pDoz->recip[n], &c_pallets[PI_BKUP].pDoz->recip[n], sizeof(RECIP));
					c_pallets[PI_CURR].dwRoute |= (c_pallets[PI_BKUP].dwRoute & (ROUTE_ALL << (n << 1)));
					wDecentMap |= (1 << c_pallets[PI_CURR].pDoz->recip[n].real.prol.cProperID);
					if (c_wWakeupEach[n] > 0)	-- c_wWakeupEach[n];	// 171019
				}
			}
			else {
				// good
				c_iDebs[n] = 0;
				wDecentMap |= (1 << c_pallets[PI_CURR].pDoz->recip[n].real.prol.cProperID);
				if (c_wWakeupEach[n] > 0)	-- c_wWakeupEach[n];	// 171019
			}
		//}
		//else	IGNORE then NON-FRAME in CU
	}

	if (c_wWakeupDtb == 0) {
		for (int n = 0; n < (int)(c_cLength + 2); n ++) {
			if (!(wDecentMap & (1 << n))) {
				c_mtxRed.Lock();
				INCWORD(c_wRed[n]);
				c_mtxRed.Unlock();
			}
		}
	}
	else	-- c_wWakeupDtb;
}

bool CPaper::GetTenor()
{
	return c_pallets[PI_CURR].pDoz->recip[FID_HEAD].real.prol.cProperID < PID_RIGHT ? false : true;
	// left:false,	right:true
}

//int CPaper::GetTuPID(int iFID)
//{
//	if (iFID >= LENGTH_TU || !GetTenor())	return iFID;
//	return iFID ^ 2;
//}
//
bool CPaper::GetRouteState(int iFID, enPALLETINDEX pi)
{
	//171019
	//return (c_pallets[pi].dwRoute & (3 << (iFID << 1))) ? true : false;
	return ((c_pallets[pi].dwRoute & (3 << (iFID << 1))) && c_wWakeupEach[iFID] == 0) ? true : false;
}

void CPaper::GetWakeupEach(WORD* pEach)
{
	for (int n = 0; n < FID_MAX; n ++)
		*pEach ++ = c_wWakeupEach[n];
}

int CPaper::RecipForm(BYTE* pMesh, BYTE ch, WORD id)
{
	int sum = 0;
	sprintf((char*)pMesh, "$%c%02d,%03d,(", ch, id, SIZE_RECIP);
	while (*pMesh != '\0') { ++ pMesh; ++ sum; }
	int n = Contain(true, pMesh);
	pMesh += n;
	sum += n;
	sprintf((char*)pMesh, "),");
	while (*pMesh != '\0')	{ ++ pMesh; ++ sum; }
	return sum;
}

void CPaper::Shoot(PVOID pVoid, WORD wLength)
{
	if (wLength > SIZE_RECIP)	wLength = SIZE_RECIP;
	memcpy(&c_shoot, pVoid, wLength);
}

void CPaper::ClearAllRed()
{
	c_mtxRed.Lock();
	for (int n = 0; n < FID_MAX; n ++)	c_wRed[n] = 0;
	c_mtxRed.Unlock();
}

// 200218
bool CPaper::GetLabFromRecip()
{
	return c_pallets[PI_CURR].pDoz->recip[FID_HEAD].real.sign.scmdEx.b.lab;
}

int CPaper::Bale(bool bType, BYTE* pMesh)
{
	int sum = 0;

	if (bType) {
		// FOR_SAFETY, to receive everything!
		//if (c_uOblige & (1 << OBLIGE_CAB)) {
			int n = FID_HEAD;
			for ( ; n < LENGTH_TU; n ++) {
				int m = RecipForm(pMesh, 'T', n);
				pMesh += m;
				sum += m;
			}
			for ( ; n < FID_MAX; n ++) {
				int m = RecipForm(pMesh, 'C', n - LENGTH_TU + 1);
				pMesh += m;
				sum += m;
			}
		//}
		//else {
		//	int m = RecipForm(pMesh, 'T', 0);
		//	pMesh += m;
		//	sum += m;
		//	m = RecipForm(pMesh, 'C', c_cFloatID - PID_RIGHTBK);
		//	pMesh += m;
		//	sum += m;
		//}
		////int m = RecipForm(pMesh, 'S', 0);
		////pMesh += m;
		////sum += m;
	}
	else {
		// FOR_SAFETY, to receive everything!
		//if (c_uOblige & (1 << OBLIGE_CAB)) {
			for (int n = FID_HEAD; n < FID_MAX; n ++) {
				memcpy(pMesh, &c_pallets[PI_CURR].pDoz->recip[n], SIZE_RECIP);
				pMesh += SIZE_RECIP;
				sum += SIZE_RECIP;
			}
		//}
		//else {
		//	memcpy(pMesh, &c_pallets[PI_CURR].pDoz->recip[0], SIZE_RECIP);
		//	pMesh += SIZE_RECIP;
		//	sum += SIZE_RECIP;
		//	memcpy(pMesh, &c_pallets[PI_CURR].pDoz->recip[c_cFloatID], SIZE_RECIP);
		//	pMesh += SIZE_RECIP;
		//	sum += SIZE_RECIP;
		//}
		////memcpy(pMesh, &c_shoot, SIZE_RECIP);
		////pMesh += SIZE_RECIP;
		////sum += SIZE_RECIP;
	}
	return sum;
}

ENTRY_CONTAINER(CPaper)
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.prol.wAddr,		sizeof(WORD) + sizeof(DWORD),	"Prol")	// wAddr, dwOrder
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.prol.cProperID,	sizeof(BYTE) * 3,				"Ref")	// cBaseID, cCarLength, cUniquen
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.prol.dbft,		sizeof(DBFTIME),				"time")	// time
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.prol.cVersion,	sizeof(BYTE),					"vno")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.prol.wOtrn,		sizeof(WORD),					"")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.logMatch,			sizeof(DBFTIME),				"lmt")	// time to match logbook file name
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.sign,				sizeof(SIGNATURE),				"Sign")	// gcmd, ecmd, cEsk, cCmk
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.motiv,			sizeof(MOTIVE),					"Moti")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.drift,			sizeof(DRIFT),					"Drif")
	//SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cDiai,			sizeof(BYTE) * LENGTH_WHEELDIA,	"Dia")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cExtTemp,			sizeof(signed char),			"Temp")
	//SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cTempSv[0],		sizeof(BYTE) * CID_MAX,			"")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.inp,				sizeof(_OCTET) * 2,				"in")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.outp,				sizeof(_DUET),					"out")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.avs,				sizeof(_ANALOGV),				"avs")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.ato,				sizeof(ATOFLAP),				"ato")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.svc,				sizeof(SVCFLAP),				"svc")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.svf,				sizeof(SVFFLAP),				"svf")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.ecu,				sizeof(ECUFLAP),				"ecu")
	//SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.hvac,				sizeof(HVACFLAP),				"hvac")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.hcb,				sizeof(HCBFLAP),				"hcb")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.dcul,				sizeof(DCUFLAP),				"dcul")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.dcur,				sizeof(DCUFLAP),				"dcur")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.rtd,				sizeof(RTDSTATEA),				"rtd")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.wFpgaVersion,		sizeof(WORD),					"fpga")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cPoleLine,		sizeof(BYTE),					"Poll")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cPoleBlock,		sizeof(BYTE),					"")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cSvcCtrlCode,		sizeof(BYTE),					"")
	// 200218
	//SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.wPoleRed[0],		sizeof(WORD) * PDEVRED_MAX,		"Polf")
	//SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.wLocRed[0],		sizeof(WORD) * DEVID_DCUL,		"Locf")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.polef,			sizeof(REDCNT),					"Polf")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.localf,			sizeof(REDCNT),					"Locf")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cMmVer,			sizeof(BYTE),					"mm")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cMmState,			sizeof(BYTE),					"")
	SCOOP(&c_pallets[PI_CURR].pDoz->recip[0].real.cst,				sizeof(CONSULTANT),				"cons")
EXIT_CONTAINER()
