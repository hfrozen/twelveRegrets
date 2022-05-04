// Fpgam.c
#include <string.h>
#include "stm32f4xx_hal.h"
#include "Refer32.h"
#include "Fpgam.h"
#include "Uart.h"
#include "HostCmd.h"

SRAM_HandleTypeDef hSram = { 0 };
FSMC_NORSRAM_TimingTypeDef ramTimType = { 0 };

BYTE	cDleBuf[SIZE_ENVELOPE];

extern IDDRAW g_id;
extern WORD	g_wProjNo;
//extern HOST	g_host;
extern LIMB	g_lm;

const BYTE g_cDefDevices[PRJ_MAX][18][MAX_PORT] = {
	{
	/*00*/	{	SDID_ATO,	SDID_ATO,	SDID_PAU,	SDID_PAU,	SDID_PIS,	SDID_PIS,	SDID_TRS,	SDID_TRS	},	// TC-CAB
	/*01*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_SIV,	SDID_SIV,	SDID_DCUR,	SDID_DCUL	},	// TC
	/*02*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL	},	// M1
	/*03*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL	},	// M2
	/*04*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL	},	// T1
	/*05*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL	},	// M2
	/*06*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_SIV,	SDID_SIV,	SDID_DCUR,	SDID_DCUL	},	// T2
	/*07*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL	},	// T1'
	/*08*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL	},	// M1
	/*09*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL	},	// M2
	/*10*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_SIV,	SDID_SIV,	SDID_DCUR,	SDID_DCUL	},	// TC
	/*11*/	{	SDID_ATO,	SDID_ATO,	SDID_PAU,	SDID_PAU,	SDID_PIS,	SDID_PIS,	SDID_TRS,	SDID_TRS	},	// TC-CAB
	/*12*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*13*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*14*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*15*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*16*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*17*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	}
	},
	{
	/*00*/	{	SDID_ATC,	SDID_ATC,	SDID_ATO,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},	// TC-TC signal
	/*01*/	{	SDID_AVC,	SDID_RFD,	SDID_TRS,	SDID_FDU,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},	// TC-TC service
	/*02*/	{	SDID_SIV,	SDID_HVAC,	SDID_BAT,	SDID_DCUL,	SDID_DCUR,	SDID_ECU,	SDID_NON,	SDID_NON	},	// CC-TC
	/*03*/	{	SDID_V3F,	SDID_HVAC,	SDID_NON,	SDID_DCUL,	SDID_DCUR,	SDID_ECU,	SDID_NON,	SDID_NON	},	// CC-M1
	/*04*/	{	SDID_V3F,	SDID_HVAC,	SDID_CMSB,	SDID_DCUL,	SDID_DCUR,	SDID_ECU,	SDID_NON,	SDID_NON	},	// CC-M2
	/*05*/	{	SDID_NON,	SDID_HVAC,	SDID_NON,	SDID_DCUL,	SDID_DCUR,	SDID_ECU,	SDID_NON,	SDID_NON	},	// CC-T1
	/*06*/	{	SDID_NON,	SDID_HVAC,	SDID_NON,	SDID_DCUL,	SDID_DCUR,	SDID_ECU,	SDID_NON,	SDID_NON	},	// CC-T2
	/*07*/	{	SDID_V3F,	SDID_HVAC,	SDID_NON,	SDID_DCUL,	SDID_DCUR,	SDID_ECU,	SDID_NON,	SDID_NON	},	// CC-M1
	/*08*/	{	SDID_V3F,	SDID_HVAC,	SDID_CMSB,	SDID_DCUL,	SDID_DCUR,	SDID_ECU,	SDID_NON,	SDID_NON	},	// CC-M2
	/*09*/	{	SDID_SIV,	SDID_HVAC,	SDID_BAT,	SDID_DCUL,	SDID_DCUR,	SDID_NON,	SDID_NON,	SDID_NON	},	// CC-TC
	/*10*/	{	SDID_ATC,	SDID_ATC,	SDID_ATO,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},	// TC-TC signal
	/*11*/	{	SDID_AVC,	SDID_RFD,	SDID_TRS,	SDID_FDU,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},	// TC-TC service
	/*12*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*13*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*14*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*15*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*16*/	{	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON,	SDID_NON	},
	/*17*/	{	SDID_RESA,	SDID_RESB,	SDID_RESA,	SDID_RESB,	SDID_RESA,	SDID_RESB,	SDID_RESA,	SDID_RESB	}	// for test
	}
};

//typedef struct _tagDEVMOLD {
//	WORD	wAddr;
//	BYTE	cBpss;
//	BYTE	cMode;
//}
const DEVMOLD g_defMold[PRJ_MAX][SDID_MAX] = {
	{
	/*00, SDID_ATC */	{	0,		0,				0	},
	/*01, SDID_ATO */	{	0,		SCBPS_19200,	(1 << SCMODE_ASYNC) | (1 << SCMODE_ODDP) | (1 << SCMODE_CRCB)	},
	/*02, SDID_PAU */	{	0x0160,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*03, SDID_PIS */	{	0x0260,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*04, SDID_TRS */	{	0x0460,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*05, SDID_AVC */	{	0,		0,				0	},
	/*06, SDID_RFD */	{	0,		0,				0	},
	/*07, SDID_SIV */	{	0xff90,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_MATCH) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*08, SDID_V3F */	{	0xff20,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_MATCH) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*09, SDID_ECU */	{	0xff10,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_MATCH) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*10, SDID_HVAC */	{	0xff70,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_MATCH) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*11, SDID_DCUL */	{	0x0170,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_MATCH) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*12, SDID_DCUR */	{	0x1170,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_MATCH) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*13, SDID_BAT */	{	0,		0,				0	},
	/*14, SDID_CMSB */	{	0,		0,				0	},
	/*15, SDID_FDU */	{	0,		0,				0	},
	/*16, SDID_RESA */	{	0,		0,				0	},
	/*17, SDID_RESB */	{	0,		0,				0	}
	},
	{
	/*00, SDID_ATC */	{	0xff30,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*01, SDID_ATO */	{	0xff40,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*02, SDID_PAU */	{	0x0160,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*03, SDID_PIS */	{	0x0260,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*04, SDID_TRS */	{	0x7305,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS) | (1 << SCMODE_MATCH)	},
	/*05, SDID_AVC */	{	0x7304,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS) | (1 << SCMODE_MATCH)	},
	/*06, SDID_RFD */	{	0x7306,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS) | (1 << SCMODE_MATCH)	},
	/*07, SDID_SIV */	{	0x7303,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*08, SDID_V3F */	{	0x7301,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*09, SDID_ECU */	{	0x7302,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*10, SDID_HVAC */	{	0x7307,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*11, SDID_DCUL */	{	0x7001,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*12, SDID_DCUR */	{	0x7011,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*13, SDID_BAT */	{	0x730b,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*14, SDID_CMSB */	{	0x7309,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*15, SDID_FDU */	{	0x7302,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS) | (1 << SCMODE_MATCH)	},
	/*16, SDID_RESA */	{	0x7303,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	},
	/*17, SDID_RESB */	{	0x7303,	SCBPS_38400,	(1 << SCMODE_HDLC) | (1 << SCMODE_NRZI) | (1 << SCMODE_FCS)	}
	}
};

const char* g_pDevNames[SDID_MAX] = {
	"ATC ",	"ATO ",	"PAU ",	"PIS ",		"TRS ",	"AVC ",	"RFD ",	"SIV ",
	"V3F ",	"ECU ",	"HVAC",	"DCUL",		"DCUR",	"BAT ",	"CMSB",	"FDU",
	"RESA",	"RESB"
};

const DWORD	g_dwBpses[16] = {
		9600,		19200,		38400,		115200,
		256000,		512000,		1152000,	1536000,
		2304000,	4608000,	38400,		38400,
		38400,		38400,		38400,		38400
};

//typedef struct _tagFRMLENGTH {
//	WORD	r;		// rx length
//	WORD	t;		// tx length
//	WORD	c;		// compare begin offset
//	WORD	l;		// compare length
//	WORD	tlo;	// trace length data position offset
//	BYTE	tlm;	// trace length data mask
//	BYTE	tls;	// trace length data shift
//} FRMLENGTH;

const FRMLENGTH g_trxl[PRJ_MAX][SDID_MAX] = {
	{
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_ATC
		{ 13, 20,  2,  7,  0,  0,  0,    0,  0 },		// SDID_ATO
		{ 26, 10,  4, 14,  0,  0,  0,    0,  0 },		// SDID_PAU
		{ 26, 33,  4, 14,  0,  0,  0,    0,  0 },		// SDID_PIS
		{  9, 10,  4,  3,  0,  0,  0,    0,  0 },		// SDID_TRS
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_AVC
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_RF
		{ 14, 18,  4,  2, 15,  0,  0, 0xf0,  4 },		// SDID_SIV
		{ 24, 36, 10, 12, 33,  0,  0, 0xf0,  4 },		// SDID_V3F
		{ 12, 33,  4,  2,  0,  0,  0,    0,  0 },		// SDID_ECU
		{ 19, 22,  4,  8,  0,  0,  0,    0,  0 },		// SDID_HVAC
		{ 12,  9,  4,  2,  0,  0,  0,    0,  0 },		// SDID_DCUL
		{ 12,  9,  4,  2,  0,  0,  0,    0,  0 },		// SDID_DCUR
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_BAT
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_CMSB
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_FDU
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_RESA
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 }		// SDID_RESB
	},
	{
		{ 15, 18,  4,  1,  0,  0,  0,    0,  0 },		// SDID_ATC
		{ 31, 26,  4, 17,  0,  0,  0,    0,  0 },		// SDID_ATO
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_PAU
		{  0,  0,  0,  0,  0,  0,  0,    0,  0 },		// SDID_PIS
		{ 57, 10,  5, 51,  0,  0,  0,    0,  0 },		// SDID_TRS
		{ 57, 35,  5, 51,  0,  0,  0,    0,  0 },		// SDID_AVC
		{ 57, 24,  5, 51,  0,  0,  0,    0,  0 },		// SDID_RF
		{ 12, 29,  4,  2,  0,  0,  0,    0,  0 },		// SDID_SIV
		{ 17, 41,  4,  5, 15,  2,  0,    0,  0 },		// SDID_V3F
		{ 15, 29,  4,  5,  0,  0,  0,    0,  0 },		// SDID_ECU
		{ 19, 22,  4,  9,  0,  0,  0,    0,  0 },		// SDID_HVAC
		{ 12,  9,  4,  2,  0,  0,  0,    0,  0 },		// SDID_DCUL
		{ 12,  9,  4,  2,  0,  0,  0,    0,  0 },		// SDID_DCUR
		{ 19, 23,  0,  0,  0,  0,  0,    0,  0 },		// SDID_BAT
		{ 14,  7,  4,  4,  0,  0,  0,    0,  0 },		// SDID_CMSB
		{ 57, 73,  5, 64,  0,  0,  0,    0,  0 },		// SDID_FDU
		{ 30, 31,  0, 28,  0,  0,  0,    0,  0 },		// SDID_RESA
		{ 31, 30,  0, 28,  0,  0,  0,    0,  0 }		// SDID_RESB
	}
};

int DleEncoder(BYTE* pSrc, int nLength)
{
	memset(cDleBuf, 0, SIZE_ENVELOPE);
	BYTE* pDest = &cDleBuf[2];
	int nAppend = 0;
	for (int n = 0; n < nLength; n ++) {
		if (*pSrc == 0x10) {
			*pDest ++ = 0x10;
			++ nAppend;
		}
		*pDest ++ = *pSrc ++;
	}
	cDleBuf[0] = 0x10;
	cDleBuf[1] = 2;
	*pDest ++ = 0x10;
	*pDest ++ = 3;
	nLength += (nAppend + 4);
	WORD crc = Crc16ffff(cDleBuf, nLength);
	*pDest ++ = HIBYTEA(crc);
	*pDest = LOBYTEA(crc);
	return nLength + 2;
}

int DleDecoder(BYTE* pDest, int nLength)
{
	BYTE* pBkup = pDest;	// for calc. crc
	bool bDle = false;
	int bi = -1;
	WORD crc;
	for (int n = 0; n < nLength; n ++) {
		BYTE ch = *pDest;
		if (bDle && (ch == 2 || ch == 3 || ch == 0x10)) {
			switch (ch) {
			case 2 :
				if (bi < 0)	bi = 0;
				else	return 1;
				break;
			case 3 :
				if (bi != 11)	return 3;
				crc = Crc16ffff(pBkup, n + 1);
				if (HIBYTEA(crc) != *(pDest + 1) || LOBYTEA(crc) != *(pDest + 2))	return 4;
				return 0;
				break;
			default :
				if (bi < 0)	return 2;
				cDleBuf[bi ++] = ch;
				break;
			}
			bDle = false;
		}
		else if (!bDle && ch == 0x10)	bDle = true;
		else {
			if (bi < 0)	return 2;
			cDleBuf[bi ++] = ch;
		}
		++ pDest;
		if (bi > SIZE_ENVELOPE)	return 5;
	}
	return 6;
}

void SccCtrl()
{
	for (int n = 0; n < MAX_PORT; n ++) {
		if (g_lm.sc[n].wStatus & (1 << SCBS_RXFULL)) {
			//g_lm.sc[n].wStatus &= ~(1 << SCBS_RXFULL);
			g_lm.sc[n].wStatus = 0;

			bool bMatch = false;
			BYTE cEnr = 1;
			if (g_lm.sc[n].r.i == g_trxl[g_wProjNo][g_lm.sc[n].m.cDevID].r) {
				if (!g_lm.sc[n].m.mode.b.b0) {		// asynch.
					if (g_lm.sc[n].m.mode.b.b5) {		// asynch. crc
						if (DleDecoder(g_lm.sc[n].r.sBuf[SB_CURRENT], g_trxl[g_wProjNo][g_lm.sc[n].m.cDevID].r) == 0) {
							memcpy(g_lm.sc[n].r.sBuf[SB_CURRENT], cDleBuf, 11);
							bMatch = true;
						}
					}
					else	bMatch = true;
				}
				else {		// hdlc
					if ((g_lm.sc[n].m.cDevID == SDID_DCUL || g_lm.sc[n].m.cDevID == SDID_DCUR)) {
						if (g_wProjNo == 0) {
							if (g_lm.sc[n].r.sBuf[SB_CURRENT][0] == g_lm.sc[n].m.addr.c[0] && (g_lm.sc[n].r.sBuf[SB_CURRENT][1] & 0xe0) == 0) {
								int id = (int)(g_lm.sc[n].r.sBuf[SB_CURRENT][1] & 0xf) - 1;
								if (id >= 0 && id < 4) {
									bMatch = true;
									cEnr = 1 << id;
								}
							}
						}
						else {
							if (g_lm.sc[n].r.sBuf[SB_CURRENT][1] == g_lm.sc[n].m.addr.c[1] && (g_lm.sc[n].r.sBuf[SB_CURRENT][0] & 0xe0) == 0) {
								int id = (int)(g_lm.sc[n].r.sBuf[SB_CURRENT][0] & 0xf) - 1;
								if (id >= 0 && id < 4) {
									bMatch = true;
									cEnr = 1 << id;
								}
							}
						}
					}
					//else if (g_lm.sc[n].m.mode.b.b1) {
					//	if (g_lm.sc[n].r.sBuf[SB_CURRENT][0] == g_lm.sc[n].m.addr.c[0] && g_lm.sc[n].r.sBuf[SB_CURRENT][1] == g_lm.sc[n].m.addr.c[1])
					//		bMatch = true;
					//}
					else	bMatch = true;
					//else if (g_lm.sc[n].r.sBuf[SB_CURRENT][0] == g_lm.sc[n].m.addr.c[0] && g_lm.sc[n].r.sBuf[SB_CURRENT][1] == g_lm.sc[n].m.addr.c[1])
					//	bMatch = true;
					//else	printf("unmatch address 0x%02x%02x 0x%02x%02x\r\n",
					//		g_lm.sc[n].r.sBuf[SB_CURRENT][0], g_lm.sc[n].r.sBuf[SB_CURRENT][1], g_lm.sc[n].m.addr.c[0], g_lm.sc[n].m.addr.c[1]);	// ?????
				}

				if (bMatch) {
					if ((g_lm.sc[n].m.enr.b.disr & cEnr) == 0)	g_lm.sc[n].wRespt = RESPOND_DELAY;
					if (!g_lm.sc[n].m.conf.b.dish) {
						int si = g_trxl[g_wProjNo][g_lm.sc[n].m.cDevID].c0;
						int leng = g_trxl[g_wProjNo][g_lm.sc[n].m.cDevID].l0;
						if (leng > 0) {
							bool bDiff = false;
							if (memcmp(&g_lm.sc[n].r.sBuf[SB_STORAGE][si], &g_lm.sc[n].r.sBuf[SB_CURRENT][si], leng))	bDiff = true;
							if (!bDiff) {
								si = g_trxl[g_wProjNo][g_lm.sc[n].m.cDevID].c1;
								leng = g_trxl[g_wProjNo][g_lm.sc[n].m.cDevID].l1;
								if (leng > 0 && memcmp(&g_lm.sc[n].r.sBuf[SB_STORAGE][si], &g_lm.sc[n].r.sBuf[SB_CURRENT][si], leng))	bDiff = true;
							}
							if (bDiff) {
								memcpy(g_lm.sc[n].r.sBuf[SB_STORAGE], g_lm.sc[n].r.sBuf[SB_CURRENT], g_trxl[g_wProjNo][g_lm.sc[n].m.cDevID].r);
								PrintSDR(n);
							}
						}
					}
				}
			}
			else	printf("ch%d received %dbytes!\r\n", n, g_lm.sc[n].r.i);
		}
	}
}

void SendSDA(int pn, bool bResp)
{
	int leng = 0;
	if (bResp && g_lm.sc[pn].r.sBuf[SB_CURRENT][2] == DEVTRACE_REQ) {
		memcpy(g_lm.sc[pn].t.sBuf[SB_CURRENT], g_lm.sc[pn].t.sBuf[SB_STORAGE], 3);
		g_lm.sc[pn].t.sBuf[SB_CURRENT][0] = g_lm.sc[pn].m.addr.c[0];
		g_lm.sc[pn].t.sBuf[SB_CURRENT][1] = g_lm.sc[pn].m.addr.c[1];
		g_lm.sc[pn].t.sBuf[SB_CURRENT][2] = DEVCHAR_CTL;
		g_lm.sc[pn].t.sBuf[SB_CURRENT][3] = DEVTRACE_ANS;
		g_lm.sc[pn].t.sBuf[SB_CURRENT][4] = g_lm.sc[pn].r.sBuf[SB_CURRENT][4];	// block no.
		g_lm.sc[pn].t.sBuf[SB_CURRENT][5] = g_lm.sc[pn].r.sBuf[SB_CURRENT][5];
		g_lm.sc[pn].t.sBuf[SB_CURRENT][6] = g_lm.sc[pn].r.sBuf[SB_CURRENT][6];
		WORD w = MAKEWORDA(g_lm.sc[pn].r.sBuf[SB_CURRENT][5], g_lm.sc[pn].r.sBuf[SB_CURRENT][6]);
		for (int n = 0; n < 100; n ++)
			g_lm.sc[pn].t.sBuf[SB_CURRENT][7 + n] = (BYTE)((w * 100 + n) & 0xff);
		SendPort(pn, g_lm.sc[pn].t.sBuf[SB_CURRENT], 107);
	}
	else {
		leng = g_lm.sc[pn].t.i;
		if (g_wProjNo == 0 && g_lm.sc[pn].m.cDevID == SDID_ATO) {
			if (memcmp(&g_lm.sc[pn].t.sBuf[SB_CURRENT][2], &g_lm.sc[pn].t.sBuf[SB_STORAGE][2], leng - 4))	// without seq[2], crc[2]
				++ g_lm.sc[pn].t.sBuf[SB_STORAGE][1];
			++ g_lm.sc[pn].t.sBuf[SB_STORAGE][1];
			memcpy(g_lm.sc[pn].t.sBuf[SB_CURRENT], g_lm.sc[pn].t.sBuf[SB_STORAGE], leng);	// 1 -> 0
			if (g_lm.sc[pn].m.mode.b.b5)	leng = DleEncoder(g_lm.sc[pn].t.sBuf[SB_CURRENT], leng);
			SendPort(pn, cDleBuf, leng);
		}
		else {
			if (g_wProjNo == 0) {
				g_lm.sc[pn].t.sBuf[SB_CURRENT][0] = g_lm.sc[pn].t.sBuf[SB_STORAGE][0];			// low address;
				if (bResp && (g_lm.sc[pn].m.cDevID == SDID_DCUL || g_lm.sc[pn].m.cDevID == SDID_DCUR))
					g_lm.sc[pn].t.sBuf[SB_CURRENT][1] = g_lm.sc[pn].r.sBuf[SB_CURRENT][1];
				else	g_lm.sc[pn].t.sBuf[SB_CURRENT][1] = g_lm.sc[pn].t.sBuf[SB_STORAGE][1];
			}
			else {
				g_lm.sc[pn].t.sBuf[SB_CURRENT][1] = g_lm.sc[pn].t.sBuf[SB_STORAGE][1];
				if (bResp && (g_lm.sc[pn].m.cDevID == SDID_DCUL || g_lm.sc[pn].m.cDevID == SDID_DCUR))
					g_lm.sc[pn].t.sBuf[SB_CURRENT][0] = g_lm.sc[pn].r.sBuf[SB_CURRENT][0];
				else	g_lm.sc[pn].t.sBuf[SB_CURRENT][0] = g_lm.sc[pn].t.sBuf[SB_STORAGE][0];
			}
			memcpy(&g_lm.sc[pn].t.sBuf[SB_CURRENT][2], &g_lm.sc[pn].t.sBuf[SB_STORAGE][2], leng - 2);	// 1 -> 0
			SendPort(pn, g_lm.sc[pn].t.sBuf[SB_CURRENT], leng);
		}
	}
}

void SendPort(int pn, BYTE* p, WORD len)
{
	PREG(pn, PROFS_COMMAND) = PCW_TXDISABLE;
	//WORD w = PREG(pn, PROFS_CONFIG);
	//DWORD dwAdd = DWREG(pn, PROFS_CONFIG);
	//WORD wLow = (WORD)(dwAdd & 0xffff);
	//WORD wHigh = (WORD)((dwAdd >> 16) & 0xffff);
	//printf("ch%d config 0x%04x%04x 0x%04x\r\n", pn, wHigh, wLow, w);
	//printf("SEND%d ", pn);
	//if (len != PREG(pn, PROFS_TXLENGTH)) {
		//printf("len %d ", len);
		PREG(pn, PROFS_TXLENGTH) = len;
	//}
	//if (len != PREG(pn, PROFS_TXLENGTH))	printf("ch%d tx length error! 0x%04x - 0x%04x\r\n", pn, len, PREG(pn, PROFS_TXLENGTH));
	WORD* wp = (WORD*)p;
	WORD wlen = (len >> 1);
	if (len & 1)	++ wlen;
	while (wlen > 0) {
		//PREG(pn, PROFS_BUFFER) = *wp;
		PREG(pn, PROFS_BUFFER) = *wp ++;
		//printf("%04x ", *wp ++);
		-- wlen;
	}
	//printf("\r\n");
	//printf("ch%d send %d...", pn, len);
	//w = PREG(pn, PROFS_CONFIG);
	//printf("before TXENABLE config(0x%04x)\r\n", w);
	PREG(pn, PROFS_COMMAND) = PCW_TXENABLE;
	//if (!(w & ((1 << PAB_TXENABLE) | 8)))	printf("ch%d tx enable error!(0x%04x)\r\n", pn, w);
	//printf("(0x%04x)\r\n", w);
	/*DWORD dw = 0;
	WORD wConf = 0;
	DWORD dwScanTxEmptyLow = 0;
	DWORD dwScanTxEmptyHigh = 0;
	for ( ; dw < 10000000; dw ++) {
		w = PREG(pn, PROFS_CONFIG);
		wConf |= w;
		if (w & 0x40) {
			if (dwScanTxEmptyLow != 0)	dwScanTxEmptyHigh = dw;
		}
		else {
			if (dwScanTxEmptyLow == 0)	dwScanTxEmptyLow = dw;
		}
		if (w & 2)	break;
	}
	if (dw >= 10000000) {
		printf("not capture flag open 0x%04x\r\n", wConf);
		printf("txEmpty low %ld high %ld\r\n", dwScanTxEmptyLow, dwScanTxEmptyHigh);
	}
	else	printf("OK! %ld\r\n", dw);*/
}

WORD CapturePort(int pn)
{
	if (pn < MAX_PORT) {
		//PREG(pn, PROFS_COMMAND) = PCW_RXDISABLE;
		WORD res = PREG(pn, PROFS_STATUS);
		//if (res & (1 << PSB_RXFULL)) {
			g_lm.sc[pn].r.i = PREG(pn, PROFS_RXLENGTH);
			if (g_lm.sc[pn].r.i > 0) {
				WORD leng = g_lm.sc[pn].r.i;
				if (leng & 1)	++ leng;
				leng >>= 1;
				WORD* wp = (WORD*)g_lm.sc[pn].r.sBuf[SB_CURRENT];
				//printf("RECV%d len %d ", pn, g_lm.sc[pn].r.i );			// ?????
				for (WORD w = 0; w < leng; w ++) {
					*wp ++ = PREG(pn, PROFS_BUFFER);
					//*wp = PREG(pn, PROFS_BUFFER);
					//printf("%04x ", *wp ++);							// ?????
				}
				//printf("\r\nstatus 0x%04x\r\n", res);		// ?????
			}
			g_lm.sc[pn].wStatus = (res | (1 << SCBS_RXFULL));
		//}
		//else	printf("ch%d non full (0x%04x)!\r\n", pn, res);
		//PREG(pn, PROFS_COMMAND) = PCW_RXENABLE;
		return res;
	}
	return 0;
}

int	InitialPort(int pn)
{
	if (pn >= MAX_PORT) {
		printf("port id(%d) over range!\r\n", pn);
		return -1;
	}

	WORD wVert[32];
	for (int n = 0; n < 32; n ++)	wVert[n] = PREG(pn, PROFS_XLIVECODE);

	for (int n = 0; n < 32; n ++) {
		if (wVert[n] != 0x375a) {
			printf("FGPA error!(%d - 0x%04x)\r\n", n, wVert[n]);
			return -2;
		}
	}

	printf("CH%d %s 0x%04x ", pn, g_pDevNames[g_lm.sc[pn].m.cDevID], g_lm.sc[pn].m.addr.w);
	memset(&g_lm.sc[pn].r, 0, sizeof(SCFRM));
	memset(&g_lm.sc[pn].t, 0, sizeof(SCFRM));

	WORD wConfig = 0;
	if (g_lm.sc[pn].m.mode.b.b0) {	// hdlc
		wConfig |= (1 << PAB_HDLC);
		printf("hdlc ");
		if (g_lm.sc[pn].m.mode.b.b1 && g_lm.sc[pn].m.cDevID != SDID_DCUL && g_lm.sc[pn].m.cDevID != SDID_DCUR) {
			wConfig |= (1 << PAB_ADDRESSMATCHENABLE);
			printf("a-match ");
		}
		if (g_lm.sc[pn].m.mode.b.b2) {
			wConfig |= (1 << PAB_MANCHESTER);
			printf("manchester ");
		}
		else if (g_lm.sc[pn].m.mode.b.b3) {
			wConfig |= (1 << PAB_NRZI);
			printf("nrzi ");
		}
		if (g_lm.sc[pn].m.mode.b.b4) {
			wConfig |= (1 << PAB_FCSENABLE);
			printf("fcs ");
		}
	}
	else {	// async.
		printf("async ");
		if (g_lm.sc[pn].m.mode.b.b1) {
			wConfig |= (1 << PAB_STOPLENGTH);
			printf("stopl ");
		}
		if (g_lm.sc[pn].m.mode.b.b2) {
			wConfig |= (1 << PAB_ODDPARITY);
			printf("oddp ");
		}
		else if (g_lm.sc[pn].m.mode.b.b3) {
			wConfig |= (1 << PAB_EVENPARITY);
			printf("evenp ");
		}
		if (!g_lm.sc[pn].m.mode.b.b5 && g_lm.sc[pn].m.mode.b.b4) {
			printf("bcc ");
		}
		else if (g_lm.sc[pn].m.mode.b.b5 && !g_lm.sc[pn].m.mode.b.b4) {
			printf("crca ");
		}
		else if (g_lm.sc[pn].m.mode.b.b5 && g_lm.sc[pn].m.mode.b.b4) {
			printf("crcb ");
		}
	}
	PREG(pn, PROFS_CONFIG) = wConfig;
	if (PREG(pn, PROFS_CONFIG) != wConfig)	printf("ch%d config error! 0x%04x - 0x%04x\r\n", pn, wConfig, PREG(pn, PROFS_CONFIG));

	if (wConfig & (1 << PAB_HDLC)) {
		PREG(pn, PROFS_ADDRESS) = g_lm.sc[pn].m.addr.w;
		if (PREG(pn, PROFS_ADDRESS) != g_lm.sc[pn].m.addr.w)
			printf("ch%d address error! 0x%04x - 0x%04x\r\n", pn, g_lm.sc[pn].m.addr.w, PREG(pn, PROFS_ADDRESS));
	}

	DWORD dwBps = g_dwBpses[g_lm.sc[pn].m.conf.b.bps];
	printf("%ldbps ", dwBps);
	//WORD brate = (WORD)(147456000U / (dwBps * 32U) - 1);
	WORD brate = (WORD)(73728000U / (dwBps * 32U) - 1);
	PREG(pn, PROFS_BAUDRATE) = brate;
	printf("(%d) ", brate);
	if (PREG(pn, PROFS_BAUDRATE) != brate)	printf("ch%d baudrate error! 0x%04x - 0x%04x\r\n", pn, brate, PREG(pn, PROFS_BAUDRATE));

	g_lm.sc[pn].r.i = g_trxl[g_wProjNo][g_lm.sc[pn].m.cDevID].r;
	g_lm.sc[pn].t.i = g_trxl[g_wProjNo][g_lm.sc[pn].m.cDevID].t;
	g_lm.sc[pn].t.sBuf[SB_STORAGE][0] = LOBYTEA(g_lm.sc[pn].m.addr.w);
	g_lm.sc[pn].t.sBuf[SB_STORAGE][1] = HIBYTEA(g_lm.sc[pn].m.addr.w);
	g_lm.sc[pn].t.sBuf[SB_STORAGE][2] = DEVCHAR_CTL;
	g_lm.sc[pn].t.sBuf[SB_STORAGE][3] = DEVFLOW_ANS;

	PREG(pn, PROFS_TXLENGTH) = g_lm.sc[pn].t.i;
	printf("t:%d r:%d ", g_lm.sc[pn].t.i, g_lm.sc[pn].r.i);
	if (PREG(pn, PROFS_TXLENGTH) != g_lm.sc[pn].t.i)
		printf("ch%d txlength error! 0x%04x - 0x%04x\r\n", pn, g_lm.sc[pn].t.i, PREG(pn, PROFS_TXLENGTH));

	PREG(pn, PROFS_SYNCLENGTH) = (WORD)(g_lm.sc[pn].m.enr.b.scl & 7);
	printf("scl:%d ", g_lm.sc[pn].m.enr.b.scl);
	if (PREG(pn, PROFS_SYNCLENGTH) != (WORD)(g_lm.sc[pn].m.enr.b.scl & 7))
		printf("ch%d synclength error! 0x%04x - 0x%04x\r\n", pn, (WORD)(g_lm.sc[pn].m.enr.b.scl & 7), PREG(pn, PROFS_SYNCLENGTH));

	if (wConfig & (1 << PAB_HDLC))
		PREG(pn, PROFS_INTRENABLE) =
				//(WORD)((1 << PIB_RXFULL) | (1 << PIB_RXFCSERROR) |
				//(1 << PIB_RXFRAMEERROR) | (1 << PIB_RXOVERFLOWERROR) |
				//(1 << PIB_RXOVERLAPERROR) | (1 << PIB_RXOVERRUNERROR) |
				//(1 << PIB_RXABORT));
				(WORD)(1 << PIB_RXFULL);
	else	PREG(pn, PROFS_INTRENABLE) = (WORD)((1 << PIB_RXFULL) | (PIB_RXPARITYERROR));

	//PREG(pn, PROFS_COMMAND) = PCW_TXDISABLE;
	//PREG(pn, PROFS_COMMAND) = PCW_RXENABLE;
	wConfig |= (1 << PAB_RXENABLE);
	PREG(pn, PROFS_CONFIG) = wConfig;

	//vert = PREG(pn, PR_STATCTRL);
	//if (!(vert & PA_RXENABLE))	printf("port %d status error 0x%04x\r\n", pn, vert);

	if (g_lm.sc[pn].m.enr.a != 0) {
		printf("disr ");
		if (g_lm.sc[pn].m.enr.b.disr & 1)	printf("1 ");
		if (g_lm.sc[pn].m.enr.b.disr & 2)	printf("2 ");
		if (g_lm.sc[pn].m.enr.b.disr & 4)	printf("4 ");
		if (g_lm.sc[pn].m.enr.b.disr & 8)	printf("8 ");
	}
	printf("\r\n");

	WORD wComp = PREG(pn, PROFS_CONFIG);
	if (wConfig != wComp)	printf("initial failed!(0x%04x -> 0x%04x)\r\n", wConfig, wComp);

	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_IT_FALLING;
	gpio.Pull = GPIO_PULLUP;
	switch (pn) {
	case 0 :
		__HAL_RCC_GPIOB_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_0;
		HAL_GPIO_Init(GPIOB, &gpio);
		HAL_NVIC_SetPriority(EXTI0_IRQn, EXTI_PRIORITY, EXTI_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		break;
	case 1 :
		__HAL_RCC_GPIOB_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_1;
		HAL_GPIO_Init(GPIOB, &gpio);
		HAL_NVIC_SetPriority(EXTI1_IRQn, EXTI_PRIORITY, EXTI_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(EXTI1_IRQn);
		break;
	case 2 :
		__HAL_RCC_GPIOB_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_2;
		HAL_GPIO_Init(GPIOB, &gpio);
		HAL_NVIC_SetPriority(EXTI2_IRQn, EXTI_PRIORITY, EXTI_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(EXTI2_IRQn);
		break;
	case 3 :
		__HAL_RCC_GPIOA_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_3;
		HAL_GPIO_Init(GPIOA, &gpio);
		HAL_NVIC_SetPriority(EXTI3_IRQn, EXTI_PRIORITY, EXTI_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(EXTI3_IRQn);
		break;
	default :
		__HAL_RCC_GPIOF_CLK_ENABLE();
		switch (pn) {
		case 4 :	gpio.Pin = GPIO_PIN_6;	break;
		case 5 :	gpio.Pin = GPIO_PIN_7;	break;
		case 6 :	gpio.Pin = GPIO_PIN_8;	break;
		default :	gpio.Pin = GPIO_PIN_9;	break;
		}
		HAL_GPIO_Init(GPIOF, &gpio);
		HAL_NVIC_SetPriority(EXTI9_5_IRQn, EXTI_PRIORITY, EXTI_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		break;
	}

	return 0;
}

void InitialDefault()
{
	for (int n = 0; n < MAX_PORT; n ++) {
		int devID = (int)g_cDefDevices[g_wProjNo][g_id.cur.address.a][n];
		if (devID != SDID_NON) {
			g_lm.sc[n].m.cDevID = (BYTE)devID;
			g_lm.sc[n].m.addr.w = g_defMold[g_wProjNo][devID].wAddr;
			g_lm.sc[n].m.mode.a = g_defMold[g_wProjNo][devID].cMode;
			g_lm.sc[n].m.conf.b.bps = g_defMold[g_wProjNo][devID].cBpss;
			g_lm.sc[n].m.enr.a = 0;
			InitialPort(n);
		}
	}
}

void ResetEach(int pn)
{
	PREG(pn, PROFS_RESET) = 1;
	for (uint32_t dw = 0; dw < 1000; dw ++);
	PREG(pn, PROFS_RESET) = 0;
	printf("ch%d initial\r\n", pn);
}

void ResetFpgam()
{
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_RESET);
	for (uint32_t dw = 0; dw < 60000; dw ++);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_SET);
	for (uint32_t dw = 0; dw < 60000; dw ++);
}

void InitialFpgam()
{
	hSram.Instance = FSMC_NORSRAM_DEVICE;
	hSram.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

	ramTimType.AddressSetupTime = 1;			// ADDSET		// 1;
	ramTimType.AddressHoldTime = 1;				// ADDHOLD=don't care	// 2;
	ramTimType.DataSetupTime = 6;				// DATAST	// 6;
	ramTimType.BusTurnAroundDuration = 4;		// BUSTURN	// 4;
	ramTimType.CLKDivision = 4;					// CLKDIV=don't care	// 2;
	ramTimType.DataLatency = 4;					// DATLAT=don't care	// 2;
	ramTimType.AccessMode = FSMC_ACCESS_MODE_A;	// ACCMODE

	hSram.Init.NSBank = FSMC_NORSRAM_BANK1;
	hSram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
	hSram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
	hSram.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
	hSram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;	// CBURSTRW
	hSram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;	// WAITPOL
	hSram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;					// WRAPMOD
	hSram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
	hSram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;		// WREN
	hSram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;				// WAITEN
	hSram.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;			// EXTMOD???
	hSram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;	// ASYNCWAIT
	hSram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
	//hSram.Init.ContinuousClock = FSMC_CONTINUOUS_CLOCK_SYNC_ONLY;
	//hSram.Init.WriteFifo = FSMC_WRITE_FIFO_DISABLE;
	//hSram.Init.PageSize = FSMC_PAGE_SIZE_NONE;

	HAL_StatusTypeDef status = { 0 };
	if ((status = HAL_SRAM_Init(&hSram, &ramTimType, &ramTimType)) != HAL_OK)	printf("HAL_SRAM_Init() failed!(%d)\r\n", status);

	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_SET);
	HAL_Delay(500);
}

void IntervalFpgam()
{
	for (int n = 0; n < MAX_PORT; n ++) {
		if (g_lm.sc[n].wRespt != 0 && -- g_lm.sc[n].wRespt == 0)	SendSDA(n, true);
	}
}

void HAL_GPIO_EXTI_Callback(WORD wPin)
{
	switch (wPin) {
	case GPIO_PIN_0 :	CapturePort(0);	break;
	case GPIO_PIN_1 :	CapturePort(1);	break;
	case GPIO_PIN_2 :	CapturePort(2);	break;
	case GPIO_PIN_3 :	CapturePort(3);	break;
	case GPIO_PIN_6 :	CapturePort(4);	break;
	case GPIO_PIN_7 :	CapturePort(5);	break;
	case GPIO_PIN_8 :	CapturePort(6);	break;
	default :			CapturePort(7);	break;
	}
}
