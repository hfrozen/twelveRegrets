/*
 * CPart.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include "CPart.h"

void CPart::Parter(union sigval sv)
{
	CPart* pPart = (CPart*)sv.sival_ptr;

#if	DEBUG <= DEBUG_DRIVER
	MSGLOG("[PART]Start handler.\r\n");
#endif
	pPart->c_bRecv = TRUE;
	pPart->c_bRecved = FALSE;
	int rleng;
	while (pPart->c_bRecv) {
		read(pPart->c_part, pPart->c_pRecv + pPart->c_uRi, 1);
		if (pPart->c_uRi == 0 && *(pPart->c_pRecv) != PARTFLAG_BEGIN)	continue;
		++ pPart->c_uRi;
		ioctl(pPart->c_part, FIONREAD, &rleng);
		read(pPart->c_part, pPart->c_pRecv + pPart->c_uRi, rleng);
		pPart->c_uRi += rleng;
		if (pPart->c_uRi >= pPart->c_uReserveRecv || pPart->c_uRi >= (SIZE_PARTBUF - 10)) {
			pPart->c_uRecvLength = pPart->c_uRi;
			pPart->c_uRi = 0;
			pPart->SendMail(MA_RECEIVEPART, pPart->c_pParent, TRUE);
			pPart->c_uReserveRecv = DEFAULT_DUCRECV;
		}
	}
	pPart->c_bRecved = TRUE;
#if	DEBUG <= DEBUG_DRIVER
	MSGLOG("[PART]Exit handler.\r\n");
#endif
}

CPart::CPart()
{
	c_pParent = NULL;
	c_pszMq = NULL;
	c_part = -1;
	c_bRecv = FALSE;
	c_bRecved = FALSE;
	c_uRi = 0;
	c_uReserveRecv = DEFAULT_DUCRECV;
	c_pRecv = NULL;
}

CPart::CPart(PVOID pParent, int mq, PSZ pszOwner)
	: CAnt(pParent, mq, pszOwner)
{
	c_pParent = pParent;
	c_part = -1;
	c_bRecv = FALSE;
	c_bRecved = FALSE;
	c_uRi = 0;
	c_uReserveRecv = DEFAULT_DUCRECV;
	c_pRecv = NULL;
}

CPart::~CPart()
{
	if (c_part >= 0)	Close();
	if (c_pRecv) {
		delete [] c_pRecv;
		c_pRecv = NULL;
	}
}

const PSZ CPart::c_pszPart = { (PSZ)"/dev/ser2" };

BOOL CPart::Open()
{
	c_part = open(c_pszPart, O_RDWR | O_NOCTTY);	// | O_NONBLOCK);
	if (c_part < 0) {
		MSGLOG("[PART]ERROR:Can not open(%s).\r\n",
				strerror(errno));
		return FALSE;
	}

	struct termios tmoa;
	struct termios tmo;
	tcgetattr(c_part, &tmoa);
	tmo = tmoa;

	cfsetispeed(&tmo, B115200);
	cfsetospeed(&tmo, B115200);
	tmo.c_cflag &= ~IHFLOW;
	tmo.c_cflag &= ~OHFLOW;
	tmo.c_cflag |= CLOCAL;
	tmo.c_cflag |= CREAD;
	tmo.c_cflag |= CS8;
	tmo.c_cflag &= ~PARENB;
	tmo.c_cflag &= ~PARODD;
	tmo.c_cflag |= HUPCL;
	tcsetattr(c_part, TCSANOW, &tmo);
	/*int flags = fcntl(c_part, F_GETFL);
	flags &= ~O_NONBLOCK;
	int res;
	if ((res = fcntl(c_part, F_SETFL, flags)) != 0) {
		MSGLOG("[PART]ERROR:fcntl f_setfl failure(%d-%s).\r\n",
				res, strerror(errno));
		Close();
		return FALSE;
	}*/

	if (c_pRecv) {
		delete [] c_pRecv;
		c_pRecv = NULL;
	}
	c_pRecv = new BYTE[SIZE_PARTBUF];
	c_uRi = 0;

	sigevent ev;
	ev.sigev_notify = SIGEV_THREAD;
	ev.sigev_notify_function = Parter;
	ev.sigev_notify_attributes = NULL;
	ev.sigev_value.sival_ptr = this;
	if (ionotify(c_part, _NOTIFY_ACTION_TRANARM, _NOTIFY_COND_INPUT, &ev) < 0) {
		MSGLOG("[PART]ERROR:ionotify failure(%s).\r\n",
				strerror(errno));
		Close();
		return FALSE;
	}
	return TRUE;
}

void CPart::Close()
{
	if (c_part >= 0) {
		if (c_bRecv && !c_bRecved) {
			while (!c_bRecved)	c_bRecv = FALSE;
		}
		c_bRecved = FALSE;
		close(c_part);
	}
	c_part = -1;
	if (c_pRecv) {
		delete [] c_pRecv;
		c_pRecv = NULL;
	}
}

UCURV CPart::Send(BYTE* pBuf, UCURV uLength)
{
	return write(c_part, pBuf, uLength);
}

UCURV CPart::Send(BYTE* pBuf, UCURV uSendLength, UCURV uRecvLength)
{
	WORD send = write(c_part, pBuf, uSendLength);
	c_uReserveRecv = uRecvLength;
	return send;
}

WORD CPart::Receive(BYTE* p)
{
	memcpy((PVOID)p, (PVOID)c_pRecv, c_uRecvLength);
	return c_uRecvLength;
}
