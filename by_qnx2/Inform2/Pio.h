/**
 * @file	Pio.h - Parallel In/Out
 * @brief	입,출력 이름과 순서를 나열한다.
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

enum enTUDINP {
	TUDIB_DEADMAN = 0,	// A, dead man
	TUDIB_FORWARD,		// A
	TUDIB_REVERSE,		// A
	TUDIB_POWERING,		// A
	TUDIB_BRAKING,		// A
	TUDIB_HCR,			// A, head car relay contact
	TUDIB_TCR,			// A, trail car relay contact
	TUDIB_DIR,			// A, door interlock relay contact. (1:close/0:open)

	TUDIB_CPRS,			// 8, A, compulsory release switch
	TUDIB_ADBS,			// A, all door bypass switch
	TUDIB_EBCOS,		// A, emergency brake cutoff switch
	TUDIB_ALCS,			// A, ac lamp control switch
	TUDIB_DLCS,			// B, dc lamp control switch
	TUDIB_EBST,			// A, emergency switch on tail car
	TUDIB_EBSH,			// A, emergency switch on head car
	TUDIB_DIAPDC,		// B, dir added psd close. (1:close/0:open)

	TUDIB_APDS,			// 16, A, air purifier switch(?)
	TUDIB_VFS,			// A, ventilation fan switch
	TUDIB_DSDBPS,		// Deadman Switch Detect bypass switch(?)
	TUDIB_CUTTRACTION,	// A, 190424,	//STUDIB_SPR19,		//TUDIB_SVCDF,		// A, service device fault
	TUDIB_PANUP,		// A, 170717	//TUDIB_HBR,			// A, hold brake
	TUDIB_PANDN,		// A, 170717	//TUDIB_SPR8,
	TUDIB_TRSF,			// B, 201016	TUDIB_SPR9,
	TUDIB_SPR10,

	TUDIB_ENCF,			// 24, 171220, B	//A, encoder fault
	TUDIB_EROS,			// emergency rescue on? switch
	TUDIB_EPANDS,		// A, emergency pantograph down switch
	TUDIB_EBSLR,		// A, emergency brake safety loop relay contact
	TUDIB_MCEBN,		// B, mater controller emergency (negate)
	TUDIB_MRPS,			// B, main reservoir pressure switch
	TUDIB_PBR,			// A, parking brake relay contact
	TUDIB_SBR,			// A, security brake relay contact

	TUDIB_AFS,			// 32, A, atp/ato fmc mode
	TUDIB_ATPEB,		// A, atp emergency (negate)
	TUDIB_ATOM,			// ato mode
	TUDIB_DMAA,			// A, auto open/auto close
	TUDIB_DMAM,			// A, auto open/manual close
	TUDIB_DMMM,			// A, manual open/manual close
	TUDIB_DCS,			// A, door close switch
	TUDIB_YMS,			// yard operate switch ?????

	TUDIB_CHCS1,		// 40, hvac operating mode switch 1
	TUDIB_CHCS2,		// hvac operating mode switch 2
	TUDIB_CHCS3,		// hvac operating mode switch 3
	TUDIB_CHCS4,		// hvac operating mode switch 4
						// mode			chcs4	chcs3	chcs2	chcs1
						// test			0		0		0		1		1
						// full cool	0		1		0		0		4
						// half cool	1		0		0		0		8
						// eva. fan		0		0		1		1		3
						// off			0		1		0		1		5
						// auto			1		0		0		1		9
						// 1/3 heat		0		1		1		1		7
						// 2/3 heat		1		0		1		1		b
						// 3/3 heat		1		1		1		1		f
	TUDIB_HLFS,			// A, hvac line fan control switch
	TUDIB_ELFS,			// A, hvac end line fan control switch
	TUDIB_SIVCN1,		// B, siv manual extension power switch 1
	TUDIB_SIVCN2,		// B, siv manual extension power switch 2

	TUDIB_ATOBR,		// 48, ATO brake relay
	TUDIB_PDS,			// A, permission door signal
	TUDIB_PRDO,			// A, permission left door open
	TUDIB_PLDO,			// A, permission right door open
	TUDIB_CT01,			// 0:normal,	1:Lab. mode
	TUDIB_CT02,			//
	TUDIB_CT04,			// Communication Extension Mode -CMSB-FDU-BMS-siv_mod // 200218
	TUDIB_TCP,			// 0:TC0,	1:TC9
						// CAUTION! liu1/liu2 be selected backplan jumper - read FPGA reg.(0x8102)
											//	   10, 8, 6, 4 <- car length
	TUDIB_CL01,			//TUDIB_SPR13,		// 56,	0, 1, 0, 1
	TUDIB_CL02,			//TUDIB_SPR14,		//		0, 0, 1, 1
	TUDIB_LOW,			//TUDIB_SPR15,		// 58, must be 0
	TUDIB_HIGH,			//TUDIB_SPR16,		// 59, must be 1
	TUDIB_SPR60,
	TUDIB_SPR61,
	TUDIB_SPR62,
	TUDIB_SPR63,

	TUDIB_MAX
};

#if (TUDIB_MAX % 8)		// sizeof(BYTE))
#error	TUDIB size is not multiple of byte!!!
#endif
//#if	(TUDIB_MAX != 64)
//#error	TUDIB size is not same to uint64_t!!!
//#endif
#define	SIZE_TUDI		(TUDIB_MAX / sizeof(BYTE))

enum enCUDINP {
	CUDIB_LD0 = 0,		// A, left door #0
	CUDIB_LD1,
	CUDIB_LD2,
	CUDIB_LD3,
	CUDIB_DIS,			// B, door not isolate
	CUDIB_EED,			// A, in door emergency handle
	CUDIB_EAD,			// A, out door emergency handle
	CUDIB_EDF,			// A, end door fault

	CUDIB_INSBR,		// 8, A, ecu-insufficient brake(lack of braking force)
	CUDIB_ECUMF,		// B, ecu-fault
	CUDIB_ECULF,
	CUDIB_NRBD,			// A, nrbd
	CUDIB_BRCS,			// B, brake release cutout cock
	CUDIB_BCCS,			// B, brake cylinder cutout
	CUDIB_ASCS,			// B, air spring cutout
	CUDIB_SPR0,			// CUDIB_ASCS,			//CUDIB_MRCO,			// main reservoir cutoff

	CUDIB_RD0,			// A, 16, right door #0
	CUDIB_RD1,
	CUDIB_RD2,
	CUDIB_RD3,
	CUDIB_PA1,			// A, passenger alarm #1
	CUDIB_PA2,			// A, passenger alarm #2
	CUDIB_FD,			// A, fire detect
	CUDIB_FDUF,			// B, fire detection unit fault

	CUDIB_SPR2,			//CUDIB_SBUCO,		// 24, security brake cutout cock
	CUDIT_SPR3,			//CUDIB_PBCO,			// parking brake cutout cock
	CUDIB_UOPS,			// A, rescue air pressure release
	CUDIB_MRCS,			// A, main reservoir cutout switch
	CUDIB_UOS,			// A, Uncoupler switch
	CUDIB_ESK,			// A, extension supply relay contact
	CUDIB_VFF,			// A, vvvf fault
	CUDIB_MDS,			// B,

	CUDIB_BVR,			// 32, A, battery voltage low
	CUDIB_SIVK,			// A
	CUDIB_SIVF,			// A, siv fault relay contact
	CUDIB_CMF,			// A, bypass relay (at CMSB failed)
	CUDIB_CMBLK,		// A, bypass CM line contact
	CUDIB_CMK,			// A, CM operating relay contact
	CUDIB_CMG,			// A, CM pressure level switch
	CUDIT_SPR7,			//CUDIB_CMNOT,		// CM not over-temp.

	CUDIB_PANPS1,		// 40, B
	CUDIB_PANPS2,		// B
	CUDIB_IPDR,			// 170717, 개별 판 다운 스위치	// CUDIB_SPR8,			//CUDIB_PANCK1,		// cutout switch
	CUDIB_UBCF,			// B 200218, Uninterruptible broadcasting controller,	CUDIB_SPR9,			//CUDIB_PANCK2,
	CUDIB_LK1,			// A, AC light closed contact #1
	CUDIB_LK2,			// A, AC light closed contact #2
	// 201016
	CUDIB_PANCOC1,		// CUDIB_SPR10, pan cutout cock
	CUDIB_PANCOC2,		// CUDIB_SPR11,

	CUDIB_CT01,			// TC:formation no.|CC:car type
	CUDIB_CT02,			//
	CUDIB_CT04,			//
	CUDIB_CT08,			//
	CUDIB_CN01,			//				|car number
	CUDIB_CN02,			//
	CUDIB_CN04,			//
	CUDIB_CN08,			//

	CUDIB_SPR12,		// 56
	CUDIB_SPR13,
	CUDIB_SPR14,		// 58, must be 0
	CUDIB_SPR15,		// 59, must be 0
	CUDIB_SPR16,
	CUDIB_SPR17,
	CUDIB_SPR18,
	CUDIB_SPR19,

	CUDIB_MAX
};
#if (CUDIB_MAX % 8)		// sizeof(BYTE))
#error	CUDIB size is not multiple of byte!!!
#endif

//#if	(CUDIB_MAX != 64)
//#error	CUDIB size is not same to uint64_t
//#endif
#define	SIZE_CUDI		(CUDIB_MAX / sizeof(BYTE))

// 201016
enum enCUDINP2 {
	CUDIB_APDS2 = CUDIB_VFF,
	CUDIB_APDS1,
	CUDIB_CMSBF = CUDIB_PANPS1,		// B
};

enum enUDOUP {
	UDOB_DCL = 0,		// interior DC light contactor open
	UDOB_ACL1,			// AC light contactor #1 close
	UDOB_ACL2,			// AC light contactor #2 close
	UDOB_ESK,			// extension contactor close
	UDOB_CMK,			// cm operate relay
	UDOB_DISTP,			// distance pulse for pa, pis
	UDOB_ALBO,			// all lamp black out
	UDOB_SPR1,

	UDOB_SPR2,
	UDOB_SPR3,
	UDOB_SPR4,
	UDOB_SPR5,
	UDOB_SPR6,
	UDOB_SPR7,
	UDOB_SPR8,
	UDOB_SPR9,

	UDOB_MAX
};
#if (UDOB_MAX % 8)		// sizeof(BYTE))
#error	UDOUP size is not multiple of byte!!!
#endif
#define	SIZE_UDO		(UDOB_MAX / sizeof(BYTE))

enum enUAINP {
	UAICH_TACHO = 0,
	UAICH_TEBE,
	UAICH_BAT,
	UAICH_TEMP,
	UAICH_SPR0,
	UAICH_SPR1,
	UAICH_SPR2,
	UAICH_SPR3,

	UAICH_MAX
};
#if (UAICH_MAX % 8)		// sizeof(BYTE))
#error	UAINP size is not multiple of byte!!!
#endif
#define	SIZE_UAICH		UAICH_MAX

enum enGPINP {
	GPDIB_LFB0,			// length form ->	00:10,	01:8
	GPDIB_LFB1,			// 					10:6,	11:4
	GPDIB_LIUX,			// 0:liu1,	1:liu2
	GPDIB_MATE,			// 0:dead,	1:alive
	GPDIB_RES0,
	GPDIB_RES1,
	GPDIB_RES2,
	GPDIB_RES3,

	GPDIB_RES4,
	GPDIB_RES5,
	GPDIB_RES6,
	GPDIB_RES7,
	GPDIB_RES8,
	GPDIB_RES9,
	GPDIB_RES10,
	GPDIB_RES11,

	GPDIB_MAX
};
#if	(GPDIB_MAX % 8)
#error	GPINP size is not multiple of byte!!!
#endif
#define	SIZE_GPI		(GPDIB_MAX % sizeof(BYTE))

enum enGPOUP {
	GPDOB_WTRIG,		// watchdog trigger
	GPDOB_EXCLUDEMATE,	//
	GPDOB_RES0,
	GPDOB_RES1,
	GPDOB_RES2,
	GPDOB_RES3,
	GPDOB_RES4,
	GPDOB_RES5,

	GPDOB_RES6,
	GPDOB_RES7,
	GPDOB_RES8,
	GPDOB_RES9,
	GPDOB_RES10,
	GPDOB_RES11,
	GPDOB_RES12,
	GPDOB_RES13,

	GPDOB_MAX
};
#if	(GPDOB_MAX % 8)
#error	GPOUTP size is not multiple of byte!!!
#endif
#define	SIZE_GPO		(GPDOB_MAX % sizeof(BYTE))

//typedef	struct _tagSHEAF {
//	BYTE	cTuInp[SIZE_TUDI];
//	BYTE	cCuInp[SIZE_CUDI];
//	BYTE	cUOutp[SIZE_UDO];
//	SHORT	vInp[SIZE_UAICH];
//	BYTE	cGinp[SIZE_GPI];
//	BYTE	cGoutp[SIZE_GPO];
//	BYTE	sText[64];
//} SHEAF;

#define	TYPICALPERIOD_PWM	2000
#define	MAXPERIOD_PWM	(TYPICALPERIOD_PWM + 100)
#define	MINPERIOD_PWM	(TYPICALPERIOD_PWM - 100)
