#pragma once

#define	GETTOOLS(p)	CTools* p = CTools::GetInstance()

class CTools
{
public:
	CTools();
	virtual ~CTools();

public:
	int		Rand(int nBase, int nPercent);
	int		ahtoi(CString str);
	int		Proportional(int real, int pscMin, int pscMax, int logMin, int logMax);
	double	Proportional(double real, double pscMin, double pscMax, double logMin, double logMax);

	static CTools*	GetInstance()	{ static CTools tools; return &tools; }
};


