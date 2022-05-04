/*
 * CSpy.h
 *
 *  Created on: 2011. 2. 15
 *      Author: Che
 */

#ifndef CSPY_H_
#define CSPY_H_

#include <Draft.h>

class CSpy
{
public:
	CSpy();
	virtual ~CSpy();

private:
	static const PSZ	c_timeNames[];
	static const PSZ	c_ccNames[];
	PVOID	c_pParent;
	char	c_buf[10];

	void	Header(char* p, int head);

public:
	void	Nameplate();
	void	ReportTime(float cps, POPERATETIME pTime);
	void	ReportBus(PTCREFERENCE pTcRef, PLCFIRM pLcFirm, PTCDOZ pDoz, PSRDUCINFO pDu);
	int	GetReportLength();
	void	SetOwner(PVOID pVoid)	{ c_pParent = pVoid; }
};

#endif /* CSPY_H_ */
