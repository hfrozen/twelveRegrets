/*
 * CPanelTc.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CPANELTC_H_
#define CPANELTC_H_

#include <Draft.h>
#include <CPio.h>

enum  {	FROMPANELFAULT_NON = 0,
		FROMPANELFAULT_DRIVER,
		FROMPANELFAULT_REFERENCE,
		FROMPANELFAULT_UNMATCHID,
		FROMPANELFAULT_UNMATCHLENGTH,
		FROMPANELFAULT_UNFORMATSERIALNO,
		FROMPANELFAULT_UNFORMATORGNO
};

enum  {	PANEL_LAMP1 = 1,
		PANEL_LAMP2 = 2,
		PANEL_LAMP3 = 0x20,
		PANEL_LAMP4 = 0x40
};
#define	PANEL_LAMPS		0x63

enum  {	LAMP_OFF = 0,
		LAMP_ON,
		LAMP_FLASH
};

class CPanelTc
{
public:
	CPanelTc();
	CPanelTc(PVOID pVoid);
	virtual ~CPanelTc();

private:
	PVOID	c_pParent;
	PTCREFERENCE	c_pTcRef;
	CPio	c_io;
	BYTE	c_nLamp;
	BYTE	c_nFlash;
	BOOL	c_nState;
#define	STATE_MAIN			1
#define	STATE_NORMALTIMER	0x10
#define	STATE_PARTNERLIVE	0x20

	BYTE	c_nStallCount;
#define	COUNT_STALL		30
	UCURV	c_uFlashTimer[8];
#define	DEBOUNCE_PANEL	16

	static const BYTE	c_nLampForm[];

	BOOL	CheckBCD(BYTE bcd);
	void	Lamp(BYTE lamp, BYTE cmd);

public:
	void	Interval();
	BOOL	Initial(PTCREFERENCE pTcRef);
	UCURV	Capture(DWORD* pDw);
	int		Junction(WORD w);
	//void	Active(BYTE id);
	void	LampCtrl(BYTE nLamp, BYTE nState);
	void	StepLamp(BYTE step);
	void	AlarmLamp(BYTE alarm);
	int		FindExec(PSZ fname);
	int		RunExec(PSZ fname, PSZ args[]);
	BOOL	RemoveExec(int pid);
	void	SetMain(BOOL cmd);
	void	SetTimer(BOOL cmd);
	BOOL	GetPartner();
	void	FlashLamp(BYTE lp)				{ LampCtrl(lp, LAMP_FLASH); }
	void	SetLamp(BYTE lp)				{ LampCtrl(lp, LAMP_ON); }
	void	ClrLamp(BYTE lp)				{ LampCtrl(lp, LAMP_OFF); }
	WORD	GetCcDev(PCARINFO pCar)			{ return c_io.GetCcDev(pCar); }
	BYTE	GetV3fQuantity(PCARINFO pCar)	{ return c_io.GetV3fQuantity(pCar); }
	BYTE	GetEcuQuantity(PCARINFO pCar)	{ return c_io.GetEcuQuantity(pCar); }
	char*	TcNamePlate(char* p)			{ return c_io.TcNamePlate(p); }
	char*	CcNamePlate(char* p)			{ return c_io.CcNamePlate(p); }
};

#endif /* CPANELTC_H_ */
