/*
 * CBeil.h
 *
 *  Created on: 2011. 5. 23
 *      Author: Che
 */

#ifndef CBEIL_H_
#define CBEIL_H_

#include <inttypes.h>
#include <stdint.h>
#include <Internal.h>
#include <CAnt.h>

#define	MAX_FILEDATE	991231235959ll
#define	MAX_DELETECYCLE	256
#define	SIZE_BUF		1024

#define	VERSION_BEIL	1.12

class CBeil	: public CAnt
{
public:
	CBeil();
	virtual ~CBeil();

private:
	int		c_hMsg;
	int		c_hShm;
	PBUCKET	c_pBucket;
	pthread_t	c_hThread;
	int64_t	c_tsize;
	int64_t	c_sizes;
	typedef struct _tagBBINDEX {
		DWORD	atc;
		DWORD	ato;
		DWORD	siv;
		DWORD	v3f;
		DWORD	ecu;
		DWORD	cmsb;
	} BBINDEX;
	BBINDEX	c_index;
	BYTE	c_nDebug;
	BYTE	c_nByCap;

#define	MAX_BUF		2048
	char	c_usbName[128];
	char	c_src[128];
	char	c_dest[128];
	char	c_buf[MAX_BUF];


	static void*	Handler(PVOID pVoid);
	void	MakeTickerbox(const char* pName, DWORD* pIndex);
	void	DeleteArchives();
	void	DeleteDirectory(char* pTarget);
	BOOL	SearchUsb();
	int64_t	GetFileLength(const char* fn);
	//int		GetFileLength(FILE* fp);
	SHORT	BuildArchiveDir();
	SHORT	BuildTargetCamp(BOOL bOppo);
	SHORT	BuildTargetDir(const char* d, BOOL bOppo);
	int64_t	GetFreeSpace(const char* pTarget);
	int64_t	DeleteTopDirectory(const char* pTarget);
	int64_t DeleteOutsideDay(const char* pTarget);
	BOOL	CompareDirectory(char* pCmp, const char* pOrg);
	int		GetFileQuantity(const char* pTarget);
	void	GetFileQuantity(const char* pDirName, PDIRINFO pQuan);
	void	GetFileQuantity(const char* pDirName, const char* pFileName, PDIRINFO pQuan);
	BOOL	PrepareFreeSpaceAtMem();
	BOOL	GetAlarmFileNameByIndex(WORD id, char* pName);
	void	CopyAlarmOnly(char* pName, const char* pTarget);
	SHORT	FileCopy(const char* dn, const char* sn);
	BOOL	FileCopyA(const char* dn, const char* sn);
	BOOL	TickerDev(WORD flag, unsigned long* pl, int size, char* pPath, BYTE* pBuf);
	void	Tickering();
	void	MakeDirectoryList();
	void	LoadAlarm();
	void	AppendDrv();
	void	WriteEnv();
	void	WorkLogbook();
	void	WriteLogbook();
	void	SelectToDelete();
	void	SelectToUsb(BOOL bCmd);
	void	InspectToUsb();
	void	TraceToUsb();
	void	TrialToUsb();
	void	SectQuantity();
	void	ReadSect();
	void	WriteSectToUsb();
	void	Working(WORD msg);

public:
	BOOL	Run(BYTE debug, BYTE cap);
	void	Shutoff();
	BOOL	CreateQueue();
	BOOL	CreateShmem();
};

#endif /* CBEIL_H_ */
