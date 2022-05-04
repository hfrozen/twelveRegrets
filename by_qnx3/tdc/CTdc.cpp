/**
 * @file	CTdc.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <process.h>
#include <dirent.h>

#include "../Base/CString.h"
#include "../Part/CTool.h"
#include "CTdc.h"

#define	_ONLY_REPORT_
#define	_PING_ONLYNOUSB_
#define	TDC_VERSION		1.01
#define	TDC_NAME		"TDC"

#include "../Base/Track.h"

#define	CLRVFD()\
	do {\
		SetText(0, (PSZ)c_cBlankInfo);\
	} while(0)

									 //0123456789abcdef
PSZ CTdc::c_pszBlankInfo =			{ "                " };
PSZ CTdc::c_pszDeviceName =			{ "TDRD RUN        " };
PSZ CTdc::c_pszTopInfo =			{ "r       " };
// 200212
//PSZ CTdc::c_pszBottomInfo =			{ "     c  " };
PSZ CTdc::c_pszBottomInfo =			{ "        " };
PSZ	CTdc::c_pszUploadSuccess =		{ "ul done!" };
PSZ	CTdc::c_pszUploadFail =			{ "ul fail!" };
PSZ	CTdc::c_pszDownloadSuccess =	{ "dl done!" };
PSZ	CTdc::c_pszDownloadFail =		{ "dl fail!" };

const CTdc::LOCALLAYER CTdc::c_asyncForm[MAX_ASYNC] = {
	{	3,	115200,	CAsync::PARITY_NON,	0,	"PORTA"	},
	{	5,	115200,	CAsync::PARITY_NON,	0,	"PORTB"	}
};

CTdc::CTdc()
{
	SetOwnerName(TDC_NAME);
	c_hStq = INVALID_HANDLE;
	c_hStp = INVALID_HANDLE;
	c_pStp = NULL;

	//c_hPtq = INVALID_HANDLE;
	//c_hPtp = INVALID_HANDLE;
	//c_pPtp = NULL;

	c_hUtq = INVALID_HANDLE;
	c_hUtp = INVALID_HANDLE;
	c_pUtp = NULL;

	c_hDtq = INVALID_HANDLE;
	c_hDtp = INVALID_HANDLE;
	c_pDtp = NULL;

	c_bServer = false;
	c_bServerPage = false;

	c_pTuner = NULL;
	for (int n = 0; n < MAX_ASYNC; n ++)	c_pAsync[n] = NULL;

	c_bPulse = false;
	c_wCycle = 0;
	c_wWarmTimer = TIME_WARM;
	c_wPauseTimer = 0;
	c_wDnPrintTimer = 0;
	c_wReportTimer = 0;
	c_wConnectInterval = 1;
	c_wState = 0;
	c_wPage = 0;
	c_iTick[0] = c_iTick[1] = 0;
	c_wRecvInterval = 0;
	for (int n = 0; n < RECVERR_MAX; n ++)	c_wRecvErr[n] = 0;
	c_qwRecvCnt = 0;

	memset(&c_env, 0, sizeof(_ENV));
	memset(&c_hs, 0, sizeof(HSCROLL));
	memset(&c_pulseTime, 0, sizeof(MSRTIME));
	c_pulseTime.wMin = 0xffff;
	memset(&c_stbail, 0, sizeof(STBAIL));
	//memset(&c_ptbail, 0, sizeof(PTBAIL));
	memset(&c_utbail, 0, sizeof(UTBAIL));
	memset(&c_dtbail, 0, sizeof(DTBAIL));
	memset(&c_smMsr, 0, sizeof(SMMSRITEM));
	//memset(&c_piMsr, 0, sizeof(PIMSRITEM));
	memset(&c_upMsr, 0, sizeof(UPMSRITEM));
	memset(&c_dnMsr, 0, sizeof(DNMSRITEM));
}

CTdc::~CTdc()
{
	Destroy();
}

void CTdc::Destroy()
{
	if (Validity(c_hDtq)) {
		mq_close(c_hDtq);
		c_hDtq = INVALID_HANDLE;
	}
	if (Validity(c_hDtp)) {
		close(c_hDtp);
		c_hDtp = INVALID_HANDLE;
		c_pDtp = NULL;
	}

	if (Validity(c_hUtq)) {
		mq_close(c_hUtq);
		c_hUtq = INVALID_HANDLE;
	}
	if (Validity(c_hUtp)) {
		close(c_hUtp);
		c_hUtp = INVALID_HANDLE;
		c_pUtp = NULL;
	}

	//if (Validity(c_hPtq)) {
	//	mq_close(c_hPtq);
	//	c_hPtq = INVALID_HANDLE;
	//}
	//if (Validity(c_hPtp)) {
	//	close(c_hPtp);
	//	c_hPtp = INVALID_HANDLE;
	//	c_pPtp = NULL;
	//}

	if (Validity(c_hStq)) {
		mq_close(c_hStq);
		c_hStq = INVALID_HANDLE;
	}
	if (Validity(c_hStp)) {
		close(c_hStp);
		c_hStp = INVALID_HANDLE;
		c_pStp = NULL;
	}

	for (int n = 0; n < MAX_ASYNC; n ++)	KILLC(c_pAsync[n]);
	if (c_pTuner != NULL)	KILLR(c_pTuner);
}

int CTdc::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int iPriority)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("TDC>ERR:can not set inherit!(%d-%s)\n", res, strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = iPriority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("TDC>ERR:can not set sch param!(%d-%s)\n", res, strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("TDC>ERR:can not set sch policy!(%d-%s)\n", res, strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, (PVOID)this);
	if (!Validity(res)) {
		TRACK("TDC>ERR:can not create thread!(%d-%s)\n", res, strerror(errno));
		return res;
	}
	pthread_attr_destroy(&attr);
	return EOK;
}

void CTdc::SetChar(int x, BYTE ch)
{
	c_mtxtw.Lock();
	c_fio.SetChar(x, ch);
	c_mtxtw.Unlock();
}

void CTdc::SetDots(int x, int iLength)
{
	c_mtxtw.Lock();
	c_fio.SetDots(x, iLength);
	c_mtxtw.Unlock();
}

void CTdc::SetText(int x, PSZ pszText)
{
	c_mtxtw.Lock();
	c_fio.SetText(x, pszText);
	c_mtxtw.Unlock();
}

void CTdc::SetScrollText(int x, int leng, PSZ pszText)
{
	strncpy((char*)c_hs.str, pszText, 256);
	printf("%s\n", c_hs.str);		// ?????
	c_hs.x = x;
	c_hs.size = leng;
	c_hs.cpos = 0;
	memcpy(c_hs.text, c_hs.str, leng);
	c_hs.text[leng] = 0;
	c_hs.wInterval = INTERVAL_HSCROLL * 2;
	SetText(c_hs.x, (PSZ)c_hs.text);
	//printf("%s\n", c_hs.text);
}

void CTdc::PrintV(WORD wPage)
{
	if ((c_wPage & 0xff) == PAGE_RESET && wPage < PAGE_PING)	SetText(DPOS_RECVMSG, c_pszTopInfo);
	// 200212
	//if (((c_wPage >> 8) & 0xff) == PAGE_RESET && wPage >= PAGE_PING) {
	//	ClrScrollText();
	//	SetText(DPOS_UDMSG, c_pszBottomInfo);
	//}

	char buf[256];
	switch (wPage) {
	case PAGE_RECVNOR :
	case PAGE_RECVFERR :
	case PAGE_RECVTERR :
		if (wPage == PAGE_RECVTERR)	sprintf(buf, "te%04d", c_wRecvErr[RECVERR_SINK]);
		else if (wPage == PAGE_RECVFERR)	sprintf(buf, "fe%04d", c_wRecvErr[RECVERR_ALL]);
		else	sprintf(buf, "r%05lld", c_qwRecvCnt);
		SetText(DPOS_RECVMSG, buf);
		c_wPage &= 0xff00;
		c_wPage |= (WORD)wPage;
		break;
	case PAGE_PING :
		//sprintf(buf, "c%01d", c_pPtp->bail.sr.wCurID);	// "c%02d", c_pPtp->bail.sr.wCurID);	// ????? bug
		//sprintf(buf, "scan c%02d", c_pPtp->bail.sr.wCurID);
		//SetText(DPOS_UDMSG, buf);
		//c_wPage &= 0xff;
		//c_wPage |= (WORD)(PAGE_PING << 8);
		break;
	case PAGE_SERVER :
		sprintf(buf, "Serv.%s", c_bServer ? "con" : "non");
		SetText(DPOS_UDMSG, buf);
		c_wPage &= 0xff;
		c_wPage |= (WORD)(PAGE_SERVER << 8);
		break;
	case PAGE_REPORTING :
	case PAGE_REPORTEND :
		SetText(DPOS_REPORTMSG, wPage == PAGE_REPORTING ? "." : " ");
		break;
	case PAGE_UPLOAD :
		sprintf(buf, "up%03d.%1d%%", c_pUtp->bail.wRatioT / 10, c_pUtp->bail.wRatioT % 10);
		SetText(DPOS_UDMSG, buf);
		c_wPage &= 0xff;
		c_wPage |= (WORD)(PAGE_UPLOAD << 8);
		break;
	case PAGE_UPLOADSUCCESS :
		SetText(DPOS_UDMSG, c_pszUploadSuccess);
		c_wPage &= 0xff;
		c_wPage |= (WORD)(PAGE_UPLOADSUCCESS << 8);
		c_wPauseTimer = TIME_PAUSE;
		break;
	case PAGE_UPLOADFAIL :
		SetText(DPOS_UDMSG, c_pszUploadFail);
		c_wPage &= 0xff;
		c_wPage |= (WORD)(PAGE_UPLOADFAIL << 8);
		c_wPauseTimer = TIME_PAUSE;
		break;
	case PAGE_DOWNLOAD :
		sprintf(buf, "dn%03d.%1d%%", c_pDtp->bail.wRatioT / 10, c_pDtp->bail.wRatioT % 10);
		SetText(DPOS_UDMSG, buf);
		c_wPage &= 0xff;
		c_wPage |= (WORD)(PAGE_DOWNLOAD << 8);
		break;
	case PAGE_DOWNLOADSUCCESS :
		SetText(DPOS_UDMSG, c_pszDownloadSuccess);
		c_wPage &= 0xff;
		c_wPage |= (WORD)(PAGE_DOWNLOADSUCCESS << 8);
		c_wPauseTimer = TIME_PAUSE;
		break;
	case PAGE_DOWNLOADFAIL :
		SetText(DPOS_UDMSG, c_pszDownloadFail);
		c_wPage &= 0xff;
		c_wPage |= (WORD)(PAGE_DOWNLOADSUCCESS << 8);
		c_wPauseTimer = TIME_PAUSE;
		break;
	default :
		SetText(0, c_pszDeviceName);
		// 200212
		//sprintf(buf, "IP:%s  ", c_env.szDip[0]);
		//SetScrollText(8, 8, buf);
		c_wPage = (WORD)(PAGE_RESET | (PAGE_RESET << 8));
		break;
	}
	//TRACK("PAGE:0x%04x 0x%02x\n", c_wPage, c_pUtp->bail.wRepState);
}

WORD CTdc::CalcBcc(BYTE* p, int length)
{
	BYTE bcco = 0;
	BYTE bcce = 0;
	for (int n = 0; n < length; n ++) {
		if (n & 1)	bcco ^= *p;
		else	bcce ^= *p;
		++ p;
	}
	return MAKEWORD(bcce, bcco);
}

void CTdc::RecvError(int iErr)
{
	if (iErr >= RECVERR_MAX)	return;

	if (iErr == RECVERR_SINK) {
		INCWORD(c_wRecvErr[RECVERR_SINK]);
		PrintV(PAGE_RECVTERR);
		if (c_wRecvErr[RECVERR_SINK] < 35)	SetDots(DPOS_RECVSINK, (int)c_wRecvErr[RECVERR_SINK]);
	}
	else {
		INCWORD(c_wRecvErr[iErr]);
		INCWORD(c_wRecvErr[RECVERR_ALL]);
		PrintV(PAGE_RECVFERR);
		if (c_wRecvErr[RECVERR_ALL] < 35)	SetDots(DPOS_RECVERR, (int)c_wRecvErr[RECVERR_ALL]);

	}
}

/*bool CTdc::ScanUploadFile()
{
	DIR* pDir = opendir(ARCHIVE_PATH);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			if (pEntry->d_name[0] == ZIPTOD_SIGN && strlen(pEntry->d_name) >= 19) {
				strncpy(c_pUtp->szUpload, pEntry->d_name, 256);
				closedir(pDir);
				return true;
			}
		}
		closedir(pDir);
	}
	return false;
}
*/
void CTdc::Respond(int rci, BYTE cCnt)
{
	if (c_pAsync[rci] != NULL) {
		memset(&c_resp, 0, sizeof(_RESP));
		c_resp.cStx = STX;
		c_resp.cCtrl = 0x30;
		c_resp.cCnt = cCnt;
		c_resp.cVer = TDC_VERSION * 10;
		c_resp.cEtx = ETX;
		WORD bcc = CalcBcc(&c_resp.cCtrl, 13);
		c_resp.cBcce = LOWBYTE(bcc);
		c_resp.cBcco = HIGHBYTE(bcc);
		c_pAsync[rci]->FreeSend((BYTE*)&c_resp, (WORD)sizeof(_RESP));
	}
}

void CTdc::TimeMonitor(PMSRTIMEDW pDestMtb, PMSRTIMEDW pSrcMtb, PSZ pszTitle)
{
	if (pSrcMtb->dwMin < DEFAULT_MINDW && (pSrcMtb->dwMin != pDestMtb->dwMin || pSrcMtb->dwMax != pDestMtb->dwMax)) {
		TRACK("TDC:%s time %ld, %ld\n", pszTitle, pSrcMtb->dwMin, pSrcMtb->dwMax);
		pDestMtb->dwMin = pSrcMtb->dwMin;
		pDestMtb->dwMax = pSrcMtb->dwMax;
	}
}

void CTdc::LoadEnv()
{
	bool bIp = false;
	char buf[1024];
	memset(buf, 0, 1024);
	FILE* fp = fopen(ENVFILE_NAME, "r");
	if (fp != NULL) {
		fread(buf, 1, 256, fp);
		fclose(fp);
		CString str;
		str = buf;
		if (str.GetLength() > 10) {
			int ipi = 0;
			int pos;
			while ((pos = str.Find(DESTIP_TITLE)) >= 0) {
				str = str.Mid(pos + strlen(DESTIP_TITLE));
				CString strIp = str;
				strIp.TrimLeft();
				pos = strIp.FindOneOf("\r\n; ");
				if (pos > 0)	strIp = strIp.Left(pos);
				strncpy(c_env.szDip[ipi], (const char*)strIp, 256);
				TRACK("TDC:dip %s\n", c_env.szDip[ipi]);
				bIp = true;
				if (++ ipi >= MAX_DIP)	break;
			}
			str = buf;
			if ((pos = str.Find(TRAINO_TITLE)) >= 0) {
				str = str.Mid(pos + strlen(TRAINO_TITLE));
				str.TrimLeft();
				c_env.wTrNo = (WORD)atoi((const char*)str);
				TRACK("TDC:train no %d\n", c_env.wTrNo);
			}

			if (c_pStp != NULL)	memcpy(&c_pStp->env, &c_env, sizeof(_ENV));
			//if (c_pPtp != NULL)	memcpy(&c_pPtp->env, &c_env, sizeof(_ENV));
			if (c_pUtp != NULL)	memcpy(&c_pUtp->env, &c_env, sizeof(_ENV));
			if (c_pDtp != NULL)	memcpy(&c_pDtp->env, &c_env, sizeof(_ENV));
		}
	}
	if (!bIp) {
		strncpy(c_env.szDip[0], DESTIP_DEFAULT, 256);
		TRACK("TDC:can not read env. dest ip = %s\n", c_env.szDip[0]);
	}

}

void CTdc::StoreEnv()
{
	if (c_pStp == NULL)	return;

	for (int n = 0; n < MAX_DIP; n ++)
		strncpy(c_pStp->env.szDip[n], c_env.szDip[n], 256);
	//TaskCmd(TASKCMD_WRITEENV);
}

int CTdc::FindFile(PSZ pPath, char cSign1, char cSign2)
{
	if (pPath == NULL || pPath[0] == '\0')	return 0;

	int iLength = 0;
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			if (cSign1 == '*' || pEntry->d_name[0] == cSign1 || (cSign2 != '\0' && pEntry->d_name[0] == cSign2))
				++ iLength;
		}
		closedir(pDir);
	}
	return iLength;
}

void CTdc::TaskCmd(QHND hQue, WORD wCmd)
{
	if (Validity(hQue)) {
		struct mq_attr attr;
		if (Validity(mq_getattr(hQue, &attr))) {
			if (attr.mq_curmsgs < attr.mq_maxmsg) {
				TASKCMD cmd;
				cmd.wCmd = wCmd;
				mq_send(hQue, (char*)&cmd, sizeof(TASKCMD), 0);
			}
			else	TRACK("TDC>ERR:task queue full!(%d)\n", wCmd);
		}
		else	TRACK("TDC>ERR:mq_getattr failed!(%d:%s)\n", wCmd, strerror(errno));
	}
	else	TRACK("TDC>ERR:invalid task queue handle!(%d)\n", wCmd);
}

void CTdc::Pulsate()
{
	// 20ms interval
	c_mtxps.Lock();
	_TWATCHBEGIN();
	if (c_bPulse)	TRACK("TDC>ERR:Pulsate() overlapped!\n");
	c_bPulse = true;

	if (c_hs.wInterval > 0 && c_hs.x >= 0 && c_hs.x < 16 &&
		c_hs.size > 1 && c_hs.size < 16 && c_hs.cpos >= 0 && c_hs.cpos < 256 &&
		strlen((char*)c_hs.str) > 1) {
		if (-- c_hs.wInterval == 0) {
			c_hs.wInterval = INTERVAL_HSCROLL;		// 1sec
			++ c_hs.cpos;
			if (((int)strlen((char*)c_hs.str) - c_hs.cpos) < c_hs.size) {
				c_hs.cpos = 0;
				c_hs.wInterval = INTERVAL_HSCROLL * 2;
			}
			memcpy(c_hs.text, &c_hs.str[c_hs.cpos], c_hs.size);
			c_hs.text[c_hs.size] = 0;
			SetText(c_hs.x, (PSZ)c_hs.text);
		}
	}

	if (c_wWarmTimer == 0 && c_wPauseTimer == 0) {
#if	!defined(_ONLY_REPORT_)
		int iCmd = 0;
#endif
		// usb process
		if (c_pDtp->bail.wCmd & (1 << DNCMD_DOWNLOAD)) {
			// downloading, print downloading rate
			//if (-- c_wDnPrintTimer == 0) {
			//	c_wDnPrintTimer = TIME_DNPRINT;
			//	PrintV(PAGE_DOWNLOAD);
			//}
			if (c_pDtp->bail.wResult != 0) {
				c_pDtp->bail.wCmd = 0;
				if (c_pDtp->bail.wResult == DNRESULT_SUCCESS)	PrintV(PAGE_DOWNLOADSUCCESS);
				else if (c_pDtp->bail.wResult > DNRESULT_NOFILES) {
					PrintV(PAGE_DOWNLOADFAIL);
					c_pDtp->bail.wState &= ~(1 << DNSTATE_SEIZEUSB);
				}
			}
			else if (-- c_wDnPrintTimer == 0) {
				c_wDnPrintTimer = TIME_DNPRINT;
				PrintV(PAGE_DOWNLOAD);
			}
		}
		else {
			if (c_pDtp->bail.wState & (1 << DNSTATE_SEIZEUSB)) {
				// !downloading && usb
				if (FindFile(ARCHIVE_PATH, ZIPTOD_SIGN, UPETOD_SIGN) > 0) {		// usb but not downloading, so search file to download
					if (c_pUtp->bail.wCmd & (1 << UPCMD_UPLOAD)) {
						c_pUtp->bail.wCmd |= (1 << UPCMD_ABORT);				// continue to abort command
						if (c_pUtp->bail.wResult != 0)	c_pUtp->bail.wCmd = 0;	// delete command then stop
					}
					else {
						c_pDtp->bail.wResult = 0;
						c_pDtp->bail.wRatioT = 0;
						c_pDtp->bail.wCmd |= (1 << DNCMD_DOWNLOAD);
						TaskCmd(c_hDtq, TASKCMD_DOWNLOAD);
						PrintV(PAGE_DOWNLOAD);
						c_wDnPrintTimer = TIME_DNPRINT;
					}
				}
				else {
					// usb && no files to download
					TaskCmd(c_hDtq, TASKCMD_SCANUSB);
				}
			}
			else {
				// !usb
				TaskCmd(c_hDtq, TASKCMD_SCANUSB);
			}
		}

		if (!(c_pDtp->bail.wCmd & (1 << DNCMD_DOWNLOAD))) {
#if	defined(_ONLY_REPORT_)
			bool server = (c_pUtp->bail.wRepState & (1 << REPST_CONNECTED)) ? true : false;
			if (server != c_bServer || !c_bServerPage) {
				c_bServer = server;
				PrintV(PAGE_SERVER);
				c_bServerPage = true;
			}
			if (-- c_wConnectInterval == 0) {
				c_wConnectInterval = INTERVAL_CONNECT;
				if (!c_bServer && !(c_pUtp->bail.wRepState & (1 << REPST_CONNECTING)))
					TaskCmd(c_hUtq, TASKCMD_CONNECT);
			}
#else
			// upload process
			if (c_pUtp->bail.wCmd & (1 << UPCMD_UPLOAD)) {
				// uploading, printf uploading rate
				PrintV(PAGE_UPLOAD);
				if (c_pUtp->bail.wResult != 0) {
					c_pUtp->bail.wCmd = 0;
					if (c_pUtp->bail.wResult == UPRESULT_SUCCESS)	PrintV(PAGE_UPLOADSUCCESS);
					else if (c_pUtp->bail.wResult > UPRESULT_ABORT)	PrintV(PAGE_UPLOADFAIL);
				}
				// no ping while uploading
				//if (!(c_pPtp->bail.wPiState & (1 << PISTATE_PINGING)))	TaskCmd(c_hPtq, TASKCMD_NARROWPING);
			}
			else {
				if (c_pPtp->bail.sr.qwShape != 0 && c_pStp->bail.wStatus == 0) {
					// !uploading && net
					if (FindFile(ARCHIVE_PATH, ZIPTOD_SIGN) > 0) {		// net but not uploading, so search file to upload
						GETTOOL(pTool);
						c_pUtp->bail.wCurIP = pTool->GetFirstBit(c_pPtp->bail.sr.qwShape, true);
						c_pUtp->bail.wResult = 0;
						c_pUtp->bail.wCmd |= (1 << UPCMD_UPLOAD);
						TaskCmd(c_hUtq, TASKCMD_UPLOAD);
					}
					else {
						// net && no files to upload
						iCmd = TASKCMD_WIDEPING;	// not upload, wide ping
						//TaskCmd(c_hPtq, TASKCMD_WIDEPING);
					}
				}
				else {
					// !net
					iCmd = TASKCMD_WIDEPING;
					//TaskCmd(c_hPtq, TASKCMD_WIDEPING);
				}
			}
#if	defined(_PING_ONLYNOUSB_)
			// ping only when no usb
			if (iCmd == TASKCMD_WIDEPING) {
				//if (!(c_pPtp->bail.wPiState & (1 << PISTATE_PINGING)))	TaskCmd(c_hPtq, TASKCMD_WIDEPING);
				//PrintV(PAGE_PING);
				if (!(c_pPtp->bail.wPiState & (1 << PISTATE_PINGING))) {
					TaskCmd(c_hPtq, TASKCMD_WIDEPING);
					PrintV(PAGE_PING);
				}
			}
#endif
#endif
		}

#if	!defined(_PING_ONLYNOUSB_)
		// ping even if usb
		if (iCmd == TASKCMD_WIDEPING) {
			if (!(c_pPtp->bail.wPiState & (1 << PISTATE_PINGING)))	TaskCmd(c_hPtq, TASKCMD_WIDEPING);
			PrintV(PAGE_PING);
		}
#endif
#if	defined(_ONLY_REPORT_)
		if (c_wReportTimer != 0 && -- c_wReportTimer == 0)
			PrintV(PAGE_REPORTEND);
#endif
	}

	if (++ c_iTick[0] >= 5) {
		// 100ms
		c_iTick[0] = 0;
		if (c_wWarmTimer > 0)	-- c_wWarmTimer;
		if (c_wPauseTimer > 0)	-- c_wPauseTimer;

		if (c_wState & (1 << STATE_RECVING)) {
			if (++ c_wRecvInterval >= MAX_RECVINTERVAL) {
				c_wRecvInterval = 0;
				c_wState &= ~(1 << STATE_RECVING);
				RecvError(RECVERR_SINK);
				TRACK("TDC>ERR:do not receive tod frame!\n");
			}
		}

		memcpy(&c_stbail, &c_pStp->bail, sizeof(STBAIL));
		//memcpy(&c_ptbail, &c_pPtp->bail, sizeof(PTBAIL));
		memcpy(&c_utbail, &c_pUtp->bail, sizeof(PTBAIL));
		memcpy(&c_dtbail, &c_pDtp->bail, sizeof(DTBAIL));

		//GETTOOL(pTool);
		//if (c_pPtp->bail.sr.qwShape != 0) {
		//	if (!(c_wState & (1 << STATE_CAPSERV))) {
		//		c_wState |= (1 << STATE_CAPSERV);
		//		TRACK("TDC:capture dip.(%s-0x%08x)\n", c_env.szDip[pTool->GetFirstBit(c_pPtp->bail.sr.qwShape, true)], c_pPtp->bail.sr.qwShape);
		//	}
		//}
		//else {
		//	if (c_wState & (1 << STATE_CAPSERV)) {
		//		c_wState &= ~(1 << STATE_CAPSERV);
		//		TRACK("TDC:release dip\n");
		//	}
		//}

		// for monitoring
		TimeMonitor(&c_smMsr.wrt, &c_stbail.mi.wrt, "write");
		//TimeMonitor(&c_piMsr.ping, &c_ptbail.mi.ping, "ping");
		TimeMonitor(&c_upMsr.up, &c_utbail.mi.up, "up");
		TimeMonitor(&c_upMsr.cp, &c_utbail.mi.cp, "con");
		TimeMonitor(&c_upMsr.rp, &c_utbail.mi.rp, "rep");
		TimeMonitor(&c_dnMsr.sc, &c_dtbail.mi.sc, "scan");
		TimeMonitor(&c_dnMsr.dn, &c_dtbail.mi.dn, "down");

		if (++ c_iTick[1] >= 5) {
			// 500ms
			c_iTick[1] = 0;
			c_gate.Send();
			//if (c_wCycle < 35) {
			//	SetDots(DPOS_RECVERR, c_wCycle ++);
			//	SetDots(DPOS_RECVSINK, 35 - c_wCycle);
			//}
		}
	}

	_TWATCHMEASURED(__sec_);
	WORD ms = __sec_ * 100000;
	c_pulseTime.wNor = ms;
	if (c_pulseTime.wMin > ms)	c_pulseTime.wMin = ms;
	if (c_pulseTime.wMax < ms)	c_pulseTime.wMax = ms;
	c_bPulse = false;
	c_mtxps.Unlock();
}

bool CTdc::Launch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	TRACK("tdc begin on %04d/%02d/%02d %02d:%02d:%02d -----------------\n",
			pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	TRACK("tdc ver. %.2f %s %s\n", TDC_VERSION, __DATE__, __TIME__);

	if (!c_fio.Initial()) {
		TRACK("TDC>ERR:fio configuration failure!\n");
		return false;
	}

	SetText(0, "boot scr");
	SetText(8, "s.      ");
	c_hReservoir = CreateReservoir(MAINQ_STR);
	if (!Validity(c_hReservoir)) {
		SetText(8, "MQUE ERR");
		TRACK("TDC>ERR:can not create message queue!\n");
		return false;
	}

	SetText(8, "s..     ");
	c_hStq = mq_open(STQ_STR, O_WRONLY);
	if (!Validity(c_hStq)) {
		SetText(8, "SQUE ERR");
		TRACK("TDC>ERR:can not open sm task queue!(%s)\n", strerror(errno));
		return false;
	}
	c_hStp = shm_open(STPACK_STR, O_RDWR, 0777);
	if (!Validity(c_hStp)) {
		SetText(8, "SMOP ERR");
		TRACK("TDC>ERR:can not open sm shared memory!(%s)\n", strerror(errno));
		return false;
	}
	c_pStp = (PSTPACK)mmap(NULL, sizeof(STPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hStp, 0);
	if (c_pStp == MAP_FAILED) {
		SetText(8, "SMMP ERR");
		TRACK("TDC>ERR:can not access to sm shared memory!(%s)\n", strerror(errno));
		return false;
	}

	SetText(8, "s...    ");
	//c_hPtq = mq_open(PTQ_STR, O_WRONLY);
	//if (!Validity(c_hPtq)) {
	//	SetText(8, "PQUE ERR");
	//	TRACK("TDC>ERR:can not open pm task queue!(%s)\n", strerror(errno));
	//	return false;
	//}
	//c_hPtp = shm_open(PTPACK_STR, O_RDWR, 0777);
	//if (!Validity(c_hPtp)) {
	//	SetText(8, "PIOP ERR");
	//	TRACK("TDC>ERR:can not open pm shared memory!(%s)\n", strerror(errno));
	//	return false;
	//}
	//c_pPtp = (PPTPACK)mmap(NULL, sizeof(PTPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hPtp, 0);
	//if (c_pPtp == MAP_FAILED) {
	//	SetText(8, "PIMP ERR");
	//	TRACK("TDC>ERR:can not access to pm shared memory!(%s)\n", strerror(errno));
	//	return false;
	//}

	c_hUtq = mq_open(UTQ_STR, O_WRONLY);
	if (!Validity(c_hUtq)) {
		SetText(8, "UQUE ERR");
		TRACK("TDC>ERR:can not open up task queue!(%s)\n", strerror(errno));
		return false;
	}
	c_hUtp = shm_open(UTPACK_STR, O_RDWR, 0777);
	if (!Validity(c_hUtp)) {
		SetText(8, "UPOP ERR");
		TRACK("TDC>ERR:can not open up shared memory!(%s)\n", strerror(errno));
		return false;
	}
	c_pUtp = (PUTPACK)mmap(NULL, sizeof(UTPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hUtp, 0);
	if (c_pUtp == MAP_FAILED) {
		SetText(8, "UPMP ERR");
		TRACK("TDC>ERR:can not access to up shared memory!(%s)\n", strerror(errno));
		return false;
	}

	c_hDtq = mq_open(DTQ_STR, O_WRONLY);
	if (!Validity(c_hDtq)) {
		SetText(8, "DQUE ERR");
		TRACK("TDC>ERR:can not open down task queue!(%s)\n", strerror(errno));
		return false;
	}
	c_hDtp = shm_open(DTPACK_STR, O_RDWR, 0777);
	if (!Validity(c_hDtp)) {
		SetText(8, "DPOP ERR");
		TRACK("TDC>ERR:can not open down shared memory!(%s)\n", strerror(errno));
		return false;
	}
	c_pDtp = (PDTPACK)mmap(NULL, sizeof(DTPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hDtp, 0);
	if (c_pDtp == MAP_FAILED) {
		SetText(8, "DPMP ERR");
		TRACK("TDC>ERR:can not access to down shared memory!(%s)\n", strerror(errno));
		return false;
	}

	SetText(8, "s....   ");
	for (int n = 0; n < MAX_ASYNC; n ++) {
		KILLC(c_pAsync[n]);
		c_pAsync[n] = new CAsync(this, c_hReservoir, c_asyncForm[n].pszName);
		if (!c_pAsync[n]->Open(c_asyncForm[n].iCh, c_asyncForm[n].dwBps, c_asyncForm[n].parity, 0)) {
			char buf[16];
			sprintf(buf, "CH.%1d ERR", n);
			SetText(8, buf);
			TRACK("TDC>ERR:%s initial failure!\n", c_asyncForm[n].pszName);
			delete c_pAsync[n];
			c_pAsync[n] = NULL;
			return false;
		}
		else	TRACK("TDC>%s initial.\n", c_asyncForm[n].pszName);
	}

	c_gate.SetParent(this);
	c_gate.SetOwnerName(TDC_NAME);
	if (!c_gate.Open())	TRACK("TDC>WARN:can not open interface to server!\n");

	// display each struct size

	SetText(8, "s.....  ");
	KILLR(c_pTuner);
	c_pTuner = new CTimer(this, "TUNE");
	if (c_pTuner->Initial(PRIORITY_PERIODTUNE, (CTimer::PTIMERFUNC)(&CTdc::Pulsate), TIMERCODE_BASE, true) != 0) {
		SetText(8, "TIMR ERR");
		TRACK("LAND>ERR:timer creation failure!\n");
		return false;
	}
	SetText(8, "s...... ");
	if (!c_pTuner->Trigger(TPERIOD_BASE)) {
		SetText(8, "TIMT ERR");
		TRACK("LAND>ERR:timer triggering failure!\n");
		return false;
	}

	SetText(8, "s.......");
	//LoadEnv();

	PrintV(PAGE_RESET);
	TRACK("TDC:launch OK\n");

	return true;
}

void CTdc::TakeReceiveAsync(DWORD dwParam)
{
	c_mtxr.Lock();
	WORD wLeng = LOWWORD(dwParam);
	WORD w = HIGHWORD(dwParam);
	BYTE state = LOWBYTE(w);
	int iCh = (int)(HIGHBYTE(w));

	int rbi = 0;
	if (iCh == c_asyncForm[0].iCh)	rbi = 0;
	else if (iCh == c_asyncForm[1].iCh)	rbi = 1;
	else {
		TRACK("TDC>ERR:received on unknown channel %d!\n", iCh);
		RecvError(RECVERR_UNKNOWNCH);
		c_mtxr.Unlock();
		return;
	}
	if (state != RECV_NORMAL) {
		TRACK("TDC>ERR:receive timeout on channel %d!\n", iCh);
		RecvError(RECVERR_TIMEOUT);
		c_mtxr.Unlock();
		return;
	}
	if (wLeng != sizeof(_TODF)) {
		TRACK("TDC>ERR:received unknown size %d on channel %d!\n", wLeng, iCh);
		RecvError(RECVERR_UNMATCHLENGTH);
		c_mtxr.Unlock();
		return;
	}

	memset(c_todf[rbi].c, 0, sizeof(_TODF));
	c_pAsync[rbi]->Bunch(c_todf[rbi].c, wLeng);
	if (c_todf[rbi].real.cStx != STX || c_todf[rbi].real.cEtx != ETX || c_todf[rbi].real.todt.real.cCtrl != 0x20) {
		TRACK("TDC>ERR:unmatched control char. on channel %d!\n", iCh);
		RecvError(RECVERR_CTRLCHAR);
		c_mtxr.Unlock();
		return;
	}

	//BYTE bcco = 0;
	//BYTE bcce = 0;
	//for (int n = 0; n < SIZE_TOD + 1; n ++) {
	//	if (n & 1)	bcco ^= c_todf[rbi].c[n + 1];
	//	else	bcce ^= c_todf[rbi].c[n + 1];
	//}
	WORD bcc = CalcBcc(&c_todf[rbi].c[1], SIZE_TOD + 1);	// until etx
	BYTE bcce = LOWBYTE(bcc);
	BYTE bcco = HIGHBYTE(bcc);
	if (c_todf[rbi].real.cBcce != bcce || c_todf[rbi].real.cBcco != bcco) {
		TRACK("TDC>ERR:unmatched bcc on channel %d!\n", iCh);
		RecvError(RECVERR_BCC);
		c_mtxr.Unlock();
		return;
	}

	++ c_qwRecvCnt;
	memcpy(c_pStp->todt.c, c_todf[rbi].real.todt.c, SIZE_TOD);
	// 200212
	memcpy(c_pUtp->todt.c, c_todf[rbi].real.todt.c, SIZE_TOD);
	TaskCmd(c_hStq, TASKCMD_APPENDTOD);
	Respond(rbi, c_todf[rbi].real.todt.real.cCnt);
	if ((c_pUtp->bail.wRepState & (1 << REPST_CONNECTED)) && !(c_pUtp->bail.wRepState & (1 << REPST_SENDING)))
		TaskCmd(c_hUtq, TASKCMD_REPORT);

	if (!(c_wState & (1 << STATE_RECVING)))	c_wState |= (1 << STATE_RECVING);
	c_wRecvInterval = 0;
	PrintV(PAGE_RECVNOR);
	//TRACK("TDC:received on ch.%d (%d)\n", iCh, c_wRecvCnt[RECV_NORMAL]);
}

BEGIN_TAGTABLE(CTdc, CPump)
	_TAGMAP(TAG_RECEIVEASYNC, CTdc::TakeReceiveAsync)
END_TAGTABLE();

int CTdc::Bale(bool bType, BYTE* pMesh)
{
	int leng = 0;
	int i = Contain(bType, pMesh);
	leng += i;
	pMesh += i;

	return leng;
}

ENTRY_CONTAINER(CTdc)
	SCOOP(&c_wState,			sizeof(WORD),			"st")
	SCOOP(&c_wWarmTimer,		sizeof(WORD),			"")
	SCOOP(&c_hs.wInterval,		sizeof(WORD),			"")
	SCOOP(&c_pulseTime,			sizeof(MSRTIME),		"")
	SCOOP(&c_wPage,				sizeof(WORD),			"pg")
	SCOOP(&c_stbail,			sizeof(STBAIL),			"sm")
	//SCOOP(&c_ptbail,			sizeof(PTBAIL),			"pi")
	SCOOP(&c_utbail,			sizeof(UTBAIL),			"up")
	SCOOP(&c_dtbail,			sizeof(DTBAIL),			"dn")
EXIT_CONTAINER()
