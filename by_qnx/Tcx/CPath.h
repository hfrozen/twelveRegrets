/*
 * CPath.h
 *
 *  Created on: 2011. 2. 16
 *      Author: Che
 */

#ifndef CPATH_H_
#define CPATH_H_

#include <Draft.h>

class CPath
{
public:
	CPath();
	virtual ~CPath();

private:
	PVOID		c_pParent;
	PTCREFERENCE	c_pTcRef;
	PLCFIRM		c_pLcFirm;
	PTCDOZ		c_pDoz;
	//BOOL		c_bTakeHcr;

	static const WORD	c_wTdir[16][2];

	// Inserted 2012/01/16
	void	ClearInform();
	void	SetInform();
	void	CopyTcFrame(BYTE nSrcID, BYTE nDestID);

public:
	void	InitialModule(PVOID pVoid);
	void	AtcPowerCtrl(BOOL cmd);
	UCURV	Control();
	UCURV	Setup(BYTE nMaster);
};

#endif /* CPATH_H_ */
