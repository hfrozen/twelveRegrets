/*
 * Archive.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	ARCHIVE_H_
#define	ARCHIVE_H_

#ifndef	_LITTLE_ENDIAN
#include <Define.h>
#include <inttypes.h>
#endif

					//   012345678901234567890123456789
#define	DRVLOG2_5		"TCMS_OPERATING_LOG_DATA_V2.5\r\n"
#define	DRVLOG2_6		"TCMS_OPERATING_LOG_DATA_V2.6\r\n"
#define	DRVLOG2_7		"TCMS_OPERATING_LOG_DATA_V2.7\r\n"

#pragma pack(1)

// for drive archive
typedef struct _tagDRIVEARCHIVES {
/* 01 */	WORD	wId;		// 0-1, index(0-65535)
/* 02 */	WORD	wFno;		// 2-3, car formation no. (ex7153)
/* 03 */	WORD	wTno;		// 4-5, train no. from ato
/* 04 */	BYTE	nLength;	// 6, car length (4,6,8)
/* 05 */	BYTE	nCno;		// 7, car no. (0-7)
/* 06 */	struct {
				BYTE	year	: 6;
				BYTE	mon		: 4;
				BYTE	day		: 5;
				BYTE	hour	: 5;
				BYTE	min		: 6;
				BYTE	sec		: 6;
			} tm;				// 8-...-11
/* 07 */	BYTE	nTci[4][8];		// 12-...-43, digital input of each tc
/* 08 */	BYTE	nTco[4][6];		// 44-...-67, digital output of each tc
/* 09 */	BYTE	nCci[8][6];		// 68-...-115, digital input of each cc
/* 10 */	BYTE	nCco[8][4];		// 116-...-147, digital output of each cc
/* 11 */	PARCEL(mode,		// 148
						odl,	// ATC6.7, open left door
						odr,	// ATC6.6, open right door
						emer,	// emergency mode
						amv,	// ATC6.4, ato mode valid
						ym,		// ATC6.3, atc yard mode
						mam,	// ATC6.2, atc manual mode
						aum,	// ATC6.1, atc auto mode
						drm );	// ATC6.0, atc driverless mode
/* 12 */	PARCEL(req,			// 149
						ahbr,	// by ATC7.7, hold brake of tcms
						bae,	// ATC4.2, brake assurance EBR
						lampt,	// ATC7.5, lamp test
						oatc,	// ATC7.4, operating atc
						kdn,	// ATC7.3, key down
						kup,	// ATC7.2, key up
						fsbr,	// ATC7.1, full service brake request
						sap );	// ATC7.0, stop and proceed
/* 13 */	PARCEL(sph,			// 150
						mwb,	// state of ato text16-17, powering when brake, if > 0 then 1
						a90k,	// ATC8.6
						a80k,	// ATC8.5
						a75k,	// ATC8.4
						a70k,	// ATC8.3
						a65k,	// ATC8.2
						edls,	// ATC8.1
						edrs );	// ATC8.0
/* 14 */	PARCEL(spl,			// 151
						a60k,	// ATC9.7
						a55k,	// ATC9.6
						a45k,	// ATC9.5
						a35k,	// ATC9.4
						a25k,	// ATC9.3
						a00k,	// ATC9.2
						ycode,	// ATC9.1
						ator );	// ATO10.1, ato ready
/* 15 */	PARCEL(oper,		// 152
						p,		// powering
						b,		// braking
						f,		// forward
						r,		// reverse
						hcr,	// TCDOB_HCR, hcr output
						ebm,	// TCDIB_EBRM, m/c emergency input
						ros,	// TCDIB_ROS, rescue input
						sbs );	// TCDIB_SBS. security brake input
/* 16 */	PARCEL(stat,		// 153
						ebcos,	// TCDIB_EBCOS, emergency cutout input
						cprs,	// TCDIB_CPRLS, compulsory release input
						ebs,	// TCDIB_EBS, emergency s/w intput
						dbz,	// TCDOB_DBZA, buzzer state
						mrps,	// CCDIB_MRPS, mrps stat of cc
						ebrs,	// TCDIB_EBBR, emegency brake bypass relay
						sp1,
						sp0 );
/* 17 */	PARCEL(door,		// 154
						dir,	// TCDIB_ADSCR, all door close input
						adsbs,	// TCDIB_ADSBS, all door bypass input
						dosl,	// TCDIB_DOSL, left door open s/w
						dosr,	// TCDIB_DOSR, right door open s/w
						ldos,	// TCDIB_LDOS, left door open s/w
						rdos,	// TCDIB_RDOS, right door open s/w
						dcs,	// TCDIB_DCS, door close s/w
						lrdcs );	// TCDIB_LRDCS, door close s/w
// Modified 2012/01/17 ... begin
///* 18 */	BYTE	nNull;		// 155
/* 18 */	PARCEL(cmsb,		// 155
						invonb,
						invona,
						timerb,
						timera,
						powerb,
						powera,
						readyb,
						readya );
// ... end
/* 19 */	WORD	wSivV[2];	// 156-157-158-159, SIV input voltage(0-3000V)
/* 20 */	DWORD	dwFairDist;	// 160-161-162-163, average atc pulse, atc text2
/* 21 */	DWORD	dwFairSpeed; // 164-165-166-167, average atc speed
/* 22 */	WORD	wDistHcr;	// 168-169, total distance at HCR(100m)
/* 23 */	DWORD	dwDist;		// 170-171-172-173, total distance(km)
/* 24 */	WORD	wDist;		// 174-175, total distance(m)
/* 25 */	WORD	wV3fPowerHcr;	// 176-177, total power at hcr(kw)
/* 26 */	DWORD	dwV3fPower;	// 178-179-180-181, total power(kw)
/* 27 */	WORD	wV3fPower;	// 182-183, total power(w)
/* 28 */	DWORD	dwRevival;	// 184-185-186-187, total revival power(kw)
/* 29 */	WORD	wRevival;	// 188-189, total revival power(w)
/* 30 */	WORD	wMrp;		// 190-191, mrp of ecu(0-10000)
/* 31 */	DWORD	dwSpeed;	// 192-193-194-195, current speed
/* 32 */	WORD	wDistSect;	// 196-197, distance on sector
/* 33 */	BYTE	nCurSt;		// 198, current station code
/* 34 */	BYTE	nNextSt;	// 199, next station code
/* 35 */	WORD	vTbeAto;	// 200-201, tebe at ato
/* 36 */	WORD	vTbeMc;		// 202-203, tebe at mascon
/* 37 */	WORD	vTbeB;		// 204-205, forcing tebe
/* 38 */	WORD	wWeightRate[8];	//206-221, weight-rate of each car
/* 39 */	WORD	wSivPowerHcr;	// 222-223, total power at hcr(kw)
/* 40 */	DWORD	dwSivPower;	// 224-225-226-227, total power(kw)
/* 41 */	WORD	wSivPower;	// 228-229, total power(w)
/* 42 */	BYTE	nBusFaultCounter[24];	// 230-231-232-233 234-235-236-237 238-239-240-241
											// 242-243-244-245 246-247-248-249 250-251-252-253
/* 43 */	BYTE	failCE;		// 254(atc4)
/* 44 */	BYTE	errCE;		// 255(atc5)
/* 45 */	BYTE	modeCE;		// 256(atc6)
/* 46 */	BYTE	reqCE;		// 257(atc7)
/* 47 */	BYTE	sphCE;		// 258(atc8)
/* 48 */	BYTE	splCE;		// 259(atc9)
/* 49 */	BYTE	posCE;		// 260(atc11)
/* 50 */	BYTE	swiCE;		// 261(atc12)
/* 51 */	BYTE	infoOE;		// 262(ato8)
/* 52 */	BYTE	armOE;		// 263(ato9)
/* 53 */	BYTE	stateOE;	// 264(ato10)
/* 54 */	BYTE	failOE;		// 265(ato11)
/* 55 */	BYTE	psmOE;		// 266(ato12)
/* 56 */	BYTE	pssOE;		// 267(ato13)
// Modified 2012/12/10
///* 57 */	BYTE	nDummy[32];	// 268-...-299
/*-56 */	BYTE	nHpbpu[2];	// 268-269(SIV TEXT20)
								// 0-250 -> 0-250C
/*-57 */	BYTE	nHpbpv[2];	// 270-271(SIV TEXT21)
/*-58 */	BYTE	nHpbpw[2];	// 272-273(SIV TEXT22)
/* 59 */	BYTE	nIq[4];		// 274-275-276-277(VVVF TEXT24)
								// 0-250 -> 0-2000A
/*-60 */	BYTE	nMf[4];		// 278-279-280-281(VVVF TEXT27)
								// 0-250 -> 0-200Hz
/*-61 */	BYTE	nBp[4];		// 282-283-284-285(VVVF TEXT28)
								// 0-250 -> 0-100%
/*-62 */	BYTE	nBfc[4];	// 286-287-288-289(VVVF TEXT29)
								// 0-250 -> 0-100%
/*-63 */	BYTE	nAsp[8];	// 290-291-292-293 294-295-296-297(ECU TEXT8)
								// 0-255 -> 0.51~5.71Kg/cm^2
/* 64 */	BYTE	nBcp[8];	// 298-299-300-301 302-303-304-305(ECU TEXT15)
								// 0-255 -> 0-765kPa
/*-65 */	BYTE	nMrp[8];	// 306-307-308-309 310-311-312-313(ECU TEXT17)
								// (0.0 ~ 10.0kg/cm2)
/* 66 */	WORD	wBrake;		// 314-315
/* 67 */	BYTE	nIssue;		// 316
/* 68 */	BYTE	nSlaveCtrl;	// 317
/* 58 */	BYTE	nDummy[12];	// 318-319-320-321
								// 322-323-324-325 326-327-328-329
} DRV_ARCH, *PDRV_ARCH;
#define	SIZE_DRVARCH	100000

typedef struct _tagTRIALARCHIVES {
/* 01 */	WORD	wId;		// 0-1
/* 02 */	struct {
				BYTE	year	: 6;
				BYTE	mon		: 4;
				BYTE	day		: 5;
				BYTE	hour	: 5;
				BYTE	min		: 6;
				BYTE	sec		: 6;
			} tm;				// 2-...-5
/* 03 */	_TV_	vTbeB;		// 6-7
/* 04 */	_TV_	vAccel;		// 8-9
/* 05 */	DWORD	dwSpeed;	// 10-11-12-13
/* 06 */	DWORD	dwDist;		// 14-15-16-17
/* 07 */	BYTE	nTrqRef[4];	// 18-19-20-21
/* 08 */	BYTE	nIq[4];		// 22-23-24-25
/* 09 */	BYTE	nIm[4];		// 26-27-28-29
/* 10 */	BYTE	nAsp[8];	// 30-...-37
/* 11 */	BYTE	nBcp[8];	// 38-...-45
/* 12 */	WORD	wPtime;		// 46-47
/* 13 */	WORD	wPdist;		// 48-49
/* 14 */	WORD	wBtime;		// 50-51
/* 15 */	WORD	wBdist;		// 52-53
/* 16 */	DWORD	dwSampleSpeed;	// 54-55-56-57
/* 17	WORD	wSampleLength;	// 58-59*/
} TRIAL_ARCH, *PTRIAL_ARCH;

// for alarm archive
typedef union _tagALARMARCHIVES {
	struct {
		BYTE	used	: 1;				// >> 63 & 1
		BYTE	alarm	: 1;	// 2		// >> 62 & 1
		BYTE	type	: 5;	// 7, 1f	// >> 57 & 1f
		BYTE	u_mon	: 4;	// 11, 0-11	// >> 53 & f
		BYTE	u_day	: 5;	// 16, 0-30	// >> 48 & 1f
		BYTE	u_hour	: 5;	// 21, 0-23	// >> 43 & 1f
		BYTE	u_min	: 6;	// 27, 0-59	// >> 37 & 3f
		BYTE	u_sec	: 6;	// 33, 0-59	// >> 31 & 3f
		WORD	code	: 10;	// 43, 3ff, 1023	// >> 21 & 3ff
		BYTE	cid		: 4;	// 47,		// >> 17 & f
		BYTE	d_hour	: 5;	// 52,		// >> 12 & 1f
		BYTE	d_min	: 6;	// 58,		// >> 6 & 3f
		BYTE	d_sec	: 6;	// 64bit	// & 3f
	} b;
	uint64_t		a;
} ARM_ARCH, *PARM_ARCH;

#define	SIZE_ARMARCH	4000000	//8388608	//16777216	// 16M block
#define	MAX_SCENE		1024

#pragma pack()

#endif	/* ARCHIVE_H */
