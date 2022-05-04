/*
 * CSpy.cpp
 *
 *  Created on: 2011. 2. 15
 *      Author: Che
 */

#include "Msg.h"
#include "CSpy.h"

#include "CTcx.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

const PSZ CSpy::c_timeNames[] = {
	(PSZ)"BEGIN",
	(PSZ)"TCOPEN",
	(PSZ)"TCOPENED",
	(PSZ)"BUSSEND",
	(PSZ)"BUSSENDED",
	(PSZ)"LOCALSEND",
	(PSZ)"LOCALSENDED",
	(PSZ)"LOCALCLOSE",
	(PSZ)"PROCBEGIN",
	(PSZ)"PROCEND",
	(PSZ)"TICKERBEGIN",
	(PSZ)"TICKEREND",
	(PSZ)"RESPONSEHU1",
	(PSZ)"RESPONSEHU2",
	(PSZ)"RESPONSETU1",
	(PSZ)"RESPONSETU2",
	(PSZ)"TCFRONT",
	(PSZ)"M1FRONT",
	(PSZ)"M2FRONT",
	(PSZ)"T1",
	(PSZ)"T2",
	(PSZ)"M1REAR",
	(PSZ)"M2REAR",
	(PSZ)"TCREAR",
	(PSZ)"BUSCLOSE",
	(PSZ)"BUSTIMEOUT",
	(PSZ)"TCCLOSE",
	(PSZ)"TCUPDATE",
	(PSZ)"CYCLEEND",
	(PSZ)"PRESENT",
	(PSZ)"PRESENTED",
	(PSZ)"TIMERBEGIN",
	(PSZ)"TIMEREND",
	(PSZ)"MAXSHEET",
	NULL
};

const PSZ CSpy::c_ccNames[] = {
	(PSZ)"TCFRONT",
	(PSZ)"M1FRONT",
	(PSZ)"M2FRONT",
	(PSZ)"T1",
	(PSZ)"T2",
	(PSZ)"M1REAR",
	(PSZ)"M2REAR",
	(PSZ)"TCREAR",
	(PSZ)"ANY",
	(PSZ)"OTHER",
	NULL
};

CSpy::CSpy()
{
	c_pParent = NULL;
}

CSpy::~CSpy()
{
}

void CSpy::Header(char* p, int head)
{
	sprintf(c_buf, "%06d", head);
	for (UCURV n = 0; n < 6; n ++)
		*p ++ = c_buf[n];
}

void CSpy::Nameplate()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	char* p = g_bufLarge;
	memset((PVOID)p, 0, SIZE_BUFLARGE);
	strcpy(p, "000000:INIT:TCYCLE:");
	TOEND(p);
	UCURV n;
	for (n = 1; c_timeNames[n] != NULL; n ++) {
		strcpy(p, c_timeNames[n]);
		TOEND(p);
		*p ++ = '|';
	}
	*p ++ = ';';

	strcpy(p, "BUSMON:REFERENCE");
	TOEND(p);
	sprintf(p, "(%s:%d)", "DUOC", sizeof(BYTE) * 6 + sizeof(WORD) + sizeof(CARFORMATION));
	TOEND(p);
	sprintf(p, "(%s:%d)", "CID", sizeof(CARID));
	TOEND(p);
	sprintf(p, "(%s:%d)", "ADHR", sizeof(BYTE) * 2 + sizeof(TDIRSTATE) + sizeof(HCRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "BUS", sizeof(BUSCYCLE));
	TOEND(p);
	sprintf(p, "(%s:%d)", "REVT", sizeof(REVOLT));
	TOEND(p);
	sprintf(p, "(%s:%d)", "OP", sizeof(OPERATESTATE));
	TOEND(p);
	sprintf(p, "(%s:%d)", "INSP", sizeof(INSPECTSTATE));
	TOEND(p);
	sprintf(p, "(%s:%d)", "PWRP", sizeof(POWERSTATE) + sizeof(PWMBACKUP));
	TOEND(p);
	sprintf(p, "(%s:%d)", "ATOC", sizeof(ATOCMD));
	TOEND(p);
	//sprintf(p, "(%s:%d)", "MCDP", sizeof(MCDSTICK));
	//TOEND(p);
	//sprintf(p, "(%s:%d)", "RESP", sizeof(INSTRUCTA));
	//TOEND(p);
	sprintf(p, "(%s:%d)", "MCRP", sizeof(MCDSTICK) + sizeof(INSTRUCTA));
	TOEND(p);
	sprintf(p, "(%s:%d)", "SP", sizeof(SPEEDCONDITION));
	TOEND(p);
	sprintf(p, "(%s:%d)", "DIST", sizeof(INTERGRAL));
	TOEND(p);
	sprintf(p, "(%s:%d)", "DEVD", sizeof(DEVICESTATE) + sizeof(DMANSTATE));
	TOEND(p);
	sprintf(p, "(%s:%d)", "DOOR", sizeof(DOORSTATE));
	TOEND(p);
	sprintf(p, "(%s:%d)", "PANT", sizeof(PANTOFLOW));
	TOEND(p);
	sprintf(p, "(%s:%d)", "MOVE", sizeof(MOVESTATE));
	TOEND(p);
	sprintf(p, "(%s:%d)", "CALM", sizeof(CARALARM));
	TOEND(p);
	sprintf(p, "(%s:%d)", "EXT", sizeof(EXTSTATE));
	TOEND(p);
	sprintf(p, "(%s:%d)", "BATC", sizeof(BATTERY) + sizeof(float));
	TOEND(p);
	sprintf(p, "(%s:%d)", "BUSM", sizeof(BUSMONIT));
	TOEND(p);
	sprintf(p, "(%s:%d)", "LCM", sizeof(LOCALMONIT));
	TOEND(p);
	sprintf(p, "(%s:%d)", "LCDT", sizeof(BYTE) * LOCALID_MAX);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DEVT", sizeof(WORD) * 8);
	TOEND(p);
	//sprintf(p, "(%s:%d)", "LOGI", sizeof(DEVTIME));
	//TOEND(p);
	sprintf(p, "(%s:%d)", "ARMS", sizeof(SCENEARMARCH));
	TOEND(p);
	sprintf(p, "(%s:%d)", "LOGS", sizeof(SCENELOGARCH));
	TOEND(p);
	sprintf(p, "(%s:%d)", "DUC", sizeof(DUCTRLINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "ESIDE", sizeof(EACHSIDEENTRY));
	TOEND(p);
	sprintf(p, "(%s:%d)", "COND", sizeof(TCONDITION));
	TOEND(p);
	sprintf(p, "(%s:%d)", "AINF", sizeof(ALARMLISTINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "BUSE", sizeof(BYTE) * SIZE_BUSFAULT);
	TOEND(p);
	sprintf(p, "(%s:%d)", "TERBK", sizeof(WORD) + SIZE_TERMINALCODEBACKUP);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DUMMY", SIZE_TCREFERENCE - sizeof(TCREFERENCEREAL));
	TOEND(p);
	*p ++ = '|';

	strcpy(p, "LC");
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ATCRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ATCRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "ATCABUFF", sizeof(ATCAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "ATCBBUFF", sizeof(ATCAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ATCAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ATCAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ATORINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ATOAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(CSCAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(PISAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(TRSAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(HTCAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(PSDAINFO));
	TOEND(p);
	*p ++ = '|';

	for (n = 0; n < LENGTH_TC; n ++) {
		sprintf(p, "TC%d", n + 1);
		TOEND(p);
		p = pTcx->TcNamePlate(p);
		*p ++ = '|';
	}
	for (n = 0; n < LENGTH_CC; n ++) {
		strcpy(p, c_ccNames[n]);
		TOEND(p);
		p = pTcx->CcNamePlate(p);
		*p ++ = '|';
	}
	sprintf(p, "DU");
	TOEND(p);
	sprintf(p, "(%s:%d)", "DU", sizeof(SRDUCINFO));
	TOEND(p);
	*p ++ = '|';
	*p ++ = ';';
	*p ++ = '\n';
	*p = NULL;
	Header(g_bufLarge, (int)strlen(g_bufLarge) - 7);
	Message(g_bufLarge, (int)strlen(g_bufLarge));
}

void CSpy::ReportTime(float cps, POPERATETIME pTime)
{
	char*p = g_bufLarge;
	memset((PVOID)p, 0, SIZE_BUFSMALL);
	sprintf(p, "000000:VARS:TCYCLE:");
	TOEND(p);

	for (UCURV n = TTIME_TCOPEN; n < TTIME_MAXSHEET; n ++) {
		if ((pTime + n)->bUse) {
			uint64_t cycle = (pTime + n)->cur - pTime->cur;
			double sec = (double)cycle / (double)cps;
			sprintf(p, "%d:%.3f|", n, sec * 1000.f);
			TOEND(p);
			(pTime + n)->bUse = FALSE;
		}
		else {
			sprintf(p, "%d:===|", n);
			TOEND(p);
		}
	}
	*p ++ = ';';
	*p ++ = '\n';
	*p = NULL;
	Header(g_bufLarge, (int)strlen(g_bufLarge) - 7);
	Message(g_bufLarge, (int)strlen(g_bufLarge));
}

void CSpy::ReportBus(PTCREFERENCE pTcRef, PLCFIRM pLcFirm,
						PTCDOZ pDoz, PSRDUCINFO pDu)
{
	char* p = g_bufLarge;
	memset((PVOID)p, 0, SIZE_BUFLARGE);
	sprintf(p, "000000:VARS:BUSMON:");
	int length = strlen("000000:VARS:BUSMON:");
	p += length;

	int size = sizeof(TCREFERENCE);
	memcpy((PVOID)p, (PVOID)pTcRef, size);
	p += size;
	length += size;

	size = sizeof(LCFIRM);
	memcpy((PVOID)p, (PVOID)pLcFirm, size);
	p += size;
	length += size;

	size = sizeof(TCDOZ);
	memcpy((PVOID)p, (PVOID)pDoz, size);
	p += size;
	length += size;

	size = sizeof(SRDUCINFO);
	memcpy((PVOID)p, (PVOID)pDu, size);
	p += size;
	length += (size + 2);

	*p ++ = ';';
	*p = NULL;

	Header(g_bufLarge, length - 7);
	Message(g_bufLarge, length);
}

int CSpy::GetReportLength()
{
	int length = strlen("000000:VARS:BUSMON:");
	length += sizeof(TCREFERENCE);
	length += sizeof(LCFIRM);
	length += sizeof(TCFIRM) * LENGTH_TC;
	length += sizeof(CCFIRM) * LENGTH_CC;
	length += (sizeof(SRDUCINFO) + 2);
	return length;
}
