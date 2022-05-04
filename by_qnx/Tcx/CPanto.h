/*
 * CPanto.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CPANTO_H_
#define CPANTO_H_

#include <CAnt.h>
#include <Draft.h>

class CPanto
{
public:
	CPanto();
	virtual ~CPanto();

private:
#define	PANTOUP_FRONT		1
#define	PANTOUP_REAR		2

protected:
	PTCREFERENCE	c_pTcRef;
	PTCDOZ	c_pDoz;

	BYTE	c_nSwPrev;
	BYTE	c_nSwBuf;
#define	PANTOSW_EDN		1
#define	PANTOSW_DN		2
#define	PANTOSW_UP		4

	BYTE	c_nSwDeb;
#define	PANTOSW_DEB		3

private:
	PVOID	c_pParent;

	enum  {
		COMPONBY_PANTO = 1,
		COMPONBY_ACMCS,
		COMPOFFBY_PANTO,
		COMPOFFBY_ACMCS,
		COMPOFFBY_XCHG
	};

	enum  {
		COMPCTRLBY_PANTO = 1,
		COMPCTRLBY_ACMCS
	};

	void	Sheet();
	void	CheckSW();
	void	CheckAcmcs();
	void	DownPreparation();
	void	Down();
	void	Up(BYTE state);
	void	Off();
	BOOL	PrevUp(BYTE state);
	void	AuxCompReady();
	void	AuxCompReadyA();
	BOOL	AuxCompStart(UCURV id);
	BOOL	AuxCompCtrl(UCURV id);
	void	CompCtrl(UCURV id, UCURV cmd);
	BOOL	ScanEarthing(UCURV cid);

public:
	void	InitialModule(PVOID pVoid);
	BOOL	Drive();
	void	AuxCompReset();
	void	Primary();
};

#endif /* CPANTO_H_ */
