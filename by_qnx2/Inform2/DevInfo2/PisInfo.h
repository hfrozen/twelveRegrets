/**
 * @file	PisInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"
#include "../Arrange.h"
#include "AisInfo.h"

#define	LADD_PIS		0x0260
#pragma pack(push, 1)

// sync, 38400, 300ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA
// SDR, tcms->pis
typedef struct _tagSRPIS {
	SRAIS	ais;			// 0~20
	// 180626
	//BYTE	cSpr[2];		// 21-22
	BYTE	cLw[CID_MAX];	// 21~30
	BYTE	cSpr[2];		// 31-32
} SRPIS;

typedef union _tagPISRTEXT {
	SRPIS	s;
	BYTE	c[sizeof(SRPIS)];
} PISRTEXT;

typedef struct _tagPISRINFO {
	WORD	wAddr;			// 0x0260
	BYTE	cCtrl;			// 0x13
	PISRTEXT	t;
} PISRINFO, *PPISRINFO;

#define	SIZE_PISSR	sizeof(PISRINFO)

// SDA, pis->tcms
typedef struct _tagSAPIS {
	BYTE	cFlow;			// 0
	__BC(	inf,			// 1
				sp1,	1,
				update,	1,
				remote,	1,
				bkup,	1,
				sp2,	4);
	BYTE	cCurrSt;		// 2, current station
	BYTE	cNextSt;		// 3, next station
	BYTE	cDestSt;		// 4, destination station
	__BW(	notr,			// 5-6, train no on the rail
				u102,	4,
				u103,	4,
				u100,	4,
				u101,	4);
	__BC(	ref,			// 7
				dnf,	1,	// descend inform
				upf,	1,	// ascend inform
				// 171130
				//sp,	5,
				//sp2,	1,
				//manu,	1,
				//sp4,	3,
				// 180704
				sp,		2,
				mchg,	1,	// master chanage demand from pis
				mma,	1,	// manual mode acknowlege
				// 180910
				//sp2,	1,
				sim,	1,	// simulated drive
				dow,	1);	// door open warning
	__BC(	dsu[10],		// 8~17
				dl,		1,
				dr,		1,
				dt,		1,
				dh,		1,
				sp,		2,
				tnsu,	1,
				dsuf,	1);
	__BC(	lcdf[10],		// 18~27
				l1,		1,
				l2,		1,
				l3,		1,
				l4,		1,
				l5,		1,
				l6,		1,
				l7,		1,
				l8,		1);
	BYTE	cVer;			// 28
	BYTE	cSpr;			// 29
} SAPIS;

typedef union _tagPISATEXT {
	SAPIS	s;
	BYTE	c[sizeof(SAPIS)];
} PISATEXT;

typedef struct _tagPISAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	PISATEXT	t;
} PISAINFO, *PPISAINFO;

#define	SIZE_PISSA	sizeof(PISAINFO)

typedef struct _tagPISFLAP {
	PISRINFO	r;
	PISAINFO	a;
} PISFLAP, *PPISFLAP;

#pragma pack(pop)

/* on the rail no.
	2nnn, 3nnn	홀수	:	외선 순환,	본선
	2nnn, 3nnn	짝수	:	내선 순환,	본선

	55nn		홀수	:	신도림,		지선,	도착역???
	55nn		짝수	:	까치산,		지선
	55nn		홀수	:	신설동,		지선
	55nn		짝수	:	성수,		지선

	99nn				:	시운전
	나머지				:	회송
*/
/* ===== 수동 모드 =====
	1. 정상 상태
		1) 선두 차		: 0x31 - TCMS에서 역 정보 등을 받아 안내한다.
		2) 후미 차		: 0x32 - 대기 상태.
	2. 한쪽에서 고장이나 통신 불량인 경우
		1) 정상인 쪽	: 0x34 - TCMS에서 역 정보 등을 받아 안내한다.
		2) 고장난 쪽	: 0x33
		- DU에서 "백업 해제" 단추가 표시되고 누름으로서 정상 상태로 복귀한다.
	3. TCMS에서 수동을 조작하는 경우.
		1) 선두 차		: 0x33
		2) 후미 차		: 0x34, SDR TEXT 5.3 = 1 - PIS가 역 정보등을 찾아서 TCMS로 전송하고 안내한다.
						:		PIS는 SD TEXT 1.4 = 1 로 응답한다.
		- TCMS에서 자동을 선택하거나 수동 상태인 PIS에서 자동으로 선택함으로서 정상 상태로 복귀한다.
	4. PIS에서 수동을 조작하는 경우.
		1) 조작한 쪽	: 0x34, SD TEXT 1.4 = 1 - PIS가 역 정보등을 찾아서 TCMS로 전송하고 안내한다.
								TCMS는 SDR TEXT 5.3을 1로 하여 전송한다.
		2) 다른 쪽		: 0x33
		- 조작한 PIS에서 자동으로 선택하거나 TCMS에서 자동을 선택함으로서 정상 상태로 복귀한다.

	1) CLand::Reform()
		pDoz->recip[x].real.motiv.lead.b.piss :	x(FID_HEAD/FID_TAIL)는 pis mode를 변경하려는 운전대.
		pDoz->recip[x].real.motiv.lead.b.pisd :	1:PIS/0:HDU - pis mode를 변경하려는 디바이스.
		CLand::c_bPisManualTrg = true;

		pDoz->recip[x].real.motiv.lead.b.pist = c_bPisManualTrg;
		pDoz->recip[x].real.motiv.lead.b.pisa = c_bPisManualAck;

	2) CLand::Broadcast...
	3) CLand::Superviose()
		if (isitme()) {
			c_bPisManualTrg = false;
*/
