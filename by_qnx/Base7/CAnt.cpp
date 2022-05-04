/*
 * CAnt.cpp
 *
 *  Created on: 2010. 09. XX
 *      Author: Che
 */

#include <pthread.h>
#include "CAnt.h"

void* CAnt::MailHandler(void* pVoid)
{
	CAnt* pAnt = (CAnt*)pVoid;
	if (pAnt == NULL)	return(0);

#if	DEBUG <= DEBUG_MAIL
	MSGLOG("[MAIL]%s:Start handler(%d).\r\n", pAnt->c_pszOwner, pAnt->c_mq);
#endif
	while (TRUE) {
		MAIL mail;
		if (IsValid(mq_receive(pAnt->c_mq, (char*)&mail.id, sizeof(MAIL), NULL))) {
#if	DEBUG <= DEBUG_MAIL
			MSGLOG("[MAIL]%s:Receive mail %d from ", pAnt->c_pszOwner, mail.id);
#endif
			if (IsValid(mail.id)) {
				CAnt* pTAnt = (CAnt*)mail.pClass;
#if	DEBUG <= DEBUG_MAIL
				MSGLOG("%X.\r\n", pTAnt);
#endif
				const MAILMAP* pMailMap = pTAnt->GetMailMap();
				if (pMailMap != NULL) {
					const MAILMAP_ARCH* pArch;
					for (pArch = pMailMap->pArch;
						pArch->iMailID >= 0 && pArch->pMailFunc != NULL; pArch ++) {
						if (pArch->iMailID == mail.id) {
							MAILFUNC pf = pArch->pMailFunc;
							(pTAnt->*pf)(mail.param);
							break;
						}
					}
				}
			}
			else {
#if	DEBUG <= DEBUG_MAIL
				MSGLOG("[MAIL]%s:Receive unknown.\r\n", pAnt->c_pszOwner);
#endif
			}
		}
	}
#if	DEBUG <= DEBUG_MAIL
	MSGLOG("[MAIL]%s:Exit Mail handler(%d).\r\n", pAnt->c_pszOwner, pAnt->c_mq);
#endif
	if (IsValid(pAnt->c_mq))	mq_close(pAnt->c_mq);
	if (IsString(pAnt->c_pszMq))	mq_unlink(pAnt->c_pszMq);
	pAnt->c_pszMq = NULL;
	return(0);
}

CAnt::CAnt()
{
	c_pParent = NULL;
	c_pszOwner = UNKNOWNOWNER;
	c_hMq = NULL;
	c_pszMq = NULL;
	c_mq = -1;
#if	DEBUG <= DEBUG_CLASS
	MSGLOG("[CLASS]%s:%s id=%d body=%X\r\n", GetClassName(), __func__, getpid(), this);
#endif
}

CAnt::CAnt(PVOID pParent)
{
	c_pParent = pParent;
	c_pszOwner = UNKNOWNOWNER;
	c_hMq = NULL;
	c_pszMq = NULL;
	c_mq = -1;
#if	DEBUG <= DEBUG_CLASS
	MSGLOG("[CLASS]%s:%s id=%d body=%X\r\n", GetClassName(), __func__, getpid(), this);
#endif
}

CAnt::CAnt(PVOID pParent, int mq)
{
	c_pParent = pParent;
	c_pszOwner = UNKNOWNOWNER;
	c_mq = mq;
	c_pszMq = NULL;
	//OpenMailQueue(pszMq);
#if	DEBUG <= DEBUG_CLASS
	MSGLOG("[CLASS]%s:%s id=%d body=%X\r\n", GetClassName(), __func__, getpid(), this);
#endif
}

CAnt::CAnt(PVOID pParent, int mq, PSZ pszOwner)
{
	c_pParent = pParent;
	c_pszOwner = pszOwner;
	c_hMq = NULL;
	c_mq = mq;
	c_pszMq = NULL;
	//OpenMailQueue(pszMq);
#if	DEBUG <= DEBUG_CLASS
	MSGLOG("[CLASS]%s:%s id=%d body=%X\r\n", c_pszOwner, __func__, getpid(), this);
#endif
}

CAnt::~CAnt()
{
	if (c_hMq != NULL)	pthread_cancel(c_hMq);
	c_pszMq = NULL;
#if	DEBUG <= DEBUG_CLASS
	if (c_pszOwner != NULL)
		MSGLOG("[CLASS~]%s:%s id=%d body=%X\r\n",
				c_pszOwner, __func__, getpid(), this);
	else	MSGLOG("[CLASS~]%s:%s id=%d body=%X\r\n",
					GetClassName(), __func__, getpid(), this);
#endif
}

int CAnt::CreateMailQueue(PSZ pszMq)
{
	if (IsString(pszMq)) {
		if (c_hMq != NULL)	pthread_cancel(c_hMq);
		if (IsValid(c_mq))	mq_close(c_mq);
		if (IsString(c_pszMq))	mq_unlink(c_pszMq);
		c_pszMq = NULL;

		struct mq_attr attr;
		attr.mq_flags = 0;
		attr.mq_maxmsg = MAIL_MAX;
		attr.mq_msgsize = sizeof(MAIL);
		c_mq = mq_open(pszMq, O_RDWR | O_CREAT,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
		if (IsValid(c_mq)) {
			int res;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			if ((res = pthread_attr_setinheritsched(&attr,
								PTHREAD_EXPLICIT_SCHED)) == 0) {
				struct sched_param param;
				param.sched_priority = PRIORITY_MAIL;
				if ((res = pthread_attr_setschedparam(&attr, &param)) == 0) {
					if ((res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) == 0) {
						if ((res = pthread_create(&c_hMq, &attr,
								&CAnt::MailHandler, (void*)this)) == 0) {
							c_pszMq = pszMq;
#if	DEBUG <= DEBUG_MAIL
							MSGLOG("[MAIL]%s:Create mail queue(%s-%d).\r\n",
										c_pszOwner, c_pszMq, c_mq);
#endif
							return c_mq;
						}
						else	MSGLOG("[MAIL]ERROR:Can not create thread for mail(%s).\r\n",
											strerror(errno));
					}
					else	MSGLOG("[MAIL]ERROR:Can not set sch policy(%d-%s).\r\n",
										res, strerror(errno));
				}
				else	MSGLOG("[MAIL]ERROR:Can not set sch param(%d-%s).\r\n",
									res, strerror(errno));
			}
			else	MSGLOG("[MAIL]ERROR:Can not set inherit(%d-%s).\r\n",
								res, strerror(errno));
			mq_close(c_mq);
			mq_unlink(pszMq);
			c_mq = -1;
		}
		else	MSGLOG("[MAIL]ERROR %s:Create mail queue(%s).\r\n",
						c_pszOwner, strerror(errno));
	}
	MSGLOG("[MAIL]ERROR %s:Invalid queue name(%s).\r\n", c_pszOwner, pszMq);

	return -1;
}

int CAnt::DeleteMailQueue(PSZ pszMq)
{
	return mq_unlink(pszMq);
}

BOOL CAnt::VerifyMail()
{
	if (c_mq < 0)	return FALSE;
	return TRUE;
}

BOOL CAnt::SendMail(int mID, PVOID pVoid, long param)
{
	MAIL mail;
	mail.id = mID;
	mail.pClass = pVoid;
	mail.param = param;
	if (IsValid(mq_send(c_mq, (char*)&mail.id, sizeof(MAIL), getprio(getpid())))) {
#if	DEBUG <= DEBUG_MAIL
		MSGLOG("[MAIL]%s:Send mail %d.\r\n", c_pszOwner, mID);
#endif
		return TRUE;
	}
	MSGLOG("[MAIL]ERROR %s:Send mail %d.\r\n", c_pszOwner, mID);
	return FALSE;
}

const char* CAnt::GetClassName() const
{
	return CAnt::_theClassName;
}

const char CAnt::_theClassName[20] = "";

const MAILMAP* CAnt::GetMailMap() const
{
	return &CAnt::_mailMap;
}

const MAILMAP CAnt::_mailMap =
{
	NULL, &CAnt::_mailArches[0]
};

const MAILMAP_ARCH CAnt::_mailArches[] =
{
	{ -1, NULL }
};
