/*
 * CSteer.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "../Component2/CPop.h"
#include "Docu.h"

#define	__STEER_MON__

class CSteer
{
public:
	CSteer();
	virtual ~CSteer();

private:
	PVOID	c_pParent;
	CMutex	c_mtx;
	CMutex	c_mts;
	CMutex	c_mtm;					// for momentum

	bool	c_bWakeup;				// ó�� ���ǵ� 0 �������� careers�� �����ϱ�����...
	bool	c_bAfsMode;				// 171219
	bool	c_bTest;

#define	DEB_DCS					5
//#define	WATCH_DOORINTERLOCK		REALBYSPREAD(5000)		//REAL2PERIOD(5000)
#define	WATCH_DOORINTERLOCK		REALBYSPREAD(10000)  //211112
	typedef struct _tagDEBEDDCS {
		CCalm <bool>	b;
		bool			bClsCmd;
		WORD			wWatch;
	} DEBDCS;
	DEBDCS			c_dcCmd;

	// ���� ������ ����� ��� �ӵ��� ���ϱ�����...
	typedef struct _tagSPEEDCALCURATE {
		uint64_t	clk;
		double		dbCur;
		double		dbSum;
		WORD		wSamplingLength;
		struct {
			double	dbPrev;
			double	dbCur;
		} dist;
		WORD		wPulseWidthTimer;
	} SPEEDCALC;
	SPEEDCALC	c_speedCalc;

	enum enCALCZEROSSEQ {
		CALCZEROSSEQ_NON = 0,
		CALCZEROSSEQ_DETECT,
		CALCZEROSSEQ_CMPMOVE,
		CALCZEROSSEQ_CMPDISTANCE,
		CALCZEROSSEQ_CMPPOWER,
		CALCZEROSSEQ_SAVECAREERS,
		CALCZEROSSEQ_END
	};
	typedef struct _tagZEROSPEEDCALCURATE {
		//bool	bModifyCareers;
		BYTE	cSeq;
		WORD	wTimer;
	} ZSCALC;
	ZSCALC		c_zsCalc;
#define	TIME_DETECTZV		REAL2PERIOD(1000)
#define	SPEED_ZERO			2000.f		// zero under 2Km/h

	BYTE		c_cMode;
	WORD		c_wEncfUnDetectTimer;
#define	TIME_ENCFUNDETECT		30		// 30 * 100ms = 2sec

	typedef struct _tagMAMIX {
		BYTE	cNotch;
		BYTE	cRealPwm;		// 180219, ���ڴ����� ������ ��
		// 180219, �Ʒ� �� ���� ���� �δ� ���� �ڵ������� ����� ������ ���������� ���� ������ PWM�� HDU�� ǥ���ϴ� PWM���� �ٸ��� �����̴�.
		BYTE	cLogicPwm;		// ���ڴ� ��� ȯ�갪, ���������� ã�� ��ġ ���� ǥ�� ������� ������ ���� HDU�� ǥ�ø��ϰ�, �ڵ������� 0~100%�� ȯ��� ���̴�.
		BYTE	cConvPwm;		// 180219 0~100%	//���ڴ� ��� �����, ���������� 5~95%�μ� ������ ��ġ ���� ã��, �ڵ������� 10~90%���� ������.
	} _MAMIX;
	_MAMIX		c_mam;

	typedef struct _tagTBEREF {
		//bool	bTenor;		// ���� ����(0:������/1:������)
		BYTE	cEffType;	// 0:�߸�,	1:����,	2:����
		BYTE	cEffort;	// 0~100%
	} TBEREF;
	TBEREF		c_tber;

// 210803
#define	CAUSEOFEB_CYCLE1	REALBYSPREAD(2000)
#define	CAUSEOFEB_CYCLE2	REALBYSPREAD(10000)		// 210630 (2000)
	typedef struct _tagBRAKES {
		struct {
			BYTE	cCause;
			BYTE	cCycle;
		} emer;
		BYTE		cNorm;
	} BRAKES;
	BRAKES			c_brake;

	typedef struct _tagNOTCHSECTION {
		BYTE	cMin;
		BYTE	cMax;
		BYTE	cTypic;
		BYTE	id;
	} NOTCHSECTION;

	MOMENTUMINFO	c_moment;	// 180405

	WORD    c_wPsdoCounter;	// 201124

#if	defined(__STEER_MON__)
	char	c_bufMon[4096];
	char*	c_pMon;
#endif

//#define	WATCH_DEADMAN		REAL2PERIOD(3000)
//#define	WATCH_DOORINTERLOCK	REAL2PERIOD(5000)
	//_TBM	c_deadm;
	//_TBM	c_doorc;

	static const NOTCHSECTION	c_notchSpectrum[2][10];

	void	Shot(int iCID, WORD wCode);
	void	Cut(int iCID, WORD wBegin, WORD wEnd = 0, WORD wFlesh = 0);
	void	Closeup(int iCID, WORD wShot, WORD wCutBegin, WORD wCutEnd);
	void	CalcTachoSpeed();
	void	CaptureZeroSpeed();
	// 180105
	//void	CapturePwm();
	void	CapturePwm();	//Notch();
	void	CaptureMode();
	bool	CheckItemSign(WORD tudib, WORD shot);
	bool	CheckItemMark(WORD tudib, WORD mbi, WORD shot);
	bool	CheckItemCause(WORD tudib, WORD ebi, WORD shot);
	bool	CheckEmergency();
	bool	CheckFSB();
	bool	CheckSecurityBrake();
	bool	CheckParkingBrake();

public:
	void	Initial();
	void	Control(PRECIP pRecip);
	void	Monitor(PRECIP pRecip);
	void	Arteriam();
	void	GetAvrLog(PLOGBOOK pLog);
	//double	GetTachoSpeed()				{ return c_calcSpeedNote.dbCur; }
	// 180405
	void	CopyMomentum(PMOMENTUMINFO pMoment);

	//BYTE	GetMode()					{ return c_mode.cDetermined; }
	BYTE	GetMode()					{ return c_cMode; }
	// 171204
	BYTE	GetLogicPwm()				{ return c_mam.cLogicPwm; }			// ���� DU�� ǥ���� �� ���̴�.
	void	SetParent(PVOID pParent)	{ c_pParent = pParent; }

	PUBLICY_CONTAINER();
};
