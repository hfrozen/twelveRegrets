/*
 * DuInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	DUINFO_H_
#define	DUINFO_H_

#include "Draft.h"

#pragma pack(1)

// for read page
#define	ADDR_INSPECTITEM	901
#define	LENGTH_INSPECTITEM	32
#define	ADDR_ENV			1200
#define	LENGTH_ENV			32
#define	ADDR_WHEEL			1218
#define	LENGTH_WHEEL		32
#define	ADDR_TRACEITEM		1279
#define	LENGTH_TRACEITEM	32
//#define	ADDR_USBITEM	1000
//#define	LENGTH_USBITEM	32

typedef struct _tagTMFAULTINFO {
	WORD	wCarID;
	WORD	wLow;
	WORD	wReal;
	WORD	wHigh;
} TMFAULTINFO, *PTMFAULTINFO;

typedef struct _tagDUREADCMDINFO {
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUREADCMDINFO;

#define	ADDR_REFLASH	12
typedef struct _tagDUREFLASH {
	WORD	state;
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUREFLASH;

#define	ADDR_PAGE		15
typedef struct _tagDUSETPAGEINFO {
	WORD	page;
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUSETPAGEINFO;

#define	ADDR_DUOPERATE	50
typedef struct _tagDUOPERATEINFO {
	WORD	wCrew;			// 50
	BUNCH(	glamp,
				s15, s14, s13, s12,
				flash,		// 11, flash error
				c0,			// 10, car 0
				c1,			// 09, car 1
				sysbk,		// 08, system backup
				error,		// 07, alarm(top screen-left)
				park,		// 06, parking lamp
				security,	// 05, security lamp
				emergency,	// 04, emergency lamp
				atcAux,		// 03, ATC aux.
				atcMain,	// 02, ATC main
				hold,		// 01, hold brake
				fs );		// 51(01), full service brake
	WORD	wOpMode;		// 52(02) : running mode(1:auto, 2:manual, 3:driverless, 4:emergency, 5:yard)
	WORD	wPower;			// 53(03) : power value(100%)
	WORD	wBrake;			// 54(04) : brake value(100%)
	WORD	wDist;			// 55(05) : distance(km)
	WORD	wAtcSpeedCode;	// 56(06) : atc speed code
	WORD	wTrainNo;		// 57(07) : train no.
	WORD	wMascon;		// 58(08) : mascon pos.
	WORD	wSpeed;			// 59(09)
	WORD	wDestine;		// 60(10) : destination
	WORD	wNextStt;		// 61(11) : next station
	WORD	wTheTrain;		// 62(12) : the train.
	WORD	wTrainID;		// 63 : train id.
	WORD	wTrainLength;	// 64 : train length
	WORD	wCarSel;		// 65, not used
	WORD	wYear;			// 66
	WORD	wDate;			// 67 : ms-month, ls-day
	WORD	wTime;			// 68 : ms-hour, ls-minute
	WORD	wSecond;		// 69
	TMFAULTINFO	alarm;		// 70-73
	BUNCH(	hlamp,
				ast1,		// alarm scroll type1
				ast0,		// alarm scroll type0
				s13, s12,
				s11, s10, s9, s8,
				s7, s6,
				tcb,		// tail backward
				hcb,		// head backward
				tcab,		// cabin is rear
				hcab,		// cabin is front
				tcf,		// tail forward
				hcf );		// 74 : head forward
	WORD	wReset;			// 75
	TMFAULTINFO	log;		// 76-79
	BUNCH(	insp,			// 80, inspection ready
				s15, s14, s13, s12,
				s11, s10,
				fwd,		// forward
				nosecur,	// no security brake
				pb7,		// mascon b7
				norescue,	// no rescue mode
				adcl,		// all door close
				manu,		// manual mode
				dmaa,		// door mode auto/auto
				mrps,		// main compressor
				zvr,
				parking );	// parking brake
	WORD	wProgress;		// 81, download percentage
	WORD	wMsg;			// 82
	BUNCH(	duc,			// 83, du control
				drvlessMsg,	// 15,
				kup,		// 14
				kdn,		// 13
				s12,		// pwmout,		// 12
				trialbe,	// 11, trial button enable
				otrvbe,		// 10, overhaul test result view button enable
				ver264,		// 09
				// Modified 2013/11/02
				//v3frst,		// 08, vvvf reset button
				heavyRecipe,	// 08, view heavy receipe
				//disdownload,	// 08, disable download(hide download button)
				logtohard,	// 07, logbook to hard disk
				inspSkip,	// 06, view continue button in inspection
				inspRepeat,	// 05, view repeat button in inspection
				inspExec,	// 04, view stop button in inspection
				inspView,	// 03, view prev/next page in inspection
				progress,	// 02, view download progress bar
				usb,		// 01, view download button
				interview);	// 00, view overhaul button
	DWORD	dwDoor[LENGTH_CC];	// 84-99
	WORD	wRescueMode;	// 100
	// WORD	wSprA[15];		// 101-115
	// WORD	wSprA[13];		// 101-113
	WORD	wDoorEachBypass;// 101
	//WORD	wSprA[12];		// 102-113
	WORD	wResetV3f;		// 102
	WORD	wSevereID;		// 103
	WORD	wSevereCode;	// 104
	WORD	wSprA[9];		// 105-113
	DWORD	dwBoucs;		// 114-115
	DWORD	dwAto;			// 116-117
	DWORD	dwV3f;			// 118-119
	DWORD	dwSiv;			// 120-121
	DWORD	dwEcu;			// 122-123
	DWORD	dwAcmkr;		// 124-125
	DWORD	dwCmsb;			// 126-127
	DWORD	dwFire;			// 128-129
	DWORD	dwPassenger;	// 130-131
	DWORD	dwAtc;			// 132-133
	DWORD	dwSpr;			// 134-135
	DWORD	dwCsc;			// 136-137
	DWORD	dwPis;			// 138-139
	DWORD	dwHtc;			// 140-141
	DWORD	dwPsd;			// 142-143
	DWORD	dwTrs;			// 144-145
	DWORD	dwTc;			// 146-147
	DWORD	dwCc;			// 148-149
	DWORD	dwIvs;			// 150-151
	DWORD	dwIes;			// 152-153
	DWORD	dwHscb;			// 154-155
	DWORD	dwCck;			// 156-157
	DWORD	dwNrbd;			// 158-159
	DWORD	dwCprl;			// 160-161
	DWORD	dwBccs1;		// 162-163
	DWORD	dwBccs2;		// 164-165
	DWORD	dwAscs1;		// 166-167
	DWORD	dwAscs2;		// 168-169
	DWORD	dwK1;			// 170-171
	DWORD	dwK6;			// 172-173
	DWORD	dwBatk;			// 174-175
	DWORD	dwBatkn;		// 176-177
	WORD	wBat1;			// 178
	WORD	wBat0;			// 179
	WORD	wLamp1;			// 180
	WORD	wLamp2;			// 181
	DWORD	dwDcu[8];		// 182-197
	BUNCH(	pform,
				s15, s14, s13, s12,
				s11, s10, s9, s8,
				s7, s6, s5,
				error,
				alarm,
				close,
				open,
				bypass );	// 198(1)
	WORD	wDoorMode;		// 199(2) : 1:AOAC, 2:AOMC, 3:MOMC
	WORD	wPattern[8];	// 200-207(3-10)
	WORD	wHcc;			// 208
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUOPERATEINFO;

typedef struct _tagDUMBDEVICE {
	WORD	wBus[12];		// 0-...-11		224
	WORD	wAtc[2];		// 12-13		236
	WORD	wAto;			// 14			238
	WORD	wV3f[4];		// 15-16-17-18	239
	WORD	wSiv[2];		// 19-20		243
	WORD	wEcu[8];		// 21-...-28	245
	WORD	wCmsb[2];		// 29-30		253
	WORD	wDcu[8][8];		// 31-...-94	255
	WORD	wCsc[2];		// 95-96		319
	WORD	wPis[2];		// 97-98		321
	WORD	wHtc[2];		// 99-100		323
	WORD	wTrs[2];		// 101-102		325
	WORD	wPsd[2];		// 103-104		327
} DUMBDEV, *PDUMBDEV;

#define	ADDR_DUMONIT	220
typedef struct _tagDUMONITINFO {
	WORD	wMainNode;		// 220
	WORD	wMainBus;		// 221
	WORD	wResvNode;		// 222
	WORD	wResvBus;		// 223
	DUMBDEV	dumb;			// 224-...-328
	WORD	wTcVer[4];		// 329-...-332
	WORD	wCcVer[8];		// 333-...-340
	WORD	wSpr[2];		// 341-342
	WORD	wDiag[4];		// 343-344-345-346, not used
	DWORD	dwPanelNo[12];	// 347-...-370, where is ?????
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUMONITINFO;				// 151word(302byte)

#define	ADDR_DUSTATUSA	500
typedef struct _tagDUSTATUSINFOA {
	WORD	wFc[4];			// 500-503
	// Modified 2012/12/17
	//WORD	wTrqRef[4];		// 504-507
	//WORD	wIq[4];			// 508-511
	WORD	wIq[4];			// 504-507
	WORD	wTrqRef[4];		// 508-511
	SHORT	tbe[4];			// 512-515
	WORD	wVl[4];			// 516-519
	WORD	wIm[4];			// 520-523
	WORD	wSpr24[6];		// 524/5/6/7/8/9
	WORD	wEd[2];			// 530-531
	WORD	wEo[2];			// 532-533
	WORD	wIo[2];			// 534-535
	WORD	wOf[2];			// 536-537
	WORD	wSiv;			// 538
	WORD	wExt;			// 539
	WORD	wAsp[8];		// 540/1/2/3/4/5/6/7
	WORD	wBcp[8];		// 548/9/0/1/2/3/4/5
	WORD	wBed[4];		// 556/7/8/9
	WORD	wBea[4];		// 560/1/2/3
	WORD	wSpr64[6];		// 564/5/6/7/8/9
	WORD	wMcDrv[4];		// 570/1/2/3
	WORD	wBat[4];		// 574/5/6/7
	WORD	wRescue;		// 578
	WORD	wSpr79;			// 579
	WORD	wLamp1;			// 580
	WORD	wLamp2;			// 581
	WORD	wMsg[40];		// 582-621
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUSTATUSINFOA;

typedef struct _tagDUATOSTATUS {
	WORD	wPsn;			// 814
	WORD	wNsn;			// 815
	WORD	wDsn;			// 816
	_TV_	vTbeB;			// 817
	_TV_	vPwbB;			// 818
	BUNCH(	st,				// 819
				tcd,	// 15
				dar,	// 14
				dcw,	// 13
				ddns,	// 12
				spr11,
				spr10,
				stb,	// 9
				idft,	// 8
				ator,	// 7
				pssmf,	// 6
				f1,		// 5
				f2,		// 4
				f3,		// 3
				f4,		// 2
				f5,		// 1
				f6 );	// 0
} DUATOSTATUS;

typedef struct _tagATCSTATUS {
	WORD	wSpeed;			// 820
	BUNCH(	sta,			// 821
				spr15,
				dchf,	// 14
				msop,	// 13
				dcop,	// 12
				mcop,	// 11
				tmir,	// 10
				mce,	// 9
				edls,	// 8
				ahbr,	// 7
				dbp,	// 6
				msf,	// 5
				spff,	// 4
				dome,	// 3
				bae,	// 2
				fsbr,	// 1
				edrs );	// 0
	BUNCH(	stb,			// 822
				spr15, spr14, spr13, spr12,
				yi,		// 11
				mi,		// 10
				ai,		// 9
				di,		// 8
				odl,	// 7
				odr,	// 6
				adp,	// 5
				amv,	// 4
				ym,		// 3
				mam,	// 2
				aum,	// 1
				drm );	// 0
	WORD	wheel;			// 823
} DUATCSTATUS;

#define	ADDR_DUSTATUSB	690		//590
typedef struct _tagDUSTATUSINFOB {
	WORD	wTcDib[4][4];	// 690	//590/1/2/3-594/5/6/7-598/9/0/1-602/3/4/5
	WORD	wTcDob[4][4];	// 706	//606/7/8/9-610/1/2/3-614/5/6/7-618/9/0/1
	WORD	wCcDib[8][3];	// 722	//622:4 625:7 628:0 631:3-634:6 637:9 640:2 643:5
	WORD	wCcDob[8][3];	// 746	//646:8 649:1 652:4 655:7-658:0 661:3 664:6 667:9
	WORD	wPsgRate[9];	// 770	//670:8
	WORD	wPsgWeight[9];	// 779	//679:7
	WORD	wSpeed[4][4];	// 788	//688-703
	WORD	wRelease;		// 804	//704 -> DetectZeroSpeedTime
	WORD	wPtime;			// 805
	WORD	wPdist;			// 806
	WORD	wBtime;			// 807
	WORD	wBdist;			// 808
	WORD	wAccel;			// 809
	WORD	wAccelGraph;	// 810
	WORD	wDecelGraph;	// 811
	WORD	wDist;			// 812
	BUNCH(	state,			// 813
			spr15, spr14, spr13, spr12,
			spr11, spr10, spr9, spr8,
			spr7, spr6, spr5, spr4,
			spr3, spr2,
			disoverhaul,	// hide overhaul button
			enDownload );	// enable trial data downloading
	DUATOSTATUS	ato;		// 814-819
	DUATCSTATUS	atc;		// 820-823
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUSTATUSINFOB;				// 179word(358byte)

typedef struct _tagDUINSPECTATC {
	WORD	wDlr;			// 1(921,936)
	WORD	wDl;			// 2(922,937)
	WORD	wDrr;			// 3(923,938)
	WORD	wDr;			// 4(924,939)
	WORD	wBrk;			// 5(925,940)
	WORD	wSlr;			// 6(926,941)
	WORD	wSl;			// 7(927,942)
	WORD	wShr;			// 8(928,943)
	WORD	wSh;			// 9(929,944)
	WORD	wSenser;		// 10(930,945)
	WORD	wSense;			// 11(931,946)
	WORD	wOslr;			// 12(932,947)
	WORD	wOshr;			// 13(933,948)
	WORD	wOsl;			// 14(934,949)
	WORD	wOsh;			// 15(935,950)
} DUINSPECTATC;

typedef struct _tagDUINSPECTSIGNAL {
	WORD	wTrs;			// 1(920)
	DUINSPECTATC	atc[2];	// 2-16(921-935), 17-31(936-950)
	WORD	wAto;			// 32(951)
} DUINSPECTSIGNAL;

typedef struct _tagDUINSPECTSIVITEM {
	WORD	v;				// (954,958)
	WORD	f;				// (955,959)
	WORD	c;				// (956,960)
	WORD	i;				// (957,961)
} DUINSPECTSIVITEM;

typedef struct _tagDUINSPECTSIV {
	WORD	wRes;			// 1(953)
	DUINSPECTSIVITEM	item[2];	// 2-9(954-961)
	WORD	wStdV;			// 10(962)
	WORD	wStdF;			// 11(963)
	DWORD	dwState;		// 12-13(964-965)
	WORD	wEsk;			// 14(966)
	WORD	wExtRes;		// 15(967)
} DUINSPECTSIV;

typedef struct _tagDUINSPECTPRESS {
	WORD	wRes;			// 1(968)
	WORD	wAsp[8];		// 2-9(969-976)
	WORD	wBcp[8];		// 10-17(977-984)
	WORD	wEtc;			// 18(985)
	WORD	wMcp;			// 19(986)
} DUINSPECTPRESS;

typedef struct _tagDUINSPECTECUITEM {
	WORD	w;				// 1
	WORD	req;			// 2
	WORD	as;				// 3
	WORD	bc;				// 4
} DUINSPECTECUITEM, *PDUINSPECTECUITEM;

typedef struct _tagDUINSPECTECU {
	WORD	wRes;			// 1(987)
	WORD	wAsRes;			// 2(988)
	DUINSPECTECUITEM	svc;	// 3-6(989-992)
	DUINSPECTECUITEM	eb;		// 7-10(993-996)
	DUINSPECTECUITEM	as;		// 11-14(997-1000)
} DUINSPECTECU;

typedef struct _tagDUINSPECTV3F {
	WORD	wRes;			// 1(1001)
	WORD	wPwr;			// 2(1002)
	WORD	wBoot;			// 3(1003)
	WORD	wRam;			// 4(1004)
	WORD	wLock;			// 5(1005)
	WORD	wVer[4];		// 6-9(1006-1009)
	WORD	wOvd;			// 10(1010)
	WORD	wOcd;			// 11(1011)
	WORD	wLoad;			// 12(1012)
} DUINSPECTV3F;

typedef struct _tagDUINSPECTDRIVE {
	WORD	wRes;			// 1(1013)
	WORD	wCapV[4];		// 2-5(1014-1017)
	WORD	wTqC[4];		// 6-9(1018-1021)
	WORD	wRtqC[4];		// 10-13(1022-1025)
	SHORT	tbe[4];			// 13-16(1026-1029)
	WORD	wVl[4];			// 17-20(1030-1033)
} DUINSPECTDRIVE;

typedef struct _tagDUINSPECTCMSB {
	WORD	wRes;			// 1(1034)
	WORD	wTime[2];		// 2-3(1035-1036)
} DUINSPECTCMSB;

typedef struct _tagDUINSPECTHEAT {
	WORD	wRes;			// 1(1037)
	WORD	wHeat[3];		// 2-4(1038-1040)
} DUINSPECTHEAT;

typedef struct _tagDUINSPECTCOOL {
	WORD	wRes;			// 1(1041)
	WORD	wCool[2];		// 1-2(1042-1043)
	WORD	wFan[2];		// 3-4(1044-1045)
	WORD	wExhaust;		// 5(1046)
	WORD	wDamper[2];		// 6-7(1047-1048)
} DUINSPECTCOOL;

typedef struct _tagDUINSPECTDOORB {
	WORD	wRes;			// 1(1049)
	WORD	wRight;			// 2(1050)
	WORD	wLeft;			// 3(1051)
} DUINSPECTDOORB;

typedef struct _tagDUINSPECTTCMS {
	WORD	wRes;			// 1(1052)
	WORD	wTc;			// 2(1053)
	WORD	wTcVer[4];		// 3-6(1054-1057)
	WORD	wCc;			// 7(1058)
	WORD	wCcVer[8];		// 8-15(1059-1066)
} DUINSPECTTCMS;

#define	ADDR_DUINSPECT	919
typedef struct _tagDUINSPECTINFO {
	WORD			wMode;	// 919
	DUINSPECTSIGNAL	sig;	// 920-951
	WORD			wNull;	// 952
	DUINSPECTSIV	siv;	// 953-967
	DUINSPECTPRESS	bc;		// 968-986
	DUINSPECTECU	ecu;	// 987-1000
	DUINSPECTV3F	v3f;	// 1001-1012
	DUINSPECTDRIVE	drv;	// 1013-1033
	DUINSPECTCMSB	cmsb;	// 1034-1036
	DUINSPECTHEAT	heat;	// 1037-1040
	DUINSPECTCOOL	cool;	// 1041-1048
	DUINSPECTDOORB	doorb;	// 1049-1051
	DUINSPECTTCMS	tcms;	// 1052-1066
	WORD			wLamp;	// 1067
	WORD			wTouch;	// 1068			149, 298-299
	WORD	wYear;			// 1069
	WORD	wDate;			// 1070 : ms-month, ls-day
	WORD	wTime;			// 1071 : ms-hour, ls-minute
	WORD	wSecond;		// 1072
	BYTE	nCSum;			// 1073	// 1069			, 300
	BYTE	nCr;			// 1073	// 1069			, 301
	BYTE	nLf;			// 1074	// 1070			, 302
} DUINSPECTINFO, *PDUINSPECTINFO;

#define	SIZE_PRESSURELIST	12
typedef struct _tagPRESSUREINFO {
	WORD	wRes[SIZE_PRESSURELIST];		// 303-,12*2
	WORD	wEtc[SIZE_PRESSURELIST];		// 327-,12*2
	WORD	wMcp[SIZE_PRESSURELIST];		// 351-,12*2
	WORD	wAsp[SIZE_PRESSURELIST][8];		// 375-,12*2*8
	WORD	wBcp[SIZE_PRESSURELIST][8];		// 567-,12*2*8	456byte
} PRESSUREINFO, *PPRESSUREINFO;

typedef struct _tagINSPECTSPEC {
	DUINSPECTINFO	di;
	PRESSUREINFO	pi;
	TBTIME			time;		// 759,763, 9
} INSPECTSPEC, *PINSPECTSPEC;

#define	ADDR_SELITEM	1101
typedef struct _tagSELINFO {
	WORD	wItem[SIZE_FILESBYDAY][3];	// 1101-1190
	WORD	wMine;					// 1191
	WORD	wOppo;					// 1192
	WORD	wSide;					// 1193
	WORD	wSub;					// 1194, liu1/liu2
	WORD	wLength;				// 1196
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUSELITEM;

#define	ADDR_DUENV		1200
typedef struct _tagDUENVINFO {
	// Modified 2013/02/05
//	DWORD	dwVar[6];			// 1200-1211	// DWORD	dwVar[4];		// 1200-1207
	DWORD	dwVar[5];			// 1200-1209
	WORD	wNull;				// 1210
	//WORD	wTrainNo;			// 1211, not use, 180102,
	WORD	wYear;				// 1212	// 1208
	WORD	wMonth;				// 1213	// 1209
	WORD	wDay;				// 1214	// 1210
	WORD	wHour;				// 1215	// 1211
	WORD	wMinute;			// 1216	// 1212
	WORD	wSec;				// 1217	// 1213
	WORD	wDia[4][4];			// 1218-1233	// 1214-1229
	WORD	wSpr[3][4];			// 1234-1245	// WORD	wSpr[4][4];			// 1230-1245
	WORD	wAtc;				// 1246
	WORD	wAto;				// 1247
	WORD	wSiv[2];			// 1248-1249
	WORD	wV3f[4];			// 1250-1253
	WORD	wEcu[8];			// 1254-1261
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUENVINFO;

#define	ADDR_DUTRACEINFO	1246
typedef struct _tagDUTRACEINFO {
	WORD	wAtc;				// 1246
	WORD	wAto;				// 1247
	WORD	wSiv[2];			// 1248-1249
	WORD	wV3f[4];			// 1250-1253
	WORD	wEcu[8];			// 1254-1261
	WORD	wNull;				// 1262
	WORD	wAtcProgress;		// 1263
	WORD	wAtoProgress;		// 1264
	WORD	wSivProgress[2];	// 1265-1266
	WORD	wV3fProgress[4];	// 1267-1270
	WORD	wEcuProgress[8];	// 1271-1278
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUTRACEINFO;

#define	ADDR_ALARMLIST	1500
// Modified 2013/11/02
//typedef struct _tagDUALARMLINE {
//	WORD	wOccurDate;		// 1, 1500
//	WORD	wOccurTime;		// 2, 1501
//	WORD	wOccurSec;		// 3, 1502
//	WORD	wEraseTime;		// 4, 1503
//	TMFAULTINFO	fc;			// 5-8
//} ALARMLINE;

#define	ALARMLIST_LINES	16
typedef struct _tagDUALARMLIST {
//	ALARMLINE	arm[ALARMLIST_LINE];	// 1500~1627
	WORD		wOccurDate[ALARMLIST_LINES];	// 1500-1515
	WORD		wOccurTime[ALARMLIST_LINES];	// 1516-1531
	WORD		wOccurEraseSec[ALARMLIST_LINES];	// 1532-1547
	WORD		wEraseTime[ALARMLIST_LINES];	// 1548-1563
	WORD		wCarID[ALARMLIST_LINES];	// 1564-1579
	WORD		wLow[ALARMLIST_LINES];		// 1580-1595
	WORD		wReal[ALARMLIST_LINES];		// 1596-1611
	WORD		wHigh[ALARMLIST_LINES];		// 1612-1627
	WORD		wCurrentPage;			// 1628
	WORD		wMaxPages;				// 1629
	WORD		wCurrentFile;			// 1630
	WORD		wMaxFiles;				// 1631
	WORD		wYearSec;				// 1632
	WORD		wDate;					// 1633
	WORD		wTime;					// 1634

	// Inserted 2012/01/16
	WORD		wSortLamp;				// 1635

	BYTE		nCSum;
	BYTE		nCr;
	BYTE		nLf;
} DUALARMLIST;

#define	ADDR_COMMMONIT	1700
typedef struct _tagDUCOMMMONIT {
	BYTE	ratc[SIZE_ATCSR + 1];
	BYTE	aatc[SIZE_ATCSA + 1];
	BYTE	rato[SIZE_ATOSR + 1];
	BYTE	aato[SIZE_ATOSA + 1];
	BYTE	rsiv[SIZE_SIVSR + 1];
	BYTE	asiv[SIZE_SIVSA + 1];
	BYTE	rv3f[SIZE_V3FSR + 1];
	BYTE	av3f[SIZE_V3FSA + 1];
	BYTE	recu[SIZE_ECUSR + 1];
	BYTE	aecu[SIZE_ECUSA + 1];
	BYTE	rcmsb[sizeof(CMSBRINFO) + 1];
	BYTE	acmsb[sizeof(CMSBAINFO) + 1];
	BYTE	nNull;
	WORD	wSortLamp;
	BYTE	nCSum;
	BYTE	nCr;
	BYTE	nLf;
} DUCOMMMONIT;

typedef union _tagDUCJOINT {
	DUREADCMDINFO	rc;
	DUSETPAGEINFO	sp;
	DUOPERATEINFO	op;
	DUMONITINFO		mp;
	DUSTATUSINFOA	sta;
	DUSTATUSINFOB	stb;
	DUENVINFO		ev;
	DUINSPECTINFO	ip;
	DUTRACEINFO		ti;
	DUALARMLIST		al;
	DUCOMMMONIT		cm;
	DUSELITEM		sl;
} DUCJOINT;

// SDR, tcms->du
// const WORD	addr = ????
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagSRDUCINFO {
	BYTE	nEsc;
	BYTE	nFlow;
	WORD	wTargetAddr;
	WORD	wSendLength;
	DUCJOINT	j;
} SRDUCINFO, *PSRDUCINFO;

typedef struct _tagSADUCINFO {
	WORD	wNull[10];
} SADUCINFO, *PSADUCINFO;

typedef struct _tagDUCINFO {
	SRDUCINFO	srDuc;
	SADUCINFO	saDuc;
} DUCINFO, *PDUCINFO;

#pragma pack()

#define	TD_LON		0x00010000	// du has little-endian but this is big-endian
#define	TD_LCOMF	0x01000000
#define	TD_LFAULT	0x00000001
#define	TD_RON		0x00800000
#define	TD_RCOMF	0x80000000
#define	TD_RFAULT	0x00000080
#define	TD_SHIFTCOMF	8
#define	TD_SHIFTFAULT	16

// to little-endlian
//					  B1B0B3B2
#define	TD_DON		0x00010000		// 1, ON
#define	TD_DCOMF	0x01000000		// 2, comm. fault
#define	TD_DCOMFALL	0xff000000
#define	TD_DFAULT	0x00000001		// 4, fault
#define	TD_DBYPASS	0x00010001		// 5, bypass
#define	TD_DCUTOUT	0x01000001		// 6, cutout

#define	TD_WCOMF	0x0100			// 2, comm. fault, word type

#define	TB4_HC	0x0001
#define	TB4_HF	0x0100
#define	TB4_RC	0x0080
#define	TB4_RF	0x8000

#define	oDuc	c_pDucInfo->srDuc.j.op
#define	mDuc	c_pDucInfo->srDuc.j.mp
#define	sDuc	c_pDucInfo->srDuc.j.sta
#define	bDuc	c_pDucInfo->srDuc.j.stb
#define	eDuc	c_pDucInfo->srDuc.j.ev
#define	uDuc	c_pDucInfo->srDuc.j.us
#define	iDuc	c_pDucInfo->srDuc.j.ip
#define	tDuc	c_pDucInfo->srDuc.j.ti
#define	aDuc	c_pDucInfo->srDuc.j.al
#define	cDuc	c_pDucInfo->srDuc.j.cm
#define	lDuc	c_pDucInfo->srDuc.j.sl

//DUCJOINT size 305
//DUHEADINFO size 103
//DUNORMALINFO size 187
//DUMONITINFO size 305
//DUSTATUSINFOA size 247
//DUSTATUSINFOB size 251
//DUENVINFO size 127
//DUUSBINFO size 7
//DUINSPECTINFO size 303
//DUTRACEINFO size 69
//DUALARMLIST size 275
//DUCOMMMONIT size 279

// for DucSetPage
enum {
	DUKEY_NON = 0,
	DUKEY_GENERAL,			// 1
	DUKEY_OVERHAUL,			// 2
	DUKEY_INSPECT,			// 3
	DUKEY_STATUS,			// 4
	DUKEY_TRIAL,			// 5

	DUKEY_RESETED = 9,
	// Modified 2013/11/02
	//DUKEY_SETENV,			// 10
	//DUKEY_SETTIME,			// 11		// not used
	DUKEY_SETENVNTIME,		// 10
	DUKEY_SETTIMEONLY,		// 11
	DUKEY_SETWHEEL,			// 12
	DUKEY_DELETE,			// 13
	DUKEY_DOWNLOAD,			// 14
	DUKEY_DELETEARCHIVES,	// 15
	DUKEY_EXITFROMDOWNLOAD,	// 16
	DUKEY_SETTRAINNO,		// 17
	DUKEY_V3FRESET,			// 18

	DUKEY_ANYPRESS = 24,

	DUKEY_COMMMONIT = 30,
	DUKEY_ALARMLIST,		// 31
	DUKEY_ALARMCOLLECTION,	// 32
	DUKEY_DEVTRACEDL,		// 33
	DUKEY_INSPECTDL,		// 34
	DUKEY_DIRBYDAY,			// 35

	DUKEY_LEFT = 41,
	DUKEY_RIGHT,		// 42
	DUKEY_UP,			// 43
	DUKEY_DOWN,			// 44
	DUKEY_OK,			// 45
	DUKEY_NG,			// 46
	DUKEY_EXIT,			// 47
	DUKEY_ESC,			// 48
	DUKEY_FACK,			// 49
	DUKEY_ABSOVERHAUL,	// 50

// Appended 2012/01/16 ... begin
	DUKEY_ALARMSORT1 = 61,
	DUKEY_ALARMSORT2,	// 62
	DUKEY_ALARMSORT3,	// 63
	DUKEY_ALARMSORT4,	// 64
	DUKEY_ALARMSORT5,	// 65
	DUKEY_ALARMSORT6,	// 66
	DUKEY_ALARMSORT7,	// 67
	DUKEY_ALARMSORT0,	// 68
	DUKEY_ALARMSORTALL,	// 69
// ... end

// Appended 2012/03/29 ... begin
//	DUKEY_PWMOUT = 90,
// ... end

	DUKEY_RESCUETRAIN = 91,
	DUKEY_RESCUECAR,
	DUKEY_RESCUEUNDERGO,	// for du control, not real du key
	DUKEY_RESCUEEXIT,		// for du control

	DUKEY_SELITEM1 = 101,	// to 130

	DUKEY_SELITEMMAX = 130,
	DUKEY_DIRSELF,			// 131
	DUKEY_DIROPPO,			// 132
	DUKEY_DIRSUB,			// 133

	// Modified 2013/11/02
	DUKEY_ALARMSCROLL = 136,

	DUKEY_MAX
};


enum  {
	DUPAGE_OPERATE,		// 0
	DUPAGE_MONIT,		// 1
	DUPAGE_STATUSA,		// 2
	DUPAGE_STATUSB,		// 3

	DUPAGE_COMMMONIT,	// 4
	DUPAGE_DIRBYDAY,	// 5
	DUPAGE_OPENNORMAL,	// 6
	DUPAGE_OPENOVERHAUL,// 7

	DUPAGE_OPENINSPECT,	// 8
	DUPAGE_OPENTIMESET,	// 9
	DUPAGE_INSPECT,		// 10
	DUPAGE_ENV,			// 11

	DUPAGE_ENVED,		// 12
	DUPAGE_TRACE,		// 13
	DUPAGE_ARMLIST,		// 14
	DUPAGE_RESCUE,		// 15

	DUPAGE_HEADPILE,	// 16
	DUPAGE_TAILPILE,	// 17

	// Modified 2013/11/02
	//DUPAGE_SETENV,		// 18, 'W' command is used from here
	//DUPAGE_SETTIME,		// 19
	DUPAGE_SETENVNTIME,	// 18, 'W' command is used from here
	DUPAGE_SETTIMEONLY,	// 19
	DUPAGE_SETTRAINNO,	// 20
	DUPAGE_SETWHEEL,	// 21
	DUPAGE_INSPECTITEM,	// 22

	DUPAGE_TRACEITEM,	// 23

	DUPAGE_MAX
};
#define	DUPAGE_CIRCLEEND	DUPAGE_COMMMONIT + 1

#endif	/* DUINFO_H_ */
