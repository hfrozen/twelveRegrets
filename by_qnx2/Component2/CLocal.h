/*
 * CLocal.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma	once

#include "Arrange.h"
#include "Draft.h"
#include "Infer.h"

#include "CDev.h"
#include "CBand.h"
#include "CPop.h"
#include "CPump.h"

class CLocal :	public CPump
{
public:
	CLocal(PVOID pParent, QHND hReservoir);
	CLocal(PVOID pParent, QHND hReservoir, PSZ pszOwnerName);
	virtual ~CLocal();

private:
#define	DEVID_LENG			DEVIDDE_UKD
	CDev*	c_pDev[DEVID_LENG];
	bool	c_bRecvMon[DEVID_LENG];
	WORD	c_wConfig;

#define	TIME_ALLOTWAIT			(1500 / 50)
	WORD	c_wAllotWait;
	BYTE	c_cDcuID[2];
	// 200218
	bool	c_bHcbAlter;		// 0:hvac, 1:cmsb
	bool	c_bHvacAlter;
	// 210803
	// 200918
	//bool	c_bEcuAlter;

	WORD	c_wRed[REALDEV_MAX][LCF_MAX];		// 13 * 8
	CMutex	c_mtx;

	static const LOCALLAYERA	c_layer[];
	static const WORD			c_wDcuAddr[2][4];

protected:
	void	Destroy();
	int		GetLIDfromCh(int ch);
	int		GetSerialID(int id);
	int		GetSubID(int id);
	void	IncreaseRed(int id, int errf);
	void	Timeout(int id);
	bool	SendSiv();
	bool	SendV3f();
	bool	SendEcu();
	// 200218
	bool	SendCmsb();
	bool	Allot(int lid);

public:
	void	Initial();
	bool	Setup(WORD wConfig);
	bool	ReSetup(WORD wConfig);
	void	Send50();
	void	Send100();
	bool	SendHvac();
	bool	SendDcul();
	bool	SendDcur();
	WORD	CaptureRed(WORD* pBuf);

	bool	Setup()							{ return Setup(DEVBF_DEF); }

protected:
	void	TakeReceiveDev(DWORD dw);
	void	TakeReceiveTerm(DWORD dw);

	PUBLISH_TAGTABLE();

	PUBLICY_CONTAINER();
};
