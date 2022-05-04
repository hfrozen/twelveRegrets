/*
 * CMoc.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include "Track.h"
#include "CMoc.h"

// 190714
#define	MC_VERSION	102		// / 100

CMoc::CMoc()
{
	c_hReservoir = c_hGet = c_hPut = INVALID_HANDLE;
	c_bSelectedByMon = false;
	c_hThread = INVALID_HANDLE;
	c_cDebug = 0;
	c_pLand = NULL;
	c_pServer = NULL;
	c_pClient = NULL;
	SetOwnerName((PSZ)"MC");
}

CMoc::~CMoc()
{
	Shutoff();
}

PVOID CMoc::McWatcherEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CMoc* pMoc = (CMoc*)pVoid;
	if (pMoc != NULL)	pMoc->Watcher();
	return NULL;
}

void CMoc::Watcher()
{
	if (c_cDebug)	TRACK("MC:start moc watcher.(%d)\n", c_hGet);
	while (true) {
		if (!Validity(mq_receive(c_hGet, (char*)&c_msg, sizeof(MCMSG), NULL))) {
			if (c_cDebug)	TRACK("MC>ERR:mq_receive!(%s)\n", strerror(errno));
			continue;
		}
		if (c_cDebug)	TRACK("MC:receive message %d form %d.\n", c_msg.wID, c_hGet);
		if (c_msg.wID == MCMSG_SEND && c_msg.dwLength > 0) {
			Send(c_msg.cBuf, c_msg.dwLength);
			if (c_cDebug)	TRACK("MC:send message %d bytes.\n", c_msg.dwLength);
		}
		else if (c_msg.wID == MCMSG_CONTS) {
			if (OpenPutQueue())	c_pLand = (PVOID)c_msg.dwLength;
		}
		else if (c_cDebug)	TRACK("MC>ERR:unknown command!(%d)\n", c_msg.wID);
	}
	if (c_cDebug)	TRACK("MC:end moc watcher.(%d)\n", c_hGet);
	mq_close(c_hGet);
	c_hGet = INVALID_HANDLE;
	mq_unlink((char*)ENTQ_STR);
}

bool CMoc::ToHex(DWORD* pDw, BYTE* p)
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

bool CMoc::Launch(BYTE cDebug)
{
	c_cDebug = cDebug;
	TRACK("MC:launch version %.2f %s\n", (double)MC_VERSION / 100.f, c_cDebug != 0 ? "debug mode" : "");

	if (!Validity(c_hReservoir = CreateReservoir((PSZ)MCQ_STR))) {
		TRACK("MC>ERR:can not create reservoir!\n");
		return false;
	}
	if (!CreateGetQueue()) {
		Shutoff();
		TRACK("MC>ERR:can not create receive queue!\n");
		return false;
	}
	c_pServer = new CServer(this, c_hReservoir, (PSZ)"SERV");
	if (c_pServer->Create(5000)) {
		c_pServer->Listen();
		TRACK("MC:listen...\n");
	}
	//uint64_t begin = ClockCycles();
	//int res = mkdir("/h/sd/mc", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	//if (res != 0 && errno != EEXIST) {
	//	TRACK("MC>ERR:can not make \"/h/sd/mc\" !\n");
	//	return true;
	//}
	//res = mkdir("/h/sd/mc/wake", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	//if (res != 0 && errno != EEXIST) {
	//	TRACK("MC>ERR:can not make \"/h/sd/mc/wake\" !\n");
	//	return true;
	//}
	//FILE* fp = fopen("/h/sd/mc/wake/mc", "w");
	//if (fp != NULL) {
	//	char buf[8];
	//	for (int n = 0; n < 8; n ++)	buf[n] = (char)n;
	//	size_t ret = fwrite(buf, 1, 8, fp);
	//	fclose(fp);
	//	if (ret != 8)	TRACK("MC>ERR:write err! %d\n", ret);
	//}
	//else	TRACK("MC>ERR:open err! %s\n", strerror(errno));
	//uint64_t end = ClockCycles();
	//double sec = (double)(end - begin) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	//
	//TRACK("MC:wakeup %fs\n", sec);

	return true;
}

void CMoc::Send(BYTE* pBuf, int iLength)
{
	if (c_pClient != NULL)
		c_pClient->Send(pBuf, iLength);
}

void CMoc::Shutoff()
{
	if (Validity(c_hThread)) {
		if (pthread_cancel(c_hThread) == EOK) {
			PVOID pResult;
			pthread_join(c_hThread, &pResult);
		}
		c_hThread = INVALID_HANDLE;
	}
	if (Validity(c_hGet)) {
		mq_close(c_hGet);
		c_hGet = INVALID_HANDLE;
		mq_unlink((char*)ENTQ_STR);
	}
	if (Validity(c_hPut)) {
		mq_close(c_hPut);
		c_hPut = INVALID_HANDLE;
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

bool CMoc::CreateGetQueue()
{
	mq_unlink((char*)ENTQ_STR);
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_MCMSG;
	attr.mq_msgsize = sizeof(MCMSG);
	if (c_cDebug)
		TRACK("MC:queue size=%d length=%d.\n", attr.mq_msgsize, attr.mq_maxmsg);

	c_hGet = mq_open((char*)ENTQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (Validity(c_hGet)) {
		int res;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		if ((res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) == 0) {
			struct sched_param param;
			param.sched_priority = PRIORITY_MSG;
			if ((res = pthread_attr_setschedparam(&attr, &param)) == 0) {
				if ((res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) == 0) {
					if ((res = pthread_create(&c_hThread, &attr, &CMoc::McWatcherEntry, (void*)this)) == 0) {
						if (c_cDebug)	TRACK("MC:create receive queue.(%d)\n", c_hGet);
						return true;
					}
					else	TRACK("MC>ERR:can not create thread for receive queue!(%s)\n", strerror(errno));
				}
				else	TRACK("MC>ERR:can not set sch policy!(%s)\n", strerror(errno));
			}
			else	TRACK("MC>ERR:can not set sch param!(%s)\n", strerror(errno));
		}
		else	TRACK("MC>ERR:can not set inherit!(%s)\n", strerror(errno));
	}
	else	TRACK("MC>ERR:can not create receive flow queue!(%s)\n", strerror(errno));

	return false;
}

bool CMoc::OpenPutQueue()
{
	if (Validity(c_hPut))	mq_close(c_hPut);
	c_hPut = mq_open((char*)MAINQ_STR, O_WRONLY);
	if (Validity(c_hPut)) {
		if (c_cDebug)	TRACK("MC:open send queue.(%d-%s)\n", c_hPut, MAINQ_STR);
		return true;
	}
	TRACK("MC>ERR:can not open send queue!(%s)\n", MAINQ_STR);
	return false;
}

bool CMoc::PutQueue(int iID, long lParam)
{
	if (!Validity(c_hPut))	return false;
	_TAG mput;
	mput.nID = iID;
	mput.pClass = c_pLand;
	mput.param = lParam;
	if (!Validity(mq_send(c_hPut, (char*)&mput.nID, sizeof(_TAG), getprio(getpid())))) {
		TRACK("MC>ERR:flow putting failure to %s!\n", MAINQ_STR);
		return false;
	}
	TRACK("MC:tag(%d) put to %s\n", iID, MAINQ_STR);
	return true;
}

void CMoc::TakeSockAccept(DWORD dw)
{
	if (c_pClient != NULL) {
		c_pClient->Close();
		KILL(c_pClient);
	}
	c_pClient = (CClient*)dw;
	char buf[128];
	c_pClient->GetPeerName((PSZ)buf);
	TRACK("MC:accept %s.\n", buf);
	//PutQueue(TAG_ACCEPT);
}

void CMoc::TakeSockReceive(DWORD dw)
{
	if (c_pClient != NULL) {
		int iLen = c_pClient->Receive((PVOID)c_recvBuf, SIZE_RECVBUF);
		if (iLen < 0) {
			TRACK("MC>ERR:receive failure!(%s)\n", strerror(errno));
			return;
		}
		c_recvBuf[iLen] = 0;
		if (!strcmp((char*)c_recvBuf, "REPTU")) {
			if (Validity(c_hPut) && PutQueue(TAG_LINKMC)) {
				c_bSelectedByMon = true;
				if (c_cDebug)	TRACK("MC:tu link success.\n");
			}
			else {
				c_bSelectedByMon = false;
				TRACK("MC>ERR:tu link failure!\n");
			}
		}
		else if (!strncmp((char*)c_recvBuf, "FRXBP ", 6)) {
			DWORD dw;
			if (ToHex(&dw, &c_recvBuf[5])) {
				PutQueue(TAG_DEBUGMC, (long)dw);
				//TRACK("MC:FRXBP %d\n", dw);
			}
		}
		else	TRACK("MC>ERR:unknown report request!\n");

		if (c_cDebug) {
			TRACK("MC:receive %d bytes.\n", iLen);
			for (int i = 0; i < iLen; i ++) {
				printf(" 0x%02x", c_recvBuf[i]);
				if (c_recvBuf[i] >= 0x20 && c_recvBuf[i] < 0x7f)	printf("(%c)", c_recvBuf[i]);
				else	printf("(.)");
			}
			printf("\n");
		}
	}
}

void CMoc::TakeSockClose(DWORD dw)
{
	if (c_pClient != NULL) {
		c_pClient->Close();
		KILL(c_pClient);
		if (c_bSelectedByMon) {
			PutQueue(TAG_UNLINKMC);
			c_bSelectedByMon = false;
		}
	}
	TRACK("MC:close.\n");
}

BEGIN_TAGTABLE(CMoc, CPump)
	_TAGMAP(TAG_SOCKACCEPT, CMoc::TakeSockAccept)
	_TAGMAP(TAG_SOCKRECV, CMoc::TakeSockReceive)
	_TAGMAP(TAG_SOCKCLOSE, CMoc::TakeSockClose)
END_TAGTABLE();
