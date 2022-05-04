/*
 * CPart.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CPART_H_
#define CPART_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/siginfo.h>
#include <sys/ioctl.h>
#include <sys/iomsg.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>

#include <men/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/z17_drv.h>

#include <time.h>

#include <CAnt.h>

#define	SIZE_PARTBUF		256

class CPart	: public CAnt
{
public:
	CPart();
	CPart(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CPart();

private:
	PVOID	c_pParent;
	int		c_part;
	UCURV	c_uRecvLength;
	UCURV	c_uReserveRecv;
	UCURV	c_uRi;
	BYTE*	c_pRecv;

	static const PSZ	c_pszPart;
	static void Parter(union sigval sv);
	void	Close();

public:
	BOOL	c_bRecv;
	BOOL	c_bRecved;

	BOOL	Open();
	UCURV	Send(BYTE* pBuf, UCURV uLength);
	UCURV	Send(BYTE* pBuf, UCURV uSendLength, UCURV uRecvLength);
	WORD	Receive(BYTE* p);
};

#endif /* CPART_H_ */
