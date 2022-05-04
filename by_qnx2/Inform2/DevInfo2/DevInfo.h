/**
 * @file	DevInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "AtoInfo.h"
#include "HduInfo.h"
#include "PauInfo.h"
#include "PisInfo.h"
#include "TrsInfo.h"
// 200218
#include "FduInfo.h"
#include "BmsInfo.h"
#include "RtdInfo.h"
#include "SivInfo.h"
#include "EcuInfo.h"
#include "V3fInfo.h"
#include "HvacInfo.h"
#include "DcuInfo.h"
#include "CmsbInfo.h"

typedef struct _tagLOCALFUND {
	WORD	wAddr;
	BYTE	cCtrl;		// 0x13
	BYTE	cFlow;		// 0x20:sdr, 0x30:sda
} LOCALPREFIX, *PLOCALPREFIX;

typedef union _tagLOCALAINFO {
	//LOCALPREFIX	lf;
	SIVAINFOTRCEX	sivEx;
	V3FAINFOTRC	v3f;
	//ECUAINFO	ecu;		// -----
	ECUAINFO2	ecu2;
	HVACAINFO	hvac;
	DCUAINFO	dcu;
	PAUAINFO	pau;
	PISAINFO	pis;
	TRSAINFO	trs;
	ATOAINFO	ato;
	HDUAINFO	hdu;
} LOCALANIFO, *PLOCALAINFO;

typedef bool (*PREQFUNC)(PVOID pVoid);

typedef struct _tagLOCALLAYERA {
	int		iCh;
	WORD	wAddr;
	DWORD	dwBps;
	WORD	wCycleTime;
	PSZ		pszName;
} LOCALLAYERA;

enum enLINEINDEX {
	LI_CURR,
	LI_BKUP,
	LI_WORK,
	LI_MAX
};

enum enLINELENGTH {
	LL_REQ,
	LL_ANS,
	LL_DEB,
	LL_MAX
};
