/**
 * @file	Prefix.h
 * @brief	�ʿ��� ���� ���ǰ� ����, �׸��� �޽��� ť�� ����ϴ� �±� ���� ���Ѵ�.
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
#define	TPERIOD_BKUPWARMUP	REAL2PERIOD(2000)			// ����� �� �� Shot()�� ����ũ ��Ű�� �ð�

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
	EPOS_BROADCASTCTIMEOUT = 0,	// 0, at slave, �������� ��ε� ĳ��Ʈ�� DTB A/B ��ο��� ������ ���� �� ������.
	EPOS_BROADCASTATIMEOUT,		// 1, at slave, �������� ��ε� ĳ��Ʈ�� DTB A���� ������ ���� �� ������.
	EPOS_BROADCASTBTIMEOUT,		// 2, at slave, �������� ��ε� ĳ��Ʈ�� DTB B���� ������ ���� �� ������.
	EPOS_RECEIVEATIMEOUT,	// 3, at all, DTB A���� ������ ä��(CC8)�� ������ �ʰ� Ÿ�� �ƿ��� �߻��� �� ������.
	EPOS_RECEIVEBTIMEOUT,	// 4, at all, DTB B���� ������ ä��(CC8)�� ������ �ʰ� Ÿ�� �ƿ��� �߻��� �� ������.
	EPOS_SHOTA,				// 5, DTBA TXCM�� �������� ��
	EPOS_SHOTB,				// 6, DTBB TXCM�� �������� ��
	EPOS_TINTAPERIOD,		// 7, at slave, CPU���� DTB A�� ������ ���� ����ϴ� ���� �ð� ���� ���� ���� �� ������.
	EPOS_TINTBPERIOD,		// 8, at slave, CPU���� DTB B�� ������ ���� ����ϴ� ���� �ð� ���� ���� ���� �� ������.
	EPOS_TINTAFALL,			// 9, at all, CPU���� �߻��� TIMER A INT.�� �����ϴµ� �ణ�� �ð� �Ҹ� ����� ������.
	EPOS_TINTBFALL,			// 10, at all, CPU���� �߻��� TIMER B INT.�� �����ϴµ� �ణ�� �ð� �Ҹ� ����� ������.
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
/*	- ���� �гο� �ִ� 4 * 2 ���� ǥ�ñ� �ι�° �ٿ���...

	00000	0000	11111	11111	<- �ڸ��� ���ڸ�
	01234	56789	01234	56789	<- �ڸ��� ���ڸ�

	ooooo	ooooo	ooooo	ooooo	<- �� ���� �Ʒ��� �ڸ� ���� ���� trouble�� ǥ��
	ooooo	ooooo	ooooo	ooooo	<- �� �ٺ��� �����ϸ� �ҹ��ڸ� �Ἥ ���ڿ��� ǥ���Ѵ�.
	ooooo	ooooo	ooooo	ooooo
	ooooo	ooooo	ooooo	ooooo
	ooooo	ooooo	ooooo	ooooo
	ooooo	ooooo	ooooo	ooooo
	ooooo	ooooo	ooooo	ooooo

	0 ~ 4�� �ڸ������� DTB trouble�� ǥ���Ѵ�.
	5 ~ 9�� �ڸ������� CPU �� hardware trouble�� ǥ���Ѵ�.
	10�� ���ʹ� ����.
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
