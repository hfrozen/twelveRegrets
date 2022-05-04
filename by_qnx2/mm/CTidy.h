/*
 * CTidy.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once
//#define	__EXT_LF64SRC
//#undef	__OFF_BITS__
//#define	__OFF_BITS__	64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>

#include "Mm.h"

#define	SIZE_DEFAULTPATH	256

class CTidy
{
public:
	CTidy();
	virtual ~CTidy();

private:
	PVOID	c_pParent;
	bool	c_bDebug;
	bool	c_bCancel;

public:
	TRHND	c_hThreadAlign;
	bool	c_bAlign;
	// 181011
	TRHND	c_hThreadBad;
	bool	c_bBadFurture;

private:
#define	MAX_ENTRY			200
	ENTRYSHAPE	c_entrys[MAX_ENTRY];	// define at RtdInfo.h
	FILELIST	c_fl;

	typedef struct _tagFILEGUIDE {
		int		iSfd;					// open for read
		int		iDfd;					// open for write
		char	szSrc[SIZE_DEFAULTPATH];
		char	szDest[SIZE_DEFAULTPATH];
	} FILEGUIDE;
	FILEGUIDE	c_curf;

	typedef struct _tagABACUSLIST {
		WORD	wCarBitmap;				// c_wAbacus[1234] : 고장 코드(ex:1234)가 발생한 객차 번호 비트
										// b0:0호차 TC,	b1:0호차 CC,	b2:1호차 CC,	......,	b10:9호차 CC,	b11:9호차 TC
		BYTE	dhour[CID_MAX];
		BYTE	dmin[CID_MAX];
		BYTE	dsec[CID_MAX];
	} ABACUSLIST;

	char	c_szDestTrace[SIZE_DEFAULTPATH];

	typedef struct _tagECUTRACEMODULE {
		WORD			wLength;
		ECUTRACETEXTS	tds;
	} ECUTRACEM;
	ECUTRACEM	c_etm;

	WORD	c_wLogbookVersion;

	static PVOID	AlignOldTroubleEntry(PVOID pVoid);
	static PVOID	DeleteBadFurtureDirEntry(PVOID pVoid);

	void	CloseCurFile();
	int64_t	GetDirSize(const char* pPath);
	int		DeleteDir(const char* pPath);
	DWORD	GetLately6(const char* pPath, char cSign, DWORD dwLimit);
	int		SortDirEntry6(const char* pPath, char cSign, int iLength);

	int		DeleteOld12(const char* pPath, char cSign);
	QWORD	GetLately12(const char* pPath, char cSign, QWORD qwLimit);
	int		SortDirEntry12(const char* pPath, char cSign, int iSi, int iLength);

	bool	MakeTraceDirOnUsb();
	WORD	ReadFile(char* pBuf, WORD wLength);
	// 171107
	WORD	AlignOldTroubleFile(const char* pFile, WORD wOldListFullMap, POLDTROUBLELIST pOtl);
	WORD	AlignOldTroubleDir(const char* pDir, WORD wOldListFullMap, POLDTROUBLELIST pOtl, uint64_t begin_t);
	// 181011
	void	DeleteBadFurtureFiles(const char* pPath);
	void	DeleteBadFurtureDir();

public:
	int64_t	GetFileSize(const char* pPath);
	int64_t	GetSpace(const char* pPath, bool bSpace);	// bSpace = 1 -> total, = 0 -> free
	int		GetFilesLength(const char* pPath, char cSign);
	DWORD	GetDateIndex(bool bPrev);
	DWORD	GetTimeIndex();
	int		CopyFile(const char* pDest, const char* pSrc, bool bPermitCancel = false);
	void	CopyLogbook(PRTDSPADE pSpd, char* pDestPath, char* pLogDir);
	void	DeleteFiles(const char* pPath, char cSign);
	int		DeletePrev6(const char* pPath);
	BYTE	GetLatestLogbookBlock(PLOGINFO pLog);
	void	LimitPrev12(const char* pPath, int iLimit, char cSign);
	void	MakeLogbookList(const char* pPath);
	DWORD	MakeLogbookEntry(const char* pPath, DWORD dwTotal);
	DWORD	MakeInspectEntry(DWORD dwTotal);
	void	ExcerptFile(PRTDSPADE pSpd, char* pLogDir);
	int		BindFileToUsb(PRTDSPADE pSpd, char* pPath);
	// 171107
	void	AlignOldTrouble();
	bool	StirAlignOldTrouble();
	// 181011
	bool	StirDeleteBadFurtureDir();
	void	ArrangeEcuTrace();
	void	MoveTraceSphere(int ldi);
	void	MoveEcuTraceSphere();

	PENTRYSHAPE	GetEntrys()					{ return c_entrys; }
	PFILELIST	GetLists()					{ return &c_fl; }
	WORD	GetCurrentLogbookVersion()		{ return c_wLogbookVersion; }
	void	SetCancel(bool bCancel)			{ c_bCancel = bCancel; }
	void	SetDebug(bool bDebug)			{ c_bDebug = bDebug; }
	void	SetParent(PVOID pParent)		{ c_pParent = pParent; }
};
