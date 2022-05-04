/*
 * CBand.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/neutrino.h>

#include "Arrange.h"
#include "CTool.h"
#include "Track.h"
#include "CBand.h"

CBand::CBand()
{
	memset(&c_record, 0, sizeof(TCHART) * 2);
}

CBand::~CBand()
{
}

const PSZ CBand::c_pszPeriodName[SIZE_DTBTCHART] = {
	(PSZ)"T0,4,",	(PSZ)"T1,4,",	(PSZ)"T2,4,",	(PSZ)"T3,4,",
	(PSZ)"C1,4,",	(PSZ)"C2,4,",	(PSZ)"C3,4,",	(PSZ)"C4,4,",
	(PSZ)"C5,4,",	(PSZ)"C6,4,",	(PSZ)"C7,4,",	(PSZ)"C8,4,",
	(PSZ)"TO,4,",	(PSZ)"CA,4,",	(PSZ)"CB,4,",	(PSZ)"CC,4,"
};

const PSZ CBand::c_pszFuncName[GENERIC_FUNC] = {
	(PSZ)"F00,2,",		// REGION_FROM
	(PSZ)"F01,2,",		// REGION_LOCALSENDBEGIN
	(PSZ)"F02,2,",		// REGION_LOCALSENDEND
	(PSZ)"F03,2,",		// REGION_POLESENDBEGIN
	(PSZ)"F04,2,",		// REGION_POLESENDEND
	(PSZ)"F05,2,",		// REGION_GAZEBEGIN
	(PSZ)"F06,2,",		// REGION_GAZEEND
	(PSZ)"F07,2,",		// REGION_GAZESENDBEGIN
	(PSZ)"F08,2,",		// REGION_GAZESENDEND
	(PSZ)"F09,2,",		// REGION_ARTERIAMEND
	(PSZ)"F10,2,",		// REGION_NEXTCHAPTEREND
	(PSZ)"F11,2,",		// REGION_PREVCHAPTEREND
	(PSZ)"F12,2,",		// REGION_UNIVCTRLBEGIN
	(PSZ)"F13,2,",		// REGION_UNIVCTRLEND
	(PSZ)"F14,2,",	(PSZ)"F15,2,",
	(PSZ)"F16,2,",	(PSZ)"F17,2,",	(PSZ)"F18,2,",	(PSZ)"F19,2,",
	(PSZ)"F20,2,",	(PSZ)"F21,2,",	(PSZ)"F22,2,",	(PSZ)"F23,2,",
	(PSZ)"F24,2,",	(PSZ)"F25,2,",	(PSZ)"F26,2,",	(PSZ)"F27,2,",
	(PSZ)"F28,2,",	(PSZ)"F29,2,"
};

void CBand::Synchro()
{
	memcpy(&c_record[1], &c_record[0], sizeof(TCHART));
	c_record[0].benchm = ClockCycles();
	c_wRegion = 0;
}

void CBand::Synchro(bool bLine)
{
	QWORD cycle = ClockCycles() - c_record[0].benchm;
	double sec = (double)cycle / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	WORD ms = (WORD)(sec * 1e3);
	if (ms > DTBPERIOD_SECTION)	Synchro();
	Section(bLine, 0);
}

void CBand::Backup()
{
	memcpy(&c_record[2], &c_record[0], sizeof(TCHART));
}

void CBand::Section(bool bLine, int sect)
{
	if (sect > SIZE_DTBTCHART)	return;

	int id = bLine ? 1 : 0;
	c_record[0].period[id][sect].cur = ClockCycles();
	c_record[0].period[id][sect].bUsed = true;
}

void CBand::Region(int regn)
{
	if (regn > GENERIC_FUNC)	return;

	c_record[0].funct[regn].cur = ClockCycles();
	c_record[0].funct[regn].bUsed = true;
	c_wRegion = (WORD)regn;
}

int CBand::Bale(bool bType, BYTE* pMesh)
{
	int sum = 0;
	if (bType) {
		sprintf((char*)pMesh, "$TIM,%d,(", sizeof(WORD) * (SIZE_DTBTCHART * 2 + GENERIC_FUNC));
		while (*pMesh != '\0')	{ ++ pMesh; ++ sum; }
		for (int n = 0; n < SIZE_DTBTCHART; n ++) {
			_POUR(pMesh, sum, c_pszPeriodName[n]);
		}
		for (int n = 0; n < GENERIC_FUNC; n ++) {
			_POUR(pMesh, sum, c_pszFuncName[n]);
		}
		*pMesh ++ = ')';
		*pMesh ++ = ',';
		sum += 2;
	}
	else {
		for (int n = 0; n < SIZE_DTBTCHART; n ++) {
			_QUARTET quar;
			for (int m = 0; m < 2; m ++) {
				if (c_record[1].period[m][n].bUsed) {
					QWORD cycle = c_record[1].period[m][n].cur - c_record[1].benchm;
					double sec = (double)cycle / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
					quar.w[m] = (WORD)(sec * 1e5);		// 0.01ms
				}
				else	quar.w[m] = 0xffff;
				if (n < (FID_MAX + 1))
					c_dtbtm[0].wTime[m][n] = quar.w[m] == 0xffff ? 0 : quar.w[m];
			}
			_DASH(pMesh, sum, &(quar.dw), sizeof(DWORD));
		}
		for (int n = 0; n < GENERIC_FUNC; n ++) {
			WORD wp;
			if (c_record[2].funct[n].bUsed) {
				QWORD cycle = c_record[2].funct[n].cur - c_record[2].benchm;				// [2]는 백업된 시간이다.
				double sec = (double)cycle / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
				wp = (WORD)(sec * 1e5);		// 0.01ms
			}
			else	wp = 0xffff;
			_DASH(pMesh, sum, &wp, sizeof(WORD));
		}
		memcpy(&c_dtbtm[1], &c_dtbtm[0], sizeof(DTBTIMES));
	}
	return sum;
}
