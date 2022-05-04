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
	2nnn, 3nnn	Ȧ��	:	�ܼ� ��ȯ,	����
	2nnn, 3nnn	¦��	:	���� ��ȯ,	����

	55nn		Ȧ��	:	�ŵ���,		����,	������???
	55nn		¦��	:	��ġ��,		����
	55nn		Ȧ��	:	�ż���,		����
	55nn		¦��	:	����,		����

	99nn				:	�ÿ���
	������				:	ȸ��
*/
/* ===== ���� ��� =====
	1. ���� ����
		1) ���� ��		: 0x31 - TCMS���� �� ���� ���� �޾� �ȳ��Ѵ�.
		2) �Ĺ� ��		: 0x32 - ��� ����.
	2. ���ʿ��� �����̳� ��� �ҷ��� ���
		1) ������ ��	: 0x34 - TCMS���� �� ���� ���� �޾� �ȳ��Ѵ�.
		2) ���峭 ��	: 0x33
		- DU���� "��� ����" ���߰� ǥ�õǰ� �������μ� ���� ���·� �����Ѵ�.
	3. TCMS���� ������ �����ϴ� ���.
		1) ���� ��		: 0x33
		2) �Ĺ� ��		: 0x34, SDR TEXT 5.3 = 1 - PIS�� �� �������� ã�Ƽ� TCMS�� �����ϰ� �ȳ��Ѵ�.
						:		PIS�� SD TEXT 1.4 = 1 �� �����Ѵ�.
		- TCMS���� �ڵ��� �����ϰų� ���� ������ PIS���� �ڵ����� ���������μ� ���� ���·� �����Ѵ�.
	4. PIS���� ������ �����ϴ� ���.
		1) ������ ��	: 0x34, SD TEXT 1.4 = 1 - PIS�� �� �������� ã�Ƽ� TCMS�� �����ϰ� �ȳ��Ѵ�.
								TCMS�� SDR TEXT 5.3�� 1�� �Ͽ� �����Ѵ�.
		2) �ٸ� ��		: 0x33
		- ������ PIS���� �ڵ����� �����ϰų� TCMS���� �ڵ��� ���������μ� ���� ���·� �����Ѵ�.

	1) CLand::Reform()
		pDoz->recip[x].real.motiv.lead.b.piss :	x(FID_HEAD/FID_TAIL)�� pis mode�� �����Ϸ��� ������.
		pDoz->recip[x].real.motiv.lead.b.pisd :	1:PIS/0:HDU - pis mode�� �����Ϸ��� ����̽�.
		CLand::c_bPisManualTrg = true;

		pDoz->recip[x].real.motiv.lead.b.pist = c_bPisManualTrg;
		pDoz->recip[x].real.motiv.lead.b.pisa = c_bPisManualAck;

	2) CLand::Broadcast...
	3) CLand::Superviose()
		if (isitme()) {
			c_bPisManualTrg = false;
*/
