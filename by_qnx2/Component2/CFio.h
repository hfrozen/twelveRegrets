/*
 * CFio.h	// Foundation to I/Ot
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "sys/siginfo.h"

#include "DuoTypes.h"
#include "Track.h"
#include "Prefix.h"
#include "Infer.h"
#include "CPop.h"

#define	NORMALWATCHDOG_CYCLE	500
#define	WAKEUPWATCHDOG_CYCLE	2000			// 17/04/05

#define	IOSET_CU	((1 << IOB_OUTP) | (1 << IOB_INPA))
#define	IOSET_TU	((1 << IOB_OUTP) | (1 << IOB_INPA) | (1 << IOB_INPB) | (1 << IOB_ADC))

class CFio
{
public:
	CFio();
	virtual ~CFio();

private:
	bool		c_bTwin;
	bool		c_bMain;
	bool		c_bInLab;
	PVOID		c_pSpace;
	BYTE		c_cDioReg;
	WORD		c_wBorn[2];
public:
	enum enIOTYPE {
		IOB_OUTP = 0,
		IOB_INPA,
		IOB_INPB,
		IOB_ADC
	};

private:
	int			c_fdAdv;
//	ch0		0 ~ 100V		0 ~ 4095
//	ch1		0 ~ 20mA		0 ~ 4095
//	ch2		-50C ~ 100C		1023 ~ 4095

#define	DEBOUNCE_CYCLE		5
	typedef struct _tagIOBUFFER {
		struct {
			INCERT		stab;
			struct {
				BYTE	deb;
				INCERT	prev;
				INCERT	cur;
			} vib;
		} in[2];
		struct {
			OUTCERT		stab;
			struct {
				BYTE	deb;
				OUTCERT	prev;
				OUTCERT	cur;
			} vib;
		} out;
	} IOBUFFER;
	IOBUFFER	c_io;
	// 171101
	_OCTET		c_octBackup[2];

	typedef struct _tagPWMBUFFER {
		BYTE	cDeb;
		BYTE	cPrev;
		BYTE	cCur;
	} PWMBUFFER;
	PWMBUFFER	c_pwm;
	WORD		c_wTacho;
	WORD		c_wTachi;

#define	LENGTH_FIOTEXT		16
	BYTE		c_cText[LENGTH_FIOTEXT];
	BYTE		c_cLamp[EPOS_MAX / 5];
	WORD		c_wLampTimer[EPOS_MAX];

	BYTE		c_cInpMons[2];	// 171029	[5];
	BYTE		c_cTrio;		// GOUTP 3비트 출력

	CMutex		c_mtx;
	CMutex		c_mto;

#define	MAX_ALLOCCODE	IOB_ADC
	static const WORD	c_wAllocCode[MAX_ALLOCCODE];
	static const DWORD	c_wCertify[MAX_ALLOCCODE];
	static const PSZ	c_szName[MAX_ALLOCCODE];
	static const int	c_iTextUnit[2][LENGTH_FIOTEXT];
	//static const BYTE	c_cLamp[5];
	static const BYTE	c_cFont[][5];

	WORD	GetInp(QWORD& qw);
	WORD	GetInpEx(QWORD& qw);
	WORD	GetOutp(WORD& w, WORD ofs = 0);
	BYTE	Debounce();
	bool	Terminate(int iTerm);
	bool	Certifier(enIOTYPE io, bool bBoot = false, bool bMsg = true);
	bool	TriggerAdv(BYTE ch);
	bool	ReadAdv(BYTE ch, WORD* pw);
	void	SetChar(int x, BYTE ch);
	void	SetChar(int x, BYTE* pCh);
	void	LampChar(int pos);
	void	SetBklight(WORD w);
	bool	CheckVmeNode(enIOTYPE io, bool bMsg = true);
	bool	ReadOutp(WORD* pw, WORD ofs = 0);
	bool	SetOutp(WORD w, WORD ofs = 0);
	bool	SetOutp(BYTE bi, bool bState, WORD ofs = 0);
	bool	MaskOutp(WORD wOut, WORD wMask, WORD ofs = 0);

protected:

public:
	WORD	GetInnersw();
	void	SetTrio(BYTE bi, bool bState);
	bool	GetAdv(BYTE ch, WORD* pw);
	WORD	GetTacho();
	DWORD	GetPwm();
	void	SetText(int x, PSZ pszText);
	//void	AlterChar(int x, BYTE ch);
	//void	RepChar(int x);
	void	Lamp(int x, bool bState = true);
	void	GetInput(INCERT& in, bool bSide = false);
	void	GetOutput(OUTCERT& out);
	bool	SetOutput(WORD w)				{ return SetOutp(w); }
	bool	SetOutput(BYTE bi, bool bState)	{ return SetOutp(bi, bState); }
	bool	SetSign(WORD w)					{ return SetOutp(w, 2); }
	bool	SetSign(BYTE bi, bool bState)	{ return SetOutp(bi, bState, 2); }
	bool	MaskSign(WORD wOut, WORD wMask)	{ return MaskOutp(wOut, wMask, 2); }
	void	EnableOutput(bool bEn);
	void	SetWatchdogCycle(WORD wCycle);
	WORD	GetWatchdogCycle(bool bCh);
	void	EnableWatchdog(bool bFs, bool bXchg);
	void	DisableWatchdog();
	WORD	TriggerWatchdog();
	WORD	GetPairCondition();
	WORD	GetGeneralReg();
	void	SetGeneralReg(WORD w);
	void	ShowCounters();
	bool	Initial(bool bRtd = false, bool bMsg = true);
	BYTE	Arteriam();
	void	Destroy();

	bool	GetMain()					{ return c_bMain; }
	bool	GetTwin()					{ return c_bTwin; }
	void	SetLaboratory(bool bInLab)	{ c_bInLab = bInLab; }
	BYTE	GetDioReg()					{ return c_cDioReg; }
	WORD	GetBorn(bool bId)			{ return c_wBorn[bId ? 1 : 0]; }
	PVOID	GetSpace()					{ return c_pSpace; }

	PUBLICY_CONTAINER();
};
