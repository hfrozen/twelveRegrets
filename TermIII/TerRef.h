// TerRef.h
#pragma once

typedef struct tagPORTATT {
	int		nPort;
	unsigned long	dwBps;
	int		nDatas;
	int		nStps;
	int		nParity;
	int		nDtr;
	int		nRts;

	void Initial()
	{
		nPort = -1;
		dwBps = 0;
		nDatas = -1;
		nStps = -1;
		nParity = -1;
		nDtr = -1;
		nRts = -1;
	}
	void Initial(int port, unsigned long bps, int dl, int sl, int pr, int dt, int rt)
	{
		nPort = port;
		dwBps = bps;
		nDatas = dl;
		nStps = sl;
		nParity = pr;
		nDtr = dt;
		nRts = rt;
	}
	int GetPort()
	{
		return nPort >= 0 ? nPort : 0;
	}
	int GetBps()
	{
		return dwBps;
	}
	int GetDataLength()
	{
		return nDatas == 7 ? 7 : 8;
	}
	int GetStopLength()
	{
		return nStps == 1 ? 1 : 0;
	}
	int GetParity()
	{
		return nParity == 1 || nParity == 2 ? nParity : 0;
	}
	int GetDtr()
	{
		return nDtr == 1 || nDtr == 2 ? nDtr : 0;
	}
	int GetRts()
	{
		return nRts >= 0 && nRts <= 3 ? nRts : 0;
	}
	void operator = (tagPORTATT& pt)
	{
		nPort = pt.nPort;
		dwBps = pt.dwBps;
		nDatas = pt.nDatas;
		nStps = pt.nStps;
		nParity = pt.nParity;
		nDtr = pt.nDtr;
		nRts = pt.nRts;
	}
} PORTATT;
