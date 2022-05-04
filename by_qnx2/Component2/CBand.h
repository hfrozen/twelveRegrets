/*
 * CBand.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include <sys/syspage.h>
#include "DuoTypes.h"
#include "Prefix.h"
#include "Arrange.h"
#include "Infer.h"

//#define	MTU_LENGTH		6			// measuring time unit length = 1000000ns
//#define	MTU_FORM(x)		1e##x
//#define	CUR_MTU			MTU_FORM(6)	// 1e6

#define	_TWATCHCLICK()	uint64_t __begin_ = ClockCycles()
#define	_TWATCHMEASURE(t, x, e)\
	uint64_t __end_ = ClockCycles();\
	double __sec_ = (double)(__end_ - __begin_) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;\
	x = (t)(__sec_ * 1e##e)

#define	_TWATCHMEASURED()\
	uint64_t __end_ = ClockCycles();\
	double __sec_ = (double)(__end_ - __begin_) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;\

#define	GETBAND(p)	CBand* p = CBand::GetInstance()

#include "CPop.h"

class CBand
{
private:
	CBand();
	virtual ~CBand();

//#define	SECT_USED		0
#define	PERIOD_TIMEOUT	FID_MAX
#define	SIZE_DTBTCHART	(FID_MAX + 4)	// 16
#define	GENERIC_FUNC	30

	typedef struct _tagTIMEFIELD {
		QWORD	cur;
		//BYTE	stt;
		bool	bUsed;
	} TFIELD;

public:
	typedef struct _tagTIMECHART {
		QWORD	benchm;
		TFIELD	period[2][SIZE_DTBTCHART];
		TFIELD	funct[GENERIC_FUNC];
	} TCHART;
	TCHART		c_record[3];
	WORD		c_wRegion;

	typedef struct _tagDTBTIMES {
		WORD	wTime[2][FID_MAX + 1];
	} DTBTIMES, *PDTBTIMES;
	DTBTIMES	c_dtbtm[2];

	static const PSZ c_pszPeriodName[SIZE_DTBTCHART];
	static const PSZ c_pszFuncName[GENERIC_FUNC];

public:
	void	Synchro();
	void	Synchro(bool bLine);
	void	Backup();
	void	Section(bool bLine, int sect);
	void	Region(int regn);
	void	ResetRegionID()					{ c_wRegion = 0; }
	WORD	GetRegionID()					{ return c_wRegion; }
	PDTBTIMES	GetDtbTimes()				{ return &c_dtbtm[1]; }


	int		Bale(bool bType, BYTE* pMesh);
	static CBand*	GetInstance()		{ static CBand band; return &band; }
};
