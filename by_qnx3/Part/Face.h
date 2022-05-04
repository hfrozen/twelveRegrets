/**
 * @file	Face.h
 * @brief
 * @details
 * @author
 * @date
 */
#pragma once

// 0x08000000 ~ .7fff		external VME area
#define	ADDR_BASE			0x08000000
#define	SIZE_BASE			0x1000000
//#define	SIZE_BASE		 0xffffff

// 0x08008000				I/O bank selector
#define	EXTBANK_SEL			0x8000	// in/out board selector

// general area
#define	GINP_DIPSW			0x8100	// cpu board dip sw
#define	GINP_BPLAN			0x8102	// backplane jumper
#define	GINPBPLAN_LIU2			0	// liu2 jumper
#define	GINPBPLAN_LIU1			2	// liu1 jumper, 
									// 20161114-입력 보드의 수를 첫번째 입력 보드의 DI59로 설정하였으나
									// 그 보드가 동작하지않을 경우에는 해결책이 없으므로 백플랜 값으로 설정한다.
#define	GOUTP				0x8104	// general output, valid ls 3bit
#define	GINP_TACHO			0x8106 	// tacho input (pulse / 100ms)
#define	GINP_PWMPD			0x8108	// pwm period time (us)
#define	GINP_PWMDT			0x810a	// pwm duty - active time (us)

#define	WATCH_CTRL			0x810c	//
#define		WATCHCTRL_FSAFE		15	// enable (fail-safe reset) watchdog
#define		WATCHCTRL_HARDS		14	// enable hard-static watchdog
#define		WATCHCTRL_CROSS		13	// enable cross-watch
#define		WATCHCTRL_VARIA		12	// enable soft-variable watchdog

#define	XWATCH_CV			0x810e	// cross-watch counter
#define	XWATCH_SV			0x8112	// cross-watch set

#define	FWATCH_SV			0x8114
#define	FWATCH_CV			0x8116

#define	SWATCH_SV			0x8118
#define	SWATCH_CV			0x811a

#define	PAIR_CTRL			0x8120	//
#define		PAIRCTRL_OUTMINE	2	// 1로 하면 디지털 출력이 가능하다
#define		PAIRCTRL_DEADOPPO	1	// 1이면 자신의 크로스 카운터(XWATCH_CV) 초과 상태,
									// 상대가 계속 리셋을 하면(WATCH_CTRL read) 초과하지못하므로
									// 상대가 동작(0)/정지(1) 상태를 알 수 있다.
#define		PAIRCTRL_DEADSELF	0	// 1이면 상대방 크로스 카운터(XWATCH_CV) 초과 상태
									// 계속 리셋을 걸면 초과하지못하므로 ?????
									// 자신이 상대방 카운터를 리셋하지 못하고 있다는 것을 나타내므로
									// LIU1이 WAKEUP할 때 이 비트가 1이면 LIU2가 메인임을 알고 다운된다.

// general counter, source = 10.24MHz
#define	GCNT_LOW			0x8180
#define	GCNT_HIGH			0x8182

// panel display
#define	TEXT_CTRL			0x8240
#define		TEXTCTRL_WIDE	1
#define		TEXTCTRL_VIEW	0

#define	TEXT_LUMI			0x8242
#define	TEXT_BUFF			0x8280

// 0x9100 ~ communication area
// access by SCREG macro
#define	COMSTAT				0		// control & status
//#define	COMSTAT_AMEN	15		// address match enable
#define		COMSTAT_TXOE	12		// tx overflow
#define		COMSTAT_TXAB	11		// tx aborted status
#define		COMSTAT_TXAC	10		// tx abort command
//#define	COMSTAT_TXEN	9		// tx enable
#define		COMSTAT_TXCM	8		// tx complete
//#define	COMSTAT_RXM1	7		// rx mode 1
//#define	COMSTAT_RXM0	6		// rx mode 0
//#define	COMSTAT_RXEN	5		// rx enable
#define		COMSTAT_RXOE	4		// rx overflow
#define		COMSTAT_RXAB	3		// rx abort
#define		COMSTAT_RXFE	2		// rx frame error
#define		COMSTAT_RXCE	1		// rx crc error
#define		COMSTAT_RXBE	0		// has been rx data
#define		COMSTAT_RXES	((1 << COMSTAT_RXBE) | (1 << COMSTAT_RXCE) | (1 << COMSTAT_RXFE) | (1 << COMSTAT_RXAB) | (1 << COMSTAT_RXOE))
//#define		COMSTAT_RXERROR	((1 << COMSTAT_RXCE) | (1 << COMSTAT_RXFE) | (1 << COMSTAT_RXAB) | (1 << COMSTAT_RXOE))
#define		COMSTAT_RXERROR	((1 << COMSTAT_RXCE) | (1 << COMSTAT_RXFE) | (1 << COMSTAT_RXOE))

#define	COMTXBF				2		// tx buffer
#define	COMRXBF				4		// rx buffer
#define	COMRXLN				6		// rx length

#define	COMCTRL				10
#define		COMCTRL_AMEN	15		// address match enable
#define		COMCTRL_TRUN	9		// start tx
									// mode,	NRZ,	NRZI,	manch,	non
#define		COMCTRL_ENC1	7		//			0,		0,		1,		1
#define		COMCTRL_ENC0	6		// 			0,		1,		0,		1
#define		COMCTRL_RXEN	5		// rx enable
#define		COMCTRL_ENCOD	((1 << COMCTRL_ENC1) | (1 << COMCTRL_ENC0))
#define		COMCTRL_NRZ		(0)
#define		COMCTRL_NRZI	(1 << COMCTRL_ENC0)
#define		COMCTRL_MANCH	(1 << COMCTRL_ENC1)

#define	COMBAUD				12		// baudrate = 147.456MHz / (32 * (baud + 1)), max 4.608MBps
#define	COMSADD				14
#define	COMRADD				16
#define	COMRCTL				24
#define		COMRCTL_CRXEN	5

#define	COMSYNC				32		// length of "7e"
#define	COMTXBB				128

#define	MAX_COMMBUF			512

#define	GENREGA				0x90fe
#define	GENREGB				0xa0fe
#define	GENREGC				0xb0fe
