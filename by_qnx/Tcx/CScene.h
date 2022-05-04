/*
 * CScene.h
 *
 *  Created on: 2011. 2. 8
 *      Author: Che
 */

#ifndef CSCENE_H_
#define CSCENE_H_

#include <pthread.h>
#include <Draft.h>
#include <Archive.h>
#include <CFile.h>
#include <Internal.h>

typedef struct _tagREGTIME {
	BYTE	day;
	BYTE	hour;
	BYTE	min;
	BYTE	sec;
} REGTIME, *PREGTIME;

typedef struct _tagREGCODE {
	REGTIME	t;
	WORD	ac;
} REGCODE, *PREGCODE;

class CScene
{
public:
	CScene();
	virtual ~CScene();

private:
	PVOID	c_pParent;
	pthread_mutex_t	c_mutex;
	PTCREFERENCE	c_pTcRef;
	PTCDOZ		c_pDoz;

	CFile		c_file;
	BOOL		c_bInitial;

	ARM_ARCH	c_armArch[SIZE_ARMARCH];

#define	SIZE_SCENE		MAX_SCENE	// 1024
	WORD	c_wTable[SIZE_SCENE];	// store all log & warning & alarms

#define	SIZE_FILM		4096
	typedef struct _tagFILM {
		WORD	ri;
		WORD	wi;
		ARM_ARCH	arm[SIZE_FILM];
	} FILM, *PFILM;
	FILM	c_film;

private:
	ARM_ARCH	c_armList[ALARMLIST_LINES];

	BYTE	BitIndex(WORD bits);
	WORD	BitByCid(BYTE cid);
	void	Revoke(BYTE ci, WORD wMsg, PDEVTIME pDt);
	void	Add(BYTE ci, WORD wMsg, BYTE nState, PDEVTIME pDt);
	void	SetFilm(PARM_ARCH pArm);
	void	Lock()		{ pthread_mutex_lock(&c_mutex); }
	void	Unlock()	{ pthread_mutex_unlock(&c_mutex); }

public:
	void	Clear();
	void	InitialModule(PVOID pVoid);
	void	Buzz(WORD wTime);
	void	ShutBuzz();
	PARM_ARCH	GetFilm();
	void	Hide(BYTE nState);
	void	Hide();
	// Appended 2013/11/02
	void	Hide(BYTE nCid, WORD wMsg);

	void	Cancel(PDEVTIME pDt);
	void	Cancel(DWORD dwIndex, PDEVTIME pDt);
	void	Shot(BYTE nCid, WORD wMsg, PDEVTIME pDt);
	void	Cut(BYTE nCid, WORD wBegin, WORD wEnd, PDEVTIME pDt);
	void	Edit(BYTE nCid, WORD wBegin, WORD wEnd, WORD wRest, PDEVTIME pDt);
	void	AidShot(PARM_ARCH pArm);
	BOOL	CheckShot(BYTE nCid, WORD wMsg);
	WORD	GetSignCode();
	WORD	GetAlarmCode();
	WORD	GetActiveAlarmCode();
	// Modified 2013/11/02
	WORD	GetActiveAlarmCode(DWORD dwIndex);
	WORD	GetSevereCode();

	DWORD	GetSevereLength();
	DWORD	GetAlarmLength();
	// Modified 2013/11/02
	DWORD	GetBellLength();

	WORD	OpenAlarm(WORD page, BYTE nCid = 8);
	PARM_ARCH	GetAlarmList();
	void	Letter(PDEVTIME pDt);
	void	Tag(PDEVTIME pDt);
	void	ResetAlarmArch();
	void	ResetDrvArch();
	WORD	GetAlarmFileMaxIndex()	{ return c_file.GetAlarmFileMaxIndex(); }
	WORD	GetAlarmFileMaxLine()	{ return c_file.GetAlarmFileMaxLine(); }
	BOOL	GetAlarmTimeByIndex(WORD id, BYTE* pName)	{ return c_file.GetAlarmTimeByIndex(id, pName); }
	BOOL	LoadCurAlarmBlock(WORD cur, WORD page)	{ return c_file.LoadCurAlarmBlock(cur, page); }
	BOOL	LoadAlarmBlock(WORD cur, WORD page)	{ return c_file.LoadAlarmBlock(cur, page); }
	void	DeleteAlarmFile()	{ c_file.DeleteAlarm(); }
	void	DeleteDrvFile()	{ c_file.DeleteDrv(); }
	PARM_ARCH	GetAlarmArch()	{ return &c_armArch[0]; }

	SHORT	SaveAlarm(PARM_ARCH pArm, DWORD size, const char* pTarget)	{ return c_file.SaveAlarm(pArm, size, pTarget); }
	DWORD	PickAlarm(PARM_ARCH pArm, DWORD size, const char* pTarget)	{ return c_file.PickAlarm(pArm, size, pTarget); }
	SHORT	LoadEnv(PENVARCHEXTENSION pEnv)	{ return c_file.LoadEnv(pEnv, (c_pTcRef->real.nProperty & PROPERTY_USEEXTERNALMEM) ? TRUE : FALSE); }
	SHORT	SaveEnvToTemp(PENVARCHEXTENSION pEnv)	{ return c_file.SaveEnvToTemp(pEnv); }
	// Modified 2013/11/02
	//SHORT	CheckEnv(PENVARCHEXTENSION pEnv, WORD length, BOOL cmd)	{ return c_file.CheckEnv(pEnv, length, cmd); }
	SHORT	CheckEnv(PENVARCHEXTENSION pEnv, BOOL cmd)	{ return c_file.CheckEnv(pEnv, cmd); }
	SHORT	SaveInspect(PINSPECTSPEC pInsp)	{ return c_file.SaveInspect(pInsp); }
	SHORT	SaveTrace(UCURV di, UCURV ci, BYTE* pTrace, DWORD dwLength)	{ return c_file.SaveTrace(di, ci, pTrace, dwLength); }
	int		LoadSect(PVOID pVoid)	{ return c_file.LoadSect(pVoid); }
	int		SaveSect(PVOID pVoid, WORD wLength)	{ return c_file.SaveSect(pVoid, wLength); }

	void	SortByDay()	{ c_file.SortByDay(); }
	void	SetFilesByDay(WORD length, DWORD* pDwDays)	{ c_file.SetFilesByDay(length, pDwDays); }
	DWORD	GetSelectedByDay(WORD id)	{ return c_file.GetSelectedByDay(id); }
	WORD	GetFilesByDayLength()	{ return c_file.GetFilesByDayLength(); }
	DWORD*	GetFilesByDay()		{ return c_file.GetFilesByDay(); }
	void	SortUnderDay(DWORD day)		{ c_file.SortUnderDay(day); }
	void	SetFilesUnderDay(WORD length, DWORD* pDwDays)	{ c_file.SetFilesUnderDay(length, pDwDays); }
	DWORD	GetSelectedUnderDay(WORD id)	{ return c_file.GetSelectedUnderDay(id); }
	WORD	GetFilesUnderDayLength()	{ return c_file.GetFilesUnderDayLength(); }
	DWORD*	GetFilesUnderDay()	{ return c_file.GetFilesUnderDay(); }
};

#endif /* CSCENE_H_ */
