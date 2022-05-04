/*
 * CPanelCc.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CPANELCC_H_
#define CPANELCC_H_

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

#define	FROMPANELFAULT_UNMATCHCARID	FROMPANELFAULT_UNMATCHLENGTH

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

class CPanelCc
{
public:
	CPanelCc();
	CPanelCc(PVOID pVoid);
	virtual ~CPanelCc();

	void	Interval();
	BOOL	Initial();
	UCURV	Capture(PCARINFO pCar);
	void	LampCtrl(BYTE nLamp, BYTE nState);
	void	StepLamp(BYTE step);
	void	AlarmLamp(BYTE alarm);
	int		FindExec(PSZ fname);
	int		RunExec(PSZ fname, PSZ args[]);
	BOOL	RemoveExec(int pid);
	WORD	GetCcDev(PCARINFO pCar)	{ return c_io.GetCcDev(pCar); }
	char*	TcNamePlate(char* p)	{ return c_io.TcNamePlate(p); }
	char*	CcNamePlate(char* p)	{ return c_io.CcNamePlate(p); }
	void	FlashLamp(BYTE lp)	{ LampCtrl(lp, LAMP_FLASH); }
	void	SetLamp(BYTE lp)	{ LampCtrl(lp, LAMP_ON); }
	void	ClrLamp(BYTE lp)	{ LampCtrl(lp, LAMP_OFF); }

private:
	PVOID	c_pParent;
	CPio	c_io;

	BYTE	c_nLamp;
	BYTE	c_nFlash;

#define	MAX_FLASHTIMER		8	// CAUSE!!! byte frame
	UCURV	c_uFlashTimer[MAX_FLASHTIMER];
#define	TIME_FLASH	300 / TIME_INTERVAL
#define	DEBOUNCE_PANEL	16

	static const BYTE	c_nLampForm[];

	BOOL	CheckBCD(BYTE bcd);
	void	Lamp(BYTE nLamp, BYTE cmd);
};

#endif /* CPANELCC_H_ */
