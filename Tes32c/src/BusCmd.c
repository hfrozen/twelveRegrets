/*
 * BusCmd.c
 *
 *  Created on: 2019. 3. 19.
 *      Author: SWCho
 */
#include <stdbool.h>
#include "Refer32.h"
#include "Uart.h"
#include "Gpio.h"
#include "Pwm.h"
#include "HostCmd.h"

const BYTE busCmdLength[6][16][2] = {
		// from PC, to PC
		{{	0x00, 0x00	},	// 0x00, null
		{	0x01, 0x00	},	// 0x01, all reset
		{	0x01, 0x03	},	// 0x02, type/id report
		{	0x00, 0x00	},	// 0x03
		{	0x00, 0x00	},	// 0x04
		{	0x00, 0x00	},	// 0x05
		{	0x00, 0x00	},	// 0x06
		{	0x00, 0x00	},	// 0x07
		{	0x00, 0x00	},	// 0x08
		{	0x00, 0x00	},	// 0x09
		{	0x00, 0x00	},	// 0x0a
		{	0x00, 0x00	},	// 0x0b
		{	0x00, 0x00	},	// 0x0c
		{	0x00, 0x00	},	// 0x0d
		{	0x00, 0x00	},	// 0x0e
		{	0x00, 0x00	}},	// 0x0f

		{{	0x01, 0x00	},	// 0x10, scu reset
		{	0x06, 0x00	},	// 0x11, initial & config.
		{	0x06, 0x00	},	// 0x12, config
		{	0x02, 0x06	},	// 0x13, status report
		{	0xff, 0x00	},	// 0x14, capture & send to device
		{	0xff, 0x00	},	// 0x15, capture only
		{	0x02, 0xff	},	// 0x16, rx frame report
		{	0x02, 0x00	},	// 0x17, initial
		{	0x09, 0x09	},	// 0x18, config
		{	0x00, 0x00	},	// 0x19
		{	0x00, 0x00	},	// 0x1a
		{	0x00, 0x00	},	// 0x1b
		{	0x00, 0x00	},	// 0x1c
		{	0x00, 0x00	},	// 0x1d
		{	0x00, 0x00	},	// 0x1e
		{	0x00, 0x00	}},	// 0x1f

		{{	0x02, 0x00	},	// 0x20, diu reset
		{	0x00, 0x00	},	// 0x21
		{	0x00, 0x00	},	// 0x22
		{	0x00, 0x00	},	// 0x23
		{	0x00, 0x00	},	// 0x24
		{	0x02, 0x06	},	// 0x25, input report
		{	0x02, 0x02	},	// 0x26, change report
		{	0x00, 0x00	},	// 0x27
		{	0x00, 0x00	},	// 0x28
		{	0x00, 0x00	},	// 0x29
		{	0x00, 0x00	},	// 0x2a
		{	0x00, 0x00	},	// 0x2b
		{	0x00, 0x00	},	// 0x2c
		{	0x00, 0x00	},	// 0x2d
		{	0x00, 0x00	},	// 0x2e
		{	0x00, 0x00	}},	// 0x2f

		{{	0x02, 0x00	},	// 0x30, dou reset
		{	0x00, 0x00	},	// 0x31
		{	0x00, 0x00	},	// 0x32
		{	0x00, 0x00	},	// 0x33
		{	0x00, 0x00	},	// 0x34
		{	0x02, 0x06	},	// 0x35, output report
		{	0x02, 0x02	},	// 0x36, change report
		{	0x06, 0x00	},	// 0x37, output by value
		{	0x04, 0x00	},	// 0x38, output by index
		{	0x00, 0x00	},	// 0x39
		{	0x00, 0x00	},	// 0x3a
		{	0x00, 0x00	},	// 0x3b
		{	0x00, 0x00	},	// 0x3c
		{	0x00, 0x00	},	// 0x3d
		{	0x00, 0x00	},	// 0x3e
		{	0x00, 0x00	}},	// 0x3f

		{{	0x02, 0x00	},	// 0x40, pou reset
		{	0x00, 0x00	},	// 0x41
		{	0x00, 0x00	},	// 0x42
		{	0x00, 0x00	},	// 0x43
		{	0x00, 0x00	},	// 0x44
		{	0x02, 0x0b	},	// 0x45, status report
		{	0x02, 0x02	},	// 0x46, change report
		{	0x0b, 0x00	},	// 0x47, pwm output
		{	0x00, 0x00	},	// 0x48
		{	0x00, 0x00	},	// 0x49
		{	0x00, 0x00	},	// 0x4a
		{	0x00, 0x00	},	// 0x4b
		{	0x00, 0x00	},	// 0x4c
		{	0x00, 0x00	},	// 0x4d
		{	0x00, 0x00	},	// 0x4e
		{	0x00, 0x00	}},	// 0x4f

		{{	0x02, 0x00	},	// 0x50, piu reset
		{	0x00, 0x00	},	// 0x51
		{	0x00, 0x00	},	// 0x52
		{	0x00, 0x00	},	// 0x53
		{	0x00, 0x00	},	// 0x54
		{	0x02, 0x0b	},	// 0x55, status report
		{	0x02, 0x02	},	// 0x56, change report
		{	0x00, 0x00	},	// 0x57
		{	0x00, 0x00	},	// 0x58
		{	0x00, 0x00	},	// 0x59
		{	0x00, 0x00	},	// 0x5a
		{	0x00, 0x00	},	// 0x5b
		{	0x00, 0x00	},	// 0x5c
		{	0x00, 0x00	},	// 0x5d
		{	0x00, 0x00	},	// 0x5e
		{	0x00, 0x00	}},	// 0x5f
};

extern IDDRAW	g_id;
extern LIMB		g_lm;
extern UARTHND	g_bus;
extern WORD		g_wShape;

BCHEAD AbstractCmd(BYTE* p, int leng, bool bDir)
{
	BCHEAD bch;
	for (int n = 0; n < leng; n ++) {
		bch.left = n;
		bch.p = &p[n];
		bch.leng = (int)p[n];
		bch.cmd.a = p[n + 1];
		bch.cid.a = p[n + 2];
		if (bch.cmd.a > 0x5f)	continue;
		int lbc = busCmdLength[bch.cmd.n.h][bch.cmd.n.l][bDir ? 0 : 1];
		if (lbc != 0xff && lbc != bch.leng)	continue;
		if (n + bch.leng > leng)	continue;
		if (bch.leng > 1 && bch.cid.ex.c0 != 2 && bch.cid.ex.c0 != 5)	continue;
		return bch;
	}
	bch.p = NULL;
	bch.leng = 0;
	return bch;
}

void ShiftCmd(BYTE* p, int shift)
{
	int n = 0;
	for ( ; shift < SIZE_ENVELOPE; n ++, shift ++)	p[n] = p[shift];
	for ( ; n < SIZE_ENVELOPE; n ++)	p[n] = 0;
}

bool PwmReport(BYTE cCmd, int nMax)
{
	bool bRep = false;
	for (int n = 0; n < nMax; n ++) {
		GetPwmIn(n, &g_lm.pio.cur[n].period.dw, &g_lm.pio.cur[n].width.dw);
		if (cCmd == BUSCMD_PIRREPORT || cCmd == BUSCMD_PORREPORT || g_lm.pio.cur[n].period.dw != g_lm.pio.prev[n].period.dw ||
			g_lm.pio.cur[n].width.dw != g_lm.pio.prev[n].width.dw) {
			g_lm.pio.prev[n].period.dw = g_lm.pio.cur[n].period.dw;
			g_lm.pio.prev[n].width.dw = g_lm.pio.cur[n].width.dw;
			PrintFB(0xb, cCmd < BUSCMD_PIRESET ? BUSRESP_POACT : BUSRESP_PIACT, ADDR_RESP, 0x50 | (BYTE)n,		// ROLE_PI, ROLE_PO
					g_lm.pio.cur[n].period.c[0], g_lm.pio.cur[n].period.c[1], g_lm.pio.cur[n].period.c[2], g_lm.pio.cur[n].period.c[3],
					g_lm.pio.cur[n].width.c[0], g_lm.pio.cur[n].width.c[1], g_lm.pio.cur[n].width.c[2], g_lm.pio.cur[n].width.c[3]);
			bRep = true;
		}
	}
	return bRep;
}

void DpioCtrl()
{
	if (g_bus.buf.r.i != 0 && g_bus.buf.wait == 0) {
		// cycle end
		BCHEAD bch = AbstractCmd(g_bus.buf.r.s, g_bus.buf.r.i, true);
		//printf("IO %d %d\r\n", g_bus.buf.r.i, bch.leng);
		if (bch.p == NULL)	g_bus.buf.r.i = 0;
		else {
			if (bch.cid.ex.ad == g_id.cur.address.a) {
				if (DEBUGDPIO())	printf("CMD %d\r\n", bch.cmd.a);
				switch (g_id.cur.role.a) {
				case ROLE_PI :
					switch (bch.cmd.a) {
					case BUSCMD_PIRESET :
						memset(&g_lm.pio, 0, sizeof(g_lm.pio));
						break;
					case BUSCMD_PIRREPORT :
					case BUSCMD_PICREPORT :
						if (!PwmReport(bch.cmd.a, PWMIN_LENGTH))	PrintFB(2, BUSRESP_PINON, ADDR_RESP);
						break;
					default :
						break;
					}
					break;
				case ROLE_PO :
					switch (bch.cmd.a) {
					case BUSCMD_PORESET :
						memset(&g_lm.pio, 0, sizeof(g_lm.pio));
						for (int n = 0; n < PWMOUT_LENGTH; n ++)	SetPwmOut(n, 0, 0);
						break;
					case BUSCMD_PORREPORT :
					case BUSCMD_POCREPORT :
						if (!PwmReport(bch.cmd.a, PWMOUT_LENGTH))	PrintFB(2, BUSRESP_PONON, ADDR_RESP);
						break;
					case BUSCMD_POPUT :
						if ((bch.p[3] & 0xf) < PWMOUT_LENGTH) {
							int n = (int)(bch.p[3] & 0xf);
							if (DEBUGDPIO())	printf("PWMCH %d\r\n", n);
							DWORD dwPeriod = MAKEDWORDA(bch.p[4], bch.p[5], bch.p[6], bch.p[7]);
							DWORD dwWidth = MAKEDWORDA(bch.p[8], bch.p[9], bch.p[10], bch.p[11]);
							SetPwmOut(n, dwPeriod, dwWidth);
						}
						break;
					default :
						break;
					}
					break;
				case ROLE_DO :
					switch (bch.cmd.a) {
					case BUSCMD_DORESET :
						g_lm.dio.prev.dw = g_lm.dio.cur.dw = 0;
						PutD32Out(g_lm.dio.cur.dw);
						break;
					case BUSCMD_DORREPORT :
					case BUSCMD_DOCREPORT :
						g_lm.dio.cur.dw = GetD32In();
						if (bch.cmd.a == BUSCMD_DORREPORT || g_lm.dio.cur.dw != g_lm.dio.prev.dw) {
							PrintFB(6, BUSRESP_DOACT, ADDR_RESP, g_lm.dio.cur.c[0], g_lm.dio.cur.c[1], g_lm.dio.cur.c[2], g_lm.dio.cur.c[3]);	// ROLE_DO
							g_lm.dio.prev.dw = g_lm.dio.cur.dw;
						}
						else	PrintFB(2, BUSRESP_DONON, ADDR_RESP);	// ROLE_DO
						break;
					case BUSCMD_DOPUTDWORD :
						g_lm.dio.cur.dw = MAKEDWORDA(bch.p[3], bch.p[4], bch.p[5], bch.p[6]);
						if (DEBUGDPIO())	printf("OUT 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n", bch.p[0], bch.p[1], bch.p[2], bch.p[3], bch.p[4], bch.p[5], bch.p[6]);
						PutD32Out(g_lm.dio.cur.dw);
						break;
					case BUSCMD_DOPUTBIT :
						if (bch.p[3] < 32 && bch.p[4] < 2) {
							if (bch.p[4] != 0)	g_lm.dio.cur.dw |= (DWORD)(1 << bch.p[3]);
							else	g_lm.dio.cur.dw &= ~((DWORD)(1 << bch.p[3]));
							PutD32Out(g_lm.dio.cur.dw);
						}
						break;
					default :
						break;
					}
					break;
				default :	// ROLE_DI
					switch (bch.cmd.a) {
					case BUSCMD_DIRESET :
						g_lm.dio.prev.dw = 0;
						break;
					case BUSCMD_DICREPORT :
						g_lm.dio.cur.dw = GetD32In();
						if (bch.cmd.a == BUSCMD_DIRREPORT || g_lm.dio.cur.dw != g_lm.dio.prev.dw) {
							PrintFB(6, BUSRESP_DIACT, ADDR_RESP, g_lm.dio.cur.c[0], g_lm.dio.cur.c[1], g_lm.dio.cur.c[2], g_lm.dio.cur.c[3]);	// ROLE_DI
							g_lm.dio.prev.dw = g_lm.dio.cur.dw;
						}
						else	PrintFB(2, BUSRESP_DINON, ADDR_RESP);	// ROLE_DI
						break;
					default :
						break;
					}
					break;
				}
			}
			if (g_bus.buf.r.i > (bch.left + bch.leng + 1)) {
				Shift(g_bus.buf.r.s, bch.left + bch.leng + 1);
				g_bus.buf.r.i -= (bch.left + bch.leng + 1);
			}
			else	g_bus.buf.r.i = 0;
		}
	}
}
