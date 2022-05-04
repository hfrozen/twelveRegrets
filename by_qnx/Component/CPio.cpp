/*
 * CPio.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include <Draft.h>
#include <Printz.h>

#include "CPio.h"

CPio::CPio()
{
	c_bState = FALSE;
}

CPio::~CPio()
{
}

const char*	CPio::c_device[] = {
	"gpio_1",
	"gpio_2",
	"gpio_3",
	"gpio_4",
	"gpio_5"
};

const CPio::ATTRIBUTE CPio::c_attr[] = {
	{ 0x00, 0x00 },
	{ 0x00, 0x00 },
	{ 0x70, 0x70 },
	{ 0xff, 0xff },
	{ 0x00, 0x00 },
};

const WORD	CPio::c_wCcDeves[3][10] = {
	{
	/* 1:TCF */		DEVBEEN_CC | DEVBEEN_SIV,
	/* 2:M1F */		DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_AUX,
	/* 3:M2F */		DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_CMSB,
	/* 4:T1 */		DEVBEEN_CC | DEVBEEN_PWM | DEVBEEN_ESK,
	/* 5:T2 */		DEVBEEN_CC,
	/* 6:M1R */		DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_AUX,
	/* 7:M2R */		DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_CMSB,
	/* 0:TCR */		DEVBEEN_CC | DEVBEEN_SIV,
					0,
					0
	},
	{
	/* 1:TCF */		DEVBEEN_CC | DEVBEEN_SIV,
	/* 2:M1F */		DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_AUX,
	/* 3:T1 */		DEVBEEN_CC | DEVBEEN_CMSB | DEVBEEN_PWM | DEVBEEN_ESK,
	/* 4:M1R */		DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_AUX,
	/* 5:M2R */		DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_CMSB,
	/* TCR */		DEVBEEN_CC | DEVBEEN_SIV,
					0,
					0,
					0,
					0
	},
	{
	/* 1:TCF */		DEVBEEN_CC | DEVBEEN_SIV,
	/* 2:T1(M1F) */	DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_AUX | DEVBEEN_PWM | DEVBEEN_ESK,
	/* 3:M2R */		DEVBEEN_CC | DEVBEEN_V3F | DEVBEEN_CMSB,
	/* 0:TCR */		DEVBEEN_CC | DEVBEEN_SIV,
					0,
					0,
					0,
					0,
					0,
					0
	}
};

/*===================================================================
 * for device position of DU
 *                  | CC1 | CC2 | CC3 | CC4 | CC5 | CC6 | CC7 | CC0 |
 *                  | B0  | B1  | B2  | B3  | B4  | B5  | B6  | B7  |
 *                  | B8  | B9  | B10 | B11 | B12 | B13 | B14 | B15 |
 *------------------+-----+-----+-----+-----+-----+-----+-----+-----|
 *SIV, IVS        8 |  v  |     |     |     |     |     |     |  v  |
 *                6 |  v  |     |     |     |     |  v  |
 *                4 |  v  |     |     |  v  |
 *------------------+-----+-----+-----+-----+-----+-----+-----+-----|
 *V3F, HSCB, CCK  8 |     |  v  |  v  |     |     |  v  |  v  |     |
 *                6 |     |  v  |     |  v  |  v  |     |
 *                4 |     |  v  |  v  |     |
 *------------------+-----+-----+-----+-----+-----+-----+-----+-----|
 *CMSB            8 |     |     |  v  |     |     |     |  v  |     |
 *                6 |     |     |  v  |     |  v  |     |
 *                4 |     |     |  v  |     |
 *------------------+-----+-----+-----+-----+-----+-----+-----+-----|
 *AUX, IES        8 |     |  v  |     |     |     |  v  |     |     |
 *                6 |     |  v  |     |  v  |     |     |
 *                4 |     |  v  |     |     |
 *=================================================================== */

BOOL CPio::Open()
{
	for (UCURV n = 0; n < LENGTH_GPIO; n ++) {
		if ((c_dev[n] = M_open(c_device[n])) < 0) {
			MSGLOG("[PIO]ERROR:Can not open %s : %s.\r\n",
					c_device[n], M_errstring(UOS_ErrnoGet()));
			return FALSE;
		}
		M_write(c_dev[n], c_attr[n].init);
		M_setstat(c_dev[n], Z17_DIRECTION, c_attr[n].attr);
	}
	c_bState = TRUE;
	return TRUE;
}

BOOL CPio::Close()
{
	for (UCURV n = 0; n < LENGTH_GPIO; n ++) {
		if (M_close(c_dev[n]) < 0) {
			MSGLOG("[PIO]ERROR:Can not close %s : %s.\r\n",
					c_device[n], M_errstring(UOS_ErrnoGet()));
			return FALSE;
		}
	}
	return TRUE;
}

DWORD CPio::ReadThumb()
{
	DWORD dw = (~Swap(Read(0)) << 12) & 0xff000;
	dw |= (~Swap(Read(1)) << 4) & 0xff0;
	dw |= (~Read(2)) & 0xf;
	return dw;
}

BYTE CPio::Read(BYTE add)
{
	if (add >= LENGTH_GPIO)	return 0;
	int32 v;
	M_read(c_dev[add], &v);
	return (BYTE)v;
}

void CPio::Write(BYTE add, BYTE n)
{
	if (add >= LENGTH_GPIO)	return;
	M_write(c_dev[add], (int32)n);
}

BYTE CPio::Swap(BYTE n)
{
	BYTE m = n;
	n = (n >> 4) & 0xf;
	m = (m << 4) & 0xf0;
	return (n | m);
}

WORD CPio::GetCcDev(PCARINFO pCar)
{
	switch (pCar->nLength) {
	case 8 :
		if (pCar->nID > 7)	return 0;
		return (c_wCcDeves[0][pCar->nID]);
		break;
	case 6 :
		if (pCar->nID > 5)	return 0;
		return (c_wCcDeves[1][pCar->nID]);
		break;
	case 4 :
		if (pCar->nID > 3)	return 0;
		return (c_wCcDeves[2][pCar->nID]);
		break;
	default :
		return 0;
		break;
	}
	return 0;
}

BYTE CPio::GetV3fQuantity(PCARINFO pCar)
{
	switch (pCar->nLength) {
	case 6 :	return 3;	break;
	case 4 :	return 2;	break;
	default :	return 4;	break;
	}
	return 4;
}

BYTE CPio::GetEcuQuantity(PCARINFO pCar)
{
	switch (pCar->nLength) {
	case 6 :	return 6;	break;
	case 4 :	return 4;	break;
	default :	return 8;	break;
	}
	return 8;
}

char* CPio::TcNamePlate(char* p)
{
	sprintf(p, "(%s:%d)", "HEAD", 28);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DIAM", 16);
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(V3FCINFO));	// 1
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ECUCINFO));	// 2
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(SIVCINFO));	// 3
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUCINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(CMSBCINFO));// 5
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ATCRINFO));	// 6
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ATORINFO));	// 8
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(CMMRINFO));	// 12
	TOEND(p);
	sprintf(p, "(%s:%d)", "MDEV", sizeof(BYTE) + sizeof(DEVERROR) * 3);
	TOEND(p);
	sprintf(p, "(%s:%d)", "SDEV", sizeof(DEVERROR) * 5);	// 13
	TOEND(p);
	sprintf(p, "(%s:%d)", "DIN", SIZE_TCDI);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DOUT", SIZE_TCDO);
	TOEND(p);
	sprintf(p, "(%s:%d)", "AIN", SIZE_TCAI * sizeof(SHORT));
	TOEND(p);
	for (UCURV m = 0; m < (LENGTH_CC / 4); m ++) {
		sprintf(p, "(%s:%d)", "ORDER", SIZE_CCDO * 4);
		TOEND(p);
	}
	sprintf(p, "(%s:%d)", "PWM", sizeof(PWMARCH));
	TOEND(p);
	sprintf(p, "(%s:%d)", "ACMD", sizeof(AUXCMD));
	TOEND(p);
	sprintf(p, "(%s:%d)", "DUCA", SIZE_HEADBK);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DUCB", SIZE_NORMALBK);
	TOEND(p);
	//sprintf(p, "(%s:%d)", "env", sizeof(ENVARCH));
	sprintf(p, "(%s:%d)", "EACH", sizeof(TCEACH));
	TOEND(p);
	sprintf(p, "(%s:%d)", "MCDS", sizeof(MCDSTICK));
	TOEND(p);
	sprintf(p, "(%s:%d)", "RLDO", SIZE_TCDO);
	TOEND(p);
	sprintf(p, "(%s:%d)", "ARM", sizeof(ARM_ARCH));
	TOEND(p);
	sprintf(p, "(%s:%d)", "DUMMY", SIZE_FIRM - sizeof(TCFIRMREAL));
	TOEND(p);
	return p;
}

char* CPio::CcNamePlate(char* p)
{
	sprintf(p, "(%s:%d)", "HEAD", 22);
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(V3FEINFO));	// 1
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ECUAINFO));	// 2
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(SIVAINFO));	// 3
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(CMSBAINFO));	// 5
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));	// 4
	TOEND(p);
	sprintf(p, "(%s:%d)", "DIN", SIZE_CCDI);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DOUT", SIZE_CCDO);
	TOEND(p);
	sprintf(p, "(%s:%d)", "PWM", sizeof(PWMARCH));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(V3FRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ECURINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(SIVRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(CMSBRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "DERR", sizeof(WORD) * 9);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DUMMY", SIZE_FIRM - sizeof(CCFIRMREAL));
	TOEND(p);
	return p;
}
