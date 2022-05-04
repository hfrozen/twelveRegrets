/*
 * CSch.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "Slight.h"
#include "Infer.h"

#define	GETSCHEME(p)	CSch* p = CSch::GetInstance()

class CSch
{
public:
	CSch();
	virtual ~CSch();

private:
	WORD	c_wCarBitmap;
	WORD	c_wFlowBitmap;
	BYTE	c_cCarLength;
	BYTE	c_cCarTypeID;
	DEVCONF	c_devConf;

	static const BYTE	c_cC2f[4][DEFAULT_CARLENGTH];
	static const WORD	c_localDeviceMap[4][DEFAULT_CARLENGTH];	// 4가지 조합, 객차 길이는 최대 10
	static const WORD	c_wSivPosBitmap[4];
	static const double	c_dbAspBase[4][DEFAULT_CARLENGTH];
	//static const double c_dbLwBase[4][DEFAULT_CARLENGTH];
	static const double c_dbLwBase10[4][DEFAULT_CARLENGTH];

public:
	void	Initial();
	void	Drawup(BYTE cCarLength);
	WORD	GetLDeviceMap(int iCID);
	int		C2F(bool bTenor, int iCID);
	int		F2C(bool bTenor, int iFID);
	int		P2C(int iPID);		// 210510
	int		GetSivID(int iCID);
	int		GetV3fID(int iCID);
	int		GetEskCID(int iEID);

	double	GetProportion(double dbPsc, double dbPscMin, double dbPscMax, double dbLogMin, double dbLogMax);
	WORD	GetProportion(WORD wPsc, WORD wPscMin, WORD wPscMax, WORD wLogMin, WORD wLogMax);
	int		GetProportion(int iPsc, int iPscMin, int iPscMax, int iLogMin, int iLogMax);
	double	BarToKgcm2(BYTE cBar);	// cBar - 0~10/0~0xff
	// 200218
	double	KPaToKgcm2(WORD wkpa);
	double	CalcLoad(int cid, double dbAsp);
	WORD	CalcLoad(int cid, WORD weight);

public:
	WORD	GetSivPosBitmap()		{ return c_wSivPosBitmap[c_cCarTypeID]; }
	WORD	GetCarBitmap()			{ return c_wCarBitmap; }
	WORD	GetFlowBitmap()			{ return c_wFlowBitmap; }
	int		GetCarFormID()			{ return (int)c_cCarTypeID; }
	PDEVCONF	GetDevConf()		{ return &c_devConf; }
	static CSch*	GetInstance()	{ static CSch sch; return &sch; }

	PUBLICY_CONTAINER();
};
