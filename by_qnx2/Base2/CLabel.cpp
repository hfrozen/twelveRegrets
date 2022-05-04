/**
 * @file	CLabel.cpp
 * @brief
 * @details
 * @author	Che-follo
 * @data	2015-11-XX
 */
#include <string.h>
#include <errno.h>

#include "Track.h"
#include "CLabel.h"

CLabel::CLabel()
{
	c_pParent = NULL;
	SetOwnerName(UNKNOWNOWNER);
#if	_DEBL(DEBUG_CLASS)
	TRACK("%s>LAB0:id = %d, body = %X.\n", c_szOwnerName, getpid(), this);
#endif
}

CLabel::CLabel(PVOID pParent)
{
	c_pParent = pParent;
	SetOwnerName(UNKNOWNOWNER);
#if	_DEBL(DEBUG_CLASS)
	TRACK("%s>LAB1:id = %d, body = %X.\n", c_szOwnerName, getpid(), this);
#endif
}

CLabel::CLabel(PSZ pszOwnerName)
{
	c_pParent = NULL;
	SetOwnerName(pszOwnerName);
#if	_DEBL(DEBUG_CLASS)
	TRACK("%s>LAB2:id = %d, body = %X.\n", c_szOwnerName, getpid(), this);
#endif
}

CLabel::CLabel(PVOID pParent, PSZ pszOwnerName)
{
	c_pParent = pParent;
	SetOwnerName(pszOwnerName);
#if	_DEBL(DEBUG_CLASS)
	TRACK("%s>LAB3:id = %d, body = %X.\n", c_szOwnerName, getpid(), this);
#endif
}

CLabel::~CLabel()
{
#if	_DEBL(DEBUG_CLASS)
	if (c_szOwnerName != NULL)	TRACK("%s>LAB~:id = %d, body = %X.\n", c_szOwnerName, getpid(), this);
	else	TRACK("%s>LAB~:id = %d, body = %X.\n", UNKNOWNOWNER, getpid(), this);
#endif
}

int CLabel::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pBase, int priority, PSZ szMaker)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("%s>%s>ERR:can not set inherit!(%d-%s)\n",c_szOwnerName, szMaker, res, strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = priority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("%s>%s>ERR:can not set sch param!(%d-%s)\n",c_szOwnerName, szMaker, res, strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("%s>%s>ERR:can not set sch policy!(%d-%s)\n", c_szOwnerName, szMaker, res, strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, pBase);
	if (!Validity(res)) {
		TRACK("%s>%s>ERR:can not create thread!(%d-%s)\n", c_szOwnerName, szMaker, res, strerror(errno));
		return res;
	}
	return EOK;
}

void CLabel::SetOwnerName(PSZ pszOwner)
{
	memset(c_szOwnerName, 0, SIZE_OWNERBUFF);
	strncpy(c_szOwnerName, pszOwner, SIZE_OWNERBUFF);
}
