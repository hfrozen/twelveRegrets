/**
 * @file	CServer.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "CClient.h"

class CServer :	public CSock
{
public:
	CServer();
	CServer(PVOID pParent, QHND hQue);
	CServer(PVOID pParent, QHND hQue, PSZ pszOwnerName);
	virtual ~CServer();
	virtual void	DoAccept();

private:
	QHND		c_hQue;
};
