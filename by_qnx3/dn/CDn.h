/**
 * @file	CDn.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "DnRefer.h"

class CDn
{
public:
	CDn();
	virtual ~CDn();

private:
	TRHND	c_hThreadWatch;
	QHND	c_hDtq;
	int		c_hDtp;
	PDTPACK	c_pDtp;
	char	c_cBuf[1024];

	static PVOID	WatchEntry(PVOID pVoid);

	void	Watch();
	void	Destroy();
	int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority);
	bool	ScanUsb();
	void	Download();
	int		SearchOtherFile(PSZ pPath, char cDeselSign);
	bool	FindOtherFile(char* pBuf, PSZ pPath, char cDeselSign);
	bool	MoveFile(PSZ pDestPath, PSZ pDestFile, PSZ pSrcPath, PSZ pSrcFile);
	int		MakeDirectory(PSZ pPath);
	void	TimeLog(PMSRTIMEDW pMtd, double sec);

public:
	bool	Launch();
};
