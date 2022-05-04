/*
 * CSocket.h
 *
 *  Created on: 2010. 09. XX
 *      Author: Che
 */

#ifndef CSOCKET_H_
#define CSOCKET_H_

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <share.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/iomsg.h>
#include <sys/ioctl.h>
#include <sys/procfs.h>
#include <sys/dcmd_chr.h>

#include <CAnt.h>

#define	MA_SOCKETACCEPT	2
#define	MA_SOCKETSEND	4
#define	MA_SOCKETRECV	6
#define	MA_SOCKETCLOSE	8

class CSocket	: public CAnt
{
public:
	CSocket();
	CSocket(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CSocket();

	pid_t	c_pid;
	BOOL	c_bListen;

	BOOL	Create(UINT uPort, int iType = SOCK_STREAM);
	BOOL	Listen(int iBacklog = 5);
	BOOL	Listening();
	BOOL	Accept(CSocket& rSock);
	BOOL	GetPeerName(PSZ pName);
	int		Receive(PVOID pBuf, int nLength, int nFlags = 0);
	int		Send(PVOID pBuf, int nLength, int nFlags = 0);
	void	Close();

	virtual void	DoAccept();
	virtual void	DoClose();
	virtual void	DoSend();
	virtual void	DoReceive();

private:
	PVOID	c_pParent;
	sockaddr_in	c_sockAddr;
	UINT	c_uPort;
public:
	int		c_iSocket;

private:
	static void	SocketHandler(union sigval sv);
	void	Initial();
	BOOL	Bind(UINT uPort);
};

#endif /* CSOCKET_H_ */
