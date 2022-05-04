/**
 * @file	CClient.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "CSock.h"

class CClient :	public CSock
{
public:
	CClient();
	CClient(PVOID pParent, QHND hQue);
	CClient(PVOID pParent, QHND hQue, PSZ pszOwnerName);
	virtual ~CClient();
	virtual void	DoReceive();
	virtual void	DoClose();
};
