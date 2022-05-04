/*
 * CTurf.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "Infer.h"

class CTurf
{
public:
	CTurf();
	virtual ~CTurf();

private:
	PVOID	c_pParent;

	enum enMATCHITEM {
		MID_TIME = 0,
		MID_ENV,
		MID_CRR,
		// 200218
		MID_CLFC,	// Clear Line fault counter
		MID_MAX
	};

	typedef struct _tagMATCHSHAKE {
		__BC(	banner,
					request,	1,
					accept,		1,
					agree,		1,
					aid,		1,		// 181019
					sp,			4);
		WORD	wRequestTimer;
		WORD	wAgreeTimer;
		WORD	wAgreeMap;
		WORD	wAttendMap;
	} MATCHSHAKE;
	MATCHSHAKE	c_match[MID_MAX];

#define	TPERIOD_TIMEMATCH		REALBYSPREAD(3000)		// 시스템 시간 설정 완료 시간
#define	TPERIOD_ENVMATCH		REALBYSPREAD(10000)		// 환경 설정 완료 시간
#define	TPERIOD_CRRMATCH		REALBYSPREAD(10000)		// 운행 이력 설정 완료 시간
#define	TPERIOD_CLFCMATCH		REALBYSPREAD(10000)		// 통신 오류 카운터 지우기 시간
#define	TPERIOD_AGREE			REALBYSPREAD(3000)

	int		ScanRequest(int mid, BYTE reqf, BYTE cstID);
	bool	ScanAgree(int mid);

public:
	void	Initial();
	void	Control(PRECIP pRecip);
	void	Monitor(PRECIP pRecip);
	void	StirTimeMatch(bool byRtd);
	void	StirEnvironMatch();
	void	StirCareerMatch();
	void	StirClearLfc();
	bool	GetTimeMatchStatus()		{ return c_match[MID_TIME].banner.b.request; }
	//bool	GetEnvironMatchStatus()		{ return c_match[MID_ENV].banner.b.request; }
	//bool	GetCareerMatchStatus()		{ return c_match[MID_CRR].banner.b.request; }
	void	SetParent(PVOID pParent)	{ c_pParent = pParent; }

	PUBLICY_CONTAINER();
};
