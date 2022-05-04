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
public:
	bool	c_bDebug;
private:
	bool	c_bCancel;

#define	MAX_ENTRY			200
	ENTRYSHAPE	c_entrys[MAX_ENTRY];			// define at RtdInfo.h

	FILELIST	c_fl;

	WORD	c_wOldListFullMap;
	OLDTROUBLES	c_oldt;							// old troubles

	typedef struct _tagFILEGUIDE {
		int		iSfd;							// open for read
		int		iDfd;							// open for write
		char	szSrc[SIZE_DEFAULTPATH];
		char	szDest[SIZE_DEFAULTPATH];
	} FILEGUIDE;
	FILEGUIDE	c_curf;

	typedef struct _tagDIRGUIDE {
		char	szName[SIZE_DEFAULTPATH];
		DWORD	dwFileIndex;
		DWORD	dwIndex;
		bool	bType;
		bool	bCut;
	} DIRGUIDE;
	DIRGUIDE	c_curd;

	typedef struct _tagABACUSLIST {
		WORD	wCarBitmap;						// c_wAbacus[1234] : 고장 코드(ex:1234)가 발생한 객차 번호 비트
												// b9:9번 객차 ~ b0:0번 객차
		BYTE	dhour;
		BYTE	dmin;
		BYTE	dsec;
	} ABACUSLIST;
	ABACUSLIST	c_abacList[MAX_TROUBLE];

	char	c_szDestTrace[SIZE_DEFAULTPATH];

	typedef struct _tagECUTRACEMODULE {
		WORD			wLength;
		ECUTRACETEXTS	tds;
	} ECUTRACEM;
	ECUTRACEM	c_etm;

	void	CloseCurFile();
	int64_t	GetFileSize(const char* pPath);
	int64_t	GetDirSize(const char* pPath);
public:
	int		DeleteDir(const char* pPath);
private:
	DWORD	GetLately6(const char* pPath, char cSign, DWORD dwLimit);
	int		SortDirEntry6(const char* pPath, char cSign, int iLength);

	int		DeleteOld12(const char* pPath, char cSign);
	QWORD	GetLately12(const char* pPath, char cSign, QWORD qwLimit);
	int		SortDirEntry12(const char* pPath, char cSign, int iSi, int iLength);

	bool	MakeTraceDirOnUsb();
	WORD	ReadFile(char* pBuf, WORD wLength);
	void	AlignOldTroubleFile(const char* pFile);
	void	AlignOldTroubleDir(const char* pDir);
	//int64_t	DeleteOutside(const char* pPath);
	//bool	SearchUsb();
public:
	int64_t	GetSpace(const char* pPath, bool bSpace);	// bSpace = 1 -> total, = 0 -> free
	int		GetFilesLength(const char* pPath, char cSign);
	DWORD	GetDateIndex(bool bPrev, int delMon);
	DWORD	GetDateIndex(bool bPrev);
	DWORD	GetTimeIndex();
	int		CopyFile(const char* pDest, const char* pSrc, bool bPermitCancel = false);
	void	CopyLogbook(PRTDSPADE pSpd, char* pDestPath, char* pLogDir);
	void	DeleteFiles(const char* pPath, char cSign);
	void	DeletePrevCpm(const char* pPath, int month);
	void	DeletePrev6(const char* pPath);
	void	LimitPrev12(const char* pPath, int iLimit, char cSign);
	void	MakeLogbookList(const char* pPath);
	DWORD	MakeLogbookEntry(const char* pPath, DWORD dwTotal);
	DWORD	MakeInspectEntry(DWORD dwTotal);
	void	ExcerptFile(PRTDSPADE pSpd, char* pLogDir);
	int		BindFileToUsb(PRTDSPADE pSpd, char* pPath);
	void	AlignOldTrouble();
	void	SaveOldTrouble();
	void	ArrangeEcuTrace();
	void	MoveTraceSphere(int ldi);
	void	MoveEcuTraceSphere();

	PENTRYSHAPE	GetEntrys()					{ return c_entrys; }
	PFILELIST	GetLists()					{ return &c_fl; }
	BYTE*	GetOldTroubleBuf()				{ return &c_oldt.c[0]; }
	void	SetCancel(bool bCancel)			{ c_bCancel = bCancel; }
	void	SetDebug(bool bDebug)			{ c_bDebug = bDebug; }
	void	SetParent(PVOID pParent)		{ c_pParent = pParent; }
};
