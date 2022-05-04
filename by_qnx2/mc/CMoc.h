/*
 * CMoc.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Mc.h"
#include "CPump.h"

#include "../Sock2/CServer.h"

class CMoc :	public CPump
{
public:
	CMoc();
	virtual ~CMoc();

private:
	QHND		c_hReservoir;
	QHND		c_hGet;
	QHND		c_hPut;
	bool		c_bSelectedByMon;
	PVOID		c_pLand;
	MCMSG		c_msg;
	TRHND		c_hThread;
	BYTE		c_cDebug;
	CServer*	c_pServer;
	CClient*	c_pClient;

#define	SIZE_RECVBUF	1024
	BYTE		c_recvBuf[SIZE_RECVBUF];
#define	SIZE_SENDBUF	10240
	BYTE		c_sendBuf[SIZE_SENDBUF];

	static PVOID	McWatcherEntry(PVOID pVoid);
	void	Watcher();
	bool	ToHex(DWORD* pDw, BYTE* p);
	bool	CreateGetQueue();
	bool	PutQueue(int iID, long lParam = 0);

public:
	bool	Launch(BYTE cDebug);
	void	Send(BYTE* pBuf, int iLength);
	void	Shutoff();
	//bool	OpenPutQueue(bool bSide);
	bool	OpenPutQueue();

protected:
	void	TakeSockAccept(DWORD dw);
	void	TakeSockReceive(DWORD dw);
	void	TakeSockClose(DWORD dw);
	PUBLISH_TAGTABLE();
};
