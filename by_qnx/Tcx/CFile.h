/*
 * CFile.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CFILE_H_
#define CFILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include <Draft.h>
#include <Archive.h>

class CFile
{
public:
	CFile();
	virtual ~CFile();

private:
	PVOID	c_pOwner;
	FILE*	c_pDrv;
	FILE*	c_pArm;
	ARM_ARCH	c_armList[ALARMLIST_LINES];
	WORD	c_wFilesByDayLength;
	DWORD	c_dwFilesByDay[SIZE_FILESBYDAY];
	DWORD	c_dwFileDay;	// ?????
	WORD	c_wFilesUnderDayLength;
	DWORD	c_dwFilesUnderDay[SIZE_FILESUNDERDAY];

	long	GetFileLength(FILE* fp);
	void	ClearFilesByDay();
	BOOL	ArrangeFileByDay(DWORD file);
	void	ClearFilesUnderDay();
	BOOL	ArrangeFileUnderDay(DWORD file);

public:
	SHORT	SaveAlarm(PARM_ARCH pArm, DWORD size, const char* pTarget);
	DWORD	PickAlarm(PARM_ARCH pArm, DWORD size, const char* pTarget);
	SHORT	AppendTrial(PTRIAL_ARCH pTrial);
	void	DeleteDrv();
	void	DeleteAlarm();
	SHORT	LoadEnv(PENVARCHEXTENSION pEnv, BOOL bExt);
	SHORT	SaveEnvToTemp(PENVARCHEXTENSION pEnv);
	// Modified 2013/11/02
	//SHORT	CheckEnv(PENVARCHEXTENSION pEnv, WORD length, BOOL cmd);
	SHORT	CheckEnv(PENVARCHEXTENSION pEnv, BOOL cmd);
	SHORT	SaveInspect(PINSPECTSPEC pInsp);
	SHORT	SaveTrace(UCURV di, UCURV ci, BYTE* pTrace, DWORD dwLength);
	int		LoadSect(PVOID pVoid);
	int		SaveSect(PVOID pVoid, WORD wLength);
	WORD	GetAlarmFileMaxIndex();
	WORD	GetAlarmFileMaxLine();
	BOOL	GetAlarmTimeByIndex(WORD id, BYTE* pName);
	void	SortByDay();
	void	SetFilesByDay(WORD length, DWORD* pDwDays);
	DWORD	GetSelectedByDay(WORD id);
	void	SortUnderDay(DWORD day);
	void	SetFilesUnderDay(WORD length, DWORD* pDwDays);
	DWORD	GetSelectedUnderDay(WORD id);
	BOOL	LoadCurAlarmBlock(WORD cur, WORD page);
	BOOL	LoadAlarmBlock(WORD cur, WORD page);
	PARM_ARCH	GetAlarmList()	{ return c_armList; }
	WORD	GetFilesByDayLength()	{ return c_wFilesByDayLength; }
	DWORD*	GetFilesByDay()			{ return &c_dwFilesByDay[0]; }
	WORD	GetFilesUnderDayLength()	{ return c_wFilesUnderDayLength; }
	DWORD*	GetFilesUnderDay()		{ return &c_dwFilesUnderDay[0]; }
	void	SetOwner(PVOID pVoid)	{ c_pOwner = pVoid; }
};

#endif /* CFILE_H_ */
