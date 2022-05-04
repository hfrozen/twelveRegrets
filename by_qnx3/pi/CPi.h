/**
 * @file	CPi.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "PiRefer.h"

class CPi
{
public:
	CPi();
	virtual ~CPi();

private :
	TRHND	c_hThreadWatch;
	QHND	c_hPtq;
	int		c_hPtp;
	PPTPACK	c_pPtp;
	char	c_cBuf[1024];

	static PVOID	WatchEntry(PVOID pVoid);

	void	Watch();
	void	Destroy();
	int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority);
	void	Ping(bool bWide);
	void	TimeLog(PMSRTIMEDW pMtd, double sec);

public:
	bool	Launch();
};
