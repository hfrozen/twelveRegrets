// Tools.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Tools.h"

#include <math.h>

CTools::CTools()
{
	srand((unsigned int)(time(NULL)));
}

CTools::~CTools()
{
}

int CTools::Rand(int nBase, int nPercent)
{
	int limit = nBase * nPercent / 100;
	if (limit > 0) {
		nBase += ((int)rand() % limit);
		if (nBase < 0)	return 0;
	}
	return nBase;
}

int CTools::ahtoi(CString str)
{
	int cv = 0;
	for (int n = 0; n < str.GetLength(); n ++) {
		TCHAR ch = str.GetAt(n);
		if ((ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F')) {
			if (ch >= L'a' && ch <= L'f')	ch &= 0xdf;
			ch -= L'0';
			if (ch > 9)	ch -= 7;
			cv <<= 4;
			cv |= (ch & 0xf);
		}
		else	break;
	}
	return cv;
}

int CTools::Proportional(int real, int pscMin, int pscMax, int logMin, int logMax)
{
	int scale = pscMax - pscMin;
	if (scale > 0)	return ((real - pscMin) * (logMax - logMin) / scale) + logMin;
	return 0;
}

double CTools::Proportional(double real, double pscMin, double pscMax, double logMin, double logMax)
{
	double scale = pscMax - pscMin;
	if (scale > 0.f)	return ((real - pscMin) * (logMax - logMin) / scale) + logMin;
	return 0.f;
}
