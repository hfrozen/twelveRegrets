/**
 * CTool.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "DuoTypes.h"

#define	GETTOOL(p)	CTool* p = CTool::GetInstance()

class CTool
{
private:
	CTool();
	virtual ~CTool();

	char	c_cBuf[256];

public:
	DWORD	ToBcd(DWORD dw);
	WORD	ToBcd(WORD w);
	WORD	ToBcd(BYTE* p, int length);
	BYTE	ToBcd(BYTE c);
	WORD	ToHex(WORD w);
	BYTE	ToHex(BYTE c);
	DWORD	ToBigEndian(DWORD dw);
	WORD	ToBigEndian(WORD w);
	WORD	BcdnXword(BYTE c0, BYTE c1);
	bool	WithinTolerance(WORD w1, WORD w2, WORD tol);
	bool	WithinTolerance(double db1, double db2, double tol);
	bool	WithinCriterion(double db1, double db2, double crit);
	//DWORD	Deviround10(DWORD dw);
	double	Deviround10(double db);
	BYTE	GetBitNumbers(WORD w, bool bState);
	BYTE	GetBitNumbers(BYTE c, bool bState);
	int		GetFirstBit(WORD w, bool bState);
	int		GetFirstBit(BYTE c, bool bState);
	BYTE	Backword(BYTE c);
	void	Rollf(const char* fmt, ...);
	int		FindProc(PSZ procName);
	int		RunProc(PSZ procName, PSZ args[]);
	bool	RemoveProc(int pid);
	static CTool*	GetInstance()			{ static CTool tool; return &tool; }
};
