/*
 * CClient.cpp
 *
 *  Created on: 2010. 10. 6
 *      Author: Che
 */

#include "CClient.h"

CClient::CClient()
{
	c_pParent = NULL;
}

CClient::CClient(PVOID pParent, int mq)
	: CSocket(pParent, mq, (PSZ)"Client")
{
	c_pParent = pParent;
}

CClient::~CClient()
{
}

void CClient::DoReceive()
{
	SendMail(MA_SOCKETRECV, c_pParent, 0);
}

void CClient::DoClose()
{
	SendMail(MA_SOCKETCLOSE, c_pParent, 0);
}
