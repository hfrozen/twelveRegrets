/*
 * CLook.h
 *
 *  Created on: 2011. 2. 21
 *      Author: Che
 */

#ifndef CLOOK_H_
#define CLOOK_H_

#include <Draft.h>

class CLook
{
public:
	CLook();
	virtual ~CLook();

private:
	PVOID	c_pParent;
	PTCREFERENCE	c_pTcRef;
	PLCFIRM		c_pLcFirm;
	PTCDOZ		c_pDoz;
	BOOL		c_bPowerOn;
	uint64_t	c_timeDist;

	typedef struct _tagV3FARCH {
		DWORD	dwSpeedBuff[LENGTH_CC * 4];
		double	dbDeviation[LENGTH_CC * 4];
		WORD	wLength;
		WORD	wRstCmd;
		WORD	wFmccoCmd;
		// Modified 2012/03/12 ... begin
		// WORD	wFmccoLength;
		WORD	wFailDck;
		// ... end
		WORD	wMaxCurrent;
	} V3FARCH;
	V3FARCH		c_v3fd;
	//WORD		c_wHeavy;

public:
	typedef	struct _tagNOTCHFORM {
		WORD	min;
		WORD	max;
		_TV_	vTbeB;
		WORD	percent;
		WORD	rescue;
	} NOTCHFORM, *PNOTCHFORM;
	static const NOTCHFORM c_notchA[];
	static const NOTCHFORM c_notchB[];

private:
	void	CheckVoltage();
	void	CtrlEach();
	void	CtrlEachEmer(PCAREMER pCe, BOOL bRef);
	BOOL	ModeHandle(BYTE mode);
	void	NonMode(BYTE atcm, BYTE atci);
	// Modified 2013/11/02... remove
	//BOOL	ScanV3fResetSignals(PV3FEINFO peV3f);
	BOOL	ScanV3fCutoutSignals(PV3FEINFO peV3f);
	void	SivAnnals(BYTE n, PSIVEINFO peSiv);
	void	V3fAnnals(BYTE n, PV3FEINFO peV3f);
	void	EcuAnnals(BYTE n, PECUAINFO paEcu);
	BOOL	CheckV3f(UCURV nCcid, PDEVICESTATE pDev);
	BOOL	CheckEcu(UCURV nCcid, PDEVICESTATE pDev);
	BOOL	CheckCmsb(UCURV nCcid);
	void	CmsbLoad(BOOL cmdA, BOOL cmdB);
	void	CalcV3fSpeed();
	void	CopyDcuErrorCount(WORD count, UCURV sid, UCURV did, UCURV cid);
	// Modified 2012/01/16 ... begin
	//void	CmsbCtrl(WORD sivf, BOOL cmd);
	void	CmsbCtrl(WORD sivf);
	void	CmsbPowerMonit(BYTE nCcid);
	// ... end
	BYTE	GetSettingBits(BYTE bits);

public:
	void	InitialModule(PVOID pVoid);
	void	ToDriverlessReady();
	void	AbsoluteEmergency();
	void	ModeCtrl();
	void	OpModeLamp(BYTE lp, BYTE state);
	void	GetMascon(PMCDSTICK pDs);
	// Modified 2012/03/05 ... begin
	// void	GetMascon(PINSTRUCTA pInst, BYTE nIndex);
	// ... end
	void	PisAnnals(PPISAINFO paPis);
	void	TrsAnnals(PTRSAINFO paTrs);
	void	HtcAnnals(PHTCAINFO paHtc);
	void	PsdAnnals(PPSDAINFO paPsd);
	void	Check();
	WORD	GetNotchRescue(BYTE n);
};

#endif /* CLOOK_H_ */
