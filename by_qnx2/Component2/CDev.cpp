/*
 * CDev.cpp
 *
 *  Created on: 2016. 4. 21.
 *      Author: SWCho
 */
#include <string.h>

#include "../Inform2/Track.h"
#include "CDev.h"

CDev::CDev()
	: CPort()
{
}

CDev::CDev(PVOID pParent, QHND hQue)
	: CPort(pParent, hQue)
{
}

CDev::CDev(PVOID pParent, QHND hQue, PSZ pszOwnerName)
	: CPort(pParent, hQue, pszOwnerName)
{
}

CDev::~CDev()
{
}

void CDev::GetPrefix(PLOCALPREFIX pPrefix)
{
	memcpy(pPrefix, c_mag.buf.c, sizeof(LOCALPREFIX));
}

void CDev::Bunch(BYTE* pDest, WORD wLength)
{
	if (pDest == NULL || wLength == 0 || wLength >= SIZE_GENSBUF) {
		TRACK("DEV>ERR!!!!!!!!!!:%s() UNREACHABLE LINE !!!!!!!!!!(0x%08X %d ch-%d)\n", __FUNCTION__, (int)pDest, wLength, c_fab.iCh);
		// 170901
		return;
	}
	memcpy(pDest, c_mag.buf.c, wLength);
}

void CDev::DoSend()		// TCMS에서는 하지않는 동작이다.
{
}

void CDev::DoReceive(bool bState)
{
	if (bState) {
		KillTrig();
		SendTag(TAG_RECEIVEDEV, c_pParent, TAGPAR(c_mag.wLength, c_mag.cStatus, c_fab.iCh));
	}
	else	SendTag(TAG_RECEIVEDEV, c_pParent, TAGPAR(0x8000, 0, c_fab.iCh));		// timeout
}
