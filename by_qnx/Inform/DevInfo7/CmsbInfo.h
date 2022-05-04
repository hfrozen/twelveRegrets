/*
 * CmsbInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	CMSBINFO_H_
#define	CMSBINFO_H_

#pragma pack(1)

// SDR, tcms->cmsb
// addr low - addr high - flow - text.... - crc low - crc high
// const WORD	addr = 0x730a;
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagCMSBRINFO {
	BYTE	nStx;		// 1
	LWORD	wAddr;		// 2 = 0x730a
	BYTE	nFlow;		// 3
	BYTE	nSpr1;		// 4
	PARCEL(	inst,		// 5
				spr7, spr6, spr5, spr4, spr3,
				cmg2,	// CMG2 on
				cmg1,	// CMG1 on
				cmr );	// CMSB on
	BYTE	nCmst;		// 6 CMSB Start Delay (0~60 sec) <-- new
	DEVTIME time;		// 7-12
	BYTE	nSpr10;		// 13
	BYTE	nEtx;		// 14
	BYTE	nBcc1;		// 15
	BYTE	nBcc2;		// 16
} CMSBRINFO, *PCMSBRINFO;

#define	DELAY_CMSBFRONT		0	// 5
#define	DELAY_CMSBREAR		0	// 10

typedef struct _tagCMSBCINFO {
	BYTE	nSpr1;		// 0
	PARCELS( inst, 2,	// 1-2
				spr7, spr6, spr5, spr4, spr3,
				cmg2,	// CMG2 on
				cmg1,	// CMG1 on
				cmr );	// CMSB on
	BYTE	nCmst[2];	// 3-4 CMSB Start Delay (0~60 sec) <-- new
	BYTE	nSpr10;		// 5
} CMSBCINFO, *PCMSBCINFO;

// sda, cmsb->tcms
// addr low - addr high - flow - text.... - crc low - crc high
// const WORD	addr = 0x730a;
// BYTE	nFlow	= 0x30;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagCMSBAINFO {
	BYTE	nStx;		// 1, -3
	LWORD	wAddr;		// 2 = 0x730a
	BYTE	nFlow;		// 3
	PARCEL(	state,		// 4
				// Modified 2013/11/02
				// spr7, spr6, srp5, spr4,
				spr7,
				lcmg,	// long cmg
				srp5, spr4,
				bypass,	// CM Bypass Mode Set
				spr2, spr1,
				invon );	// INV On
	PARCEL(	fail,		// 5
				os,		// Output Short
				olr,	// OLR  Over Load Relay
				invoc,	// OCR
				invol,	// Thermal Over load
				ovr,	// dc input over voltage
				lvr,	// dc input low voltage
				// Modified 2013/11/02
				// spr1, spr0 );
				p15,
				p5 );
	BYTE	nSpr3;		// 6
	BYTE	nSpr4;		// 7
	BYTE	nEtx;		// 8
	BYTE	nBcc1;		// 9
	BYTE	nBcc2;		// 10
} CMSBAINFO, *PCMSBAINFO;

#pragma pack()

#endif	/* CMSBINFO_H_ */
