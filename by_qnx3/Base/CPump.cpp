/**
 * @file	CPump.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "Track.h"
#include "CPump.h"

PVOID CPump::PumpWatcherEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CPump* pPump = (CPump*)pVoid;
	if (pPump != NULL)	pPump->Watcher();
	pthread_exit(NULL);
}

CPump::CPump()
	: CLabel()
{
	c_hReservoir = INVALID_HANDLE;
	c_pszReservoir = NULL;
	c_hThread = INVALID_HANDLE;
}

CPump::CPump(PVOID pParent, QHND hReservoir)
	: CLabel(pParent)
{
	c_hReservoir = hReservoir;
	c_pszReservoir = NULL;
	c_hThread = INVALID_HANDLE;
}

CPump::CPump(PVOID pParent, QHND hReservoir, PSZ pszOwnerName)
	: CLabel(pParent, pszOwnerName)
{
	c_hReservoir = hReservoir;
	c_pszReservoir = NULL;
	c_hThread = INVALID_HANDLE;
}

CPump::CPump(PSZ pszOwnerName)
	: CLabel(pszOwnerName)
{
	c_hReservoir = INVALID_HANDLE;
	c_pszReservoir = NULL;
	c_hThread = INVALID_HANDLE;
}

CPump::~CPump()
{
	DeleteReservoir();
}

void CPump::Watcher()
{
#if	_DEBL(DEBUG_HANDLER)
	TRACK("%s>PUMP:handler start.\n", c_szOwnerName);
#endif
	while (true) {
		_TAG tag;
		if (Validity(mq_receive(c_hReservoir, (char*)&tag.nID, sizeof(_TAG), NULL))) {
			if (Validity(tag.nID)) {
#if	_DEBL(DEBUG_TAG)
				TRACK("%s>PUMP:receive tag(%d).\n", GetClassName(), tag.nID);
#endif
				CPump* pPump = (CPump*)tag.pClass;
				if (pPump != NULL) {
					const TAGTABLE* pTable = pPump->GetTagTable();
					if (pTable != NULL) {
						const TAGENVELOPE* pEnvelope = pTable->pEnvelope;
						if (pEnvelope != NULL) {
							for ( ; Validity(pEnvelope->nID) && pEnvelope->pFunc != NULL; pEnvelope ++) {
								if (pEnvelope->nID == tag.nID) {
									PTAGFUNC pTf = pEnvelope->pFunc;
									(pPump->*pTf)(tag.param);
									break;
								}
							}
						}
						else	TRACK("%s>PUMP:receive tag(%d) but null envelope!\n", GetClassName(), tag.nID);
					}
					else	TRACK("%s>PUMP:receive tag(%d) but null table!\n", GetClassName(), tag.nID);
				}
				else	TRACK("%s>PUMP:receive tag(%d) but null class!\n", GetClassName(), tag.nID);
			}
			else {
#if	_DEBL(DEBUG_TAG)
				TRACK("%s>PUMP:receive Unknown.\n", c_szOwnerName);
#endif
			}
		}
	}
#if	_DEBL(DEBUG_HANDLER)
	TRACK("%s>PUMP:handler end.\n", c_szOwnerName);
#endif
	//mq_close(c_hReservoir);
	//mq_unlink((char*)c_pszReservoir);
	//c_pszReservoir = NULL;
}

QHND CPump::CreateReservoir(PSZ pszReservoir)
{
	if (pszReservoir == NULL) {
		TRACK("%s>PUMP>ERR:Invalid reservoir name!\n", c_szOwnerName);
		return -2;
	}
	if (c_pszReservoir != NULL && !strcmp((char*)c_pszReservoir, (char*)pszReservoir)) {
		TRACK("%s>PUMP>ERR:Reservoir(%s) has already been created!\n", c_szOwnerName, pszReservoir);
		return -1;
	}
	if (Validity(c_hThread)) {
#if	_DEBL(DEBUG_TAG)
		TRACK("%s>PUMP:cancel exist thread!\n", c_szOwnerName);
#endif
		pthread_cancel(c_hThread);
		c_hThread = INVALID_HANDLE;
	}
	if (Validity(c_hReservoir)) {
#if	_DEBL(DEBUG_TAG)
		TRACK("%s>PUMP:close exist reservoir!\n", c_szOwnerName);
#endif
		mq_close(c_hReservoir);
		c_hReservoir = INVALID_HANDLE;
	}
	if (c_pszReservoir != NULL) {
#if	_DEBL(DEBUG_TAG)
		TRACK("%s>PUMP:unlink exist reservoir!\n", c_szOwnerName);
#endif
		mq_unlink((char*)c_pszReservoir);
		c_pszReservoir = NULL;
	}

	struct mq_attr qattr;
	qattr.mq_flags = 0;
	qattr.mq_maxmsg = MAX_TAG;
	qattr.mq_msgsize = sizeof(_TAG);
	c_hReservoir = mq_open((char*)pszReservoir, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &qattr);
	if (Validity(c_hReservoir)) {
		c_pszReservoir = pszReservoir;
		int res = CreateThread(&c_hThread, &CPump::PumpWatcherEntry, (PVOID)this, PRIORITY_TAG, (PSZ)"PUMP");
		if (Validity(res)) {
#if	_DEBL(DEBUG_TAG)
			TRACK("%s>PUMP:create reservoir.(%s)\n", c_szOwnerName, c_pszReservoir);
#endif
			return c_hReservoir;
		}
		mq_close(c_hReservoir);
		mq_unlink((char*)c_pszReservoir);
		c_hReservoir = INVALID_HANDLE;
		c_pszReservoir = NULL;
		TRACK("%s>PUMP>ERR:can not create reservoir thread!\n", c_szOwnerName);
	}
	else	TRACK("%s>PUMP>ERR:can not create reservoir %s!(%s)\n", c_szOwnerName, pszReservoir, strerror(errno));
	return -3;
}

int CPump::DeleteReservoir()
{
	if (Validity(c_hThread)) {
		if (pthread_cancel(c_hThread) == EOK) {
			PVOID pResult;
			if (pthread_join(c_hThread, &pResult) != EOK) {
#if	_DEBL(DEBUG_TAG)
				TRACK("%s>PUMP>ERR:can not join thread!\n", c_szOwnerName);
#endif
			}
		}
		else {
#if	_DEBL(DEBUG_TAG)
			TRACK("%s>PUMP>ERR:can not cancel thread!\n", c_szOwnerName);
#endif
		}
		c_hThread = INVALID_HANDLE;
	}
	int res = 0;
	if (c_pszReservoir != NULL) {
		if (Validity(c_hReservoir)) {
			if (!Validity(res = mq_close(c_hReservoir)))
				TRACK("%s>PUMP>ERR:can not close reservoir!(%s)\n", c_szOwnerName, strerror(errno));
			else {
#if	_DEBL(DEBUG_TAG)
				TRACK("%s>PUMP:close reservoir.\n", c_szOwnerName);
#endif
			}
			c_hReservoir = INVALID_HANDLE;
		}
		if (!Validity(res = mq_unlink((char*)c_pszReservoir)))
			TRACK("%s>PUMP>ERR:can not unlink reservoir %s!(%d-%s)\n", c_szOwnerName, c_pszReservoir, res, strerror(errno));
		else {
#if	_DEBL(DEBUG_TAG)
			TRACK("%s>PUMP:unlink reservoir.\n", c_szOwnerName);
#endif
		}
		c_pszReservoir = NULL;
	}
	return res;
}

bool CPump::SendTag(int nID, PVOID pVoid, long param)
{
	if (!Validity(c_hReservoir)) {
		TRACK("%s>PUMP>ERR:send flow-id(%d) to invalid reservoir!\n", c_szOwnerName, nID);
		return false;
	}
	_TAG tag;
	tag.nID = nID;
	tag.pClass = pVoid;
	tag.param = param;
	int res = mq_send(c_hReservoir, (char*)&tag.nID, sizeof(_TAG), getprio(getpid()));
	if (Validity(res)) {
#if	_DEBL(DEBUG_TAG)
		TRACK("%s>PUMP:send tag(%d).\n", c_szOwnerName, nID);
#endif
		return true;
	}
	TRACK("%s>PUMP>ERR:send tag!(%d)-%s\n", c_szOwnerName, nID, strerror(errno));
	return false;
}

const char* CPump::GetClassName() const
{
	return CPump::theClassName;
}

const TAGTABLE* CPump::GetTagTable() const
{
	return &CPump::tagTable;
}

const char CPump::theClassName[24] = "";

const TAGTABLE CPump::tagTable =
{
	NULL,
	&CPump::tagEnvelope[0]
};

const TAGENVELOPE CPump::tagEnvelope[] =
{
	{	-1,	NULL	}
};
