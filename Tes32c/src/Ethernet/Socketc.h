// Socket.h
#pragma once
#include "Ethernet\WizConfc.h"

#define SOCKET					uint8_t 	// SOCKET type define for legacy driver

#define SOCK_OK					1			// Result is OK about socket process.
#define SOCK_BUSY				0			// Socket is busy on processing the operation. Valid only Non-block IO Mode.
#define SOCK_FATAL				-1000		// Result is fatal error about socket process.

#define SOCK_ERROR				0
#define SOCKERR_SOCKNUM			(SOCK_ERROR - 1)	// Invalid socket number
#define SOCKERR_SOCKOPT			(SOCK_ERROR - 2)	// Invalid socket option
#define SOCKERR_SOCKINIT		(SOCK_ERROR - 3)	// Socket is not initialized or SIPR is Zero IP address when Sn_MR_TCP
#define SOCKERR_SOCKCLOSED		(SOCK_ERROR - 4)	// Socket unexpectedly closed.
#define SOCKERR_SOCKMODE		(SOCK_ERROR - 5)	// Invalid socket mode for socket operation.
#define SOCKERR_SOCKFLAG		(SOCK_ERROR - 6)	// Invalid socket flag
#define SOCKERR_SOCKSTATUS		(SOCK_ERROR - 7)	// Invalid socket status for socket operation.
#define SOCKERR_ARG				(SOCK_ERROR - 10)	// Invalid argument.
#define SOCKERR_PORTZERO		(SOCK_ERROR - 11)	// Port number is zero
#define SOCKERR_IPINVALID		(SOCK_ERROR - 12)	// Invalid IP address
#define SOCKERR_TIMEOUT			(SOCK_ERROR - 13)	// Timeout occurred
#define SOCKERR_DATALEN			(SOCK_ERROR - 14)	// Data length is zero or greater than buffer max size.
#define SOCKERR_BUFFER			(SOCK_ERROR - 15)	// Socket buffer is not enough for data communication.

#define SOCKFATAL_PACKLEN		(SOCK_FATAL - 1)	// Invalid packet length. Fatal Error.

#define SF_ETHER_OWN			(Sn_MR_MFEN)	// Sn_MR_MACRAW, Receive only the packet as broadcast, multicast and own packet
#define SF_IGMP_VER2			(Sn_MR_MC)		// Sn_MR_UDP with \ref SF_MULTI_ENABLE, Select IGMP version 2.
#define SF_TCP_NODELAY			(Sn_MR_ND)		// Sn_MR_TCP, Use to nodelayed ack.
#define SF_MULTI_ENABLE			(Sn_MR_MULTI)	// Sn_MR_UDP, Enable multicast mode.

#define SF_BROAD_BLOCK			(Sn_MR_BCASTB)	// Sn_MR_UDP or @ref Sn_MR_MACRAW, Block broadcast packet. Valid only in W5500
#define SF_MULTI_BLOCK			(Sn_MR_MMB)		// Sn_MR_MACRAW, Block multicast packet. Valid only in W5500
#define SF_IPv6_BLOCK			(Sn_MR_MIP6B)	// Sn_MR_MACRAW, Block IPv6 packet. Valid only in W5500
#define SF_UNI_BLOCK			(Sn_MR_UCASTB)	// Sn_MR_UDP with \ref SF_MULTI_ENABLE. Valid only in W5500

#define SF_IO_NONBLOCK			0x01		// Socket nonblock io mode. It used parameter in \ref socket().

// UDP & MACRAW Packet Infomation
#define PACK_FIRST				0x80		// In Non-TCP packet, It indicates to start receiving a packet. (When W5300, This flag can be applied)
#define PACK_REMAINED			0x01		// In Non-TCP packet, It indicates to remain a packet to be received. (When W5300, This flag can be applied)
#define PACK_COMPLETED			0x00		// In Non-TCP packet, It indicates to complete to receive a packet. (When W5300, This flag can be applied)
#define PACK_FIFOBYTE			0x02		// Valid only W5300, It indicate to have read already the Sn_RX_FIFOR.

int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag);
int8_t sclose(uint8_t sn);
int8_t listen(uint8_t sn);
int8_t connect(uint8_t sn, uint8_t * addr, uint16_t port);
int8_t disconnect(uint8_t sn);
int32_t send(uint8_t sn, uint8_t * buf, uint16_t len);
int32_t recv(uint8_t sn, uint8_t * buf, uint16_t len);
int32_t sendto(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port);
int32_t recvfrom(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port);

#define SOCK_IO_BLOCK			0  // Socket Block IO Mode in @ref setsockopt().
#define SOCK_IO_NONBLOCK		1  // Socket Non-block IO Mode in @ref setsockopt().

typedef enum
{
	SIK_CONNECTED		= (1 << 0),	// connected
	SIK_DISCONNECTED	= (1 << 1),	// disconnected
	SIK_RECEIVED		= (1 << 2),	// data received
	SIK_TIMEOUT			= (1 << 3),	// timeout occurred
	SIK_SENT			= (1 << 4),	// send ok
	SIK_ALL				= 0x1F		// all interrupt
} sockint_kind;

typedef enum {
	CS_SET_IOMODE,		// set socket IO mode with @ref SOCK_IO_BLOCK or @ref SOCK_IO_NONBLOCK
	CS_GET_IOMODE,		// get socket IO mode
	CS_GET_MAXTXBUF,	// get the size of socket buffer allocated in TX memory
	CS_GET_MAXRXBUF,	// get the size of socket buffer allocated in RX memory
	CS_CLR_INTERRUPT,	// clear the interrupt of socket with @ref sockint_kind
	CS_GET_INTERRUPT,	// get the socket interrupt. refer to @ref sockint_kind
	CS_SET_INTMASK,		// set the interrupt mask of socket with @ref sockint_kind, Not supported in W5100
	CS_GET_INTMASK		// get the masked interrupt of socket. refer to @ref sockint_kind, Not supported in W5100
} ctlsock_type;

typedef enum {
	SO_FLAG,			// Valid only in getsockopt(), For set flag of socket refer to <I>flag</I> in @ref socket().
	SO_TTL,				// Set TTL. @ref Sn_TTL  ( @ref setSn_TTL(), @ref getSn_TTL() )
	SO_TOS,				// Set TOS. @ref Sn_TOS  ( @ref setSn_TOS(), @ref getSn_TOS() )
	SO_MSS,				// Set MSS. @ref Sn_MSSR ( @ref setSn_MSSR(), @ref getSn_MSSR() )
	SO_DESTIP,			// Set the destination IP address. @ref Sn_DIPR ( @ref setSn_DIPR(), @ref getSn_DIPR() )
	SO_DESTPORT,		// Set the destination Port number. @ref Sn_DPORT ( @ref setSn_DPORT(), @ref getSn_DPORT() )
	SO_KEEPALIVESEND,	// Valid only in setsockopt. Manually send keep-alive packet in TCP mode, Not supported in W5100
	SO_KEEPALIVEAUTO,	// Set/Get keep-alive auto transmission timer in TCP mode, Not supported in W5100, W5200
	SO_SENDBUF,			// Valid only in getsockopt. Get the free data size of Socekt TX buffer. @ref Sn_TX_FSR, @ref getSn_TX_FSR()
	SO_RECVBUF,			// Valid only in getsockopt. Get the received data size in socket RX buffer. @ref Sn_RX_RSR, @ref getSn_RX_RSR()
	SO_STATUS,			// Valid only in getsockopt. Get the socket status. @ref Sn_SR, @ref getSn_SR()
	SO_REMAINSIZE,		// Valid only in getsockopt. Get the remained packet size in other then TCP mode.
	SO_PACKINFO			// Valid only in getsockopt. Get the packet information as @ref PACK_FIRST, @ref PACK_REMAINED, and @ref PACK_COMPLETED in other then TCP mode.
} sockopt_type;

int8_t ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg);
int8_t setsockopt(uint8_t sn, sockopt_type sotype, void* arg);
int8_t getsockopt(uint8_t sn, sockopt_type sotype, void* arg);
