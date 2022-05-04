/*
 * PcioInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	PCIOINFO_H_
#define	PCIOINFO_H_

// ***** D A I/O *****

enum  {	TCDIB_FBFACK = 0,	// alarm buzzer shutoff switch
		TCDIB_EM,		// 01, emergency mode
		TCDIB_MM,		// 02, manual mode
		TCDIB_DM,		// 03, driveless mode
		TCDIB_F,		// 04, forward
		TCDIB_R,		// 05, reverse
		TCDIB_DADS,		// 06, deadman switch from m/c
		TCDIB_P,		// 07, powering from m/c

		TCDIB_C,		// 08, coasting from m/c
		TCDIB_B,		// 09, brakeing from m/c
		TCDIB_PANUS,	// 10, pantograph up switch
		TCDIB_PANDS,	// 11, pantograph down switch
		TCDIB_EPANDS,	// 12, pantograph emergency down switch(negate)
		TCDIB_ROS,		// 13, rescue switch
		TCDIB_ROPR,		// 14, rescue powering
		TCDIB_ROBR,		// 15, rescue braking

		TCDIB_ZVBS,		// 16, ???
		TCDIB_HBCOS,	// 17, hold brake cutout switch
		TCDIB_EPCOR,	// 18, emergency powering cutout relay, emergency brake request
		TCDIB_PBBPS,	// 19, parking brake bypass switch
		TCDIB_EBCOS,	// 20, emergency brake cutout switch
		TCDIB_EBRA,		// 21, emergency brake request from atc(negate)
		TCDIB_EBS,		// 22, emergency brake switch
		TCDIB_SBS,		// 23, security brake switch

		TCDIB_PBRR,		// 24, parking brake relese request(negate)
		TCDIB_CPRLS,	// 25, compulsory release switch
		TCDIB_EBBR,		// 26, ???emergency brake bypass
		TCDIB_HES,		// 27
		TCDIB_EBRM,		// 28, emergency brake request from m/c(negate)
		TCDIB_SPR29,	// 29
		TCDIB_BATK,		// 30
		TCDIB_BATKN,	// 31

		TCDIB_DAA,		// 32, door mode auto open/close
		TCDIB_DAM,		// 33, door mode auto open/manual close
		TCDIB_DMM,		// 34, door mode manual open/close
		TCDIB_ADSCR,	// 35, all door close switch
		TCDIB_ADSBS,	// 36, all door close switch bypass
		TCDIB_DOSR,		// 37, right door open switch
		TCDIB_DOSL,		// 38, left door open switch
		TCDIB_DCS,		// 39, door close switch

		TCDIB_DROS,		// 40
		TCDIB_RDOS,		// 41
		TCDIB_LRDCS,	// 42
		TCDIB_LRDROS,	// 43
		TCDIB_LDOS,		// 44
		TCDIB_PALS,		// 45, passenger alarm release switch
		TCDIB_LCS1,		// 46
		TCDIB_LCS2,		// 47

		TCDIB_LCDK,		// 48
		TCDIB_HTCF,		// 49
		TCDIB_PISF,		// 50
		TCDIB_TRSF,		// 51
		TCDIB_RFCF,		// 52
		TCDIB_TG0,		// 53
		TCDIB_TG1,		// 54
		TCDIB_RESVL,	// 55, TCDIB_RFCF <- TCDIB_SPR52,

		TCDIB_MAX		// 56
};	// TCDIB;
#define	SIZE_TCDI		8	// 8byte, 7 + 1 spare or align
#define	TCDI_MODE		0
#define	TCDIM_EM		2
#define	TCDIM_MM		4
#define	TCDIM_DM		8
#define	TCDIM_ONLY		(TCDIM_EM | TCDIM_MM | TCDIM_DM)
#define	TCDI_DOORMODE	4
#define	TCDIM_DOORONLY	7

enum  {	TCDOB_BATK = 0,	// battery load relay
		TCDOB_BCID,		// 01, battery contactor in driverless
		TCDOB_PAAK,		// 02, passenger alarm lamp
		TCDOB_BPA,		// 03, passenger alarm buzzer
		TCDOB_DCL,		// 04, door closed lamp
		TCDOB_DAOACL,	// 05, door mode auto open/close lamp
		TCDOB_DAOMCL,	// 06, door mode auto open/manual close lamp
		TCDOB_DMOMCL,	// 07, door mode manual open/close lamp

		TCDOB_EMML,		// 08, emergency mode lamp
		TCDOB_YAML,		// 09, yard mode lamp
		TCDOB_MAML,		// 10, manual mode lamp
		TCDOB_AUML,		// 11, auto mode lamp
		TCDOB_DRML,		// 12, driverless mode lamp
		TCDOB_FACKLP,	// 13, alarm lamp
		TCDOB_AUML1,	// 14, auto mode lamp at desk
		TCDOB_DBZA,		// 15, alarm buzzer

		TCDOB_EPANDNLP,	// 16, pantograph emergency down lamp
		TCDOB_ESLP,		// 17, extension supply relay
		TCDOB_SPR18,	// 18, ???
		TCDOB_EBLP,		// 19, emergency brake lamp
		TCDOB_PBLP,		// 20, parking brake lamp
		TCDOB_SBLP,		// 21, security brake lamp
		TCDOB_CPRLP,	// 22, compulsory release lamp
		TCDOB_EBCOSLP,	// 23, emergency brake cutout lamp

		TCDOB_ROLR,		// 24, loco rescue
		TCDOB_ROPR,		// 25, rescue powering
		TCDOB_ROBR,		// 26, rescue braking
		TCDOB_HCR,		// 27, head control relay
		TCDOB_TCR,		// 28, tail control relay
		TCDOB_ZVR,		// 29, zero velocity detected
		TCDOB_TCWDR,	// 30, ???
		TCDOB_DPLP,		// 31, atc departure permit lamp

		TCDOB_DCALS,	// 32, speed pulse
		TCDOB_TDO,		// 33, door open, ~TCDIB_ADSCR
		TCDOB_RESVL,	// 34
		TCDOB_SPR35,	// 35
		TCDOB_SPR36,	// 36
		TCDOB_SPR37,	// 37
		TCDOB_SPR38,	// 38
		TCDOB_SPR39,	// 39

		TCDOB_ATCON,	// 40, atc on relay
		TCDOB_SPR41,	// 41
		TCDOB_TSET,		// 42
		TCDOB_ESET,		// 43
		TCDOB_SPR44,	// 44
		TCDOB_DUHOLD,	// 45
		TCDOB_ACT,		// 46
		TCDOB_HEAD,		// 47

		TCDOB_MAX		// 48
};	// TCDOB;
#define	SIZE_TCDO		6
#define	TCDO_DOORMODELAMP	0
#define	TCDOM_DOORAOAC		0x20
#define	TCDOM_DOORAOMC		0x40
#define	TCDOM_DOORMOMC		0x80
#define	TCDOM_DOORLAMPES	0xe0
#define	TCDO_MODELAMP	1
#define	TCDOM_EMLP		1
#define	TCDOM_YMLP		2
#define	TCDOM_MMLP		4
#define	TCDOM_AMLP		8
#define	TCDOM_DMLP		0x10
#define	TCDOM_AMLP1		0x40

#define	TCDOM_MODELAMPES	0x18

enum  {	NESTED_ROLR = 0,	// TCDOB_ROLR
		NESTED_ROPR,		// TCDOB_ROPR
		NESTED_ROBR,		// TCDOB_ROBR
		NESTED_B3,
		NESTED_B4,
		NESTED_B5,
		NESTED_B6,
		NESTED_B7,
		NESTED_MAX
};

enum  {	CCDIB_IVS = 0,	// switch between high voltage and inverter
		CCDIB_IVF,		// 01, fuse between high voltage and inverter
		CCDIB_SIVK,		// 02, siv output relay
		CCDIB_SIVFR,	// 03, siv fault relay
		CCDIB_RFN,		// 04, battery charge contact
		CCDIB_SPR5,		// 05
		CCDIB_SPR6,		// 06
		CCDIB_SPR7,		// 07

		CCDIB_BCCS1,	// 08, brake cylinder 1 not out
		CCDIB_BCCS2,	// 09, brake cylinder 2 not out
		CCDIB_BOUCS,	// 10, bou cutout
		CCDIB_ASCS1,	// 11, air spring 1 not cutout
		CCDIB_ASCS2,	// 12, air spring 2 not cutout
		CCDIB_MRPS,		// 13, main reservoir pressure switch
		CCDIB_SPR14,	// 14
		CCDIB_SPR15,	// 15

		CCDIB_ECUF,		// 16, ecu failed(negate)
		CCDIB_NRBD,		// 17, non release brake detect
		CCDIB_EMR,		// 18, passenger emergency request
		CCDIB_SPR19,	// 19
		CCDIB_LK1,		// 20,
		CCDIB_LK2,		// 21
		CCDIB_LRR,		// 22, load reduction
		CCDIB_SPR23,	// 23

		CCDIB_BHEKN,	// 24,
		CCDIB_K1,		// 25, contact monitor
		CCDIB_K4,		// 26, contact monitor
		CCDIB_FDN,		// 27, passenger fire alarm
		CCDIB_K6,		// 28, contact monitor
		CCDIB_OCCR,		// 29, opposite car door close
		CCDIB_SPR30,	// 30
		CCDIB_SPR31,	// 31

		CCDIB_DSL1,		// 32, door state
		CCDIB_DSL2,		// 33
		CCDIB_DSL3,		// 34
		CCDIB_DSL4,		// 35
		CCDIB_DSR1,		// 36
		CCDIB_DSR2,		// 37
		CCDIB_DSR3,		// 38
		CCDIB_DSR4,		// 39

		CCDIB_DNBS,		// 40, 1->all door not by pass, 0->any door bypass
		CCDIB_SPR41,	// 41
		CCDIB_SPR42,	// 42
		CCDIB_SPR43,	// 43
		CCDIB_SPR44,	// 44
		CCDIB_SPR45,	// 45
		CCDIB_SPR46,	// 46
		CCDIB_SPR47,	// 47

		CCDIB_MAX		// 48
};	// CCDIB;	// TC CAR
#define	CCDI_DOOR		4
#define	SIZE_CCDI		6
// M1 CAR
#define	CCDIB_DCK		CCDIB_IVS	// v3f active contact(0->v3f failed)
#define	CCDIB_PANPS1	CCDIB_IVF	// pantograph 1 status
#define	CCDIB_PANPS2	CCDIB_SIVK	// pantograph 2 status
#define	CCDIB_IESPE		CCDIB_SIVFR	// inverter earthing switch to earth position
#define	CCDIB_IESPS		CCDIB_RFN	// inverter earthing switch to service position
#define	CCDIB_ACMCS		CCDIB_SPR41	// aux. compressor on
#define	CCDIB_ACMG		CCDIB_SPR42	// aux. compressor pressure low
#define	CCDIB_ACMKR		CCDIB_SPR43	// aux. compressor contactor relay closed
// M2 CAR
//#define	CCDIB_CMNRF		CCDIB_SPR41
#define	CCDIB_CMIF		CCDIB_SPR41	// cm-inverter fault
#define	CCDIB_CMG		CCDIB_SPR42	// cm pressure switch(1->low)
// T1 CAR
#define	CCDIB_ESKC		CCDIB_SPR41	// extension supply cotact
// T2 CAR

enum  {	CCDOB_OCRDO = 0,	// opposite car right door open
		CCDOB_DOR1,		// 01, right door open
		CCDOB_DOR2,		// 02
		CCDOB_DOR3,		// 03
		CCDOB_DOR4,		// 04
		CCDOB_DROR,		// 05, right door reopen
		CCDOB_SPR6,		// 06
		CCDOB_SPR7,		// 07

		CCDOB_OCLDO,	// 08, opposite car left door open
		CCDOB_DOL1,		// 09, left door open
		CCDOB_DOL2,		// 10
		CCDOB_DOL3,		// 11
		CCDOB_DOL4,		// 12
		CCDOB_DROL,		// 13, left door reopen
		CCDOB_SPR14,	// 14
		CCDOB_SPR15,	// 15

		CCDOB_NRB,		// 16, scan non release brake
		CCDOB_CPRL,		// 17, compulsory release
		CCDOB_BHEK,		// 18,
		CCDOB_SPR19,	// 19
		CCDOB_SPR20,	// 20
		CCDOB_SPR21,	// 21
		CCDOB_SPR22,	// 22
		CCDOB_SPR23,	// 23

		CCDOB_EIAL,		// 24
		CCDOB_LRR1,		// 25, load resuction relay 1
		CCDOB_LRR2,		// 26, load resuction relay 2
		CCDOB_K1L,		// 27
		CCDOB_K2L,		// 28
		CCDOB_K3L,		// 29
		CCDOB_SPR30,	// 30
		CCDOB_SPR31,	// 31

		CCDOB_MAX		// 32
};	// CCDOB;	// TC CAR
#define	SIZE_CCDO		4
// M1 CAR
#define	CCDOB_ACMKR		CCDOB_SPR19	// aux. compressor contactor relay
#define	CCDOB_PANUR		CCDOB_SPR20	// pantograph up
#define	CCDOB_PANDR		CCDOB_SPR21	// pantograph down
// M2 CAR
// T1 CAR
#define	CCDOB_ESK		CCDOB_SPR30	// extension supply relay
// T2 CAR

enum  {	TCAI_CH0 = 0,
		TCAI_CH1,
		TCAI_CH2,
		TCAI_CH3,
		TCAI_MAX
};	// TCAI;

#define	SIZE_TCAI		TCAI_MAX

enum  {	TCAO_TICL = 0,
		TCAO_MAX
};	// TCAO;

#define	SIZE_TCAO		2

#endif	/* PCIOINFO_H_ */
