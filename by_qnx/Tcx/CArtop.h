/*
 * CArtop.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CARTOP_H_
#define CARTOP_H_

#include <Draft.h>

class CArtop
{
public:
	CArtop();
	virtual ~CArtop();

protected:
	PTCREFERENCE	c_pTcRef;
	PLCFIRM		c_pLcFirm;
	PTCDOZ		c_pDoz;

private:
	PVOID	c_pParent;
	UCURV	c_uMcDrvVarDeb;
	UCURV	c_uMcDrvPosDeb;
// Modified 2012/09/17 ... begin
#define	DEB_MCDRV		20	// 10	// 5		// 2
// end

// Modified 2013/02/05
//	WORD	c_wPwmDeb;
//#define	DEBOUNCE_PWM	5

	BOOL	c_bCapedEpcorSrc;
	BOOL	c_bEpcor;
	BOOL	c_bEpcorBuf;
	WORD	c_wEpcorDeb;
#define	DEBOUNCE_EPCOR	5

	typedef struct _tagNRBTRIGGER {
		BYTE	nState;
#define	NRBSTATE_TRIGGER	1
#define	NRBSTATE_OUTPUTING	2
		BYTE	nNull;
		WORD	wTimer;
#define	TIME_NRBOUTPUT	1000 / TIME_INTERVAL
	} NRBTRIG;
	NRBTRIG	c_nrbt;

	enum  {
		PWM_CONVTOBA,
		PWM_CONVTOB,
		PWM_CONVTOP
	};

	WORD	CalcWithRounded(long v, long num, long den);
	void	PwmToPower(BYTE convSign);
	void	PowerToPwm(BYTE nIndex);
	WORD	ScanBrakeShot(WORD brake, BOOL mode);
	BOOL	ScanEmergency();
	BOOL	CheckEmergencyBrake();
	BOOL	CheckBccoOverNine();
	BOOL	CheckFullServiceBrake();
	BOOL	CheckFsbAtMwb();
	void	ForceFullServiceBrake(BOOL bFsb);
	BOOL	CheckHoldBrake();
	BOOL	CheckNonReleaseBrake();
	BOOL	CheckCompulsoryRelease();
	BOOL	CheckSecurityBrake();
	BOOL	CheckParkingRelease();
	void	AtoToTbe(_TV_ vTbe);
	BYTE	GetSettingBits(WORD bits);
	void	ClrPwmOutput()
	{
		CLRNBIT(OWN_SPACEA, NESTED_ROLR);
		CLRNBIT(OWN_SPACEA, NESTED_ROPR);
		CLRNBIT(OWN_SPACEA, NESTED_ROBR);
	}

public:
	void	InitialModule(PVOID pVoid);
	void	ReleaseAll();
	void	RecoverAll();
	void	Drive();
	WORD	ConvPwb();
	//void	RescueCtrl();
	BOOL	GetEpcor()	{ return c_bEpcor; }
	void	ForceHoldBrake(BOOL cmd)
	{
		c_pDoz->tcs[OWN_SPACEA].real.cEcu.inst.b.hb =
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.state.b.hb = cmd;
	}
};

#endif /* CARTOP_H_ */
