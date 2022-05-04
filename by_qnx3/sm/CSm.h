/**
 * @file	CSm.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "SmRefer.h"
#include "../Part/CPop.h"

class CSm
{
public:
	CSm();
	virtual ~CSm();

private:
	TRHND		c_hThreadWatch;
	TRHND		c_hThreadAlign;
	CSemaphore	c_sema;
	QHND		c_hStq;
	int			c_hStp;
	PSTPACK		c_pStp;

	BYTE		c_cPrevSec;
	TODFINDEX	c_curFid;
	TODFINDEX	c_alignFid;
	bool		c_bAligning;

	static PVOID	WatchEntry(PVOID pVoid);
	static PVOID	FileAlignEntry(PVOID pVoid);

	void	Watch();
	void	Destroy();
	int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority);
	int		CreateThreadA(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority);
	bool	AppendTod();
	int		MakeDirectory(PSZ pPath);
	int		DeleteFile(PSZ pPath, PSZ pFile);
	TODFINDEX	ToddBcd2Hex(TODDLM tdl);
	TODFINDEX	ToddStr2Hex(char* pStr);
	void	TodtBcd2Str(TODDLM tdl, char* pStr);
	void	TimeLog(PMSRTIMEDW pMtd, double sec);
	int64_t	GetSpace(PSZ pPath, bool bSpace);
	bool	ZipFile(PSZ pPath);
	void	FileAlign();
	void	FileCounter(PSZ pPath);

public:
	bool	Launch();
};
