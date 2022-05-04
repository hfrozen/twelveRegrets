/**
 * @file	CTool.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include <inttypes.h>
#include "../Common/DefTypes.h"

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
	double	Deviround10(double db);
	BYTE	GetBitNumbers(WORD w, bool bState);
	BYTE	GetBitNumbers(BYTE c, bool bState);
	int		GetFirstBit(QWORD qw, bool bState);
	int		GetFirstBit(DWORD dw, bool bState);
	int		GetFirstBit(WORD w, bool bState);
	int		GetFirstBit(BYTE c, bool bState);
	BYTE	Backword(BYTE c);
	void	Rollf(const char* fmt, ...);
	int		FindStr(char* pDest, char* pSrc);
	int		FindOneOf(char* pDest, char* pSrc);
	char*	TrimRight(char* p);
	char*	TrimLeft(char* p);
	char*	Extract(char* pDest, char* pSrc, char* pConst);
	//int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority, PSZ szName);
	//int		MakeDirectory(PSZ pPath);
	//uint64_t	GetFilesSize(PSZ pPath, char cSign);
	//int64_t	GetFileSize(PSZ pPath, PSZ pFile);
	//int		GetFileSize(PSZ pPath);
	//int		SearchFile(PSZ pPath, char cSign1, char cSign2 = '\0');
	//bool	FindFile(char* pBuf, PSZ pPath, int iFileNameLength, char cSign1, char cSign2 = '\0');
	//bool	RenameFile(PSZ pNew, PSZ pOld);
	//bool	MoveFile(PSZ pDestPath, PSZ pDestFile, PSZ pSrcPath, PSZ pSrcFile);
	//bool	CopyFile(PSZ pDestPath, PSZ pDestFile, PSZ pSrcPath, PSZ pSrcFile);
	//bool	CopyFile(PSZ pDest, PSZ pSrc);
	//int		DeleteFile(PSZ pPath, PSZ pFile);
	int		FindProc(PSZ procName);
	int		RunProc(PSZ procName, PSZ args[]);
	bool	RemoveProc(int pid);
	static CTool*	GetInstance()			{ static CTool tool; return &tool; }
};
