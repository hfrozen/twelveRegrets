/*
 * AtcInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	ATCINFO_H_
#define	ATCINFO_H_

#pragma pack(1)

// SDR, tcms->atc
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagSRATC {
	PARCEL(	state,		// 1
				ctd,	// clear trace data
				mir,	// mode inhibit request, 1:auto, drvl inhibit
				afsba,	// ATC-FSB applied
				mcp,	// M/C in coast position
				mpp,	// M/C in power position
				mfsbp,	// M/C in FSB position
				ebras,	// EBR applied status, 0:EB
				adc );	// all doors closed
	DEVTIME time;		// 2-7
	_TV_	vSpr;		// 8-9, fix to 0x00
	WORD	wChk;		// 10-11
} SRATC;

typedef struct _tagTRATC {
	BYTE	nIndex;
} TRATC;

typedef union _tagATCRTEXT {
	SRATC	sr;
	TRATC	tc;
} ATCRTEXT;

typedef struct _tagATCRINFO {
	LWORD	wAddr;		// = 0xff30
	BYTE	nCtrl;		// , fix to 0x13
	BYTE	nFlow;		// 0, fix to 0x20;
	ATCRTEXT	r;
} ATCRINFO, *PATCRINFO;

#define	SIZE_ATCSR		(sizeof(SRATC) + 4)
#define	SIZE_ATCTR		(sizeof(TRATC) + 4)

// sda, atc->tcms
// BYTE	nFlow	= 0x30;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef	struct _tagSAATC {
	BYTE	nAccn;		// 1, ATC speed count cycle(0-255) increment count
	BYTE	nAccp;		// 2, ATC speed count cycle pulse (0-255)p/100ms
	BYTE	nSpd;		// 3, ATC speed (0-255)Km/h
	PARCEL(	fail,		// 4
				tccf,	// TCMS comminication check Fault
				spr6,	// Automatic Door Open Fault
				msf,	// Mode Switch FSBR
				spff,	// Speed Probe Fault FSBR
				dome,	// Doors Open/ Motion EBR
				bae,	// Brake Assurance EBR
				saar,	// Spurious ATC System Reset
				atcf );	// ATC Hardware Fault
	PARCEL(	err,		// 5
				adcf,	// ATC/ADU Comm Fault
				spr6,
				aacf,	// *ATC/TWC Comm. Fault	// ATC/TWC Comm Fault
				akuf,	// ATC Key-up Fault
				ncaf,	// non controlling atc fault
				dchf,	// direction controller handle FSBR
				mce,	// master controller EBR
				aduf );	// ADU fault
	PARCEL(	mode,		// 6
				odl,	// Open left door
				odr,	// Open right door
				adp,	// ATC Departure Permit
				amv,	// ATC mode valid
				ym,		// Yard mode
				mam,	// ATC Manual mode
				aum,	// Automatic mode
				drm );	// Driver- less mode
#define	ATCMODE_VALID	0x10
#define	ATCMODE_YM		8
#define	ATCMODE_MM		4
#define	ATCMODE_AM		2
#define	ATCMODE_DM		1
#define	ATCMODE_ONLY	(ATCMODE_YM | ATCMODE_MM | ATCMODE_AM | ATCMODE_DM)

	PARCEL(	req,		// 7
				ahbr,	// ATC Holding brake request
				dbp,	// Departure button pressed
				lampt,	// Lamp test
				oatc,	// Operating ATC
				kdn,	// Key Down
				kup,	// Key Up
				fsbr,	// Full service Brake Request
				sap );	// Stop and Proceed
	PARCEL(	sph,		// 8
				wkuf,	// Wayside Key up failure
				a90k,	// 90 km/h
				a80k,	// 80 km/h
				a75k,	// 75 km/h
				a70k,	// 70 km/h
				a65k,	// 65 km/h
				edls,	// EDL Status
				edrs );	// EDR Status
	PARCEL(	spl,		// 9
				a60k,	// 60 km/h
				a55k,	// 55 km/h
				a45k,	// 45 km/h
				a35k,	// 35 km/h
				a25k,	// 25 km/h
				a00k,	// 0 km/h
				ycode,	// Yard Code
				ncode );// No Code
	PARCEL(	tdws,		// 10
				tdn3, tdn2, tdn1, tdn0,	// No. of Trace data(0 - 8)(Hex)
				wsi4, wsi3, wsi2, wsi1 );	// Wheel Size Index 1
	PARCEL(	pos,		// 11
				spr7, spr6,
				nmto,	// no motion timeout fault
				msop,	// mode switch out position
				dcop,	// direction controller out of position
				mcop,	// master controller out of position
				tmir,	// TCMS mode invalid request admitted
				sp0 );	// Automatic door closed fault
	// BYTE	spr12;		// 12
	PARCEL(swi,			// 12, * Mode switch input
				spr7, spr6, spr5, spr4,
				yi,		//
				mi,		//
				ai,		//
				di );	//
#define	ATCSW_YI		8
#define	ATCSW_MI		4
#define	ATCSW_AI		2
#define	ATCSW_DI		1
#define	ATCSW_ONLY		(ATCSW_YI | ATCSW_MI | ATCSW_AI | ATCSW_DI)

	WORD	wChk;		// 13-14, big-endian
} SAATC;

#define	TRACEBLENGTH_ATC	100

typedef struct _tagTAATC {
	BYTE	nIndex;
	BYTE	nText[TRACEBLENGTH_ATC];
} TAATC;

typedef union _tagATCATEXT {
	SAATC	st;
	TAATC	tr;
} ATCATEXT;

typedef struct _tagATCAINFO {
	LWORD	wAddr;		// = 0xff30
	BYTE	nCtrl;		// , fix to 0x13
	BYTE	nFlow;		// 0
	ATCATEXT	d;
} ATCAINFO, *PATCAINFO;

#define	SIZE_ATCSA		(sizeof(SAATC) + 4)
#define	SIZE_ATCTA		(sizeof(TAATC) + 4)

typedef struct _tagATCSTEXT {
	BYTE	n[20];
} ATCSTEXT;

#define	LENGTH_SAMPLINGATC	30

typedef struct _tagATCSTABLE {
	ATCSTEXT	sd[LENGTH_SAMPLINGATC];
} ATCSTABLE, *PATCSTABLE;	// 600byte

#define	MAXBLOCK_ATC	50	// 600 * 8 = 4800byte -> 5000byte
#define	MAXTRACELENGTH_ATC	8

typedef struct _tagATCCHASE {
	TRACEARCH	ta;
	BYTE		d[MAXBLOCK_ATC][TRACEBLENGTH_ATC];
} ATCCHASE, *PATCCHASE;

#pragma pack()

#endif	/* ATCINFO_H_ */
