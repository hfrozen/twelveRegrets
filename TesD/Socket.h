// Socket.h

#ifndef SOCKET_H_
#define SOCKET_H_

#include "ReferTes.h"

#define	EnableSocketInt()\
	do {\
		cli();\
		EIFR |= (1 << INTF0);\
		EIMSK |= (1 << INT0);\
		sei();\
	} while (0)

#define	DisableSocketInt()	EIMSK &= ~(1 << INT0)

#define	MAX_SOCKET		4
#define	SOCKET_MEM		0x8000

#define SOCKET_OPMODE	SOCKET_MEM
#define SOCKET_GWS		(SOCKET_MEM + 0x0001)
#define SOCKET_SNMS		(SOCKET_MEM + 0x0005)
#define SOCKET_SHWS		(SOCKET_MEM + 0x0009)
#define SOCKET_SIP		(SOCKET_MEM + 0x000f)
#define SOCKET_INTR		(SOCKET_MEM + 0x0015)
#define SOCKET_IMSK		(SOCKET_MEM + 0x0016)
#define SOCKET_RTTS		(SOCKET_MEM + 0x0017)
#define SOCKET_RTC		(SOCKET_MEM + 0x0019)
#define SOCKET_RSIZE	(SOCKET_MEM + 0x001a)
#define SOCKET_TSIZE	(SOCKET_MEM + 0x001b)
#define SOCKET_PATTS	(SOCKET_MEM + 0x001c)
#define SOCKET_PTIMER	(SOCKET_MEM + 0x0028)
#define SOCKET_PMAGIC	(SOCKET_MEM + 0x0029)
#define SOCKET_URIPS	(SOCKET_MEM + 0x002a)
#define SOCKET_URPTS	(SOCKET_MEM + 0x002e)
#define SOCKET_EACH		(SOCKET_MEM + 0x0400)
#define SOCKET_EACHSIZE	0x0100
#define SOCK_MODE(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0000)
#define SOCK_CMD(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0001)
#define SOCK_INTR(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0002)
#define SOCK_STATUS(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0003)
#define SOCK_PORT(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0004)
#define SOCK_DHAR(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0006)
#define SOCK_DIP(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x000c)
#define SOCK_DPORT(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0010)
#define SOCK_MSSIZE(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0012)
#define SOCK_IRMODE(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0014)
#define SOCK_TOS(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0015)
#define SOCK_TTL(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0016)
#define SOCK_TESIZE(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0020)
#define SOCK_TXRPTR(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0022)
#define SOCK_TXWPTR(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0024)
#define SOCK_RDSIZE(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0026)
#define SOCK_RXRPTR(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x0028)
#define SOCK_RXWPTR(ch)	(SOCKET_EACH + ch * SOCKET_EACHSIZE + 0x002a)
#define	SOCK_BUFSIZE	0x800
#define	SOCK_BUFMASK	0x7ff
#define	SOCK_TXBUF(ch)	(SOCKET_MEM + 0x4000 + ch * SOCK_BUFSIZE)
#define	SOCK_RXBUF(ch)	(SOCKET_MEM + 0x6000 + ch * SOCK_BUFSIZE)

//	MODE register values
#define SOCKETM_RST				0x80	// reset
#define SOCKETM_PB				0x10	// ping block
#define SOCKETM_PPPOE			0x08	// enable pppoe
#define SOCKETM_LB				0x04	// little or big endian selector in indirect mode
#define SOCKETM_AI				0x02	// auto-increment in indirect mode
#define SOCKETM_IND				0x01	// enable indirect mode

//	INTR register values
#define SOCKETINTR_CONFLICT		0x80	// check ip confict
#define SOCKETINTR_UNREACH		0x40	// get the destination unreachable message in UDP sending
#define SOCKETINTR_PPPCLS		0x20	// get the PPPoE close message
#define SOCKETINTR_SOCK(ch)		(0x01 << ch)	// check socket interrupt

//	SOCK_MODE values
#define SOCKMODE_MULTI			0x80	// support multicating
#define SOCKMODE_NDACK			0x20	// No Delayed Ack(TCP) flag
#define SOCKMODE_PPPOE			0x05	// PPPoE
#define SOCKMODE_MACRAW			0x04	// MAC LAYER RAW SOCK
#define SOCKMODE_IPRAW			0x03	// IP LAYER RAW SOCK
#define SOCKMODE_UDP			0x02	// UDP
#define SOCKMODE_TCP			0x01	// TCP
#define SOCKMODE_CLOSE			0x00	// unused socket

//	Sn_CR values
#define SOCKCMD_RECV			0x40	// update rxbuf pointer, recv data
#define SOCKCMD_SENDKEEP		0x22	// send keep alive message
#define SOCKCMD_SENDMAC			0x21	// send data with MAC address, so without ARP process
#define SOCKCMD_SEND			0x20	// updata txbuf pointer, send data
#define SOCKCMD_CLOSE			0x10	// close socket
#define SOCKCMD_DISCONNECT		0x08	// send closing reqeuset in tcp mode
#define SOCKCMD_CONNECT			0x04	// send connection request in tcp mode(Client mode)
#define SOCKCMD_LISTEN			0x02	// wait connection request in tcp mode(Server mode)
#define SOCKCMD_OPEN			0x01	// initialize or open socket

//	Sn_IR values
#define SOCKINTR_SENDOK			0x10	// complete sending
#define SOCKINTR_TIMEOUT		0x08	// assert timeout
#define SOCKINTR_RECV			0x04	// receiving data
#define SOCKINTR_DISCONNECT		0x02	// closed socket
#define SOCKINTR_CONNECT		0x01	// established connection

//	Sn_SR values
#define SOCKSTATUS_PPPOE		0x5f	// pppoe socket
#define SOCKSTATUS_MACRAW		0x42	// mac raw mode socket
#define SOCKSTATUS_IPRAW		0x32	// ip raw mode socket
#define SOCKSTATUS_UDP			0x22	// udp socket
#define SOCKSTATUS_LASTACK		0x1d	// closing state
#define SOCKSTATUS_CLOSEWAIT	0x1c	// closing state
#define SOCKSTATUS_TIMEWAIT		0x1b	// closing state
#define SOCKSTATUS_CLOSING		0x1a	// closing state
#define SOCKSTATUS_ENDWAIT		0x18	// closing state
#define SOCKSTATUS_ESTABLISHED	0x17	// success to connect
#define SOCKSTATUS_SYNRECV		0x16	// connection state
#define SOCKSTATUS_SYNSENT		0x15	// connection state
#define SOCKSTATUS_LISTEN		0x14	// listen state
#define SOCKSTATUS_INIT			0x13	// init. state
#define SOCKSTATUS_CLOSED		0x00	// closed

//	IP PROTOCOL
#define	SOCKIPPROTOCOL_RAW		255		// Raw IP packet
#define SOCKIPPROTOCOL_ND		77		// UNOFFICIAL net disk protocol
#define SOCKIPPROTOCOL_IDP		22		// XNS idp
#define SOCKIPPROTOCOL_UDP		17		// UDP
#define SOCKIPPROTOCOL_PUP		12		// PUP
#define SOCKIPPROTOCOL_TCP		6		// TCP
#define SOCKIPPROTOCOL_GGP		3		// Gateway^2 (deprecated)
#define SOCKIPPROTOCOL_IGMP		2		// Internet group management protocol
#define SOCKIPPROTOCOL_ICMP		1		// Control message protocol
#define SOCKIPPROTOCOL_IP		0		// Dummy for IP

BYTE ReadSockByte(WORD add);
void WriteSockByte(WORD add, BYTE n);
WORD ReadSockWord(WORD add);
void WriteSockWord(WORD add, WORD w);
void ReadSockBuf(WORD add, BYTE* pBuf, WORD len);
void WriteSockBuf(WORD add, BYTE* pBuf, WORD len);

void SoftResetSock(void);
DWORD GetGatewayAddress(void);
void SetGatewayAddress(DWORD dwGw);
DWORD GetSubnetMaskAddress(void);
void SetSubnetMaskAddress(DWORD dwMsk);
void GetSrcHwAddress(BYTE* add);
void SetSrcHwAddress(BYTE* add);	// set local MAC address
DWORD GetSrcIpAddress(void);
void SetSrcIpAddress(DWORD dwIp);
WORD GetRetryTime(void);
void SetRetryTime(WORD t);
BYTE GetRetryCount(void);
void SetRetryCount(BYTE cnt);
void SetSockIntMask(BYTE mask);
//BYTE GetSocketIntr(void);
//void SetSockSegmentSize(BYTE id, WORD size);
//void SetProtocolIrraw(BYTE id, BYTE proto);
BYTE GetSockIntr(BYTE id);
BYTE GetSockStatus(BYTE id);
WORD GetSockTxFreeSize(BYTE id);
WORD GetSockRecvedSize(BYTE id);
void GetSockDestHwAddress(BYTE id, BYTE* add);
void SetSockDestHwAddress(BYTE id, BYTE* add);
DWORD GetSockDestIpAddress(BYTE id);
void SetSockDestIpAddress(BYTE id, DWORD dwIp);
//WORD GetSockDestPort(BYTE id);
//void SetSockDestPort(BYTE id, WORD wPort);
//void SetSockTTL(BYTE id, BYTE ttl);

void SendSockContinue(BYTE id, BYTE* src, WORD len);
void RecvSockContinue(BYTE id, BYTE* dest, WORD len);
void WriteSock(BYTE id, volatile BYTE* src, volatile BYTE* dest, WORD len);
void ReadSock(BYTE id, volatile BYTE* src, volatile BYTE* dest, WORD len);

// ====== socket operation =====
void OpenSock(BYTE id, BYTE protocol, WORD port, BYTE flag);	// tcp, udp, ipraw
void CloseSock(BYTE id);
BOOL ListenSock(BYTE id);	// tcp passive
BOOL ConnectSock(BYTE id, DWORD dwIp, WORD port);	// tcp active
void DisconnectSock(BYTE id);
WORD SendSock(BYTE id, BYTE* buf, WORD len);	// tcp
WORD RecvSock(BYTE id, BYTE* buf, WORD len);	// tcp
void SockISR(void);

#endif /* SOCKET_H_ */