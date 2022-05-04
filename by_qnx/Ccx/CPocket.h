/*
 * CPocket.h
 *
 *  Created on: 2010. 12. 23
 *      Author: Che
 */

#ifndef CPOCKET_H_
#define CPOCKET_H_

#include <CAnt.h>
#include <CDevice.h>

enum  {	DEVFAULT_NON = 0,
		DEVFAULT_OPENMAIL = -1,
		DEVFAULT_OPENDEVICE = -2,
		DEVFAULT_HASNOTFIRM = -3
};

class CPocket	: public CAnt
{
public:
	CPocket();
	CPocket(PVOID pParent, int mq);
	CPocket(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CPocket();

private:
	PVOID	c_pParent;
	int		c_mail;

	CDevice*	c_pDev[DEVID_MAX];
	WORD		c_wDataType[DEVID_MAX];

private:
	WORD	c_wActiveDev;
	SHORT	c_doorFlow;

	typedef struct _tagINITARCH {
		UCURV	uCh;
		WORD	wAddr;
		UCURV	uMode;
		UCURV	uBps;
		UCURV	uSendLength;
		UCURV	uRecvLength;
		UCURV	uCycleTime;
		PSZ		pszName;
	} INITARCH, *PINITARCH;
	static const INITARCH	c_initArch[];
	static const TRLENGTH	c_wDataLengthByType[DEVID_MAX + 1][DATATYPE_MAX];

protected:
	PCCDOZ		c_pDoz;
	PSIVAINFO	c_paSiv;
	PV3FAINFO	c_paV3f;
	PECUAINFO	c_paEcu;
	PDCUAINFO	c_paDcul;
	PDCUAINFO	c_paDcur;

public:
	WORD	c_wPocketDeb[DEVID_MAX];
	WORD	c_wDoorDeb[8];
protected:
	SIVAINFO	c_aSiv;
	V3FAINFO	c_aV3f;
	ECUAINFO	c_aEcu;
	CMSBAINFO	c_aCmsb;
	DCUAINFO	c_aDcul;
	DCUAINFO	c_aDcur;
//public:
	DCUAINFO	c_aDcua[8];

protected:
	void	MaReceiveDevice(DWORD dwParam);
	DECLARE_MAILMAP();

public:
	int		InitDev();
	int		SendV3f();
	int		SendSivEcu();
	int		SendDcu();
	int		SendCmsb();
	void	InitialModule();
};

#endif /* CPOCKET_H_ */
