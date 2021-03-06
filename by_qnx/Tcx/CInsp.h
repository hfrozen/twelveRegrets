/*
 * CInsp.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CINSP_H_
#define CINSP_H_

#include <Draft.h>
#include <Archive.h>
#include "DuInfo.h"

enum {
	INSPECTSND_BEGIN = 0,
	INSPECTSND_DOORRIGHT,
	INSPECTSND_EB,
	INSPECTSND_FSB,
	INSPECTSND_SPEEDCODEL,
	INSPECTSND_SPEEDCODEH,
	INSPECTSND_SENSITIVITY,
	INSPECTSND_OVERSPEEDL,
	INSPECTSND_OVERSPEEDH,
	INSPECTSND_END,
	INSPECTSND_DELAY,
	INSPECTSND_MAX
};

enum {
	SNDINSPECTSTEP_BEGIN = 0,
	SNDINSPECTSTEP_TESTTRS,
	SNDINSPECTSTEP_TESTTRSDELAY,
	SNDINSPECTSTEP_TESTATCA,
	SNDINSPECTSTEP_TESTATCB,
	SNDINSPECTSTEP_TESTATO,
	SNDINSPECTSTEP_TESTATODELAY,
	SNDINSPECTSTEP_TESTTWC,
	SNDINSPECTSTEP_SHUTOFF,
	SNDINSPECTSTEP_END,
	SNDINSPECTSTEP_MAX
};

enum {
	SIVINSPECTSTEP_BEGIN = 0,
	SIVINSPECTSTEP_WAITTE,
	SIVINSPECTSTEP_WAITTC,
	SIVINSPECTSTEP_RESULT,
	SIVINSPECTSTEP_END,
	SIVINSPECTSTEP_MAX
};

enum {
	EXTINSPECTSTEP_BEGIN = 0,
	EXTINSPECTSTEP_WAITMSG,
	EXTINSPECTSTEP_WAITSIVK0,
	EXTINSPECTSTEP_ESKONATSIV0,
	EXTINSPECTSTEP_WAITESKC0,
	EXTINSPECTSTEP_OKSIV0,
	EXTINSPECTSTEP_RESETESK0,
	EXTINSPECTSTEP_WAITRESETESK0,
	EXTINSPECTSTEP_RESETSIVK0,
	EXTINSPECTSTEP_WAITRESETSIVK0,
	EXTINSPECTSTEP_WAITSIVK1,
	EXTINSPECTSTEP_ESKONATSIV1,
	EXTINSPECTSTEP_WAITESKC1,
	EXTINSPECTSTEP_OKSIV1,
	EXTINSPECTSTEP_RESETESK1,
	EXTINSPECTSTEP_WAITRESETESK1,
	EXTINSPECTSTEP_ESKABSOFF,
	EXTINSPECTSTEP_ESKABSOFFWAIT,
	EXTINSPECTSTEP_RESETALL,
	EXTINSPECTSTEP_WAITRESETALL,
	EXTINSPECTSTEP_MAX
};

enum {
	INSPECTECU_BEGIN = 0,
	INSPECTECU_DELAY,
	INSPECTECU_WAITOOT,
	INSPECTECU_WAITTC,
	INSPECTECU_RESULT,
	INSPECTECU_JUDGE,
	INSPECTECU_END,
	INSPECTECU_MAX
};

enum {
	ECUINSPECTSTEP_BEGIN = 0,
	ECUINSPECTSTEP_WAITNPOS,
	ECUINSPECTSTEP_WAITC,
	ECUINSPECTSTEP_SERVICE,
	ECUINSPECTSTEP_EMER,
	ECUINSPECTSTEP_ANTI,
	ECUINSPECTSTEP_SHUTOFF,
	ECUINSPECTSTEP_WAITB7POS,
	ECUINSPECTSTEP_END,
	ECUINSPECTSTEP_MAX
};

enum {
	BCPINSPECTSTEP_BEGIN = 0,
	BCPINSPECTSTEP_WAITHBCOS,
	BCPINSPECTSTEP_READLOOP,
	BCPINSPECTSTEP_DELAY,
	BCPINSPECTSTEP_VIEW,
	BCPINSPECTSTEP_WAITNPOS,
	BCPINSPECTSTEP_CHECKEB,
	BCPINSPECTSTEP_DELAYB,
	BCPINSPECTSTEP_VIEWB,
	BCPINSPECTSTEP_WAITUEB,
	BCPINSPECTSTEP_WAITUHBCOS,
	BCPINSPECTSTEP_END,
	BCPINSPECTSTEP_MAX
};

enum {
	V3FINSPECTSTEP_BEGIN = 0,
	V3FINSPECTSTEP_WAITSECURYTY,
	V3FINSPECTSTEP_WAITTCE0,
	V3FINSPECTSTEP_WAITTCE1,
	V3FINSPECTSTEP_RESULT,
	V3FINSPECTSTEP_WAIT,
	V3FINSPECTSTEP_JUDGE,
	V3FINSPECTSTEP_WAITUNSECURYTY,
	V3FINSPECTSTEP_END,
	V3FINSPECTSTEP_MAX
};

enum {
	DRVINSPECTSTEP_BEGIN = 0,
	DRVINSPECTSTEP_WAITSECURYTY,
	DRVINSPECTSTEP_WAITP1,
	DRVINSPECTSTEP_WAITONLY,
	DRVINSPECTSTEP_RESULT,
	DRVINSPECTSTEP_WAITUNSECURYTY,
	DRVINSPECTSTEP_END,
	DRVINSPECTSTEP_MAX
};

enum {
	CMSBINSPECTSTEP_BEGIN = 0,
	CMSBINSPECTSTEP_WAITCMG,
	CMSBINSPECTSTEP_JUDGE,
	CMSBINSPECTSTEP_SHUTDOWN,
	CMSBINSPECTSTEP_END,
	CMSBINSPECTSTEP_MAX
};

enum {
	COOLINSPECTSTEP_BEGIN,
	COOLINSPECTSTEP_WAITEXC,
	COOLINSPECTSTEP_WAITEND,
	COOLINSPECTSTEP_JUDGE,
	COOLINSPECTSTEP_ALLNG,
	COOLINSPECTSTEP_WAIT,
	COOLINSPECTSTEP_END,
	COOLINSPECTSTEP_MAX
};

enum {
	HEATINSPECTSTEP_BEGIN,
	HEATINSPECTSTEP_WAITEXC,
	HEATINSPECTSTEP_WAITEND,
	HEATINSPECTSTEP_JUDGE,
	HEATINSPECTSTEP_ALLNG,
	HEATINSPECTSTEP_WAIT,
	HEATINSPECTSTEP_END,
	HEATINSPECTSTEP_MAX
};

enum {
	DOORBKUPINSPECTSTEP_BEGIN,
	DOORBKUPINSPECTSTEP_WAITMM,
	DOORBKUPINSPECTSTEP_OPENRIGHT,
	DOORBKUPINSPECTSTEP_OPENEDRIGHT,
	DOORBKUPINSPECTSTEP_CLOSERIGHT,
	DOORBKUPINSPECTSTEP_CLOSEDRIGHT,
	DOORBKUPINSPECTSTEP_JUDGERIGHT,
	DOORBKUPINSPECTSTEP_OPENLEFT,
	DOORBKUPINSPECTSTEP_OPENEDLEFT,
	DOORBKUPINSPECTSTEP_CLOSELEFT,
	DOORBKUPINSPECTSTEP_CLOSEDLEFT,
	DOORBKUPINSPECTSTEP_JUDGELEFT,
	DOORBKUPINSPECTSTEP_TURNAA,
	DOORBKUPINSPECTSTEP_WAITAA,
	DOORBKUPINSPECTSTEP_END,
	DOORBKUPINSPECTSTEP_MAX
};

enum {
	TCMSINSPECTSTEP_BEGIN = 0,
	TCMSINSPECTSTEP_WAIT,
	TCMSINSPECTSTEP_JUDGE,
	TCMSINSPECTSTEP_WAITA,
	TCMSINSPECTSTEP_END,
	TCMSINSPECTSTEP_MAX
};

enum {
	LAMPINSPECTSTEP_BEGIN = 0,
	LAMPINSPECTSTEP_WAIT,
	LAMPINSPECTSTEP_WAITA,
	LAMPINSPECTSTEP_END,
	LAMPINSPECTSTEP_MAX
};

class CInsp
{
public:
	CInsp();
	virtual ~CInsp();

protected:
	PTCREFERENCE	c_pTcRef;
	PLCFIRM		c_pLcFirm;
	PTCDOZ		c_pDoz;

private:
	PVOID	c_pParent;

	BYTE	c_nSubStep;
	WORD	c_wCcForm;
	BOOL	c_bCmd;
	BOOL	c_bPause;
	BOOL	c_bDelay;
	BOOL	c_bWait;
	BOOL	c_bResult;
	WORD	c_wDoorWait;
	WORD	c_wDoorRes;
	BYTE	c_nDoorStep[8];
	BYTE	c_wBuf[4];
	uint64_t	c_clock[8];

	INSPECTSPEC		c_spec;

	static const WORD	c_wAtoPage[2][INSPECTSND_MAX][INSPECTMODE_MAX];
	static const WORD	c_wBcStep[SIZE_PRESSURELIST][2];
	static const WORD	c_wAsp[2][8];
	static const WORD	c_wBcp[10][8];
	static const WORD	c_wSivJudge[2][2];
	static const WORD	c_wHangupMsgWithBell[];

	typedef BOOL (CInsp::*INSPECTF)();
	typedef struct _tagINSPECTFLOW{
		WORD	wItem;
		INSPECTF	func;
		WORD	wPage[INSPECTMODE_MAX];
	} INSPECTFLOW;
	static const INSPECTFLOW	c_flow[];

	enum  {
		INSPKEY_NON = 0,
		INSPKEY_SKIP,
		INSPKEY_REPEAT
	};

	void	WaitBegin(WORD sec);
	void	BitCtrl(BOOL bCtrl, WORD wRef, WORD* pTar);
	void	ChgPage(WORD page);
	void	HangupWithBell(WORD msg);
	BYTE	WaitRepeatOrSkip();
	WORD	GetAtoPage();
	void	AtoRequest(BOOL bCmd, BYTE nBcdCmd, WORD delay);
	void	BitsToOK(BYTE ref, WORD* pW);
	WORD	BitsToNG(WORD w);
	BOOL	AtoInspect(WORD delay);
	BOOL	AtoInspect(WORD delay, WORD flag, WORD* pRes);
	BOOL	AtoInspect(WORD delay, WORD flag, WORD* pRes, WORD* pSubRes);
	BOOL	AtoInspect(WORD delay, WORD* pRes, WORD* pSubRes);
	BOOL	SndInspect();
	BOOL	SignalnDoorItem();
	BOOL	SivItem();
	BOOL	ExtItem();
	void	EcuResult(BYTE nCid, BYTE nRes, BYTE kind);
	BOOL	EcuInspect(BYTE nKind);
	BOOL	EcuItem();
	void	JudgePressure(UCURV id, WORD pos);
	void	ReadPressure(BOOL bCmd);
	void	InitPressure();
	void	BottleBcp(UCURV id, BOOL bDir);
	BOOL	BcpItem();
	void	V3fResult(BYTE nCid, BYTE nRes);
	BOOL	V3fItem();
	BOOL	PwrItem();
	void	JudgeHtc(BYTE judge, WORD* pSubRes, WORD flag, WORD* pRes);
	BOOL	CoolItem();
	BOOL	HeatItem();
	BOOL	CmsbItem();
	BOOL	DoorBackupItem();
	BOOL	TcmsItem();
	BOOL	LampItem();
	BOOL	Delay(WORD delay);
	void	Exit();

public:
	void	InitialModule(PVOID pVoid);
	BOOL	Check();
	WORD	GetPage();
	void	ResetStep();
	PDUINSPECTINFO	GetInfo()	{ return &c_spec.di; }
	PINSPECTSPEC	GetSpec()	{ return &c_spec; }
};

#endif /* CINSP_H_ */
