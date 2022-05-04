/*
 * CClient.h
 *
 *  Created on: 2010. 10. 6
 *      Author: Che
 */

#ifndef CCLIENT_H_
#define CCLIENT_H_

#include "CSocket.h"

class CClient	: public CSocket {
public:
	CClient();
	CClient(PVOID pParent, int mq);
	virtual ~CClient();
	virtual void	DoReceive();
	virtual void	DoClose();

private:
	PVOID	c_pParent;
};

#endif /* CCLIENT_H_ */
