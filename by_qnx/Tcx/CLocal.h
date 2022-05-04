/*
 * CLocal.h
 *
 *  Created on: 2010. 11. 16
 *      Author: Che
 */

#ifndef CLOCAL_H_
#define CLOCAL_H_

#include <CAnt.h>
#include <CDevice.h>
#include <CPart.h>

enum  {	LOCALFAULT_NON = 0,
		LOCALFAULT_OPENMAIL = -1,
		LOCALFAULT_OPENDEVICE = -2,
		LOCALFAULT_HASNOTFIRM = -3
};

class CLocal	: public CAnt
{
public:
	CLocal();
	CLocal(PVOID pParent, int mq);
	CLocal(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CLocal();

private:
	PVOID	c_pParent;
	int		c_mail;
	SHORT	c_uInterFlow;
	BOOL	c_bAlter;
	int		c_prio;

#define	LOCALID_INT		LOCALID_PIS		// 5
#define	LOCALID_SIZE	LOCALID_INT + 1	// 6
	CDevice*	c_pDev[LOCALID_SIZE];
	CPart*		c_pDuc;

private:
	typedef struct _tagATCCHECK {
		BOOL	bAlter;
		BYTE	nCycle;
	} ATCCHECK;
	ATCCHECK	c_atcChk;
#define	CYCLE_ATCCHECK	4
#define	ATCCHKWORDA		0xaaaa
#define	ATCCHKWORDB		0x5555

	BYTE		c_nDucBulk[256];

	typedef struct _tagINITARCH {
		UCURV		uCh;
		WORD		wAddr;
		UCURV		uMode;
		UCURV		uBps;
		UCURV		uSendLength;
		UCURV		uRecvLength;
		UCURV		uCycleTime;
		PSZ			pszName;
	} INITARCH, *PINITARCH;
	static const INITARCH c_initArch[];

	static const TRLENGTH	c_wDataLengthByType[LOCALID_MAX + 1][DATATYPE_MAX];

protected:
	PTCREFERENCE	c_pTcRef;
	PLCFIRM		c_pLcFirm;
	PTCDOZ		c_pDoz;
	PDUCINFO	c_pDucInfo;
	ATCAINFO	c_aAtcNormal[2];	// for debounce, only sd
	ATCAINFO	c_aAtcInsp[2];		// only inspect data
	ATCAINFO	c_aAtcTrace[2];		// only trace data
	ATOAINFO	c_aAtoNormal;		// for debounce, only sd
	ATOAINFO	c_aAtoInsp;			// only inspect data
	ATOAINFO	c_aAtoTrace;		// only trace data
	CSCAINFO	c_aCsc;
	PISAINFO	c_aPis;
	TRSAINFO	c_aTrs;
	HTCAINFO	c_aHtc;
	PSDAINFO	c_aPsd;
	WORD		c_wCycle;

	void	MaReceiveDevice(DWORD dwParam);
	void	MaReceivePart(DWORD dwParam);
	DECLARE_MAILMAP();

private:
	WORD	c_wAck;

	void	CalcBcc(BYTE* pDest, UCURV uLength);
	void	CheckTimeClose(UCURV id, BYTE* pDest, BYTE* pBkup);
	void	CheckTimerConnection();

public:
	void	Dumbest(UCURV id);
	int		InitDev();
	int		Send50ms();
	int		Send100ms();
	int		Send200ms();
	int		SendDu();
	BYTE	GetCabNo(UCURV id);
	UCURV	CheckBcc(UCURV id);
	UCURV	CheckBcc(BYTE* pDest, UCURV uLength);
	UCURV	CheckAtcReply(PATCAINFO paAtc);
	UCURV	CheckAtoReply(PATOAINFO paAto);
	//UCURV	CheckCscReply(PCSCAINFO paCsc);
	//UCURV	CheckPisReply(PPISAINFO paPis);
	//UCURV	CheckTrsReply(PTRSAINFO paTrs);
	//UCURV	CheckHtcReply(PHTCAINFO paHtc);
	//UCURV	CheckPsdReply(PPSDAINFO paPsd);
	void	TraceToNormalABS(UCURV id);
	BOOL	ToTrace(UCURV id);
	void	AtcAnnals(PATCAINFO paAtc, BOOL bMain);
	void	AtcAnnals(PATCAINFO paAtc);
	void	AtoAnnals(PATOAINFO paAto);
	void	InitialModule();
	PATCAINFO	GetAtca()		{ return &c_aAtcNormal[0]; }
	PATCAINFO	GetAtcb()		{ return &c_aAtcNormal[1]; }
	PATOAINFO	GetAto(BOOL type = TRUE);
	void	SetPrioL(int prio)	{ c_prio = prio; }
};

#endif /* CLOCAL_H_ */
