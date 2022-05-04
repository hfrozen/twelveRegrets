/*
 * CAnt.h
 *
 *  Created on: 2010. 09. XX
 *      Author: Che
 */

#ifndef CANT_H_
#define CANT_H_

#include <assert.h>
#include <stdio.h>
#include <sched.h>
#include <process.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <new.h>

#include "Printz.h"
#include "Define.h"

#define	ASSERT		assert

struct MAILMAP_ARCH;
struct MAILMAP {
	const MAILMAP*	pBase;
	const MAILMAP_ARCH* pArch;
};

#define DECLARE_MAILMAP()\
public:\
virtual const char* GetClassName() const;\
private:\
static const MAILMAP_ARCH _mailArches[];\
protected:\
static const char _theClassName[];\
static const MAILMAP _mailMap;\
virtual const MAILMAP* GetMailMap() const;

#define	BEGIN_MAIL(thisClass, baseClass)\
const char* thisClass::GetClassName() const\
{ return thisClass::_theClassName; }\
const MAILMAP* thisClass::GetMailMap() const\
{ return &thisClass::_mailMap; }\
const char thisClass::_theClassName[20] = #thisClass;\
const MAILMAP thisClass::_mailMap =\
{ &baseClass::_mailMap, &thisClass::_mailArches[0] };\
const MAILMAP_ARCH thisClass::_mailArches[] =\
{\

#define END_MAIL()\
{ -1, (MAILFUNC)0 }\
}\

#define	CREATEMQ(pszMq)	CreateMailQueue(pszMq)
#define	DELETEMQ(pszMq)	DeleteMailQueue(pszMq)

#define	UNKNOWNOWNER	(PSZ)"Unknown"

class CAnt
{
public:
	CAnt();
	CAnt(PVOID pParent);
	CAnt(PVOID pParent, int mq);
	CAnt(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CAnt();

	int		CreateMailQueue(PSZ pszMq);
	int		DeleteMailQueue(PSZ pszMq);
	BOOL	SendMail(int mID, PVOID pVoid, long param);
	void	SetOwner(PSZ pszOwner)	{ c_pszOwner = pszOwner; }

protected:
	DECLARE_MAILMAP()

private:
	pthread_t	c_hMq;

public:
	mqd_t	c_mq;
	PSZ		c_pszMq;

	BOOL	VerifyMail();

private:
	PVOID	c_pParent;
	PSZ		c_pszOwner;

	static void*	MailHandler(void* pVoid);
};

typedef void (CAnt::*MAILFUNC)(long param);
struct MAILMAP_ARCH {
	int			iMailID;
	MAILFUNC	pMailFunc;
};

#define _MAIL(id, func)\
	{ id, (MAILFUNC)(void (CAnt::*)())&func },

#endif /* CANT_H_ */
