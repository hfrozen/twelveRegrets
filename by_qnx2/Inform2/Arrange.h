/**
 * @file	Arrange.h
 * @brief	객차와 각 장치들 ID.를 정한다.
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "DuoTypes.h"

// 고유 번호
enum enPROPERINDEX {
/* 00 */	PID_LEFT = 0,
/* 01 */	PID_LEFTBK,
/* 02 */	PID_RIGHT,
/* 03 */	PID_RIGHTBK,
/* 04 */	PID_PAS,
//* 05 */	PID_PAS2,
//* 06 */	PID_PAS3,
//* 07 */	PID_PAS4,
//* 08 */	PID_PAS5,
//* 09 */	PID_PAS6,
//* 10 */	PID_PAS7,
//* 11 */	PID_PAS8,
/* 12 */	PID_MAX = 12
};

// 통신 순서에 따른 나열
enum enFLOWINDEX {
/* 00 */	FID_HEAD = 0,	// 0xffff
/* 01 */	FID_HEADBK,		// 0xa502
/* 02 */	FID_TAIL,		// 0xa504
/* 03 */	FID_TAILBK,		// 0xa506
/* 04 */	FID_PAS,		// 0xa508
//* 05 */	FID_PAS2,		// 0xa50a
//* 06 */	FID_PAS3,		// 0xa50c
//* 07 */	FID_PAS4,		// 0xa50e
//* 08 */	FID_PAS5,		// 0xa510
//* 09 */	FID_PAS6,		// 0xa512
//* 10 */	FID_PAS7,		// 0xa514
//* 11 */	FID_PAS8,		// 0xa516
/* 12 */	FID_MAX = 12
};

#define	LENGTH_TU			PID_PAS
#define	LENGTH_CU			(PID_MAX - LENGTH_TU)

// 객차 순서를 그냥 나열, 차별 상황을 분석하거나 고장 분류에 사용,
enum enCARLINEAR {			// 10,		8,		6,		4,
/* 00 */	CID_CAR0 = 0,	// TC,		TC,		TC,		TC
/* 01 */	CID_CAR1,		// M1,		M1,		M1,		T1/M1
/* 02 */	CID_CAR2,		// M2,		M2,		T1,		M2
/* 03 */	CID_CAR3,		// T1,		T1,		M1,
/* 04 */	CID_CAR4,		// M2,		T2,		M2,
/* 05 */	CID_CAR5,		// T2,		M1,
/* 06 */	CID_CAR6,		// T1',		M2,
/* 07 */	CID_CAR7,		// M1,
/* 08 */	CID_CAR8,		// M2,
/* 09 */	CID_CAR9,		// TC,		TC,		TC,		TC
			CID_MAX
};

enum enCARLINEAREX {
	CID_BOW = CID_MAX,		// 고장 분류에서 해당 방향 선두 객차 아이디로 변환된다
	CID_STERN,				// 고장 분류에서 해당 방향 후미 객차 아이디로 변환된다
	CID_ALL					// 고장 분류에서 전 차량을 표시한다
};

#define	DEFAULT_OTRN		0x1111		// 전원 투입 후 열차 번호가 0이면 TRS 통신이 이루어지지 않는다. 하여 이 번호를 기본으로 둔다.
#define	DEFAULT_CARLENGTH	CID_MAX
#define	DEFAULT_CARBITMAP	0x3ff
#define	DEFAULT_FLOWBITMAP	0xfff

#define	DTB_BAUDRATE		1536000		//2304000		//4608000
#define	DTB_BROADCASTADDR	0xffff
#define	DTB_BASEADDR		0xa500
#define	DTB_LASTADDR(leng)	((WORD)(DTB_BASEADDR + ((LENGTH_TU + leng - 3) << 1)))
// TC에서 CC기능도 겸하므로 3을 빼어야한다.

#define	VALID_DTBADDR(add, leng)	(add == DTB_BROADCASTADDR || (add > DTB_BASEADDR && add <= DTB_LASTADDR(leng)))

#define	DTB_IDTOADDR(id)	((WORD)(DTB_BASEADDR + (id << 1)))
#define	DTB_ADDRTOID(add)	((add - DTB_BASEADDR) >> 1)

enum enDTBSTATE {
	DTB_NON = 0,
	DTB_RBROADCAST,
	DTB_RFULL,
	// receive fault
	DTBF_RTIMEOUT,
	DTBF_RADDR,		// 맞지 않는 주소
	DTBF_RADDRB,	// 어드레스 매치 상태(CU)에서 브로드캐스팅 주소가 아닐 떄
	DTBF_RSIZE,
	DTBF_RORDER,
	DTBF_RECHO,
	DTBF_MAX
};

// train device ID.
enum enPOLEDEVINDEX {
	PDEVID_ATO = 0,
	PDEVID_HDU,
	PDEVID_RTD,
	PDEVID_PAU,
	PDEVID_PIS,
	PDEVID_TRS,
	// 200218
	PDEVID_FDU,
	PDEVID_BMS,
	PDEVID_UKD,		// unknown device
	PDEVID_MAX		// 9	// 7
};

enum { PDEVDTB_MAX = PDEVID_UKD };

//enum enPOLEREDINDEX {
//	PDEVRED_ATO = 0,
//	PDEVRED_RTD,
//	PDEVRED_PAU,
//	PDEVRED_PIS,
//	PDEVRED_TRS,
//	// 200218
//	PDEVRED_FDU,
//	PDEVRED_BMS,
//	PDEVRED_MAX		// 5
//};
//
// car device ID.
enum enLOTDEVINDEX {
	DEVID_SIV = 0,
	DEVID_V3F,
	DEVID_ECU,
	DEVID_HVAC,
	// 200218
	DEVID_CMSB,
	DEVID_DCUL,
	DEVID_DCUR,
	DEVID_CM,		// 200218, DEVID_CMSB가 따로 정의되면서 여기는 의미가 없어졌다.
	DEVID_ESK,
	DEVID_PD,		// passage door
	DEVID_PAN,
	DEVID_MAX
};

enum { DEVID_DCU = DEVID_DCUL };
enum { DEVIDDE_UKD = DEVID_CM, DEVIDDE_MAX };	// 7, 8 } comm. devices
enum { REALDEV_MAX = DEVIDDE_MAX + 6 };			// 14
enum { LDEVDTB_MAX = DEVIDDE_MAX + 5 };			// 13
	// 0:siv, 1:v3f, 2:ecu, 3:hvac, 4~7:dcul, 8~11:dcur, 12:cm
	// 0:siv, 1:v3f, 2:ecu, 3:hvac, 4:cmsb, 5~8:dcul, 9~12:dcur, 13:unknown

#define	DEVBF_SIV		(1 << DEVID_SIV)		// 1
#define	DEVBF_V3F		(1 << DEVID_V3F)		// 2
#define	DEVBF_ECU		(1 << DEVID_ECU)		// 4
#define	DEVBF_HVAC		(1 << DEVID_HVAC)		// 8
// 200218
#define	DEVBF_CMSB		(1 << DEVID_CMSB)		// 10	// 40
#define	DEVBF_DCUL		(1 << DEVID_DCUL)		// 20	// 10
#define	DEVBF_DCUR		(1 << DEVID_DCUR)		// 40	// 20
#define	DEVBF_CM		(1 << DEVID_CM)			// 80
#define	DEVBF_ESK		(1 << DEVID_ESK)		// 100
#define	DEVBF_PD		(1 << DEVID_PD)			// 200
#define	DEVBF_PAN		(1 << DEVID_PAN)		// 400

#define	DEVBF_DEF		(DEVBF_DCUL | DEVBF_DCUR | DEVBF_HVAC | DEVBF_ECU)
#define	DEVBF_BCH		(DEVBF_SIV | DEVBF_V3F | DEVBF_ECU | DEVBF_HVAC | DEVBF_DCUL | DEVBF_DCUR)

#define	SIZE_MAXCARLENGTH		16

// 통신용 제어 문자
#define	DEVCHAR_CTL		0x13
#define	DEVFLOW_REQ		0x20
#define	DEVFLOW_ANS		0x30
#define	DEVTRACE_REQ	0x21
#define	DEVTRACE_ANS	0x31

// CPU 내장 통신 포트 할당
enum enPSCCCHA {
	PSCCHA_ATO = 3,		// /dev/ser3
	PSCCHA_HDU = 5,		// /dev/ser5
	PSCCHA_MAX
};

// FPGA 통신 포트 할당
// physical port no. to FPGA
enum enPSCCHB {
	PSCCHB_RTD = 4,		// to connect FPGA port 2
	PSCCHB_SVC,			// follow...
	PSCCHB_HVAC,
	PSCCHB_ECU,
	PSCCHB_SVF,			// SIV/V3F
	PSCCHB_DCUR,		// to connect FPGA port 9
	PSCCHB_DCUL,
	PSCCHB_MAX
};
// FPGA port 0,1 used to DTB

enum enSIVML {	SIV_MAX = 3	};
enum enCMML  {	CM_MAX = 3	};
enum enV3FML {	V3F_MAX = 5	};
enum enECUML {	ECU_MAX = CID_MAX	};
enum enHVACML {	HVAC_MAX = CID_MAX	};
// 200218
enum enCMSBML {	CMSB_MAX = 3	};
