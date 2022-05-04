/**
 * @file	Insp.h
 * @brief	����� �˻�, �ϻ� �˻�, ���� �˻翡�� �����Ǵ� ����
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "Prefix.h"

enum enINSPECTITEM {
	INSPITEM_PDT,		// 1
	INSPITEM_DAILY,		// 2
	INSPITEM_MONTHLY,	// 4
	INSPITEM_ALL,		// 8
	INSPITEM_TCL,		// 0x10
	INSPITEM_ECU,		// 0x20
	INSPITEM_V3F,		// 0x40
	INSPITEM_SIV,		// 0x80
	INSPITEM_COOL,		// 0x100
	INSPITEM_HEAT,		// 0x200
	INSPITEM_COOLSEL,	// 0x400
	INSPITEM_HEATSEL,	// 0x800
	INSPITEM_MAX
};
#define	INSP_MODE	((1 << INSPITEM_PDT) | (1 << INSPITEM_DAILY) | (1 << INSPITEM_MONTHLY))

enum enINSPSTEP {
	INSPSTEP_NON = 0,
	INSPSTEP_RESET,			// branch

	INSPSTEP_DTBREADY,		// 2
	INSPSTEP_DTBCHECK,
	INSPSTEP_DTBJUDGE,

	INSPSTEP_POLEREADY,		// 5
	INSPSTEP_POLECHECK,
	INSPSTEP_POLEJUDGE,

	INSPSTEP_LOCALREADY,	// 8
	INSPSTEP_LOCALCHECK,
	INSPSTEP_LOCALJUDGE,	// branch

	INSPSTEP_ECUB0READY,	// 11
	INSPSTEP_ECUB0ENTRY,
	INSPSTEP_ECUB0CHECK,
	INSPSTEP_ECUB0JUDGE,

	INSPSTEP_ECUB7READY,	// 15
	INSPSTEP_ECUB7ENTRY,
	INSPSTEP_ECUB7CHECK,
	INSPSTEP_ECUB7JUDGE,

	INSPSTEP_ECUEBREADY,	// 19
	INSPSTEP_ECUEBENTRY,
	INSPSTEP_ECUEBCHECK,
	INSPSTEP_ECUEBJUDGE,

	INSPSTEP_ECUASREADY,	// 23
	INSPSTEP_ECUASENTRY,
	INSPSTEP_ECUASCHECK,
	INSPSTEP_ECUASJUDGE,	// branch

	INSPSTEP_V3FREADY,		// 27
	INSPSTEP_V3FIGNITE,
	//INSPSTEP_V3FREADYB,		//180103	// 170928
	INSPSTEP_V3FENTRY,
	INSPSTEP_V3FCHECK,
	INSPSTEP_V3FJUDGE,		// branch

	INSPSTEP_SIVREADY,		// 33
	INSPSTEP_SIVENTRY,
	INSPSTEP_SIVCHECK,
	INSPSTEP_SIVJUDGE,		// branch

	INSPSTEP_COOLREADY,		// 37
	INSPSTEP_COOLENTRY,
	INSPSTEP_COOLCHECK,
	INSPSTEP_COOLJUDGE,		// end

	INSPSTEP_HEATREADY,		// 41
	INSPSTEP_HEATENTRY,
	INSPSTEP_HEATCHECK,
	INSPSTEP_HEATJUDGE,		// end

	INSPSTEP_PAUSE,			// 45
	INSPSTEP_CANCELREADY,	// 46
	INSPSTEP_ENDREADY,		// 47
	INSPSTEP_ENDENTRY,		// 48
	INSPSTEP_FINAL,
	INSPSTEP_MAX
};

#define	DELAY_INSPECTCHECK	REALBYSPREAD(5000)
#define	DELAY_INSPECTJUDGE	REALBYSPREAD(5000)

#define	INSPECTNGLIMIT_DTB		0
#define	INSPECTNGLIMIT_POLE		0
#define	INSPECTNGLIMIT_LOCAL	0
#define	INSPLINE_FCOUNT		0