/**
 * @file	Prefix.h
 * @brief	필요한 여러 조건과 제한, 그리고 메시지 큐에 사용하는 태그 등을 정한다.
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#define	TPERIOD_BASE			10
#define	REAL2PERIOD(x)			(x / TPERIOD_BASE)
#define	DTBPERIOD_SECTION		6		// 3ms, 512byte sending time = 1.12ms at 4.608MBps
#define	DTBPERIOD_MARGIN		0		//3	//0	//-1		//8
#define	DTBPERIOD_CUMARGIN		1

#define	TPERIOD_SPREAD		REAL2PERIOD(100)			// (100 / TPERIOD_BASE)
#define	TPERIOD_GEN			REAL2PERIOD(100)
#define	TPERIOD_ERRORLAMP	REAL2PERIOD(500)

#define	TPERIOD_WAITBROADCASTF	(TPERIOD_SPREAD * 1.2)	// slave wait time for broadcast, first time
#define	TPERIOD_WAITBROADCASTC	TPERIOD_SPREAD			// slave wait time for broadcast, continue...
#define	TPERIOD_WICKET			REAL2PERIOD(200)		//(400)		// (1000 / TPERIOD_BASE)

// 171122
#define	TPERIOD_BKUPWARMUP	REAL2PERIOD(2000)			// 백업이 될 때 Shot()을 마스크 시키는 시간

//#define	DEBUGDTB_INTTERRUPT
//#define	DEBUGLOCAL_ERRORTEXT
//#define	DEBUGLOCAL_STATETEXT
//#define	DEBUGPOLE_ERRORTEXT
//#define	DEBUGPOLE_STATETEXT
//#define	DEBUG_LOCAL
//#define	DEBUG_POLE

#define	REALBYSPREAD(x)			(REAL2PERIOD(x) / TPERIOD_SPREAD)

//#define	LINEDEB_DTB				5
#define	LINEDEB_DTB				10	//211210  100ms*10=1000ms
#define	LINEDEB_POLE			3
// 210803
//#define	LINEDEB_POLEA1			20	// 170918	// 170906, at 200
//#define	LINEDEB_POLEA2			30	// 201207	20, at 24
//#define	LINEDEB_POLEB			10	// 201207
//#define	LINEDEB_POLEC			20	// 210219
//#define	LINEDEB_POLED			50	// 210615
#define	LINEDEB_POLEA			10
#define	LINEDEB_POLEB			20
#define	LINEDEB_POLEC			30
#define	LINEDEB_POLED			50
#define	LINEDEB_LOCAL			10	// 170918	// 1709063
//#define	LINEDEB_DOOR			3

#define	LINEDEB_RECOVER			3			// 170914

#define	DISTANCE_PULSE			25.f
#define	PERIOD_PULSE			REAL2PERIOD(500)

enum enERRPOS {
	EPOS_BROADCASTCTIMEOUT = 0,	// 0, at slave, 마스터의 브로드 캐스트가 DTB A/B 모두에서 들어오지 않을 때 켜진다.
	EPOS_BROADCASTATIMEOUT,		// 1, at slave, 마스터의 브로드 캐스트가 DTB A에서 들어오지 않을 때 켜진다.
	EPOS_BROADCASTBTIMEOUT,		// 2, at slave, 마스터의 브로드 캐스트가 DTB B에서 들어오지 않을 때 켜진다.
	EPOS_RECEIVEATIMEOUT,	// 3, at all, DTB A에서 마지막 채널(CC8)이 들어오지 않고 타임 아웃이 발생한 때 켜진다.
	EPOS_RECEIVEBTIMEOUT,	// 4, at all, DTB B에서 마지막 채널(CC8)이 들어오지 않고 타임 아웃이 발생한 때 켜진다.
	EPOS_SHOTA,				// 5, DTBA TXCM이 뜨지않을 때
	EPOS_SHOTB,				// 6, DTBB TXCM이 뜨지않을 때
	EPOS_TINTAPERIOD,		// 7, at slave, CPU에서 DTB A의 응답을 위해 사용하는 지연 시간 값이 맞지 않을 때 켜진다.
	EPOS_TINTBPERIOD,		// 8, at slave, CPU에서 DTB B의 응답을 위해 사용하는 지연 시간 값이 맞지 않을 때 켜진다.
	EPOS_TINTAFALL,			// 9, at all, CPU에서 발생한 TIMER A INT.를 제거하는데 약간의 시간 소모가 생기면 켜진다.
	EPOS_TINTBFALL,			// 10, at all, CPU에서 발생한 TIMER B INT.를 제거하는데 약간의 시간 소모가 생기면 켜진다.
	//EPOS_TDUBUFOVER,		// 11,
	//EPOS_TDUBUFUNDER,		// 12,
	EPOS_TMMBUFOVER,		// 11,
	EPOS_TMMBUFUNDER,		// 12,
	EPOS_TRTDBUFOVER,		// 13,
	EPOS_TRTDBUFUNDER,		// 14,
	EPOS_LBUFOVER,			// 15,
	EPOS_LBUFUNDER,			// 16,
	EPOS_17,
	EPOS_18,
	EPOS_19,				//
	EPOS_MAX,				//
};
/*	- 전면 패널에 있는 4 * 2 문자 표시기 두번째 줄에서...

	00000	0000	11111	11111	<- 자릿수 십자리
	01234	56789	01234	56789	<- 자릿수 일자리

	ooooo	ooooo	ooooo	ooooo	<- 이 줄은 아래의 자릿 수에 따라 trouble을 표시
	ooooo	ooooo	ooooo	ooooo	<- 이 줄부터 가능하면 소문자만 써서 문자열을 표시한다.
	ooooo	ooooo	ooooo	ooooo
	ooooo	ooooo	ooooo	ooooo
	ooooo	ooooo	ooooo	ooooo
	ooooo	ooooo	ooooo	ooooo
	ooooo	ooooo	ooooo	ooooo

	0 ~ 4번 자리까지는 DTB trouble을 표시한다.
	5 ~ 9번 자리까지는 CPU 내 hardware trouble을 표시한다.
	10번 부터는 예약.
*/

#define	DEBUG_FPGA

#if	defined(DEBUG_FPGA)
#define	DEBUG_FPGASC
#define	DEBUG_FPGAIO
#endif

#define	DEBUG_ALL				0
#define	DEBUG_CLASS				1
#define	DEBUG_HANDLER			2
#define	DEBUG_TAG				3
#define	DEBUG_INTR				4
#define	DEBUG_DRIVER			5
#define	DEBUG_COMM				6
#define	DEBUG_INFO				7
#define	DEBUG_NON				8

#define	_DEBUGL_				DEBUG_DRIVER	//DEBUG_CLASS

#define	_DEBL(x)				(_DEBUGL_ <= x)

//#define	DEBUG_VIRTUALFUNC
//#define	DEBUG_POLESEQ
#define	DEBUG_HDURECV

enum enMESSAGEID {
	TAG_ACCEPT = 1,
	TAG_LINKMC,
	TAG_UNLINKMC,
	TAG_DEBUGMC,
	TAG_SENDBUS,
	TAG_RECEIVEBUS,
	TAG_RECEIVELOCAL,
	TAG_RECEIVEPOLE,
	TAG_RECEIVEDEV,
	TAG_RECEIVETERM,
	TAG_SAVEECUTRACE,
	TAG_END
};

//#define	PRIORITY_TAG			30
//#define	PRIORITY_PERIODTUNE		28
#define	MAX_MANCHESTER			4

#define	PRIORITY_PERIODBUS		30		// where ???
#define	PRIORITY_INTRBUS		29

#define	PRIORITY_TAG			26
#define	PRIORITY_PERIODTUNE		24

#define	PRIORITY_PERIODPORTA	23
#define	PRIORITY_PERIODPORTB	22
#define	PRIORITY_PERIODPORTC	22
#define	PRIORITY_PERIODTERM		22
#define	PRIORITY_INTRPORTB		21
#define	PRIORITY_INTRPORTC		21
#define	PRIORITY_INTRTERM		21
#define	PRIORITY_PERIODFIO		20

#define	PRIORITY_MSG			16
#define	PRIORITY_POST			16
#define	PRIORITY_MXINTR			16
#define	PRIORITY_DEFAULT		10

#define	UNKNOWNOWNER		(PSZ)"SloopJohnB"
#define	SIZE_OWNERBUFF		256

#define	MAINQ_STR			"/tsq"		// ts pump queue

#if	defined(ASSERTP)
#undef	ASSERTP
#endif

#include <assert.h>
#if	_DEBL(DEBUG_INFO)
#define	ASSERT			assert
#define	ASSERTP(p)		assert(p != NULL)
#else
#define	ASSERT
#define	ASSERTP(p)
#endif


//#define SIMULATOR			0	//20220125
