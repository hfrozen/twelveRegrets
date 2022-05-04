// Socket.c

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

#include "Socket.h"
#include "Tes5.h"

extern BYTE		cDebug;
extern TIMES	time;

extern BYTE cSockIntFlag[MAX_SOCKET];
extern WORD	wSockRxiDeb[MAX_SOCKET];
//extern WORD	wSockIntrCycle;

#define EXBYTE(p)	*((volatile BYTE*)(p))

#define GETEXWORD(x)\
	((WORD)((WORD)(EXBYTE(x)) << 8) | (WORD)((WORD)(EXBYTE(x + 1))))

#define SETEXWORD(x, y)\
	do {\
		EXBYTE(x) = (BYTE)((y >> 8) & 0xff);\
		EXBYTE(x + 1) = (BYTE)(y & 0xff);\
	} while (0)

#define GETEXDWORD(x)\
	((((DWORD)(EXBYTE(x)) << 24) & 0xff000000) | (((DWORD)(EXBYTE(x + 1)) << 16) & 0xff0000) |\
	(((DWORD)(EXBYTE(x + 2)) << 8) & 0xff00) | (((DWORD)(EXBYTE(x + 3))) & 0xff))

#define SETEXDWORD(x, y)\
	do {\
		EXBYTE(x) = (BYTE)((y >> 24) & 0xff);\
		EXBYTE(x + 1) = (BYTE)((y >> 16) & 0xff);\
		EXBYTE(x + 2) = (BYTE)((y >> 8) & 0xff);\
		EXBYTE(x + 3) = (BYTE)(y & 0xff);\
	} while (0)

#define GETEXWDWORD(x, y)\
	do {\
		y[0] = EXBYTE(x);\
		y[1] = EXBYTE(x + 1);\
		y[2] = EXBYTE(x + 2);\
		y[3] = EXBYTE(x + 3);\
		y[4] = EXBYTE(x + 4);\
		y[5] = EXBYTE(x + 5);\
	} while (0)

#define SETEXWDWORD(x, y)\
	do {\
		EXBYTE(x) = y[0];\
		EXBYTE(x + 1) = y[1];\
		EXBYTE(x + 2) = y[2];\
		EXBYTE(x + 3) = y[3];\
		EXBYTE(x + 4) = y[4];\
		EXBYTE(x + 5) = y[5];\
	} while (0)

void WriteSockByte(WORD add, BYTE n)
{
	//DisableSocketInt();
	EXBYTE(add) = n;
	//EnableSocketInt();
}

BYTE ReadSockByte(WORD add)
{
	//DisableSocketInt();
	BYTE n = EXBYTE(add);
	//EnableSocketInt();
	return n;
}

void WriteSockWord(WORD add, WORD w)
{
	//DisableSocketInt();
	EXBYTE(add) = HIBYTE(w);
	EXBYTE(add + 1) = LOBYTE(w);
	//EnableSocketInt();
}

WORD ReadSockWord(WORD add)
{
	//DisableSocketInt();
	WORD w = ((WORD)(EXBYTE(add)) << 8) | (WORD)(EXBYTE(add + 1));
	//EnableSocketInt();
	return w;
}

void WriteSockBuf(WORD add, BYTE* pBuf, WORD leng)
{
	//DisableSocketInt();
	memcpy((BYTE*)add, pBuf, leng);
	//EnableSocketInt();
}

void ReadSockBuf(WORD add, BYTE* pBuf, WORD leng)
{
	//DisableSocketInt();
	memcpy(pBuf, (BYTE*)add, leng);
	//EnableSocketInt();
}

void SoftResetSock(void)
{
	//DisableSocketInt();
	EXBYTE(SOCKET_OPMODE) = SOCKETM_RST;
	EXBYTE(SOCKET_TSIZE) = 0x55;
	EXBYTE(SOCKET_RSIZE) = 0x55;

	for (BYTE n = 0; n < MAX_SOCKET; n ++) {
		cSockIntFlag[n] = 0;
		wSockRxiDeb[n] = 0;
	}
	//EnableSocketInt();
}

DWORD GetGatewayAddress(void)
{
	return GETEXDWORD(SOCKET_GWS);
}

void SetGatewayAddress(DWORD dwGw)
{
	SETEXDWORD(SOCKET_GWS, dwGw);
}

DWORD GetSubnetMaskAddress(void)
{
	return GETEXDWORD(SOCKET_SNMS);
}

void SetSubnetMaskAddress(DWORD dwMsk)
{
	SETEXDWORD(SOCKET_SNMS, dwMsk);
}

void GetSrcHwAddress(BYTE* add)
{
	GETEXWDWORD(SOCKET_SHWS, add);
}

void SetSrcHwAddress(BYTE* add)
{
	SETEXWDWORD(SOCKET_SHWS, add);
}

DWORD GetSrcIpAddress(void)
{
	return GETEXDWORD(SOCKET_SIP);
}

void SetSrcIpAddress(DWORD dwIp)
{
	SETEXDWORD(SOCKET_SIP, dwIp);
}

WORD GetRetryTime(void)
{
	return ReadSockWord(SOCKET_RTTS);
}

void SetRetryTime(WORD t)
{
	WriteSockWord(SOCKET_RTTS, t);
}

BYTE GetRetryCount(void)
{
	return ReadSockByte(SOCKET_RTC);
}

void SetRetryCount(BYTE cnt)
{
	WriteSockByte(SOCKET_RTC, cnt);
}

void SetSockIntMask(BYTE mask)
{
	WriteSockByte(SOCKET_IMSK, mask);
}

//BYTE GetSocketIntr(void)
//{
//	return ReadSockByte(SOCKET_INTR);
//}
//
//void SetSockSegmentSize(BYTE id, WORD size)
//{
//	WriteSockWord(SOCK_MSSIZE(id), size);
//}
//
//void SetProtocolIrraw(BYTE id, BYTE proto)
//{
//	WriteSockByte(SOCK_IRMODE(id), proto);
//}
//
BYTE GetSockIntr(BYTE id)
{
	return ReadSockByte(SOCK_INTR(id));
}

BYTE GetSockStatus(BYTE id)
{
	return ReadSockByte(SOCK_STATUS(id));
}

WORD GetSockTxFreeSize(BYTE id)
{
	WORD w1 = 0, w2 = 0;
	do {
		w1 = ReadSockWord(SOCK_TESIZE(id));
		if (w1)	w2 = ReadSockWord(SOCK_TESIZE(id));
	} while (w1 != w2);
	return w1;
}

WORD GetSockRecvedSize(BYTE id)
{
	WORD w = ReadSockWord(SOCK_RDSIZE(id));
	if (w) {
		if (w == wSockRxiDeb[id])	return w;
		else	wSockRxiDeb[id] = w;
	}
	return 0;
}

void GetSockDestHwAddress(BYTE id, BYTE* add)
{
	GETEXWDWORD(SOCK_DHAR(id), add);
}

void SetSockDestHwAddress(BYTE id, BYTE* add)
{
	SETEXWDWORD(SOCK_DHAR(id), add);
}

DWORD GetSockDestIpAddress(BYTE id)
{
	return GETEXDWORD(SOCK_DIP(id));
}

void SetSockDestIpAddress(BYTE id, DWORD dwIp)
{
	SETEXDWORD(SOCK_DIP(id), dwIp);
}

//WORD GetSockDestPort(BYTE id)
//{
//	return GETEXWORD(SOCK_DPORT(id));
//}
//
//void SetSockDestPort(BYTE id, WORD wPort)
//{
//	SETEXWORD(SOCK_DPORT(id), wPort);
//}
//
//void SetSockTTL(BYTE id, BYTE ttl)
//{
//	WriteSockByte(SOCK_TTL(id), ttl);
//}
//
void SendSockContinue(BYTE id, BYTE* src, WORD len)
{
	WORD ofs = ReadSockWord(SOCK_TXWPTR(id));
	WriteSock(id, src, (BYTE*)ofs, len);
	WriteSockWord(SOCK_TXWPTR(id), ofs + len);
}

void RecvSockContinue(BYTE id, BYTE* dest, WORD len)
{
	WORD ofs = ReadSockWord(SOCK_RXRPTR(id));
	ReadSock(id, (BYTE*)ofs, dest, len);
	WriteSockWord(SOCK_RXRPTR(id), ofs + len);
}

void WriteSock(BYTE id, volatile BYTE* src, volatile BYTE* dest, WORD len)
{
	WORD dest_mask = (WORD)dest & SOCK_BUFMASK;
	BYTE* dest_ptr = (BYTE*)(SOCK_TXBUF(id) + dest_mask);
	if (dest_mask + len > SOCK_BUFSIZE) {
		WORD size = SOCK_BUFSIZE - dest_mask;
		WriteSockBuf((WORD)dest_ptr, (BYTE*)src, size);
		WriteSockBuf(SOCK_TXBUF(id), (BYTE*)(src + size), len - size);
	}
	else	WriteSockBuf((WORD)dest_ptr, (BYTE*)src, len);
}

void ReadSock(BYTE id, volatile BYTE* src, volatile BYTE* dest, WORD len)
{
	WORD src_mask = (WORD)src & SOCK_BUFMASK;
	BYTE* src_ptr = (BYTE*)(SOCK_RXBUF(id) + src_mask);
	if (src_mask + len > SOCK_BUFSIZE) {
		WORD size = SOCK_BUFSIZE - src_mask;
		ReadSockBuf((WORD)src_ptr, (BYTE*)dest, size);
		ReadSockBuf(SOCK_RXBUF(id), (BYTE*)(dest + size), len - size);
	}
	else	ReadSockBuf((WORD)src_ptr, (BYTE*)dest, len);
}

void OpenSock(BYTE id, BYTE protocol, WORD port, BYTE flag)
{
	//CloseSock(id);
	BYTE status;
	do {
		WriteSockByte(SOCK_MODE(id), protocol | flag);
		WriteSockWord(SOCK_PORT(id), port);
		WriteSockByte(SOCK_CMD(id), SOCKCMD_OPEN);
		while (ReadSockByte(SOCK_CMD(id)));

		if ((status = GetSockStatus(id)) != SOCKSTATUS_INIT)
			CloseSock(id);
	} while (status != SOCKSTATUS_INIT);
}

void CloseSock(BYTE id)
{
	WriteSockByte(SOCK_CMD(id), SOCKCMD_CLOSE);
	//while (ReadSockByte(SOCK_CMD(id)));
	cSockIntFlag[id] = 0;
	if (SockM())	TRACE(PSTR("Close socket!!!!!\r\n"));
}

BOOL ListenSock(BYTE id)
{
	if (GetSockStatus(id) == SOCKSTATUS_INIT) {
		WriteSockByte(SOCK_CMD(id), SOCKCMD_LISTEN);
		if (GetSockStatus(id) != SOCKSTATUS_LISTEN) {
			CloseSock(id);
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL ConnectSock(BYTE id, DWORD dwIp, WORD port)
{
	if (dwIp == 0xffffffff || dwIp == 0 || port == 0)	return FALSE;
	SetSockDestIpAddress(id, dwIp);
	WriteSockWord(SOCK_DPORT(id), port);
	WriteSockByte(SOCK_CMD(id), SOCKCMD_CONNECT);
	while (ReadSockByte(SOCK_CMD(id)));
	return TRUE;
}

void DisconnectSock(BYTE id)
{
	WriteSockByte(SOCK_CMD(id), SOCKCMD_DISCONNECT);
	while (ReadSockByte(SOCK_CMD(id)));
}

WORD SendSock(BYTE id, BYTE* buf, WORD len)
{
	WORD wSended;
	if (len > SOCK_BUFSIZE)	wSended = GetSockTxFreeSize(id);
	else	wSended = len;
	WORD size;
	do {
		size = GetSockTxFreeSize(id);
		WORD status = ReadSockByte(SOCK_STATUS(id));
		if (status != SOCKSTATUS_ESTABLISHED && status != SOCKSTATUS_CLOSEWAIT) {
			wSended = 0;
			break;
		}
	} while (size < wSended);
	SendSockContinue(id, buf, wSended);
	WriteSockByte(SOCK_CMD(id), SOCKCMD_SEND);
	while (ReadSockByte(SOCK_CMD(id)));
	while ((cSockIntFlag[id] & SOCKINTR_SENDOK) != SOCKINTR_SENDOK) {
		if (ReadSockByte(SOCK_STATUS(id)) == SOCKSTATUS_CLOSED) {		// ?????
			CloseSock(id);
			return 0;
		}
	}
	cSockIntFlag[id] &= ~SOCKINTR_SENDOK;
	return wSended;
}

WORD RecvSock(BYTE id, BYTE* buf, WORD len)
{
	WORD wRecved = 0;
	if (len > 0) {
		RecvSockContinue(id, buf, len);
		WriteSockByte(SOCK_CMD(id), SOCKCMD_RECV);
		while (ReadSockByte(SOCK_CMD(id)));
		wRecved = len;
	}
	return wRecved;
}

void SockISR(void)
{
	//DisableSocketInt();

	BYTE intr = ReadSockByte(SOCKET_INTR);
	do {
		//if (intr & SOCKETINTR_CONFLICT) {
		//}
		//if (intr & SOCKETINTR_UNREACH) {
		//}
		//WriteSockByte(SOCKET_INTR, 0xf0);
		if ((intr & SOCKETINTR_CONFLICT) || (intr & SOCKETINTR_UNREACH) || (intr & SOCKETINTR_PPPCLS)) {
			WriteSockByte(SOCKET_INTR, 0xe0);
		}
		if (intr & SOCKETINTR_SOCK(0)) {
			cSockIntFlag[0] |= ReadSockByte(SOCK_INTR(0)); // can be come to over two times.
			WriteSockByte(SOCK_INTR(0), cSockIntFlag[0]);
		}
		if (intr & SOCKETINTR_SOCK(1)) {
			cSockIntFlag[1] |= ReadSockByte(SOCK_INTR(1));
			WriteSockByte(SOCK_INTR(1), cSockIntFlag[1]);
		}
		if (intr & SOCKETINTR_SOCK(2)) {
			cSockIntFlag[2] |= ReadSockByte(SOCK_INTR(2));
			WriteSockByte(SOCK_INTR(2), cSockIntFlag[2]);
		}
		if (intr & SOCKETINTR_SOCK(3)) {
			cSockIntFlag[3] |= ReadSockByte(SOCK_INTR(3));
			WriteSockByte(SOCK_INTR(3), cSockIntFlag[3]);
		}
		intr = ReadSockByte(SOCKET_INTR);
	} while (intr != 0);
	//++ wSockIntrCycle;
	//EnableSocketInt();
}
