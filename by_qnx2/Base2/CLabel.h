/**
 * @file	CLabel.h
 * @brief	베이스 클라스
 * @details	클라스 이름을 설정하여 디버그에서 사용한다.
 * @author	Che-follo
 * @data	2015-11-XX
 */
#pragma once

#include "DuoTypes.h"
#include "Slight.h"
#include "Prefix.h"

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
