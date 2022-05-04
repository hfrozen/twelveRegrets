/*
 * CUsual.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

class CUsual
{
public:
	CUsual();
	virtual ~CUsual();

private:
	PVOID	c_pParent;
	PVOID	c_pProp;
	//WORD	c_wCmgbf;		// 19/12/05
	//WORD	c_wSivbf;		// 19/12/05
	bool	c_bFireDialog;	// ?????

	//WORD	c_wScreenOut;

	static const WORD	c_wPatterns[4][CID_MAX];

	void	CheckTriple(PBUNDLES pBund);
	void	CheckQuintuplet(PBUNDLES pBund);
	void	CheckOctet(PBUNDLES pBund);

public:
	void	Initial();
	void	HeadLine();
	void	ServiceA();
	void	ServiceB();
	//bool	GetFireDialogState()					{ return c_bFireDialog; }
	//WORD	GetScreenOut()							{ return c_wScreenOut; }
	void	SetProp(PVOID pProp, PVOID pParent)		{ c_pProp = pProp; c_pParent = pParent; }
};
