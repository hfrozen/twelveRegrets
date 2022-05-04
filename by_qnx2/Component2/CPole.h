/*
 * CPole.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "Arrange.h"
#include "Draft.h"

#include "CDev.h"
#include "CTerm.h"
#include "CBand.h"
#include "CPop.h"
#include "CPump.h"

class CPole :	public CPump
{
public:
	CPole(PVOID pParent, QHND hReservoir);
	CPole(PVOID pParent, QHND hReservoir, PSZ pszOwnerName);
	virtual ~CPole();

private:
	typedef union _tagPOLENODE {
		CTerm*	pTerm;
		CDev*	pDev;
	} _POLENODE;
enum  { REALPDEVCH_MAX = PDEVID_RTD + 2 };
	_POLENODE	c_nd[REALPDEVCH_MAX];

	BYTE	c_cAtoSeq;
	BYTE	c_cAtoChg;
	BYTE	c_cSvcCurID;
	BYTE	c_cSvcReceivedID;
	BYTE	c_cLimit;
	//WORD	c_wHduSendTime;
	bool	c_bRecvMon[PDEVID_MAX];

#define	SIZE_DLEBUF		256
	BYTE	c_cDle[3][SIZE_DLEBUF];

	enum enATODLEFAIL {
		ADFAIL_NON = 0,
		ADFAIL_STX,
		ADFAIL_WITHOUTSTX,
		ADFAIL_SIZE,
		ADFAIL_CRC,
		ADFAIL_OVER,
		ADFAIL_ETX
	};
	WORD	c_wRed[PDEVID_MAX][LCF_MAX];		// 7 * 8, with unknown id

	CMutex	c_mtx;

	static const LOCALLAYERA	c_layer[];
	static const BYTE	c_cSvcSequence[5];		// 3
	static const WORD	c_crc16Table[256];

#define	PDEV_LENG		3
#define	PDEVEX_LENG		5

protected:
	void	Destroy();
	WORD	Crc161d0f(BYTE* p, WORD wLength);
	WORD	Crc16ffff(BYTE* p, WORD wLength);
	WORD	DleEncoder(BYTE* pSrc, WORD wLength);
	int		DleDecoder(BYTE* pDest, WORD wLength);
	void	IncreaseRed(int pdi, int errf);
	BYTE	GetPDIFromCurID(int curi);
	BYTE	GetCurIDFromPDI(int pdi);
	BYTE	GetPreviousPDI(int pdi);
	void	Timeout(int pdi);
	bool	SendPau();
	bool	SendPis();
	bool	SendTrs();
	bool	SendFdu();
	bool	SendBms();
	void	ReceiveRtd(WORD wLeng, BYTE cState, bool bReal);
	void	ReceiveSvc(WORD wLeng, BYTE cState, bool bReal);
	void	ReceiveAto(WORD wLeng, BYTE cState, bool bReal);
	void	ReceiveHdu(WORD wLeng, BYTE cState, bool bReal);

public:
	void	Initial();
	bool	SendAto();
	bool	SendHdu();
	bool	SendHduA();
	bool	SendHdu(BYTE* pBuf, WORD wLength);
	bool	SendHduA(BYTE* pBuf, WORD wLength);
	bool	SendRtd();
	bool	SendSvc();
	bool	Setup();
	WORD	CaptureRed(WORD* pBuf);

protected:
	void	TakeReceiveDev(DWORD dw);
	void	TakeReceiveTerm(DWORD dw);

	PUBLISH_TAGTABLE();

	PUBLICY_CONTAINER();
};
