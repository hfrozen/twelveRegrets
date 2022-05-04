/*
 * HostCmd.c
 *
 *  Created on: 2019. 3. 19.
 *      Author: SWCho
 */
#include <stdbool.h>
#include "Ethernet/W5500/W5500c.h"
#include "Ethernet/Socketc.h"

#include "Refer32.h"
#include "Spi.h"
#include "Uart.h"
#include "BusCmd.h"
#include "Fpgam.h"
#include "HostCmd.h"

BYTE prevSocketState = 0xff;
BYTE dip[4] = { 0, 0, 0, 0 };
WORD port = 0;

extern WORD	g_wProjNo;
extern IDENTITY	g_id;
extern BYTE	g_wShape;
extern LIMB	g_lm;
extern HOST g_host;
extern UARTHND g_bus;
//extern const FRMLENGTH g_trxl[PRJ_MAX][SDID_MAX];

void Shift(BYTE* p, int sn)
{
	int n = 0;
	for ( ; sn < SIZE_ENVELOPE; n ++, sn ++)	p[n] = p[sn];
	for ( ; n < SIZE_ENVELOPE; n ++)	p[n] = 0;
}

void InitialHost(BYTE iip)
{
	InitialSpi1();
	Reset5500();
	InitialW5500((uint8_t)iip);
	NetConfig();
}

// A1:
// TR, SC:	소켓을 연결하고 수신 프레임이 있으면 g_host.r(index:g_host.r.i)에 저장한다.
void ScanHost(void)
{
	BYTE curSocketState = getSn_SR(SOCK_SERVER);
	if (curSocketState != prevSocketState) {
		prevSocketState = curSocketState;
		if (DEBUGSOCKET())	printf("[SOCK]socket state 0x%02x.\r\n", prevSocketState);
	}
	WORD len;
	DWORD ret;
	switch (curSocketState) {
		case SOCK_ESTABLISHED :
			if (!(g_wShape & (1 << SHAPE_CONNECTCLIENT))) {
				g_wShape |= (1 << SHAPE_CONNECTCLIENT);
				getSn_DIPR(SOCK_SERVER, dip);
				port = getSn_DPORT(SOCK_SERVER);
				if (DEBUGSOCKET())	printf("[SOCK]%d.%d.%d.%d[%d] connected.\r\n", dip[0], dip[1], dip[2], dip[3], port);
			}
			if ((len = getSn_RX_RSR(SOCK_SERVER)) > 0) {
				if (len > (SIZE_ENVELOPE - g_host.r.i))	len = SIZE_ENVELOPE - g_host.r.i;
				len = recv(SOCK_SERVER, &g_host.r.s[g_host.r.i], len);
				g_host.r.i += len;
				if (DEBUGSOCKET())	printf("[SOCK]recv %d bytes\r\n", len);
#if	0
				memcpy(g_host.t.s, g_host.r.s, g_host.r.i);
				g_host.t.i = g_host.r.i;
				g_host.r.i = 0;
				if ((ret = send(SOCK_SERVER, g_host.t.s, g_host.t.i)) == g_host.t.i) {
					send(SOCK_SERVER, (BYTE*)"\r\n", 2);
				}
				else {
					if (DEBUGSOCKET())	printf("[SOCK]send failed!(%ld)\r\n", ret);
				}
#endif
			}
			break;
		case SOCK_CLOSE_WAIT :
			g_wShape &= ~(1 << SHAPE_CONNECTCLIENT);
			if ((ret = disconnect(SOCK_SERVER)) == SOCK_OK) {
				if (DEBUGSOCKET())	printf("[SOCK]%d.%d.%d.%d[%d] close.\r\n", dip[0], dip[1], dip[2], dip[3], port);
				dip[0] = dip[1] = dip[2] = dip[3] = 0;
				port = 0;
			}
			else {
				if (DEBUGSOCKET())	printf("[SOCK]close failed!(%ld)\r\n", ret);
			}
			break;
		case SOCK_INIT :
			if ((ret = listen(SOCK_SERVER)) == SOCK_OK) {
				if (DEBUGSOCKET())	printf("[SOCK]listen...\r\n");
			}
			else {
				if (DEBUGSOCKET())	printf("[SOCK]initial failed!(%ld)\r\n", ret);
			}
			break;
		case SOCK_CLOSED :
			g_wShape &= ~(1 << SHAPE_CONNECTCLIENT);
			if ((ret = socket(SOCK_SERVER, Sn_MR_TCP, SOCK_PORT, Sn_MR_TCP)) == SOCK_SERVER) {
				if (DEBUGSOCKET())	printf("[SOCK]open...\r\n");
			}
			else {
				if (DEBUGSOCKET())	printf("[SOCK]open failed!(%ld)\r\n", ret);
			}
			break;
		default :	break;
	}
}

// A2
// 소켓을 통해 g_host.r에 저장된 프레임을 한 구문씩 가져와서...가져온 후, g_host.r.i를 다음 구문으로 이동한다.
// TR:	g_bus.buf.t.s(index:g_bus.buf.t.i)에 저장한다.
// SC:	구문의 명령을 실행한다. 노드의 SDR을 PC로 보낸다.
void ActionHost(void)
{
	BCHEAD bch = AbstractCmd(g_host.r.s, g_host.r.i, true);
	if (bch.p == NULL)	g_host.r.i = 0;
	else {
		if (g_id.role.a == ROLE_TR) {
			memcpy(&g_bus.buf.t.s[g_bus.buf.t.i], bch.p, bch.leng + 1);
			g_bus.buf.t.i += (bch.leng + 1);
		}
		else {
			if (bch.cid.n.l < 8) {
				switch (bch.cmd.a) {
				case BUSCMD_SCRESET :		// 0x10
					memset(&g_lm.sc, 0, sizeof(SCBUF));
					ResetFpgam();
					break;
				case BUSCMD_SCINITEX :		// 0x17
					if ((bch.p[2] & 0xf) < MAX_PORT) {
						int id = (int)(bch.p[2] & 0xf);
						memset(&g_lm.sc[id], 0, sizeof(SCBUF));
						ResetEach(id);
					}
					break;
				case BUSCMD_SCCONFIGEX :	// 0x18
					if ((bch.p[2] & 0xf) < MAX_PORT && bch.p[3] < PRJ_MAX && bch.p[4] < SDID_MAX) {
						g_wProjNo = (WORD)bch.p[3];
						int id = (int)(bch.p[2] & 0xf);
						memset(&g_lm.sc[id].r, 0, sizeof(SCFRM));
						memset(&g_lm.sc[id].t, 0, sizeof(SCFRM));
						g_lm.sc[id].m.cDevID = bch.p[4];
						g_lm.sc[id].m.addr.w = MAKEWORDA(bch.p[5], bch.p[6]);
						g_lm.sc[id].m.mode.a = bch.p[7];
						g_lm.sc[id].m.conf.a = bch.p[8];
						g_lm.sc[id].m.enr.a = bch.p[9];
						InitialPort(id);
					}
					break;
				case BUSCMD_SCSREPORTEX :	// 0x19
					if ((bch.p[2] & 0xf) < MAX_PORT) {
						int n = (int)(bch.p[2] & 0xf);
						PrintFH(9, 0x18, 0x50 | (BYTE)n, (BYTE)(g_wProjNo & 0xff),
								g_lm.sc[n].m.addr.c[0], g_lm.sc[n].m.addr.c[1],
								g_lm.sc[n].m.cDevID, g_lm.sc[n].m.enr.a,
								g_lm.sc[n].m.mode.a, g_lm.sc[n].m.conf.a);
					}
					break;
				case BUSCMD_SCSEND :		// 0x14
				case BUSCMD_SCCAP :			// 0x15
					if ((bch.p[2] & 0xf) < MAX_PORT && bch.leng > 2) {
						int n = (int)(bch.p[2] & 0xf);
						memcpy(g_lm.sc[n].t.sBuf[SB_STORAGE], bch.p + 3, bch.leng - 2);
						g_lm.sc[n].t.i = bch.leng - 2;
						if (bch.cmd.a == BUSCMD_SCSEND)	SendSDA(n, false);
					}
					break;
				case BUSCMD_SCRREPORT :		// 0x16
					if ((bch.p[2] & 0xf) < MAX_PORT) 	PrintSDR((int)(bch.p[2] & 0xf));
					break;
				default :
					break;
				}
			}
		}
		if (g_host.r.i > (bch.left + bch.leng + 1)) {
			Shift(g_host.r.s, bch.left + bch.leng + 1);
			if (g_host.r.i > (bch.left + bch.leng + 1))	g_host.r.i -= (bch.left + bch.leng + 1);
			else	g_host.r.i = 0;
		}
		else	g_host.r.i = 0;
	}
}

void PrintSDR(int id)
{
	memcpy(&g_host.t.s[3], g_lm.sc[id].r.sBuf[SB_CURRENT], g_lm.sc[id].r.i);
	g_host.t.s[0] = g_lm.sc[id].r.i + 2;
	g_host.t.s[1] = BUSCMD_SCRREPORT;
	g_host.t.s[2] = 0xa0 | (BYTE)id;
	PrintSH(g_host.t.s, g_lm.sc[id].r.i + 3);		// to PC
}

DWORD PrintSH(BYTE* p, int leng)
{
	if (g_wShape & (1 << SHAPE_CONNECTCLIENT)) {
		if (DEBUGSOCKET())	printf("[SOCK]send %d bytes\r\n", leng);
		return (DWORD)send(SOCK_SERVER, (uint8_t*)p, (uint16_t)leng);
	}
	else	return 0;
}

void PrintFH(int leng, ...)
{
	BYTE buf[SIZE_ENVELOPE];

	va_list vl;
	va_start(vl, leng);
	int m = 0;
	buf[m ++] = (char)leng;
	for (int n = 0; n < leng; n ++)
		buf[m ++] = (char)va_arg(vl, int);
	va_end(vl);
	buf[m] = 0;
	PrintSH(buf, m);
}
