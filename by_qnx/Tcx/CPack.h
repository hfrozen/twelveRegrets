/*
 * CPack.h
 *
 *  Created on: 2011. 2. 24
 *      Author: Che
 */

#ifndef CPACK_H_
#define CPACK_H_

#include <Draft.h>

class CPack
{
public:
	CPack();
	virtual ~CPack();

private:
	PVOID		c_pParent;
	PTCREFERENCE	c_pTcRef;
	PLCFIRM		c_pLcFirm;
	PTCDOZ		c_pDoz;

	void	PackTime(BOOL cmd);
	void	FormatTime(PTBTIME pTb);
	BYTE	ToBcdB(BYTE n);
	void	PackAtc(PATCRINFO prAtc);
	void	PackAto(PATORINFO prAto);

public:
	void	InitialModule(PVOID pVoid);
	WORD	ToBcdW(WORD w);
	void	Package();
	void	PackLocal();
	void	FormatDevTime(PDEVTIME pDev);
};

#endif /* CPACK_H_ */
