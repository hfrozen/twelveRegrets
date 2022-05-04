/**
 * @file	AisInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#pragma pack(push, 1)

typedef struct _tagSRAIS {
	BYTE	cFlow;			// 0
	BYTE	cCode;			// 1, PA code
	BYTE	cCurrSt;		// 2, current station
	BYTE	cNextSt;		// 3, next station
	BYTE	cDestSt;		// 4, destination station
	__BC(	ref,			// 5
				nrd,	1,	// next right door
				nld,	1,	// next left door
				dcw,	1,	// door close warning
				// 180704
				sp,		3,
				// 180704
				//manu,	1,
				//sp,		2,
				dnf,	1,	// descend inform
				upf,	1);	// ascend inform
	__BW(	no,				// 6-7, train no
				u102,	4,
				u103,	4,
				u100,	4,
				u101,	4);
	__BC(	hd,			// 8
				fire,	1,
				trs0,	1,
				trs9,	1,
				// 180704
				//sp,		3,
				// 210630
				//sp,		2,
				sp,		1,
				mbc,	1,
				mmr,	1,	// master (manual mode) demand
				tcr,	1,
				hcr,	1);
	WORD	wDist;			// 9-10, H/L, distance 5m/bit
	__BW(	fd,				// 11-12, fire detection
				c0,		1,
				c1,		1,
				c2,		1,
				c3,		1,
				c4,		1,
				c5,		1,
				c6,		1,
				c7,		1,
				c8,		1,
				c9,		1,
				sp,		6);
	__BW(	pa,				// 13-14, emergency interphone detection
				c0,		1,
				c1,		1,
				c2,		1,
				c3,		1,
				c4,		1,
				c5,		1,
				c6,		1,
				c7,		1,
				c8,		1,
				c9,		1,
				sp,		6);
	DEVTIME	devt;			// 15~20
} SRAIS, *PSRAIS;

#pragma pack(pop)
