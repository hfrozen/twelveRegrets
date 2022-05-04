/*
 * CGate.cpp
 *
 *  Created on: 2011. 1. 4
 *      Author: Che
 */

#include <Printz.h>
#include "CGate.h"

void* CGate::Handler(void* pVoid)
{
	CGate* pGate = (CGate*)pVoid;
	if (pGate == NULL)	return(0);

	if (pGate->c_nDebug)
		MSGLOG("[GATE]:Start server handler(%d).\r\n", pGate->c_hMsg);
	while (TRUE) {
		int res;
		if ((res = mq_receive(pGate->c_hMsg, (char*)&pGate->c_msg, sizeof(SMSG), NULL)) >= 0) {
			if (pGate->c_nDebug)
				MSGLOG("[GATE]:Receive message(%d) from %d.\r\n", pGate->c_msg.wID, pGate->c_hMsg);
			if (pGate->c_msg.wID == MSG_SEND && pGate->c_msg.dwLength > 0) {
				pGate->Send(pGate->c_msg.nBuf, pGate->c_msg.dwLength);
				if (pGate->c_nDebug) {
					MSGLOG("[GATE]:Send message1(%d).\r\n", pGate->c_msg.dwLength);
				}
			}
			else if (pGate->c_msg.wID == MSG_CONTC) {
				if (pGate->OpenConnect(TRUE))
					pGate->c_pVoid = (PVOID)pGate->c_msg.dwLength;
			}
			else if (pGate->c_msg.wID == MSG_CONCC) {
				if (pGate->OpenConnect(FALSE))
					pGate->c_pVoid = (PVOID)pGate->c_msg.dwLength;
			}
		}
		else {
			if (pGate->c_nDebug) {
				MSGLOG("[GATE]ERROR:Receive message(%d-%s).\r\n", res, strerror(errno));
			}
		}
	}
	if (pGate->c_nDebug)
		MSGLOG("[GATE]:End server handler(%d).\r\n", pGate->c_hMsg);
	if (IsValid(pGate->c_hMsg)) {
		mq_close(pGate->c_hMsg);
		pGate->c_hMsg = -1;
	}
	mq_unlink(NAME_SERV);
	return(0);
}

CGate::CGate()
{
	c_mail = -1;
	c_hMsg = -1;
	c_hCon = -1;
	c_hThread = NULL;
	c_nDebug = 0;
	c_pServer = NULL;
	c_pClient = NULL;
}

CGate::~CGate()
{
	Shutoff();
}

BOOL CGate::Run(BYTE debug)
{
	c_nDebug = debug;
	MSGLOG("Gate v%.2f\r\n", VERSION_GATE);
	if ((c_mail = CREATEMQ(NAME_GATE)) < 0) {
		MSGLOG("[GATE]ERROR:Can not create mail queue.\r\n");
		return FALSE;
	}
	if (!CreateQueue()) {
		DELETEMQ(NAME_GATE);
		MSGLOG("[GATE]ERROR:Can not create message queue.\r\n");
		return FALSE;
	}
	c_pServer = new CServer(this, c_mail);
	c_pServer->Create(5000);
	c_pServer->Listen();
	MSGLOG("[GATE]:Listen...\r\n");
	return TRUE;
}

void CGate::Send(BYTE* buf, int length)
{
	if (c_pClient != NULL) {
		c_pClient->Send(buf, length);
		if (c_nDebug) {
			BYTE* p = &buf[0];
			while (length > 0) {
				MSGLOG("%02X ", *p ++);
				-- length;
			}
			MSGLOG("\r\n");
		}
	}
}

void CGate::Shutoff()
{
	if (c_hThread != NULL) {
		pthread_cancel(c_hThread);
		c_hThread = NULL;
	}
	if (IsValid(c_hMsg)) {
		mq_close(c_hMsg);
		c_hMsg = -1;
	}
	if (IsValid(c_hCon)) {
		mq_close(c_hCon);
		c_hCon = -1;
	}
	KILL(c_pServer);
	KILL(c_pClient);
	DELETEMQ(NAME_GATE);
	mq_unlink(NAME_SERV);
}

BOOL CGate::CreateQueue()
{
	mq_unlink(NAME_SERV);
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MSG_MAX;
	attr.mq_msgsize = sizeof(SMSG);
	if (c_nDebug)
		MSGLOG("[GATE] Queue size=%d length=%d.\r\n", attr.mq_msgsize, attr.mq_maxmsg);

	c_hMsg = mq_open(NAME_SERV, O_RDWR | O_CREAT,
						S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (IsValid(c_hMsg)) {
		int res;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		if ((res = pthread_attr_setinheritsched(&attr,
							PTHREAD_EXPLICIT_SCHED)) == 0) {
			struct sched_param param;
			param.sched_priority = PRIORITY_MSG;
			if ((res = pthread_attr_setschedparam(&attr, &param)) == 0) {
				if ((res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) == 0) {
					if ((res = pthread_create(&c_hThread, &attr,
							&CGate::Handler, (void*)this)) == 0) {
						if (c_nDebug)
							MSGLOG("[GATE]:Create message queue(%d).\r\n", c_hMsg);
						return TRUE;
					}
					else	MSGLOG("[GATE]ERROR:Can not create thread for msg(%s).\r\n",
								strerror(errno));
				}
				else	MSGLOG("[GATE]ERROR:Can not set sch policy(%d-%s).\r\n", res, strerror(errno));
			}
			else	MSGLOG("[GATE]ERROR:Can not set sch param(%d-%s).\r\n", res, strerror(errno));
		}
		else	MSGLOG("[GATE]ERROR:Can not set inherit(%d-%s).\r\n", res, strerror(errno));
		mq_close(c_hMsg);
		mq_unlink(NAME_SERV);
		c_hMsg = -1;
	}
	else {
		MSGLOG("[GATE]ERROR:Create mail queue(%s).\r\n",
					strerror(errno));
	}
	return FALSE;
}

BOOL CGate::OpenConnect(BOOL side)
{
	if (IsValid(c_hCon))	mq_close(c_hCon);
	c_hCon = mq_open(side ? NAME_TCMQ : NAME_CCMQ, O_WRONLY);
	if (IsValid(c_hCon)) {
		if (c_nDebug)
			MSGLOG("[GATE]:Open connect(%d).\r\n", c_hCon);
		return TRUE;
	}
	if (c_nDebug)
		MSGLOG("[GATE]ERROR:Can not open connect.\r\n");
	return FALSE;
}

BOOL CGate::SendMail(int mID)
{
	if (c_hCon < 0)	return FALSE;
	MAIL mail;
	mail.id = mID;
	mail.pClass = c_pVoid;
	mail.param = 0;
	if (IsValid(mq_send(c_hCon, (char*)&mail.id, sizeof(MAIL), getprio(getpid())))) {
		if (c_nDebug)
			MSGLOG("[GATE]:Send mail %d.\r\n", mID);
		return TRUE;
	}
	MSGLOG("[GATE]ERROR:Send mail %d.\r\n", mID);
	return FALSE;
}

void CGate::MaSocketAccept(DWORD dwParam)
{
	if (c_pClient != NULL) {
		c_pClient->Close();
		delete c_pClient;
		c_pClient = NULL;
	}
	c_pClient = c_pServer->GetAccept();
	char buf[128];
	c_pClient->GetPeerName((PSZ)buf);
	MSGLOG("[GATE]:Accept %s.\r\n", buf);
	SendMail(MA_ACCEPT);
}

void CGate::MaSocketReceive(DWORD dwParam)
{
}

void CGate::MaSocketClose(DWORD dwParam)
{
	if (c_pClient != NULL) {
		c_pClient->Close();
		delete c_pClient;
		c_pClient = NULL;
	}
	MSGLOG("[GATE]:Close.\r\n");
}


BEGIN_MAIL(CGate, CAnt)
	_MAIL(MA_SOCKETACCEPT, CGate::MaSocketAccept)
	_MAIL(MA_SOCKETRECV, CGate::MaSocketReceive)
	_MAIL(MA_SOCKETCLOSE, CGate::MaSocketClose)
END_MAIL();

