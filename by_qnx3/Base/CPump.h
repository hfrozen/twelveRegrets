/**
 * @file	CPump.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once

#include <sched.h>
#include "../Common/DefTypes.h"

#define	MAX_TAG		1024

/** @typedef _tagTAG
 * 큐로 보내는 태그 형식
 */
typedef struct _tagTAG {
	int		nID;			///< 태그 아이디
	PVOID	pClass;			///< 태그를 전달할 클래스 포인터
	long	param;			///< 같이 전달하는 파라메터
} _TAG, *_PTAG;

struct TAGENVELOPE;
struct TAGTABLE {
	const TAGTABLE*		pBase;			// must remove
	const TAGENVELOPE*	pEnvelope;
};

#define	PUBLISH_TAGTABLE()\
	public:\
		virtual const char* GetClassName()	const;\
	private:\
		static const TAGENVELOPE tagEnvelope[];\
	protected:\
		static const char theClassName[];\
		static const TAGTABLE tagTable;\
		virtual const TAGTABLE* GetTagTable() const;

#define	BEGIN_TAGTABLE(thisClass, baseClass)\
const char* thisClass::GetClassName() const\
	{ return thisClass::theClassName; }\
const TAGTABLE* thisClass::GetTagTable() const\
	{ return &thisClass::tagTable; }\
const char thisClass::theClassName[24] = #thisClass;\
const TAGTABLE thisClass::tagTable =\
	{ &baseClass::tagTable, &thisClass::tagEnvelope[0] };\
const TAGENVELOPE thisClass::tagEnvelope[] = {

#define END_TAGTABLE()\
{ -1, (PTAGFUNC)NULL }\
}

#include "CLabel.h"

class CPump :	public CLabel
{
public:
	CPump();
	CPump(PVOID pParent, QHND hReservoir);
	CPump(PVOID pParent, QHND hReservoir, PSZ pszOwnerName);
	CPump(PSZ pszOwnerName);
	virtual ~CPump();
	virtual int	Bale(BYTE* pBowl)		{ return 0; }

	QHND	c_hReservoir;
	PSZ		c_pszReservoir;

	QHND	CreateReservoir(PSZ pszReservoir);
	int		DeleteReservoir();
	bool	SendTag(int nID, PVOID pVoid, long param);
	bool	IsValidTag()				{ return Validity(c_hReservoir) ? true : false; };

private:
	TRHND	c_hThread;
	static PVOID	PumpWatcherEntry(PVOID pVoid);
	void	Watcher();

	PUBLISH_TAGTABLE();
};

typedef void (CPump::*PTAGFUNC) (long param);
struct TAGENVELOPE {
	int			nID;
	PTAGFUNC	pFunc;
};

//#define	_TAGMAP(nID, pFunc)	{ nID, (PTAGFUNC)(void (CPump::*)())pFunc },
#define	_TAGMAP(nID, func)	{ nID, (PTAGFUNC)(void (CPump::*)())&func },
