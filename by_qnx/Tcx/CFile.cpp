/*
 * CFile.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include "CFile.h"
#include "CTcx.h"

CFile::CFile()
{
	c_pOwner = NULL;
	c_pDrv = NULL;
	c_pArm = NULL;
	c_wFilesByDayLength = 0;
	c_wFilesUnderDayLength = 0;
	c_dwFileDay = 0;
}

CFile::~CFile()
{
}

long CFile::GetFileLength(FILE* fp)
{
	if (fp == NULL)	return 0L;

	long org, size;
	if ((org = ftell(fp)) < 0)			return -1L;
	if (fseek(fp, 0L, SEEK_END) != 0)	return -2L;
	if ((size = ftell(fp)) < 0)			return -3L;
	if (fseek(fp, org, SEEK_SET) != 0)	return -4L;
	return size;
}

SHORT CFile::SaveAlarm(PARM_ARCH pArm, DWORD size, const char* pTarget)
{
	FILE* fp;
	if ((fp = fopen(pTarget, "w")) == NULL)	return FILEERROR_CANNOTCREATEARM;
	//for (DWORD dw = 0; dw < size; dw ++)
	//	fwrite((PVOID)&(pArm + dw)->a, sizeof(ARM_ARCH), 1, fp);
	for (DWORD dw = size; dw > 0; dw --)
		fwrite((PVOID)&(pArm + dw - 1)->a, sizeof(ARM_ARCH), 1, fp);
	fclose(fp);
	MSGLOG("[FILE]Write Alarm block %ld.\r\n", size);
	return FILEERROR_NON;
}

DWORD CFile::PickAlarm(PARM_ARCH pArm, DWORD size, const char* pTarget)
{
	FILE* fp;
	if ((fp = fopen(pTarget, "w")) == NULL)	return 0;
	DWORD length = 0;
	//for (DWORD dw = 0; dw < size; dw ++) {
	//	if ((pArm + dw)->b.used != 0 && (pArm + dw)->b.alarm != 0) {
	//		fwrite((PVOID)&(pArm + dw)->a, sizeof(ARM_ARCH), 1, fp);
	//		++ length;
	//	}
	//}
	for (DWORD dw = size; dw > 0; dw --) {
		if ((pArm + dw - 1)->b.used != 0 && (pArm + dw - 1)->b.alarm != 0) {
			fwrite((PVOID)&(pArm + dw - 1)->a, sizeof(ARM_ARCH), 1, fp);
			++ length;
		}
	}
	fclose(fp);
	MSGLOG("[FILE]Pickup Alarm block %ld.\r\n", length);
	return length;
}

SHORT CFile::AppendTrial(PTRIAL_ARCH pTrial)
{
	FILE* fp = fopen(TRIAL_PATHT, "a+");
	if (fp != NULL) {
		fwrite(pTrial, sizeof(TRIAL_ARCH), 1, fp);
		fclose(fp);
		return 0;
	}
	else	MSGLOG("[FILE]ERROR:Trial file can not open(%s).\r\n", strerror(errno));
	return -1;
}

void CFile::DeleteDrv()
{
	unlink(DRV_PATHT);
}

void CFile::DeleteAlarm()
{
	unlink(ARM_PATHT);
}

SHORT CFile::LoadEnv(PENVARCHEXTENSION pEnv, BOOL bExt)
{
	FILE* fp;
	if (bExt) {
		if ((fp = fopen(ENV_PATHE, "r")) != NULL) {	// from external memory
			SHORT ret = fread(pEnv->nRoll, 1, sizeof(ENVARCHEXTENSION), fp);
			fclose(fp);
			MSGLOG("[FILE]Load enviroment file from external memory.(size %d)\r\n", ret);
			// Modified 2013/11/02
			//return CheckEnv(pEnv, ret, TRUE);
			return CheckEnv(pEnv, TRUE);
		}
	}
	if ((fp = fopen(ENV_PATHN, "r")) != NULL) {		// from hard disk
		SHORT ret = fread(pEnv->nRoll, 1, sizeof(ENVARCHEXTENSION), fp);
		fclose(fp);
		MSGLOG("[FILE]Load enviroment file from hard disk.(size %d)\r\n", ret);
		// Modified 2013/11/02
		//return CheckEnv(pEnv, ret, TRUE);
		return CheckEnv(pEnv, TRUE);
	}
	MSGLOG("[FILE]ERROR:Load enviroment file.\r\n");
	return -1;
}

SHORT CFile::SaveEnvToTemp(PENVARCHEXTENSION pEnv)
{
	FILE* fp;
	if ((fp = fopen(ENV_PATHT, "w")) == NULL) {
		MSGLOG("[FILE]ERROR:Save enviroment file.\r\n");
		return -1;
	}
	fwrite(pEnv, 1, sizeof(ENVARCHEXTENSION), fp);
	fclose(fp);
	MSGLOG("[FILE]Save enviroment file.(size %d)\r\n", sizeof(ENVARCHEXTENSION));
	return 0;
}

// Modified 2013/11/02
//SHORT CFile::CheckEnv(PENVARCHEXTENSION pEnv, WORD length, BOOL cmd)
SHORT CFile::CheckEnv(PENVARCHEXTENSION pEnv, BOOL cmd)
{
//	if (length != sizeof(ENVARCHEXTENSION)) {
//		ENVARCH old;
//		memcpy((PVOID)&old, (PVOID)pEnv, sizeof(ENVARCH));
//		memset((PVOID)pEnv, 0, sizeof(ENVARCHEXTENSION));
//		memcpy((PVOID)&pEnv->real.w, (PVOID)old.real.w, SIZE_WHEELCAR * SIZE_WHEELAXEL);
//		memcpy((PVOID)&pEnv->real.entire.elem.devTime, (PVOID)&old.real.entire.elem.devTime, sizeof(DEVTIME));
//		pEnv->real.entire.elem.dbDist = old.real.entire.elem.dbDist;
//		pEnv->real.entire.elem.dbTime = old.real.entire.elem.dbTime;
//		pEnv->real.entire.elem.dbV3fPower = old.real.entire.elem.dbPower;
//		pEnv->real.entire.elem.dbRevival = old.real.entire.elem.dbRevival;
//		pEnv->real.entire.elem.wTrainNo = old.real.entire.elem.wTrainNo;
//	}
	for (UCURV n = 0; n < SIZE_WHEELCAR; n ++) {
		for (UCURV m = 0; m < SIZE_WHEELAXEL; m ++) {
			if (pEnv->real.w[n][m] < MIN_WHEELDIA ||
					pEnv->real.w[n][m] > MAX_WHEELDIA) {
				if (!cmd)	return (n * SIZE_WHEELCAR + m + 1);
				pEnv->real.w[n][m] = DEFAULT_WHEELDIA;
			}
		}
	}
	return 0;
}

SHORT CFile::SaveInspect(PINSPECTSPEC pInsp)
{
	FILE* fp;
	if ((fp = fopen(INSP_PATHT, "w")) == NULL) {
		MSGLOG("[FILE]ERROR:Save inspect data file.\r\n");
		return -1;
	}
	fwrite(pInsp, sizeof(INSPECTSPEC), 1, fp);
	fclose(fp);
	MSGLOG("[FILE]Save inspect data file.(size %d)\r\n", sizeof(INSPECTSPEC));
	return 0;
}

SHORT CFile::SaveTrace(WORD di, WORD ci, BYTE* pTrace, DWORD dwLength)
{
	char nBuf[128];
	switch (di) {
	case TRACEID_SIV :
		sprintf(nBuf, "%s%c", TSIV_PATHT, 'a' + ci);
		break;
	case TRACEID_V3F :
		sprintf(nBuf, "%s%c", TV3F_PATHT, 'a' + ci);
		break;
	case TRACEID_ECU :
		sprintf(nBuf, "%s%c", TECU_PATHT, 'a' + ci);
		break;
	case TRACEID_ATC :
		sprintf(nBuf, "%s", TATC_PATHT);
		break;
	case TRACEID_ATO :
		sprintf(nBuf, "%s", TATO_PATHT);
		break;
	default :
		return 0;
		break;
	}
	FILE* fp;
	if ((fp = fopen(nBuf, "w")) == NULL) {
		MSGLOG("[FILE]ERROR:Save trace data file.\r\n");
		return -1;
	}
	fwrite(pTrace, sizeof(BYTE), dwLength, fp);
	fclose(fp);
	MSGLOG("[FILE]Save trace data file.(size %ld)\r\n", dwLength);
	return 0;
}

int CFile::LoadSect(PVOID pVoid)
{
	FILE* fp;
	if ((fp = fopen(SECT_PATHT, "r")) == NULL) {
		MSGLOG("[FILE]ERROR:Load sect file.\r\n");
		return -1;
	}
	int size = (int)fread(pVoid, sizeof(BYTE), SIZE_MATETEXT, fp);
	fclose(fp);
	if (size == 0)	MSGLOG("[FILE]ERROR:Load sect size is zero.\r\n");
	return size;
}

int CFile::SaveSect(PVOID pVoid, WORD wLength)
{
	FILE* fp;
	if ((fp = fopen(SECT_PATHT, "w")) == NULL) {
		MSGLOG("[FILE]ERROR:Save sect file.\r\n");
		return -1;
	}
	if (wLength > SIZE_MATETEXT)	wLength = SIZE_MATETEXT;
	int size = (int)fwrite(pVoid, sizeof(BYTE), wLength, fp);
	fclose(fp);
	if (size == 0)	MSGLOG("[FILE]ERROR:Save sect size is zero(%d).\r\n", wLength);
	return size;
}

WORD CFile::GetAlarmFileMaxIndex()
{
	FILE* fp;
	if ((fp = fopen(DIR_PATHT, "r")) == NULL)	return 0;
	long size = GetFileLength(fp);
	if (size > 0) {
		char* pBufA = (char*)malloc(size);
		if (pBufA == NULL)	return 0;
		char* pBuf = pBufA;
		fread(pBuf, sizeof(BYTE), size, fp);
		fclose(fp);
		WORD max = 0;
		while (*pBuf != 0) {
			if (*pBuf == '*') {
				++ pBuf;
				WORD index = (WORD)atoi(pBuf);
				if (index > max)	max = index;
			}
			else ++ pBuf;
		}
		free(pBufA);
		return max;
	}
	else	fclose(fp);

	return 0;
}

WORD CFile::GetAlarmFileMaxLine()
{
	FILE* fs;
	if ((fs = fopen(ARM_PATHET, "r")) == NULL) {
		MSGLOG("[FILE]ERROR:Can not open file.(%s)\r\n", ARM_PATHET);
		return 0;
	}
	WORD max = 0;
	long size = GetFileLength(fs);
	fclose(fs);
	if (size > 0) {
		WORD line = size / sizeof(ARM_ARCH);
		if ((line % sizeof(ARM_ARCH)) > 0)	++ line;
		max = line / ALARMLIST_LINES;
		if ((line % ALARMLIST_LINES) > 0)	++ max;
	}
	return max;
}

BOOL CFile::GetAlarmTimeByIndex(WORD id, BYTE* pName)
{
	*pName = 0;
	BOOL res = FALSE;
	FILE* fp;
	if ((fp = fopen(DIR_PATHT, "r")) == NULL) {
		MSGLOG("[FILE]ERROR:Can not open directory list file at alarm list.\r\n");
		return FALSE;
	}
	long size = GetFileLength(fp);
	if (size > 0) {
		char* pBufA = (char*)malloc(size);
		if (pBufA == NULL) {
			MSGLOG("[FILE}ERROR:malloc() at GetAlarmNameByIndex.\r\n");
			return FALSE;
		}
		char* pBuf = pBufA;
		fread(pBuf, sizeof(BYTE), size, fp);
		fclose(fp);
		while (*pBuf != 0 && !res) {
			if (*pBuf == '*') {
				++ pBuf;
				WORD index = (WORD)atoi(pBuf);
				if (index == id) {
					while (*pBuf != ':' && *pBuf != '\r' && *pBuf != '\n' && *pBuf != 0)	++ pBuf;
					if (*pBuf == ':') {
						++ pBuf;
						while ((*pBuf >= '0' && *pBuf <= '9') || *pBuf == 'D')
							*pName ++ = *pBuf ++;
						*pName = 0;
						res = TRUE;
					}
				}
			}
			else	++ pBuf;
		}
		free(pBufA);
	}
	else	fclose(fp);

	return res;
}

void CFile::SortByDay()
{
	ClearFilesByDay();

	char buf[10];
	FILE* fp;
	if ((fp = fopen(DIR_PATHT, "r")) == NULL) {
		MSGLOG("[FILE]ERROR:Can not open directory list file at sort by day.\r\n");
		return;
	}
	long size = GetFileLength(fp);
	if (size > 0) {
		char* pBufA = (char*)malloc(size);
		if (pBufA == NULL) {
			MSGLOG("[FILE]ERROR:malloc() at SortByDay.\r\n");
			return;
		}
		char* pBuf = pBufA;
		fread(pBuf, sizeof(BYTE), size, fp);
		fclose(fp);
		while (*pBuf != 0) {
			if (*pBuf == '*') {
				while (*pBuf != 'D' && *pBuf != 0)	++ pBuf;
				if (*pBuf == 'D') {
					++ pBuf;
					memcpy((PVOID)&buf[0], (PVOID)pBuf, 6);	// only yy-mm-dd
					buf[6] = 0;
					DWORD dw = (DWORD)atoll(buf);
					if (dw > 0)	ArrangeFileByDay(dw);
				}
			}
			else	++ pBuf;
		}
		free(pBufA);
	}
	else	fclose(fp);
}

void CFile::SetFilesByDay(WORD length, DWORD* pDwDays)
{
	ClearFilesByDay();
	if (length >= SIZE_FILESBYDAY)	length = SIZE_FILESBYDAY;
	c_wFilesByDayLength = length;
	for (WORD w = 0; w < length; w ++)
		c_dwFilesByDay[w] = *pDwDays ++;
}

void CFile::ClearFilesByDay()
{
	for (UCURV n = 0; n < SIZE_FILESBYDAY; n ++)
		c_dwFilesByDay[n] = 0;
	c_wFilesByDayLength = 0;
}

BOOL CFile::ArrangeFileByDay(DWORD file)
{
	if (file == 0)	return FALSE;

	file %= 1000000l;
	DWORD dw = file;
	dw %= 10000;
	BYTE month = (BYTE)(dw / 100);
	BYTE day = (BYTE)(dw % 100);
	if (month == 0 || day == 0)	return FALSE;

	for (UCURV n = 0; n < SIZE_FILESBYDAY; n ++) {
		if (c_dwFilesByDay[n] == 0) {
			c_dwFilesByDay[n] = file;
			++ c_wFilesByDayLength;
			return TRUE;
		}
		else if (c_dwFilesByDay[n] == file)	return TRUE;
		// Modified 2013/02/19
		//else if (c_dwFilesByDay[n] < file) {
		//	for (UCURV m = SIZE_FILESBYDAY - 1; m > n; m --)
		//		c_dwFilesByDay[m] = c_dwFilesByDay[m - 1];
		//	c_dwFilesByDay[n] = file;
		//	++ c_wFilesByDayLength;
		//	return TRUE;
		//}
	}
	return FALSE;
}

DWORD CFile::GetSelectedByDay(WORD id)
{
	if (id < SIZE_FILESBYDAY)	return c_dwFilesByDay[id];
	return 0;
}

void CFile::SortUnderDay(DWORD day)
{
	ClearFilesUnderDay();

	char buf[10];
	FILE* fp;
	if ((fp = fopen(DIR_PATHT, "r")) == NULL) {
		MSGLOG("[FILE]ERROR:Can not open directory list file at sort under day.\r\n");
		return;
	}
	long size = GetFileLength(fp);
	if (size > 0) {
		char* pBufA = (char*)malloc(size);
		if (pBufA == NULL) {
			MSGLOG("[FILE]ERROR:malloc() at SortUnderDay.\r\n");
			return;
		}
		char* pBuf = pBufA;
		fread(pBuf, sizeof(BYTE), size, fp);
		fclose(fp);
		while (*pBuf != 0) {
			if (*pBuf == '*') {
				while (*pBuf != 'D' && *pBuf != 0)	++ pBuf;
				if (*pBuf == 'D') {
					++ pBuf;
					memcpy((PVOID)&buf[0], (PVOID)pBuf, 6);	// only yy-mm-dd
					buf[6] = 0;
					DWORD dw = (DWORD)atoll(buf);
					if (dw == day) {
						pBuf += 6;
						memcpy((PVOID)&buf[0], (PVOID)pBuf, 6);	// only hh-mm-ss
						buf[6] = 0;
						dw = (DWORD)atoll(buf);
						ArrangeFileUnderDay(dw);
					}
				}
			}
			else	++ pBuf;
		}
		free(pBufA);
	}
	else	fclose(fp);
}

void CFile::SetFilesUnderDay(WORD length, DWORD* pDwDays)
{
	ClearFilesUnderDay();
	if (length >= SIZE_FILESUNDERDAY)	length =SIZE_FILESUNDERDAY;
	c_wFilesUnderDayLength = length;
	for (WORD w = 0; w < length; w ++)
		c_dwFilesUnderDay[w] = *pDwDays ++;
}

void CFile::ClearFilesUnderDay()
{
	for (UCURV n = 0; n < SIZE_FILESUNDERDAY; n ++)
		c_dwFilesUnderDay[n] = 0;
	c_wFilesUnderDayLength = 0;
	c_dwFileDay = 0;
}

BOOL CFile::ArrangeFileUnderDay(DWORD file)
{
	if (file == 0)	return FALSE;

	for (UCURV n = 0; n < SIZE_FILESUNDERDAY; n ++) {
		if (c_dwFilesUnderDay[n] == 0) {
			c_dwFilesUnderDay[n] = file;
			++ c_wFilesUnderDayLength;
			return TRUE;
		}
		else if (c_dwFilesUnderDay[n] == file)	return TRUE;
		else if (c_dwFilesUnderDay[n] < file) {
			for (UCURV m = SIZE_FILESUNDERDAY - 1; m > n; m --)
				c_dwFilesUnderDay[m] = c_dwFilesUnderDay[m - 1];
			c_dwFilesUnderDay[n] = file;
			++ c_wFilesUnderDayLength;
			return TRUE;
		}
	}
	return FALSE;
}

DWORD CFile::GetSelectedUnderDay(WORD id)
{
	if (id < SIZE_FILESUNDERDAY)	return c_dwFilesUnderDay[id];
	return 0;
}

BOOL CFile::LoadCurAlarmBlock(WORD cur, WORD page)
{
	memset((PVOID)&c_armList, 0, sizeof(ARM_ARCH) * ALARMLIST_LINES);
	if (page < 1 || cur >= page)	return FALSE;
	FILE* fp;
	if ((fp = fopen(ARM_PATHCT, "r")) == NULL) {
		MSGLOG("[FILE]ERROR:Can not open file.(%s)\r\n", ARM_PATHCT);
		return FALSE;
	}

	long org = ftell(fp);
	fseek(fp, cur * sizeof(ARM_ARCH) * ALARMLIST_LINES, SEEK_SET);
	fread((PVOID)&c_armList, sizeof(ARM_ARCH), ALARMLIST_LINES, fp);
	fseek(fp, org, SEEK_SET);
	fclose(fp);
	return TRUE;
}

BOOL CFile::LoadAlarmBlock(WORD cur, WORD page)
{
	memset((PVOID)&c_armList, 0, sizeof(ARM_ARCH) * ALARMLIST_LINES);
	if (page < 1 || cur >= page)	return FALSE;
	FILE* fp;
	if ((fp = fopen(ARM_PATHET, "r")) == NULL) {
		MSGLOG("[FILE]ERROR:Can not open file.(%s)\r\n", ARM_PATHET);
		return FALSE;
	}

	long org = ftell(fp);
	fseek(fp, cur * sizeof(ARM_ARCH) * ALARMLIST_LINES, SEEK_SET);
	fread((PVOID)&c_armList, sizeof(ARM_ARCH), ALARMLIST_LINES, fp);
	fseek(fp, org, SEEK_SET);
	fclose(fp);
	return TRUE;
}
