/*
 * CMate.cpp
 *
 *  Created on: 2012. 6. 13
 *      Author: Che
 */

#include "CMate.h"
#include "CTcx.h"

CMate::CMate()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pDoz = NULL;
	c_wProgress = 0;
}

CMate::~CMate()
{
}

void CMate::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pDoz = pTcx->GetTcDoz();
	c_pSend = pTcx->GetSendPert();
	c_pTake = pTcx->GetTakePert();
}

BOOL CMate::Research()
{
	if (c_pTcRef->real.es.nDownloadStep & DOWNLOADSTEP_READYEXITBUS) {
		CTcx* pTcx = (CTcx*)c_pParent;
		pTcx->InterTrigger(&c_pTcRef->real.es.wSyncTimer, TIME_BUSBEMPTYMONIT);
	}
	switch (c_pTake->nHcrInfo) {
	case REQ_PERTDIR :
		DirectoryToPert();
		return FALSE;
		break;
	case ACK_PERTDIR :
		DirectoryFromPert();
		return TRUE;
		break;
	case REQ_PERTFILE :
		FileToPert();
		return FALSE;
		break;
	case ACK_PERTFILE :
		FileFromPert();
		return TRUE;
		break;
	case ABORT_PERT :
		MSGLOG("[MATE]Receive download abort message.\r\n");
		if (c_pTcRef->real.es.nMasterDuty < LENGTH_TC &&
				c_pTcRef->real.cid.nTcAddr[c_pTcRef->real.es.nMasterDuty] == c_pTcRef->real.nAddr)
			ActiveClose();
		else	PassiveClose();
		break;
	default :
		MSGLOG("[MATE]ERROR:unknown command(%d).\r\n", c_pTake->nHcrInfo);
		break;
	}
	return FALSE;
}

// Active
void CMate::RequestDir()
{
	CTcx* pTcx = (CTcx*)c_pParent;

	memset((PVOID)c_pSend, 0, sizeof(TCPERT));
	c_wProgress = 0;
	WORD sel = (WORD)(c_pTcRef->real.du.nSel - 1);
	c_pSend->cont.dir.dwFile[0] = c_pTcRef->real.es.dwSelectedDay = pTcx->GetSelectedByDay(sel);
	c_pSend->nHcrInfo = REQ_PERTDIR;
	pTcx->SendPert(PERTMSG_NEW);	// to slave
}

// Passive
void CMate::DirectoryToPert()
{
	CTcx* pTcx = (CTcx*)c_pParent;

	c_pTcRef->real.es.dwSelectedDay = c_pTake->cont.dir.dwFile[0];
	if (c_pTcRef->real.es.dwSelectedDay == 0) {
		MSGLOG("[MATE]ERROR:not selected any day.\r\n");
		ContinueDirectoryToPert(FALSE);
	}
	else	pTcx->SectQuantity(c_pTcRef->real.es.dwSelectedDay);
}

void CMate::ContinueDirectoryToPert(BOOL bValid)
{
	CTcx* pTcx = (CTcx*)c_pParent;

	memset((PVOID)c_pSend, 0, sizeof(TCPERT));
	if (bValid) {
		pTcx->SortUnderDay(c_pTcRef->real.es.dwSelectedDay);
		c_pSend->cont.dir.wLength = pTcx->GetFilesUnderDayLength();
		c_pSend->cont.dir.dwTotalSize = c_pTcRef->real.es.dwTotalSize;
		MSGLOG("[MATE]%ld has %d directory(%ld).\r\n",
				c_pTcRef->real.es.dwSelectedDay, c_pSend->cont.dir.wLength, c_pTcRef->real.es.dwTotalSize);
		if (c_pSend->cont.dir.wLength >= SIZE_FILESUNDERDAY)
			c_pSend->cont.dir.wLength = SIZE_FILESUNDERDAY;
		for (WORD w = 0; w < c_pSend->cont.dir.wLength; w ++)
			c_pSend->cont.dir.dwFile[w] = pTcx->GetSelectedUnderDay(w);
	}
	else	c_pSend->cont.dir.wLength = 0xffff;
	c_pSend->nHcrInfo = ACK_PERTDIR;
	pTcx->SendPert(PERTMSG_ACK);	// to master
}

// Active
void CMate::DirectoryFromPert()
{
	CTcx* pTcx = (CTcx*)c_pParent;

	WORD wLength = c_pTake->cont.dir.wLength;
	if (wLength > 0 && wLength < 0xffff) {
		if (wLength >= SIZE_FILESUNDERDAY)	wLength = SIZE_FILESUNDERDAY;
		pTcx->SetFilesUnderDay(wLength, &c_pTake->cont.dir.dwFile[0]);
		c_pTcRef->real.es.dwTotalSize = c_pTake->cont.dir.dwTotalSize;
		if (c_pTcRef->real.es.dwTotalSize > 0) {
			c_pTcRef->real.es.dwRecvSize = 0;
			// start file transfer
			MSGLOG("[MATE]download length is %d(%ld).\r\n", wLength, c_pTcRef->real.es.dwTotalSize);
			memset((PVOID)c_pSend, 0, sizeof(TCPERT));
			c_pSend->cont.text.nDirectoryID = 0;
			c_pSend->cont.text.nFileID = 0;
			c_pSend->cont.text.dwOffset = 0;
			c_pSend->nHcrInfo = REQ_PERTFILE;
			c_pTcRef->real.es.dwSelectedTime = pTcx->GetSelectedUnderDay((WORD)c_pSend->cont.text.nDirectoryID);
			pTcx->SendPert(PERTMSG_NEW);	// to slave
		}
		else {
			MSGLOG("[MATE]ERROR:receive size.\r\n");
			ActiveClose();
		}
	}
	else {
		MSGLOG("[MATE]Received directory emtpy, close.\r\n");
		ActiveClose();
		if (wLength == 0) {
			pTcx->Hangup(FILEMSG_BEGIN + FILEERROR_NOTFOUNDANYFILEATBUS);
			MSGLOG("[MATE]ERROR:empty directory by bus.\r\n");
			c_pTcRef->real.nState &= ~(STATE_PROGRESSING | STATE_DOWNLOADING);
		}
		else {
			pTcx->Hangup(FILEMSG_BEGIN + FILEERROR_CANNOTREADANYFILEATBUS);
			MSGLOG("[MATE]ERROR:can not read directory by bus.\r\n");
			c_pTcRef->real.nState &= ~(STATE_PROGRESSING | STATE_DOWNLOADING);
		}
	}
}

// Passive
void CMate::FileToPert()
{
	CTcx* pTcx = (CTcx*)c_pParent;

	pTcx->ReadSect(c_pTake->cont.text.nDirectoryID, c_pTake->cont.text.nFileID, c_pTake->cont.text.dwOffset);
}

void CMate::ContinueFileToPert(BOOL bValid)
{
	CTcx* pTcx = (CTcx*)c_pParent;

	c_pSend->cont.text.nDirectoryID = c_pTake->cont.text.nDirectoryID;
	c_pSend->cont.text.nFileID = c_pTake->cont.text.nFileID;
	c_pSend->cont.text.dwOffset = c_pTake->cont.text.dwOffset;
	if (bValid) {
		int size = pTcx->LoadSect((PVOID)&c_pSend->cont.text.nText[0]);
		if (size < 0)	size = 0;
		c_pSend->cont.text.wSize = (WORD)size;
	}
	else {
		c_pSend->cont.text.wSize = 0xffff;
		MSGLOG("[MATE]ERROR:load sect.\r\n");
	}
	c_pSend->nHcrInfo = ACK_PERTFILE;
	pTcx->SendPert(PERTMSG_ACK);	// to master
}

// Active
void CMate::FileFromPert()
{
	CTcx* pTcx = (CTcx*)c_pParent;

	// Modified 2013/03/11
	if (c_pTcRef->real.es.nMasterDuty < LENGTH_TC &&
			(c_pDoz->tcs[c_pTcRef->real.es.nMasterDuty].real.nIssue & ISSUE_KILLDOWNLOAD)) {
		ActiveClose();
		return;
	}

	if (c_pSend->cont.text.nDirectoryID == c_pTake->cont.text.nDirectoryID &&
			c_pSend->cont.text.nFileID == c_pTake->cont.text.nFileID &&
			c_pSend->cont.text.dwOffset == c_pTake->cont.text.dwOffset) {
		if (c_pTake->cont.text.wSize == 0)	ContinueFileFromPert(TRUE);
		else if (c_pTake->cont.text.wSize > SIZE_MATETEXT) {
			//INCBYTE(c_pTcRef->dummy[1]);
			c_pSend->nHcrInfo = REQ_PERTFILE;
			pTcx->SendPert(PERTMSG_AGAIN);	// to slave
			//ContinueFileFromPert(TRUE);
		}
		else {
			pTcx->SaveSect((PVOID)&c_pTake->cont.text.nText[0], c_pTake->cont.text.wSize);
			pTcx->WriteSect(c_pTake->cont.text.nDirectoryID, c_pTake->cont.text.nFileID, c_pTake->cont.text.dwOffset);
		}
	}
	else {
		//INCBYTE(c_pTcRef->dummy[2]);
		c_pSend->nHcrInfo = REQ_PERTFILE;
		pTcx->SendPert(PERTMSG_AGAIN);	// to slave
	}
}

void CMate::ContinueFileFromPert(BOOL bValid)
{
	CTcx* pTcx = (CTcx*)c_pParent;

	c_pTcRef->real.es.dwRecvSize += (DWORD)c_pTake->cont.text.wSize;
	c_wProgress = (WORD)(c_pTcRef->real.es.dwRecvSize * 100 / c_pTcRef->real.es.dwTotalSize);

	if (bValid && c_pTake->cont.text.wSize <= SIZE_MATETEXT) {
		if (c_pTake->cont.text.wSize < SIZE_MATETEXT) {
			c_pSend->cont.text.dwOffset = 0;
			if (++ c_pSend->cont.text.nFileID >= 4) {
				c_pSend->cont.text.nFileID = 0;
				if (++ c_pSend->cont.text.nDirectoryID >= pTcx->GetFilesUnderDayLength()) {
					MSGLOG("[MATE]end of download active close. %d of %d\r\n", c_pSend->cont.text.nDirectoryID, pTcx->GetFilesUnderDayLength());
					c_pSend->cont.text.nDirectoryID = 0;
					ActiveClose();
					pTcx->Hangup(FILEMSG_BEGIN + FILEWORK_END);
					return;
				}
				c_pTcRef->real.es.dwSelectedTime = pTcx->GetSelectedUnderDay((WORD)c_pSend->cont.text.nDirectoryID);
			}
		}
		else	++ c_pSend->cont.text.dwOffset;
	}
	c_pSend->nHcrInfo = REQ_PERTFILE;
	pTcx->SendPert(PERTMSG_NEW);	// to slave
}

void CMate::PassiveClose()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pSend->nHcrInfo = ABORT_PERT;
	pTcx->SendPert(PERTMSG_ACK);	// to master
	c_pTcRef->real.es.nDownloadStep &= ~DOWNLOADSTEP_ONLY;
	c_pTcRef->real.es.nDownloadStep |= DOWNLOADSTEP_READYEXITBUS;
	MSGLOG("[MATE]Passve close.\r\n");
}

void CMate::ActiveClose()
{
	c_pTcRef->real.es.nDownloadStep &= ~DOWNLOADSTEP_ONLY;
	c_pTcRef->real.es.nDownloadStep |= DOWNLOADSTEP_READYEXITBUS;
	MSGLOG("[MATE]Active close.\r\n");
}
