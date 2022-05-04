/*
 * CSch.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>

#include "Track.h"
#include "../Inform2/DevInfo2/HduInfo.h"
#include "CPaper.h"
#include "CSch.h"

const BYTE CSch::c_cC2f[4][DEFAULT_CARLENGTH] = {
		{	0,	4,	5,	6,	7,		8,	9,	10,	11,	2	},
		{	0,	4,	5,	6,	7,		8,	9,	2,	0xff,	0xff	},
		{	0,	4,	5,	6,	7,		2,	0xff,	0xff,	0xff,	0xff	},
		{	0,	4,	5,	2,	0xff,	0xff,	0xff,	0xff,	0xff,	0xff	}
};

const WORD CSch::c_localDeviceMap[4][DEFAULT_CARLENGTH] = {
	{
/*00,TC */	DEVBF_DEF | DEVBF_SIV,
/*01,M1 */	DEVBF_DEF | DEVBF_V3F,
/*02,M2 */	DEVBF_DEF | DEVBF_V3F | DEVBF_PAN | DEVBF_ESK,
/*03,T1 */	DEVBF_DEF,
/*04,M2 */	DEVBF_DEF | DEVBF_V3F | DEVBF_PAN,
/*05,T2 */	DEVBF_DEF | DEVBF_SIV,
/*06,T1'*/	DEVBF_DEF |				DEVBF_ESK,
/*07,M1 */	DEVBF_DEF | DEVBF_V3F,
/*08,M2 */	DEVBF_DEF | DEVBF_V3F | DEVBF_PAN,
/*09,TC */	DEVBF_DEF | DEVBF_SIV
	},
	{
/*00,TC */	DEVBF_DEF | DEVBF_SIV,
/*01,M1 */	DEVBF_DEF | DEVBF_V3F,
/*02,M2 */	DEVBF_DEF | DEVBF_V3F | DEVBF_PAN,
/*03,T1 */	DEVBF_DEF |				DEVBF_ESK,
/*04,T2 */	DEVBF_DEF,
/*05,M1 */	DEVBF_DEF | DEVBF_V3F,
/*06,M2 */	DEVBF_DEF | DEVBF_V3F | DEVBF_PAN,
/*07,TC */	DEVBF_DEF | DEVBF_SIV,
/*0-*/		0,
/*0-*/		0
	},
	{
/*00,TC */	DEVBF_DEF | DEVBF_SIV,
/*01,M1 */	DEVBF_DEF | DEVBF_V3F,
// 190813
/*02,M2 */	//DEVBF_DEF |	DEVBF_V3F | DEVBF_PAN,
/*03,T1 */	//DEVBF_DEF |				DEVBF_ESK,
/*02,M2 */	DEVBF_DEF |	DEVBF_V3F | DEVBF_PAN | DEVBF_ESK,
/*03,T1 */	DEVBF_DEF,
/*04,M2 */	DEVBF_DEF | DEVBF_V3F | DEVBF_PAN,
/*05,TC */	DEVBF_DEF | DEVBF_SIV,
/*0-*/		0,
/*0-*/		0,
/*0-*/		0,
/*0-*/		0
	},
	{
/*00,TC */	DEVBF_DEF | DEVBF_SIV,
/*01,M2 */	DEVBF_DEF | DEVBF_V3F | DEVBF_ESK | DEVBF_PAN,
/*02,M1 */	DEVBF_DEF | DEVBF_V3F,
/*03,TC */	DEVBF_DEF |	DEVBF_SIV,
/*0-*/		0,
/*0-*/		0,
/*0-*/		0,
/*0-*/		0,
/*0-*/		0,
/*0-*/		0
	}
};

const WORD	CSch::c_wSivPosBitmap[4] = {
		0x221,	0x81,	0x21,	0x09
};

#define	ASPV_TC		2.29f	// Kg/cm^2
#define	ASPV_M1		1.94f
#define	ASPV_M2		2.08f
#define	ASPV_T1		2.26f
#define	ASPV_T2		2.16f
//TC	2.29f,
//M1	1.94f,
//M2	2.08f,
//T1	2.26f,
//M2	2.08f,
//T2	2.16f,
//T1	2.26f,
//M1	1.94f,
//M2	2.08f,
//TC	2.29f

const double CSch::c_dbAspBase[4][DEFAULT_CARLENGTH] = {
	{
/*00,TC */	ASPV_TC,
/*01,M1 */	ASPV_M1,
/*02,M2 */	ASPV_M2,
/*03,T1 */	ASPV_T1,
/*04,M2 */	ASPV_M2,
/*05,T2 */	ASPV_T2,
/*06,T1'*/	ASPV_T1,
/*07,M1 */	ASPV_M1,
/*08,M2 */	ASPV_M2,
/*09,TC */	ASPV_TC
	},
	{
/*00,TC */	ASPV_TC,
/*01,M1 */	ASPV_M1,
/*02,M2 */	ASPV_M2,
/*03,T1 */	ASPV_T1,
/*04,T2 */	ASPV_T2,
/*05,M1 */	ASPV_M1,
/*06,M2 */	ASPV_M2,
/*07,TC */	ASPV_TC,
/*0-*/		0.f,
/*0-*/		0.f
	},
	{
/*00,TC */	ASPV_TC,
/*01,M1 */	ASPV_M1,
/*02,T1 */	ASPV_T1,
/*03,M1 */	ASPV_M1,
/*04,M2 */	ASPV_M2,
/*05,TC */	ASPV_TC,
/*0-*/		0.f,
/*0-*/		0.f,
/*0-*/		0.f,
/*0-*/		0.f
	},
	{
/*00,TC */	ASPV_TC,
/*01,M1 */	ASPV_M1,
/*02,M2 */	ASPV_M2,
/*03,TC */	ASPV_TC,
/*0-*/		0.f,
/*0-*/		0.f,
/*0-*/		0.f,
/*0-*/		0.f,
/*0-*/		0.f,
/*0-*/		0.f
	}
};

//const double CSch::c_dbLwBase[4][DEFAULT_CARLENGTH] = {
//		{
//	/*00,TC */	35.50f,		//24.05f,
//	/*01,M1 */	35.75f,		//35.74f,		//21.18f,
//	/*02,M2 */	36.80f,		//36.64f,		//22.08f,
//	/*03,T1 */	31.20f,		//29.62f,		//18.31f,
//	/*04,M2 */	36.80f,		//36.61f,		//22.05f,
//	/*05,T2 */	33.29f,		//21.98f,
//	/*06,T1'*/	31.20f,		//29.70f,		//18.39f,
//	/*07,M1 */	35.75f,		//35.74f,		//21.18f,
//	/*08,M2 */	36.80f,		//36.60f,		//22.04f,
//	/*09,TC */	35.50f,		//24.05f
//		},
//		{
//	/*00,TC */	35.50f,		//24.05f,
//	/*01,M1 */	35.75f,		//35.74f,		//21.18f,
//	/*02,M2 */	36.80f,		//36.64f,		//22.08f,
//	/*03,T1 */	31.20f,		//29.62f,		//18.31f,
//	/*04,T2 */	33.29f,		//21.98f,
//	/*05,M1 */	35.75f,		//35.74f,		//21.18f,
//	/*06,M2 */	36.80f,		//36.61f,		//22.05f,
//	/*07,TC */	35.50f,		//24.05f,
//	/*0-*/		0.f,
//	/*0-*/		0.f
//		},
//		{
//	/*00,TC */	35.50f,		//24.05f,
//	/*01,M1 */	35.75f,		//35.74f,		//21.18f,
//	/*02,T1 */	31.20f,		//29.62f,		//18.31f,
//	/*03,M1 */	36.80f,		//36.64f,		//22.08f,
//	/*04,M2 */	36.80f,		//36.61f,		//22.05f,
//	/*05,TC */	35.50f,		//24.05f,
//	/*0-*/		0.f,
//	/*0-*/		0.f,
//	/*0-*/		0.f,
//	/*0-*/		0.f
//		},
//		{
//	/*00,TC */	35.50f,		//24.05f,
//	/*01,M1 */	36.80f,		//36.64f,		//22.08f,
//	/*02,M2 */	36.80f,		//36.60f,		//22.04f,
//	/*03,TC */	35.50f,		//24.05f,
//	/*0-*/		0.f,
//	/*0-*/		0.f,
//	/*0-*/		0.f,
//	/*0-*/		0.f,
//	/*0-*/		0.f,
//	/*0-*/		0.f
//		}
//};
//
const double CSch::c_dbLwBase10[4][DEFAULT_CARLENGTH] = {
		{
	/*00,TC */	355.0f,
	/*01,M1 */	357.5f,		//357.4f,
	/*02,M2 */	376.0f,		//366.4f,
	/*03,T1 */	315.0f,		//296.2f,
	/*04,M2 */	369.0f,		//366.1f,
	/*05,T2 */	332.9f,
	/*06,T1'*/	315.0f,		//297.0f,
	/*07,M1 */	359.7f,		//357.4f,
	/*08,M2 */	369.5f,		//366.0f,
	/*09,TC */	355.0f
		},
		{
	/*00,TC */	355.0f,
	/*01,M1 */	357.5f,		//357.4f,
	/*02,M2 */	376.0f,		//366.4f,
	/*03,T1 */	315.0f,		//296.2f,
	/*04,T2 */	332.9f,
	/*05,M1 */	357.5f,		//357.4f,
	/*06,M2 */	369.0f,		//366.1f,
	/*07,TC */	355.0f,
	/*0-*/		0.f,
	/*0-*/		0.f
		},
		//{
	/*00,TC */	//355.0f,
	/*01,M1 */	//357.5f,		//357.4f,
	/*02,T1 */	//315.0f,		//296.2f,
	/*03,M1 */	//376.0f,		//366.4f,
	/*04,M2 */	//369.0f,		//366.1f,
	/*05,TC */	//355.0f,
	/*0-*/		//0.f,
	/*0-*/		//0.f,
	/*0-*/		//0.f,
	/*0-*/		//0.f
		//},
		{
	/*00,TC */	352.83f,
	/*01,M1 */	365.9f,
	/*02,M2 */	371.7f,
	/*03,T1 */	304.13f,
	/*04,M2 */	372.93f,
	/*05,TC */	353.37f,
	/*0-*/		0.f,
	/*0-*/		0.f,
	/*0-*/		0.f,
	/*0-*/		0.f
		},
		{
	/*00,TC */	355.0f,
	/*01,M1 */	376.0f,		//366.4f,
	/*02,M2 */	369.5f,		//366.0f,
	/*03,TC */	355.0f,
	/*0-*/		0.f,
	/*0-*/		0.f,
	/*0-*/		0.f,
	/*0-*/		0.f,
	/*0-*/		0.f,
	/*0-*/		0.f
		}
};

CSch::CSch()
{
	c_wCarBitmap = DEFAULT_CARBITMAP;
	c_wFlowBitmap = DEFAULT_FLOWBITMAP;
	c_cCarLength = DEFAULT_CARLENGTH;
	c_cCarTypeID = 0;
	memset(&c_devConf, 0, sizeof(DEVCONF));
}

CSch::~CSch()
{
}

void CSch::Initial()
{
}

void CSch::Drawup(BYTE cCarLength)
{
	switch (cCarLength) {
	case 4 :	c_cCarLength = 4;	c_cCarTypeID = 3;	break;
	case 6 :	c_cCarLength = 6;	c_cCarTypeID = 2;	break;
	case 8 :	c_cCarLength = 8;	c_cCarTypeID = 1;	break;
	default :	c_cCarLength = 10;	c_cCarTypeID = 0;	break;
	}

	c_wCarBitmap = c_wFlowBitmap = 0;
	memset(&c_devConf, 0, sizeof(DEVCONF));

	int n;
	for (n = 0; n < (int)c_cCarLength; n ++) {
		c_wCarBitmap |= (1 << n);
		c_wFlowBitmap |= (1 << n);
		WORD wDevMap = GetLDeviceMap(n);
		if (wDevMap & DEVBF_SIV) {
			c_devConf.siv.cPos[c_devConf.siv.cLength ++] = (BYTE)n;
			c_devConf.siv.wMap |= (1 << n);
		}
		if (wDevMap & DEVBF_V3F) {
			c_devConf.v3f.cPos[c_devConf.v3f.cLength ++] = (BYTE)n;
			c_devConf.v3f.wMap |= (1 << n);
		}
		if (wDevMap & DEVBF_PAN) {
			c_devConf.pan.cPos[c_devConf.pan.cLength ++] = (BYTE)n;
			c_devConf.pan.wMap |= (1 << n);
		}
		if (wDevMap & DEVBF_ESK) {
			c_devConf.esk.cPos[c_devConf.esk.cLength ++] = (BYTE)n;
			c_devConf.esk.wMap |= (1 << n);
		}
	}
	for ( ; n < (int)(c_cCarLength + 2); n ++)
		c_wFlowBitmap |= (1 << n);
}

WORD CSch::GetLDeviceMap(int iCID)
{
	if (iCID >= CID_MAX) {
		TRACK("SCH>ERR!!!!!!!!!!:%s() UNREACHABLE LINE A !!!!!!!!!!\n", __FUNCTION__);
		return 0;
	}
	return c_localDeviceMap[c_cCarTypeID][iCID];
}

/*	방향에 맞게 객차 번호에 따라 flow 순서를 가져온다
	cid		0	1	2	3	4	5	6	7	8	9
	left	0	4	5	6	7	8	9	10	11	2
			2	4	5	6	7	8	9	10	11	0	right
*/
int CSch::C2F(bool bTenor, int iCID)
{
	// 객차 번호를 기준으로 할 때...
	int fid = (int)c_cC2f[c_cCarTypeID][iCID];
	if (bTenor && (fid == 0 || fid == 2))	fid ^= 2;	// 오른쪽 진행 방향에서는 0호차와 9호차가 바뀐다.
	return fid;
}

int CSch::F2C(bool bTenor, int iFID)
{
	// flow 순서를 기준으로 할 때...
	if (iFID < LENGTH_TU) {
		if (iFID < FID_TAIL)	return bTenor ? c_cCarLength - 1 : 0;
		else	return bTenor ? 0 : c_cCarLength - 1;
	}
	return iFID - FID_TAILBK;
}

// 210510
int CSch::P2C(int iPID)
{
	if (iPID < PID_RIGHT)	return 0;
	else if (iPID < PID_PAS) {
		GETPAPER(pPaper);
		return (pPaper->GetLength() - 1);
	}
	else	return iPID - PID_PAS + 1;
}

int CSch::GetSivID(int iCID)
{
	for (int n = 0; n < (int)c_devConf.siv.cLength; n ++) {
		if (iCID == (int)c_devConf.siv.cPos[n])	return n;
	}
	return 0xff;
}

int CSch::GetV3fID(int iCID)
{
	for (int n = 0; n < (int)c_devConf.v3f.cLength; n ++) {
		if (iCID == (int)c_devConf.v3f.cPos[n])	return n;
	}
	return 0xff;
}

int CSch::GetEskCID(int iEID)
{
	if (iEID >= (int)c_devConf.esk.cLength)	iEID = 0;
	return (int)c_devConf.esk.cPos[iEID];
}

double CSch::GetProportion(double dbPsc, double dbPscMin, double dbPscMax, double dbLogMin, double dbLogMax)
{
	if (dbPsc < dbPscMin)	return dbLogMin;
	double dbPscScale = dbPscMax - dbPscMin;
	if (dbPscScale > 0.f)	return (((dbPsc - dbPscMin) * (dbLogMax - dbLogMin) / dbPscScale) + dbLogMin);
	TRACK("SCH>ERR:double div0!\n");
	return 0.f;
}

WORD CSch::GetProportion(WORD wPsc, WORD wPscMin, WORD wPscMax, WORD wLogMin, WORD wLogMax)
{
	if (wPsc < wPscMin)	return wLogMin;
	WORD wPscScale = wPscMax - wPscMin;
	if (wPscScale > 0)	return (((wPsc - wPscMin) * (wLogMax - wLogMin) / wPscScale) + wLogMin);
	TRACK("SCH>ERR:WORD div0!\n");
	return 0;
}

int CSch::GetProportion(int iPsc, int iPscMin, int iPscMax, int iLogMin, int iLogMax)
{
	if (iPsc < iPscMin)	return iLogMin;
	int iPscScale = iPscMax - iPscMin;
	if (iPscScale != 0)	return (((iPsc - iPscMin) * (iLogMax - iLogMin) / iPscScale) + iLogMin);
	TRACK("SCH:ERR:int div0!\n");
	return 0;
}

double CSch::BarToKgcm2(BYTE cBar)	// c - 0~10/0~0xff -> 10.1972 ~ 10.2 Kg/cm^2
{
	int v = GetProportion((int)cBar, (int)0, (int)255, (int)0, (int)10000);	// to 10.000bar
	return ((double)v / 1000.f * 1.01972f);		// to kg/cm^2
}

// 200218
double CSch::KPaToKgcm2(WORD wkpa)
{
	//return ((double)ckpa * 1.01972 * 0.01f * 10);	"0.1" -> "1"
	return ((double)wkpa * 0.0101972f);		// 0.101972f
}

double CSch::CalcLoad(int cid, double dbAsp)
{
	double dbLoad = 0.f;
	if (dbAsp > c_dbAspBase[c_cCarTypeID][cid]) {
		dbLoad = (dbAsp - c_dbAspBase[c_cCarTypeID][cid]) / 1.91f * 200.f;
		if (dbLoad > 200.f)	dbLoad = 200.f;
	}
	return dbLoad;
}

WORD CSch::CalcLoad(int cid, WORD weight)
{
	double dbLoad = 0.f;
	double dbWeight = (double)weight;
	if (dbWeight > c_dbLwBase10[c_cCarTypeID][cid]) {
		//dbLoad = dbWeight - c_dbLwBase[c_cCarTypeID][cid]) * 10.f;	// / 10.f * 100.f;
		dbLoad = dbWeight - c_dbLwBase10[c_cCarTypeID][cid];	// / 10.f * 100.f;, 정원이 10ton이다.
	}
	return dbLoad;
}

ENTRY_CONTAINER(CSch)
	SCOOP(&c_cCarLength,			sizeof(BYTE),	"Sch")
	SCOOP(&c_cCarTypeID,			sizeof(BYTE),	"")
	SCOOP(&c_wFlowBitmap,			sizeof(WORD),	"")
	SCOOP(&c_devConf.pan.cLength,	sizeof(BYTE),	"pan")
	SCOOP(&c_devConf.pan.cPos[0],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.pan.cPos[1],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.pan.cPos[2],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.pan.cPos[3],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.siv.cLength,	sizeof(BYTE),	"siv")
	SCOOP(&c_devConf.siv.cPos[0],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.siv.cPos[1],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.siv.cPos[2],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.siv.cPos[3],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.v3f.cLength,	sizeof(BYTE),	"v3f")
	//SCOOP(&c_devConf.v3f.cPos[0],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.v3f.cPos[1],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.v3f.cPos[2],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.v3f.cPos[3],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.v3f.cPos[4],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.v3f.cPos[5],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.pan.cLength,	sizeof(BYTE),	"pan")
	//SCOOP(&c_devConf.pan.cPos[0],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.pan.cPos[1],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.pan.cPos[2],	sizeof(BYTE),	"")
	//SCOOP(&c_devConf.pan.cPos[3],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.esk.cLength,	sizeof(BYTE),	"esk")
	SCOOP(&c_devConf.esk.cPos[0],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.esk.cPos[1],	sizeof(BYTE),	"")
	SCOOP(&c_devConf.esk.cPos[2],	sizeof(BYTE),	"")
EXIT_CONTAINER()
