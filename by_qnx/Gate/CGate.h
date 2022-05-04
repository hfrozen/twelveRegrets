/*
 * CGate.h
 *
 *  Created on: 2011. 1. 4
 *      Author: Che
 */

#ifndef CGATE_H_
#define CGATE_H_

#include <unistd.h>

#include <CAnt.h>
#include "../Inform/Internal.h"

#include "CServer.h"

#define	VERSION_GATE	1.01

class CGate	: public CAnt
{
public:
	CGate();
	virtual ~CGate();

private:
	int			c_mail;
	int			c_hMsg;
	int			c_hCon;
	PVOID		c_pVoid;
	SMSG		c_msg;
	pthread_t	c_hThread;
	BYTE		c_nDebug;
	CServer*	c_pServer;
	CClient*	c_pClient;

	static void*	Handler(void* pVoid);

	BOOL	CreateQueue();
	BOOL	SendMail(int mID);

protected:
	void	MaSocketAccept(DWORD dwParam);
	void	MaSocketReceive(DWORD dwParam);
	void	MaSocketClose(DWORD dwParam);
	DECLARE_MAILMAP();

public:
	BOOL	OpenConnect(BOOL side);
	BOOL	Run(BYTE debug);
	void	Send(BYTE* buf, int length);
	void	Shutoff();
};

#endif /* CGATE_H_ */
