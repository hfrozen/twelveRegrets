/**
 * @file	CLabel.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once

#include "../Common/DefTypes.h"
#include "../Common/Refer.h"

#define	OWNER_ADD(b, str)\
	char b[SIZE_OWNERBUFF];\
	do {\
		strncpy(b, c_szOwnerName, SIZE_OWNERBUFF);\
		strncat(b, (char*)str, SIZE_OWNERBUFF - strlen(c_szOwnerName));\
	} while (0)

#define	OWNER_ADDC(b, str)\
	do {\
		strncpy(b, c_szOwnerName, SIZE_OWNERBUFF);\
		strncat(b, (char*)str, SIZE_OWNERBUFF - strlen(c_szOwnerName));\
	} while (0)

class CLabel
{
public:
	CLabel();
	CLabel(PVOID pParent);
	CLabel(PSZ pszOwnerName);
	CLabel(PVOID pParent, PSZ pszOwnerName);
	virtual ~CLabel();

	int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pBase, int priority, PSZ szMaker);
	void	SetOwnerName(PSZ pszOwner);
	void	SetParent(PVOID pParent)	{ c_pParent = pParent; }

protected:
	char		c_szOwnerName[SIZE_OWNERBUFF];
	PVOID		c_pParent;
};
