/*
 * CProbe.cpp
 *
 *  Created on: 2010. 12. 16
 *      Author: Che
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <Printz.h>

#include "CProbe.h"

CProbe::CProbe()
{
}

CProbe::~CProbe()
{
	Close();
}

void CProbe::ClearBuf()
{
	for (UCURV n = 0; n < SIZE_PROBEBUF; n ++)
		c_nBuf[n] = 0;
}

void CProbe::PrintMfError(int ch, int pch, int res, PSZ msg)
{
	if (res < MFB_SUCCESS)
		MSGLOG("%s%d-%d(%d-%s).\r\n", msg, ch, pch, res, strerror(errno));
	else	MSGLOG("%s%d-%d(%d).\r\n", msg, ch, pch, res);
}

const int CProbe::c_probeChes[] = {
	MF_ADCA, MF_ADCB, MF_ADCC, MF_ADCD,
	_PDI, _PDI + 1, _PDI + 2, MF_DINA,
	_PDO, _PDO + 1, _PDO + 2, MF_DOUTB,
	0
};

#define	CH_AIN	0
#define	CH_DIN	CH_AIN + SIZE_AIN
#define	CH_DOUT	CH_DIN + SIZE_DIN

int CProbe::Open()
{
	MFSTATUS status;
	memset(&status, 0, sizeof(MFSTATUS));
	int res;
	for (UCURV n = 0; c_probeChes[n] != 0; n ++) {
		if ((res = mfOpen(c_probeChes[n], &status)) != MFB_SUCCESS) {
			PrintMfError(c_probeChes[n], c_probeChes[n], res,
					(PSZ)"[PROB]ERROR:Can not open port");
			return res;
		}
	}
	return 0;
}

int CProbe::Close()
{
	int res;
	for (UCURV n = 0; c_probeChes[n] != 0; n ++) {
		if ((res = mfClose(c_probeChes[n])) != MFB_SUCCESS) {
			PrintMfError(c_probeChes[n], c_probeChes[n], res,
					(PSZ)"[PROB]ERROR:Can not close port");
			return res;
		}
	}
	return 0;
}

int CProbe::Geta(UCURV ch, WORD* pV)
{
	if (ch >= SIZE_AIN) {
#if	DEBUG <= DEBUG_DRIVER
		MSGLOG("[PROB]ERROR:Invalid analog input ch.\r\n");
#endif
		*pV = 0xffff;
		return OUTOF_PROBECH;
	}

	ClearBuf();
	int res;
	int length = 0;
	if ((res = mfRead((int)c_probeChes[ch + CH_AIN],
			(char*)&c_nBuf[0], &length)) != MFB_SUCCESS) {
		//PrintMfError(ch, c_probeChes[ch + CH_AIN], res,
		//		(PSZ)"[PROB]ERROR:Can not read port");
		return res;
	}
	*pV = MAKEWORD(c_nBuf[0], c_nBuf[1]);
	return 0;
}

int CProbe::Getd(UCURV ch, WORD* pV)
{
	if (ch >= SIZE_DIN) {
#if	DEBUG <= DEBUG_DRIVER
		MSGLOG("[PROB]ERROR:Invalid digital input ch.\r\n");
#endif
		*pV = 0xffff;
		return OUTOF_PROBECH;
	}

	ClearBuf();
	int res;
	int length = 0;
	if ((res = mfRead((int)c_probeChes[ch + CH_DIN],
			(char*)&c_nBuf[0], &length)) != MFB_SUCCESS) {
		//PrintMfError(ch, c_probeChes[ch + CH_DIN], res,
		//		(PSZ)"[PROB]ERROR:Can not read port");
		return res;
	}
	*pV = MAKEWORD(c_nBuf[0], c_nBuf[1]);
	return 0;
}

int CProbe::Putd(UCURV ch, WORD* pV)
{
	if (ch >= SIZE_DOUTEX) {
#if	DEBUG <= DEBUG_DRIVER
		MSGLOG("[PROB]ERROR:Invalid digital output ch.\r\n");
#endif
		return OUTOF_PROBECH;
	}

	ClearBuf();
	int res;
	int length = ch < 3 ? 4 : 3;

	c_nBuf[0] = HIGHBYTE(*pV);
	c_nBuf[1] = LOWBYTE(*pV);

	if ((res = mfWrite(c_probeChes[ch + CH_DOUT],
			(char*)&c_nBuf[0], &length)) != MFB_SUCCESS) {
		PrintMfError(ch, c_probeChes[ch + CH_DOUT], res,
				(PSZ)"[PROB]ERROR:Can not write port");
		return res;
	}
	/*if (ch == 2) {
		WORD w = *pV & 0x100 ? 0x8000 : 0;
		c_nBuf[0] = HIGHBYTE(w);
		c_nBuf[1] = LOWBYTE(w);
		if ((res = mfWrite(CH_DOEXT, (char*)&c_nBuf[0], &length)) != MFB_SUCCESS) {
			PrintMfError(ch, CH_DOEXT, res,
					(PSZ)"[PROB]ERROR:Can not write port");
			return res;
		}
	}*/
	return 0;
}
