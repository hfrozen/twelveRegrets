/**
 * @file	CWm.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "CPump.h"
#include "CServer.h"
#include "WmRefer.h"

class CWm :	public CPump
{
public:
	CWm();
	virtual ~CWm();

private:
	QHND		c_hSock;
	QHND		c_hPull;
	QHND		c_hPush;
	bool		c_bSelectedByMon;
	PVOID		c_pHost;
	WMMSG		c_msg;
	TRHND		c_hThread;
	BYTE		c_cDebug;
	CServer*	c_pServer;
	CClient*	c_pClient;

#define	SIZE_RECVBUF	1024
	BYTE		c_recvBuf[SIZE_RECVBUF];
#define	SIZE_SENDBUF	10240
	BYTE		c_sendBuf[SIZE_SENDBUF];

	static PVOID	WatcherEntry(PVOID pVoid);
	void	Watcher();
	bool	ToHex(DWORD* pDw, BYTE* p);
	bool	CreateGetQueue();
	bool	PushQueue(int iID, long lParam = 0);

public:
	bool	Launch(BYTE cDebug);
	void	Send(BYTE* pBuf, int iLength);
	void	Shutoff();
	bool	OpenPushQueue();

protected:
	void	TakeSockAccept(DWORD dw);
	void	TakeSockReceive(DWORD dw);
	void	TakeSockClose(DWORD dw);
	PUBLISH_TAGTABLE();
};
