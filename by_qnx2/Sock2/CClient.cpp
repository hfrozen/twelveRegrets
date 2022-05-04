/*
 * CClient.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include "CClient.h"

CClient::CClient()
	: CSock()
{
	c_pParent = NULL;
}

CClient::CClient(PVOID pParent, QHND hQue)
	: CSock(pParent, hQue)
{
	//c_pParent = pParent;
}

CClient::CClient(PVOID pParent, QHND hQue, PSZ pszOwnerName)
	: CSock(pParent, hQue, pszOwnerName)
{
	//c_pParent = pParent;
}

CClient::~CClient()
{
}

void CClient::DoReceive()
{
	SendTag(TAG_SOCKRECV, c_pParent, 0);
}

void CClient::DoClose()
{
	SendTag(TAG_SOCKCLOSE, c_pParent, 0);
}
