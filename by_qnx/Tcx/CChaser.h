/*
 * CChaser.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CCHASER_H_
#define CCHASER_H_

#include <Draft.h>

class CChaser
{
public:
	CChaser();
	virtual ~CChaser();

protected:
	PTCREFERENCE	c_pTcRef;
	PTCDOZ		c_pDoz;

private:
	PVOID	c_pParent;

#define	MAX_DEV_LENGTH	10
	ATCCHASE	c_atcch;
	ATOCHASE	c_atoch;
	SIVCHASE	c_sivch[MAX_DEV_LENGTH];
	V3FCHASE	c_v3fch[MAX_DEV_LENGTH];
	ECUCHASE	c_ecuch[MAX_DEV_LENGTH];

	BYTE	BitIndex(BYTE bits);
	WORD	CalcProgress(WORD cur, WORD max);
	UCURV	CountEcuTraceMark(BYTE* p);

public:
	void	InitialModule(PVOID pVoid);
	PTRACEARCH	GetTraceArch(UCURV di, UCURV ci);
	void	TraceBegin(UCURV di, UCURV ci);
	void	TraceInitial();
	void	AtcTraceCtrl(PATCAINFO paAtc);
	void	AtoTraceCtrl(PATOAINFO paAto);
	BOOL	Link();

	PSIVCHASE	GetSivChase(UCURV cid)	{ return &c_sivch[cid]; }
	PV3FCHASE	GetV3fChase(UCURV cid)	{ return &c_v3fch[cid]; }
	PECUCHASE	GetEcuChase(UCURV cid)	{ return &c_ecuch[cid]; }
};

#endif /* CCHASER_H_ */
