/**
 * @file	CWm.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#include "Track.h"
#include "CWm.h"

#define	WM_VERSION	102		// / 100

CWm::CWm()
{
	c_hSock = c_hPull = c_hPush = INVALID_HANDLE;
	c_bSelectedByMon = false;
	c_hThread = INVALID_HANDLE;
	c_cDebug = 0;
	c_pHost = NULL;
	c_pServer = NULL;
	c_pClient = NULL;
	SetOwnerName("WM");
}

CWm::~CWm()
{
	Shutoff();
}

PVOID CWm::WatcherEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CWm* pWm = (CWm*)pVoid;
	if (pWm != NULL)	pWm->Watcher();
	pthread_exit(NULL);
	return NULL;
}

void CWm::Watcher()
{
	if (c_cDebug)	TRACK("WM:start moc watcher.(%d)\n", c_hPull);
	while (true) {
		if (!Validity(mq_receive(c_hPull, (char*)&c_msg, sizeof(WMMSG), NULL))) {
			if (c_cDebug)	TRACK("WM>ERR:mq_receive!(%s)\n", strerror(errno));
			continue;
		}
		if (c_cDebug)	TRACK("WM:receive message %d form %d.\n", c_msg.wID, c_hPull);
		if (c_msg.wID == WMMSG_SEND && c_msg.dwLength > 0) {
			Send(c_msg.cBuf, c_msg.dwLength);
			if (c_cDebug)	TRACK("WM:send message %d bytes.\n", c_msg.dwLength);
		}
		else if (c_msg.wID == WMMSG_CONTS) {
			if (OpenPushQueue())	c_pHost = (PVOID)c_msg.dwLength;
		}
		else if (c_cDebug)	TRACK("WM>ERR:unknown command!(%d)\n", c_msg.wID);
	}
	if (c_cDebug)	TRACK("WM:end wm watcher.(%d)\n", c_hPull);
	mq_close(c_hPull);
	c_hPull = INVALID_HANDLE;
	mq_unlink(PULLQ_STR);
}

bool CWm::ToHex(DWORD* pDw, BYTE* p)
{
	while (*p != 0) {
		if (*p >= 'a' && *p <= 'f')	*p &= ~0x20;
		if ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F'))	break;
		++ p;
	}
	if (*p == 0)	return false;

	*pDw = 0;
	while (*p != 0) {
		BYTE c = *p - '0';
		if (c > 9)	c -= 7;
		*pDw <<= 4;
		*pDw |= (DWORD)c;
		++ p;
	}
	return true;
}

bool CWm::Launch(BYTE cDebug)
{
	c_cDebug = cDebug;
	TRACK("WM:launch version %.2f %s\n", (double)WM_VERSION / 100.f, c_cDebug != 0 ? "debug mode" : "");

	if (!Validity(c_hReservoir = CreateReservoir(WMQ_STR))) {
		TRACK("WM>ERR:can not create reservoir!\n");
		return false;
	}
	if (!CreateGetQueue()) {
		Shutoff();
		TRACK("WM>ERR:can not create receive queue!\n");
		return false;
	}
	c_pServer = new CServer(this, c_hReservoir, (PSZ)"SERV");
	if (c_pServer->Create(5000)) {
		BYTE ipn[256];
		memset(ipn, 0, 256);
		c_pServer->GetPeerName((const char*)ipn);
		c_pServer->Listen();
		TRACK("WM:IP %s listen...\n", ipn);
	}
	return true;
}

void CWm::Send(BYTE* pBuf, int iLength)
{
	if (c_pClient != NULL)
		c_pClient->Send(pBuf, iLength);
}

void CWm::Shutoff()
{
	if (Validity(c_hThread)) {
		int res = pthread_cancel(c_hThread);
		PVOID pResult;
		if (res == EOK)	res = pthread_join(c_hThread, &pResult);
		if (res != EOK)	TRACK("WM>ERR:thread shutdown!(%s)\n", strerror(errno));
		c_hThread = INVALID_HANDLE;
	}
	if (Validity(c_hPull)) {
		mq_close(c_hPull);
		c_hPull = INVALID_HANDLE;
		mq_unlink(PULLQ_STR);
	}
	if (Validity(c_hPush)) {
		mq_close(c_hPush);
		c_hPush = INVALID_HANDLE;
	}
	if (c_pClient != NULL) {
		c_pClient->Close();
		KILL(c_pClient);
	}
	if (c_pServer != NULL) {
		c_pServer->Close();
		KILL(c_pServer);
	}
	DeleteReservoir();
}

bool CWm::CreateGetQueue()
{
	mq_unlink(PULLQ_STR);
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_WMMSG;
	attr.mq_msgsize = sizeof(WMMSG);
	if (c_cDebug)
		TRACK("WM:queue size=%d length=%d.\n", attr.mq_msgsize, attr.mq_maxmsg);

	c_hPull = mq_open(PULLQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (Validity(c_hPull)) {
		int res;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		if ((res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) == 0) {
			struct sched_param param;
			param.sched_priority = PRIORITY_TASKCMD;
			if ((res = pthread_attr_setschedparam(&attr, &param)) == 0) {
				if ((res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) == 0) {
					if ((res = pthread_create(&c_hThread, &attr, &CWm::WatcherEntry, (void*)this)) == 0) {
						if (c_cDebug)	TRACK("WM:create receive queue.(%d)\n", c_hPull);
						return true;
					}
					else	TRACK("WM>ERR:can not create thread for receive queue!(%s)\n", strerror(errno));
				}
				else	TRACK("WM>ERR:can not set sch policy!(%s)\n", strerror(errno));
			}
			else	TRACK("WM>ERR:can not set sch param!(%s)\n", strerror(errno));
		}
		else	TRACK("WM>ERR:can not set inherit!(%s)\n", strerror(errno));
	}
	else	TRACK("WM>ERR:can not create receive flow queue!(%s)\n", strerror(errno));

	return false;
}

bool CWm::OpenPushQueue()
{
	if (Validity(c_hPush))	mq_close(c_hPush);
	c_hPush = mq_open((char*)MAINQ_STR, O_WRONLY);
	if (Validity(c_hPush)) {
		if (c_cDebug)	TRACK("WM:open push queue.(%d-%s)\n", c_hPush, MAINQ_STR);
		return true;
	}
	TRACK("WM>ERR:can not open push queue!(%s)\n", MAINQ_STR);
	return false;
}

bool CWm::PushQueue(int iID, long lParam)
{
	if (!Validity(c_hPush))	return false;
	_TAG push;
	push.nID = iID;
	push.pClass = c_pHost;
	push.param = lParam;
	if (!Validity(mq_send(c_hPush, (char*)&push.nID, sizeof(_TAG), getprio(getpid())))) {
		TRACK("WM>ERR:flow putting failure to %s!\n", MAINQ_STR);
		return false;
	}
	TRACK("WM:tag(%d) push to %s\n", iID, MAINQ_STR);
	return true;
}

void CWm::TakeSockAccept(DWORD dw)
{
	if (c_pClient != NULL) {
		c_pClient->Close();
		KILL(c_pClient);
	}
	c_pClient = (CClient*)dw;
	char buf[128];
	c_pClient->GetPeerName((PSZ)buf);
	TRACK("WM:accept %s.\n", buf);
	//PushQueue(TAG_ACCEPT);
}

void CWm::TakeSockReceive(DWORD dw)
{
	if (c_pClient != NULL) {
		int iLen = c_pClient->Receive((PVOID)c_recvBuf, SIZE_RECVBUF);
		if (iLen < 0) {
			TRACK("WM>ERR:receive failure!(%s)\n", strerror(errno));
			return;
		}
		c_recvBuf[iLen] = 0;
		if (!strcmp((char*)c_recvBuf, "REPTU")) {
			if (Validity(c_hPush) && PushQueue(TAG_LINKWM)) {
				c_bSelectedByMon = true;
				if (c_cDebug)	TRACK("WM:tu link success.\n");
			}
			else {
				c_bSelectedByMon = false;
				TRACK("WM>ERR:tu link failure!\n");
			}
		}
		else if (!strncmp((char*)c_recvBuf, "FRXBP ", 6)) {
			DWORD dw;
			if (ToHex(&dw, &c_recvBuf[5])) {
				PushQueue(TAG_DEBUGWM, (long)dw);
				//TRACK("WM:FRXBP %d\n", dw);
			}
		}
		else	TRACK("WM>ERR:unknown report request!\n");

		if (c_cDebug) {
			TRACK("WM:receive %d bytes.\n", iLen);
			for (int i = 0; i < iLen; i ++) {
				printf(" 0x%02x", c_recvBuf[i]);
				if (c_recvBuf[i] >= 0x20 && c_recvBuf[i] < 0x7f)	printf("(%c)", c_recvBuf[i]);
				else	printf("(.)");
			}
			printf("\n");
		}
	}
}

void CWm::TakeSockClose(DWORD dw)
{
	if (c_pClient != NULL) {
		c_pClient->Close();
		KILL(c_pClient);
		if (c_bSelectedByMon) {
			PushQueue(TAG_UNLINKWM);
			c_bSelectedByMon = false;
		}
	}
	TRACK("QM:close.\n");
}

BEGIN_TAGTABLE(CWm, CPump)
	_TAGMAP(TAG_SOCKACCEPT, CWm::TakeSockAccept)
	_TAGMAP(TAG_SOCKRECV, CWm::TakeSockReceive)
	_TAGMAP(TAG_SOCKCLOSE, CWm::TakeSockClose)
END_TAGTABLE();
