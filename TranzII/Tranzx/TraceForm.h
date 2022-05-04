/*
 * TraceForm.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	TRACEFORM_H_
#define	TRACEFORM_H_

typedef struct _tagATCTRACEFORM {
	BYTE	nText1;		// 0
	BYTE	nText2;		// 1
	BYTE	nText3;		// 2
	struct {
		BYTE	bManualDoorOpenClose	: 1;
		BYTE	bSpare					: 1;
		BYTE	b4550CarrierFreq		: 1;
		BYTE	b5525CarrierFreq		: 1;
		BYTE	nSpeedCode				: 4;
	} nText4;			// 3
	BYTE	nText5;		// 4
	BYTE	nText6;		// 5
	BYTE	nText7;		// 6
	BYTE	nText8;		// 7
	BYTE	nText9;		// 8
	BYTE	nTrainSpeed;	// 9
	struct {
		BYTE	month;	// 10,	1-12
		BYTE	day;	// 11,	1-31
		BYTE	hour;	// 12,	0-23
		BYTE	minute;	// 13,	0-59
		BYTE	sec;	// 14,	0-59
	} time;
	BYTE	nTachoPulse;	// 15
	BYTE	nAtoRestrictiveCode;	// 16
	BYTE	nSpare[2];	// 17, 18
	BYTE	nFaultCode;	// 19
} ATCTRACEFORM, *PATCTRACEFORM;
ATCTRACEFORM	tAtc[8][30];

typedef struct _tagATOTRACEFORM {
	struct {
		BYTE	nCode					: 5;
		BYTE	bProgramStopFunction	: 1;
		BYTE	bAtoReady				: 1;
		BYTE	bActivateAto			: 1;
	} nText1;			// 0
	BYTE	nText2;		// 1
	BYTE	nText3;		// 2
	BYTE	nText4;		// 3
	BYTE	nText5;		// 4
	BYTE	nText6;		// 5
	BYTE	nVelocity;	// 6
	BYTE	nSpeed;		// 7
	struct {
		BYTE	bPb			: 1;
		BYTE	nPercent	: 7
	} nText9;			// 8
	BYTE	nPb;		// 9
	BYTE	nBrakePermit;	// 10
	struct {
		BYTE	nNextTwcCode	: 4;
		BYTE	nCurTwcCode		: 4;
	} nText12;			// 11
	BYTE	nNextStation;	// 12
	BYTE	nCurStation;	// 13
	WORD	wDistance;		// 14-15
	BYTE	nFaultCode;		// 16
	BYTE	nDestination;	// 17
	BYTE	nFixedSpeed;	// 18
	WORD	wReceivedNo;	// 19-20
	WORD	wTranmitNo;		// 21-22
	struct {
		BYTE	month;	// 23,	1-12
		BYTE	day;	// 24,	1-31
		BYTE	hour;	// 25,	0-23
		BYTE	minute;	// 26,	0-59
		BYTE	sec;	// 27,	0-59
	} time;
} ATOTRACEFORM, *PATOTRACEFORM;
ATOTRACEFORM	tAto[8][30];

typedef struct _tagSIVTRACEUNIT {
	WORD	wEs;		// 0-1
	WORD	wIs;		// 2-3
	WORD	wFcv;		// 4-5
	WORD	wEo;		// 6-7
	WORD	wIo;		// 8-9
	BYTE	nSpare10;	// 10
	BYTE	nText12;	// 11
	BYTE	nText13;	// 12
	BYTE	nText14;	// 13
	BYTE	nText15;	// 14
	BYTE	nText16;	// 15
	BYTE	nText17;	// 16
	BYTE	nText18;	// 17
} SIVTRACEUNIT;

typedef struct _tagSIVTRACEFORM {
	BYTE	nFaultCode;	// 0
	struct {
		BYTE	year;	// 1
		BYTE	month;	// 2
		BYTE	day;	// 3
		BYTE	hour;	// 4
		BYTE	minute;	// 5
	} time;
	BYTE	nSpr6;		// 6
	BYTE	nSpr7;		// 7
	SIVTRACEUNIT	unit[201];
} SIVTRACEFORM, *PSIVTRACEFORM;
SIVTRACEFORM	tSiv[10];

typedef struct _tagV3FTRACEUNIT {
	BYTE	nText1;		// 0
	BYTE	nText2;		// 1
	BYTE	nSpare3;	// 2
	BYTE	nText4;		// 3
	BYTE	nText5;		// 4
	BYTE	nText6;		// 5
	BYTE	nSpare7;	// 6
	BYTE	nSpare8;	// 7
	WORD	wLineV;		// 8-9
	WORD	wFcV;		// 10-11
	WORD	wDcI;		// 12-13
	WORD	wTorqueI;	// 14-15
	WORD	wTorqueICmd;	// 16-17
	WORD	wFluxI;		// 18-19
	WORD	wFluxICmd;	// 20-21
	WORD	wMdIndex;	// 22-23
	WORD	wIoU;		// 24-25
	WORD	wIoV;		// 26-27
	WORD	wIoW;		// 28-29
	WORD	wMotorI;	// 30-31
	WORD	wIntFreq;	// 32-33
	WORD	wSlipFreq;	// 34-35
	WORD	wLwPwm;		// 36-37
	WORD	wAlpha;		// 38-39
	WORD	wDtPwm;		// 40-41
	WORD	wTorqueEst;	// 42-43
	WORD	wWmSim;		// 44-45
	WORD	wSpare46;	// 46-47
	WORD	wSpare48;	// 48-49
} V3FTRACEUNIT;

typedef struct _tagV3FTRACEFORM {
	BYTE	nForm;		// 0
	BYTE	nSpare1[2];	// 1-2
	BYTE	nFaultCode;	// 3
	struct {
		BYTE	year;	// 4
		BYTE	month;	// 5
		BYTE	day;	// 6
		BYTE	nSpare7;	// 7
		BYTE	hour;	// 8
		BYTE	minute;	// 9
		BYTE	sec;	// 10
	} time;
	BYTE	nSpare11;	// 11
	V3FTRACEUNIT	unit[201];
} V3FTRACEFORM, *PV3FTRACEFORM;
V3FTRACEFORM	tV3f[10];

typedef struct _tagECUTRACEUNIT {
	BYTE	nText1;		// 0
	BYTE	nText2;		// 1
	BYTE	nText3;		// 2
	BYTE	nText4;		// 3
	WORD	wTbe;		// 4-5
	BYTE	nText7;		// 6
	BYTE	nText8;		// 7
	BYTE	nAspF;		// 8
	BYTE	nAspR;		// 9
	BYTE	nAspO;		// 10
	BYTE	nWeight;	// 11
	BYTE	nRbed;		// 12
	BYTE	nRbea;		// 13
	BYTE	nAbr;		// 14
	BYTE	nAcp;		// 15
	BYTE	nBcp;		// 16
	BYTE	nAxle1;		// 17
	BYTE	nAxle2;		// 18
	BYTE	nAxle3;		// 19
	BYTE	nAxle4;		// 20
	BYTE	nAxle;		// 21
} ECUTRACEUNIT;

typedef struct _tagECUTRACEFORM {
	BYTE	nFaultCode;	// 0
	struct {
		BYTE	year;	// 1
		BYTE	month;	// 2
		BYTE	day;	// 3
		BYTE	hour;	// 4
		BYTE	minute;	// 5
		BYTE	sec;	// 6
	} bcdTime;
	BYTE	nSpare7[15]	// 7-21
	ECUTRACEUNIT	unit[81];
} ECUTRACEFORM, *PECUTRACEFORM;
ECUTRACEFORM	tEcu[5];

#endif
