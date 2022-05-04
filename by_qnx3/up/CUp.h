/**
 * @file	CUp.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "UpRefer.h"

class CUp
{
public:
	CUp();
	virtual ~CUp();

private:
	int		c_iSock;			// 200212
	TRHND	c_hThreadWatch;
	QHND	c_hUtq;
	int		c_hUtp;
	PUTPACK	c_pUtp;
	WORD	c_wPortNo;
	char	c_cIpBuf[32];
	char	c_cBuf[1024];
	BYTE	c_cTodBuf[SIZE_TOD];

	static PVOID	WatchEntry(PVOID pVoid);

	void	Watch();
	void	Destroy();
	int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority);
	bool	Connect();		// 200212
	bool	Bridge();		// 200212
	bool	Send();			// 200212
	bool	Report();		// 200212
	void	Upload();
	bool	Transfer(PSZ pPath, char* pName);
	bool	MoveFile(PSZ pDestPath, PSZ pDestFile, PSZ pSrcPath, PSZ pSrcFile);
	bool	RenameFile(PSZ pNew, PSZ pOld);
	int		FindFile(PSZ pPath, char cSign1, char cSign2 = '\0');
	bool	FindFile(char* pBuf, PSZ pPath, int iFileNameLength, char cSign1, char cSign2 = '\0');
	int64_t	GetFileSize(PSZ pPath, PSZ pFile);
	int64_t	GetFileSize(PSZ pPath);
	void	TimeLog(PMSRTIMEDW pMtd, double sec);

public:
	bool	Launch(int argc, char* argv1, char* argv2);
};
