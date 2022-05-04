/*
 * CDump.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

class CDump
{
public:
	CDump();
	virtual ~CDump();

private:
	PVOID	c_pParent;
	PVOID	c_pProp;

#define	LENGTH_PDTENTRY			16	// 화면에 표시되는 각 항목의 갯수
#define	LENGTH_DAILYENTRY		8
#define	LENGTH_MONTHLYENTRY		6
#define	LENGTH_LOGBOOKENTRY		32	// 171213, 16
#define	MAXLENGTH_ENTRY			(LENGTH_PDTENTRY + LENGTH_DAILYENTRY + LENGTH_MONTHLYENTRY)
	DWORD	c_dwEntrySelMap;
	ENTRYSHAPE	c_entrys[MAXLENGTH_ENTRY];		// 패스없이 파일 이름만 저장된다.

	static const BYTE	c_cLineBar[FID_MAX];
	static const DWORD	c_dwFillBar[2][7][2];

	//void	CopyRedBlock(int di, int si, bool bInv);

public:
	void	ClearEntryList();
	void	PioState();
	void	LineText(WORD wCid, WORD dhid);
	void	LineState();
	WORD	GetSortListByPage(WORD wPage, bool bTrouble);
	//WORD	GetSortList(WORD wPage, bool bTrouble, bool bAll);
	WORD	GetEachListByPage(WORD wID, WORD wPage);
	//WORD	GetTroubleForEachCar(WORD wID, WORD wPage);
	PENTRYSHAPE	GetEntry(int n);
	void	SetEntrySelMap(DWORD dwSel);
	DWORD	MonoEntryList(PVOID pVoid);
	DWORD	TriEntryList(PVOID pVoid, _QUARTET quTri);
	void	SetItemText();
	DWORD	GetEntrySelMap()						{ return c_dwEntrySelMap; }
	void	SetProp(PVOID pProp, PVOID pParent)		{ c_pProp = pProp; c_pParent = pParent; }
};
