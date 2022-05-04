/*
 * CServer.h
 *
 *  Created on: 2010. 10. 6
 *      Author: Che
 */

#ifndef CSERVER_H_
#define CSERVER_H_

#include "CClient.h"

class CServer	: public CSocket
{
public:
	CServer();
	CServer(PVOID pParent, int mq);
	virtual ~CServer();
	virtual void	DoAccept();

	CClient*	GetAccept();

private:
	PVOID		c_pParent;
	int			c_mq;
	CClient*	c_pClient;
};

#endif /* CSERVER_H_ */
