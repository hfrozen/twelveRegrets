/*
 * CExt.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CEXT_H_
#define CEXT_H_

#include <CAnt.h>
#include <Draft.h>

class CExt
{
public:
	CExt();
	virtual ~CExt();

protected:
	PTCREFERENCE	c_pTcRef;
	PTCDOZ	c_pDoz;

public:
//	BOOL	c_bReduction;

private:
	PVOID	c_pParent;

	UCURV	CheckEnterExt();
	BOOL	CheckExitExt();
	UCURV	ScanExtNode(BYTE nCcid);
	void	LoadControl(BOOL cmd);
	void	LoadReduction(BOOL cmd1, BOOL cmd2);

public:
	void	InitialModule(PVOID pVoid);
	void	Check();
};

#endif /* CEXT_H_ */
