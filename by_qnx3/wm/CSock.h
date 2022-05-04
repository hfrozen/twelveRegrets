/**
 * @file	CSock.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <share.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/iomsg.h>
#include <sys/ioctl.h>
#include <sys/procfs.h>
#include <sys/dcmd_chr.h>

#include "CPump.h"

enum enTAGSOCKET {
	TAG_SOCKACCEPT = 101,
	TAG_SOCKSEND,
	TAG_SOCKRECV,
	TAG_SOCKCLOSE,
	TAG_SOCKEND
};

class CSock :	public CPump
{
public:
	CSock();
	CSock(PVOID pParent, QHND hQue);
	CSock(PVOID pParent, QHND hQue, PSZ pszOwnerName);
	virtual ~CSock();

	pid_t	c_pid;
	bool	c_bListen;
	int		c_iSock;

	bool	Create(unsigned int uPort, int iType = SOCK_STREAM);
	bool	Listen(int iBacklog = 5);
	bool	Listening();
	bool	Accept(CSock& rSock);
	bool	GetPeerName(PSZ pName);
	int		Receive(PVOID pBuf, int iLength, int iFlags = 0);
	int		Send(PVOID pBuf, int iLength, int iFlags = 0);
	void	Close();

	virtual	void	DoAccept()		{}
	virtual void	DoClose()		{}
	virtual void	DoSend()		{}
	virtual void	DoReceive()		{}

private:
	sockaddr_in		c_sockAddr;
	unsigned int	c_uPort;

	static void SockHandler(union sigval sv);
	void	Initial();
	bool	Bind(unsigned int uPort);
};
