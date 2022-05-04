// Scc.c

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

#include "Scc.h"
#include "Tes5.h"

extern BYTE		cTesState;
extern BYTE		cDebug;
extern TIMES	time;
extern LIMB		lm;
//extern BYTE		cEmergencyCare;
extern _FLAGS	fl[4];
//extern BYTE		cEnf;

extern void SendByte(BYTE c);
extern void SendChar(BYTE c);

const FBYTE defDevices[12][8] = {
/*00*/	{	SDID_ATO,	SDID_ATO,	SDID_PAU,	SDID_PAU,	SDID_PIS,	SDID_PIS,	SDID_TRS,	SDID_TRS	},
/*01*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_SIV,	SDID_SIV,	SDID_DCUR,	SDID_DCUL,	},
/*02*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL,	},
/*03*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL,	},
/*04*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL,	},
/*05*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL,	},
/*06*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_SIV,	SDID_SIV,	SDID_DCUR,	SDID_DCUL,	},
/*07*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL,	},
/*08*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL,	},
/*09*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_V3F,	SDID_V3F,	SDID_DCUR,	SDID_DCUL,	},
/*10*/	{	SDID_HVAC,	SDID_HVAC,	SDID_ECU,	SDID_ECU,	SDID_SIV,	SDID_SIV,	SDID_DCUR,	SDID_DCUL,	},
/*11*/	{	SDID_ATO,	SDID_ATO,	SDID_PAU,	SDID_PAU,	SDID_PIS,	SDID_PIS,	SDID_TRS,	SDID_TRS	}
};

enum enCONFID {
	CONFID_ATTR,
	CONFID_MASK,
	CONFID_MODE,
	CONFID_ADDR,
	CONFID_MAX
};

const FWORD defSccConf[14][CONFID_MAX] = {
	{ SDID_ATO,		0x00,		(SCCMODE_ASYNCCRC << 4) | SCCBPS_19200,	0xffff },	// 0x21
	{ SDID_RTD,		0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0xff40 },	// 0x32
	{ SDID_PAU,		0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0x0160 },
	{ SDID_PIS,		0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0x0260 },
	{ SDID_TRS,		0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0x0460 },
	{ SDID_PDU,		0x00,		(SCCMODE_ASYNC << 4) | SCCBPS_115200,	0xffff },
	{ SDID_SIV,		0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0xff90 },
	{ SDID_V3F,		0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0xff20 },
	{ SDID_ECU,		0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0xff10 },
	{ SDID_HVAC,	0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0xff70 },
	{ SDID_DCUL,	0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0x0170 },
	{ SDID_DCUR,	0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0x1170 },
	{ SDID_ATON,	0xffff,		(SCCMODE_ASYNC << 4) | SCCBPS_19200,	0x4444 },
	{ SDID_TRSN,	0xffff,		(SCCMODE_SYNC << 4) | SCCBPS_38400,		0x8888 }
};

const FBYTE defSccResvLeng[12] = {
	/*SDID_ATO*/	13,		// with crc
	/*SDID_RTD*/	9,
	/*SDID_PAU*/	26,
	/*SDID_PIS*/	26,
	/*SDID_TRS*/	9,
	/*SDID_PDU*/	10,		//100,
	/*SDID_SIV*/	14,
	/*SDID_V3F*/	24,
	/*SDID_ECU*/	12,
	/*SDID_HVAC*/	19,
	/*SDID_DCUL*/	12,
	/*SDID_DCUR*/	12
};

const FBYTE defSccSendLeng[12] = {
	/*SDID_ATO*/	20,
	/*SDID_RTD*/	9,
	/*SDID_PAU*/	10,
	/*SDID_PIS*/	33,
	/*SDID_TRS*/	10,
	/*SDID_PDU*/	10,		//100,
	/*SDID_SIV*/	18,
	/*SDID_V3F*/	36,
	/*SDID_ECU*/	33,
	/*SDID_HVAC*/	22,
	/*SDID_DCUL*/	9,
	/*SDID_DCUR*/	9

};

const FWORD wBaudrates[8] = {
	958,	// 9600
	478,	// 19200
	238,	// 38400
	78,		// 115200
	34,		// 256000
	16,		// 512000
	7,		// 1024000
	4		// 1536000
};

const FBYTE	asyncInitTable[][2] = {
	{	9,	0xc0	},	// force hardware reset
	//{	4,	0x04	},	// x1 clock, 1stop, no parity	// +0x44,	// 2, x16 clock, 1stop, no parity
	{	4,	0x05	},	// x1 clock, 1stop, odd parity	// +0x44,	// 2, x16 clock, 1stop, no parity
	{	3,	0xc0	},	// rx 8bit, rx disable
	{	5,	0x60	},	// tx 8bit, DTR, RTS, tx off
	{	6,	0x00	},	// address
	{	7,	0x00	},	// sync char.
	{	10,	0x00	},	//
	{	11,	0x56	},	// tx & rx = BRG out, trxc= BRG out
	{	12, 0xee	},	// baudrate
	{	13,	0x00	},	// baudrate
	{	14,	0x83	},	// BRG source, BRG enable	0x01
	{	15,	0x00	},	// ext. int. disable
	{	3,	0xc1	},	// rx enable
	{	5,	WR5CMD_ASYNC | 2	},	// rx enable
	{	1,	0x12	},	// rx int., tx int.
	{	9,	0x0a	},	// 0x2a,	},	// no intack, MIE, non vector
	{	0xff,	0xff	}
};

const FBYTE	syncInitTable[][2] = {
	{	9,	0xc0	},	//
	{	4,	0x20	},	// X1 mode, sdlc, sync enable
	{	1,	0x00	},	// disable DMA & intr.
	{	3,	0xc8	},	// rx 8bits, rx CRC enable, !address search, !rx enable
	{	5,	0x61	},	// tx 8bits, tx CRC enable
	{	6,	0xff	},	// address
	{	7,	0x7e	},	// sync char
	{	10,	0xa8	},	// CRC preset=1, NRZI, mark/flag idle
	{	11,	0x57	},	// tx clk = rx clk = bg out,
	{	12,	0xee	},	//
	{	13,	0x00	},	//
	{	14,	0x03	},	// bgsrc=PCLK, bgenable
	{	15,	0x01	},	// enhancements Enable
	{	7,	0x2b	},	// rec crc, txd high, auto rts, auto eom, !auto RTS Deactivation
	{	3,	0xc9	},	// !address search // cd	},	// rx enable, address search mode
	{	5,	WR5CMD_SYNC | 2	},	// rx enable
	{	0,	0xc0	},	// reset tx underrun/eom latch
	{	0,	0x80	},	// reset tx CRC
	{	0,	0x40	},	// reset rx CRC checker
	{	0,	0x30	},	// reset error
	{	0,	0x10	},	// reset ext/status
	{	0,	0x28	},	// reset tx int. pending
	{	15,	0x40	},	// tx underrun/eom ie
	{	1,	0x12	},	// intr. on all rx char. or special condition & tx
	{	9,	0x2a	},	// no intack, mie, non vector
	{	0xff, 0xff	}
};

//const FBYTE dmaInitTable[][2] = {
//	{	9,	0xc0	},	//
//	{	4,	0x20	},	//
//	{	1,	0x40	},	// DMA request function
//	{	3,	0xfc	},	// rx8, auto enable, enter hunt enable, rx crc, address search
//	{	5,	0x63	},	// tx8, rts, tx crc,
//	{	6,	0x02	},	//
//	{	7,	0x7e	},	//
//	{	9,	0x02	},	// no vector
//	{	10,	0x00	},	// !crc preset, nrz, mark/flag idle
//	{	11,	0x08	},	// clk
//	{	14,	0x00	},	// null
//	{	3,	0xfd	},	// rx enable
//	{	5,	0x6b	},	// tx enable
//	{	0,	0x80	},	// reset rx crc gen.
//	{	1,	0xc0	},	// dma enable
//	{	15,	0x08	},	// DCD IE
//	{	0,	0x10	},	// reset ext/status int.
//	{	0,	0x10	},	//
//	{	1,	0xc9	},	// rx 1st int, tx int
//	{	9,	0x0a	},	// MIE, NV
//	{	0xff, 0xff	}
//};
//
const FWORD crc16Table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

//const BYTE mdefSccResvLeng[12] = {
//	/*SDID_ATO*/	13,
//	/*SDID_RTD*/	9,
//	/*SDID_PA*/		14,
//	/*SDID_PIS*/	14,
//	/*SDID_TRS*/	9,
//	/*SDID_PDU*/	100,
//	/*SDID_SIV*/	14,
//	/*SDID_V3F*/	24,
//	/*SDID_ECU*/	12,
//	/*SDID_HVAC*/	16,
//	/*SDID_DCUL*/	12,
//	/*SDID_DCUR*/	12
//};

WORD Crc161d0f(BYTE* p, BYTE leng)
{
	WORD crc = 0x1d0f;
	for (BYTE n = 0; n < leng; n ++) {
		WORD ti = ((crc >> 8) ^ *p ++) & 0xff;
		crc = (crc << 8) ^ pgm_read_word((WORD)&crc16Table[ti]);
	}
	return crc;
}

WORD Crc16ffff(BYTE* p, BYTE leng)
{
	WORD crc = 0xffff;
	for (BYTE n = 0; n < leng; n ++) {
		crc ^= (unsigned short)*p ++ << 8;
		for (BYTE m = 0; m < 8; m ++)
			crc = crc << 1 ^ (crc & 0x8000 ? 0x1021 : 0);
	}
	return crc;
}

void InitialChannels(BYTE cid)
{
	TRACE(PSTR("SCC Initial for unit#%d.\r\n"), cid);
	for (BYTE n = 0; n < 8; n ++) {
		TRACE(PSTR("ch.%d "), n);
		BYTE did = pgm_read_byte((WORD)&defDevices[cid][n]);
		NTRACE(PSTR("div id = %02d "), did);
		WORD w = pgm_read_word((WORD)&defSccConf[did][CONFID_ATTR]);	// attribute
		lm.scc.pt[n].cf.att.a = (BYTE)w;					// (BYTE)(w & 0xf0) | (did & 0xf);
		NTRACE(PSTR("attr = %02X "), lm.scc.pt[n].cf.att.a);
		lm.scc.pt[n].cf.addr.w = pgm_read_word((WORD)&defSccConf[did][CONFID_ADDR]);
		NTRACE(PSTR("addr = %04X "), lm.scc.pt[n].cf.addr.w);
		w = pgm_read_word((WORD)&defSccConf[did][CONFID_MODE]);	// mode
		lm.scc.pt[n].cf.tp.a = (BYTE)w;
		NTRACE(PSTR("mode = %02X\r\n"), lm.scc.pt[n].cf.tp.a);
		//for (BYTE m = 0; m < SIZE_SCCBUF; m ++)	lm.scc.pt[n].tx.sBuf[m] = m + 1;
		if (n & 1)	InitialScc(n >> 1);
	}
}

void InitialScc(BYTE did)
{
	if (did < 4) {
		const BYTE* p = lm.scc.pt[(did << 1) + 1].cf.tp.b.mode != SCCMODE_SYNC ? &asyncInitTable[1][0] : &syncInitTable[1][0];

		for (BYTE n = 0; n < 100; n ++) {
			SCCC(did, 0) = 9;
			SCCC(did, 0) = 0xc0;
			SCCC(did, 1) = 9;
			SCCC(did, 1) = 0xc0;
		}

		for (BYTE n = 0; n < 2; n ++) {
			WORD wptr = (WORD)p;
			BYTE reg, dt;
			for ( ; (reg = pgm_read_byte(wptr ++)) != 0xff; ) {
				dt = pgm_read_byte(wptr ++);
				//if (reg == 6 && lm.scc.pt[(did << 1) + 1].cf.tp.b.mode == SCCMODE_SYNC) {
				//	SCCC(did, n) = 6;
				//	SCCC(did, n) = 0xff;	//LOBYTE(lm.scc.pt[(did << 1) + n].cf.addr);
				//	if (ComaM2())	printf_P(PSTR("06 %02X\r\n"), LOBYTE(lm.scc.pt[(did << 1) + n].cf.addr));
				//}
				//else
				if (reg == 12) {
					WORD w = pgm_read_word((WORD)&wBaudrates[lm.scc.pt[(did << 1) + n].cf.tp.b.bps]);
					SCCC(did, n) = 12;
					SCCC(did, n) = LOBYTE(w);
					SCCC(did, n) = 13;
					SCCC(did, n) = HIBYTE(w);
					if (ComaM2())	printf_P(PSTR("0C %02X\r\n0D %02X\r\n"), LOBYTE(w), HIBYTE(w));
					wptr += 2;
				}
				else {
					SCCC(did, n) = reg;
					SCCC(did, n) = dt;
					if (ComaM2())	printf_P(PSTR("%02X %02X\r\n"), reg, dt);
				}
			}
		}
	}
}

//BOOL SccRxFull(BYTE id)
//{
//	return (lm.scc.cState & (1 << id) ? TRUE : FALSE);
//}
//
SCCCHS* GetSccBuf(BYTE id)
{
	return &lm.scc.pt[id];
}

void SendScc(BYTE id)
{
	SCCCHS* pSccCh = GetSccBuf(id);
	if (pSccCh->cf.tp.b.mode != SCCMODE_SYNC)	EnableTxAsync(id);
	else	EnableTxSync(id);

	for (BYTE c = 0; c < 200; c++);
	for (BYTE c = 0; c < 200; c++);
	for (BYTE c = 0; c < 200; c++);
	for (BYTE c = 0; c < 200; c++);
	cli();
	pSccCh->TXI = 1;
	SCCD((id >> 1), (id & 1)) = pSccCh->TXB[0][0];
	sei();
}

void ScanScc()	// 50us
{
	T1FALL();
	for (BYTE n = 0; n < 8; n ++) {
		if (lm.scc.pt[(n & 0xfe) + 1].cf.tp.b.mode != SCCMODE_SYNC && lm.scc.pt[n].RXR > 0)
			-- lm.scc.pt[n].RXR;
		if (lm.scc.pt[n].TXCW != 0 && -- lm.scc.pt[n].TXCW == 0) {
			if (lm.scc.pt[(n & 0xfe) + 1].cf.tp.b.mode != SCCMODE_SYNC) {
				lm.scc.pt[n].RXR = CYCLE_INHIBITECHO;
				DisableTxAsync(n);
			}
			else	DisableTxSync(n);
		}
		if (!(lm.scc.cState & (1 << n)) && lm.scc.pt[n].RXI > 0 &&
			lm.scc.pt[n].RXCW != 0 && -- lm.scc.pt[n].RXCW == 0) {
			lm.scc.cState |= (1 << n);
		}
		//if (!(lm.scc.cState & (1 << n)) && lm.scc.pt[n].RXI > 0) {
		//	if (lm.scc.pt[n].RXCW != 0) {
		//		if (-- lm.scc.pt[n].RXCW == 0)	lm.scc.cState |= (1 << n);
		//		if (n == 0) {
		//			if (cEnf == 0) {
		//				printf_P(PSTR("WC=%d\r\n"), lm.scc.pt[n].RXCW);
		//				cEnf = 1;
		//			}
		//		}
		//	}
		//}
	}
	T1RISE();
}

BYTE GetSendLeng(BYTE id)
{
	BYTE leng = pgm_read_byte((WORD)&defSccSendLeng[id]);
	return leng;
}

//WORD GetDeviceAddress(BYTE did)
//{
//	WORD w = pgm_read_word((WORD)&defSccConf[did][CONFID_ADDR]);	// addr
//	return w;
//}
//
/*
void SccISR(BYTE di)
{
	BYTE state = 0;
	do {
		SCCC(di, 0) = 3;						// rrf3
		state = SCCC(di, 0);
		if (state & 0x20) {
			BYTE c = SCCD(di, 0);
			SCCCHS* pSccCh = &lm.scc.pt[(di << 1)];
			if (!(lm.scc.cState & RXFBYDICH(di, 0)) && pSccCh->RXI < SIZE_SCCBUF) {
				pSccCh->RXB[0][pSccCh->RXI] = c;
				pSccCh->RXI ++;
				if (pSccCh->cf.tp.b.mode == SCCMODE_SYNC) {
					SCCC(di, 0) = 1;
					c = SCCC(di, 0);
					if (c & 0x80)	pSccCh->RXCT = WAIT_RXCLOSE;
				}
				else {
					if (pSccCh->TXCT != 0 || pSccCh->RXR != 0) {
						-- pSccCh->RXI;
					}
					else	pSccCh->RXCT = WAIT_RXCLOSE;
				}
			}
		}
		if (state & 0x10) {
			SCCCHS* pSccCh = &lm.scc.pt[(di << 1)];
			if (pSccCh->TXI < pSccCh->TXR)
				SCCD(di, 0) = pSccCh->TXB[0][pSccCh->TXI ++];
			else {
				SCCC(di, 0) = 0x28;
				pSccCh->TXCT = WAIT_TXCLOSE;
			}
		}
		if (state & 8)		SCCC(di, 0) = 0x10;	// reset ext/status int
		if (state & 4) {
			BYTE c = SCCD(di, 1);
			SCCCHS* pSccCh = &lm.scc.pt[(di << 1) + 1];
			if (!(lm.scc.cState & RXFBYDICH(di, 1)) && pSccCh->RXI < SIZE_SCCBUF) {
				pSccCh->RXB[0][pSccCh->RXI ++] = c;
				if (pSccCh->cf.tp.b.mode == SCCMODE_SYNC) {
					SCCC(di, 1) = 1;
					c = SCCC(di, 1);
					if (c & 0x80)	pSccCh->RXCT = WAIT_RXCLOSE;
				}
				else {
					if (pSccCh->TXCT != 0 || pSccCh->RXR != 0)
						-- pSccCh->RXI;
					else	pSccCh->RXCT = WAIT_RXCLOSE;
				}
			}
		}
		if (state & 2) {
			SCCCHS* pSccCh = &lm.scc.pt[(di << 1) + 1];
			if (pSccCh->TXI < pSccCh->TXR)
				SCCD(di, 1) = pSccCh->TXB[0][pSccCh->TXI ++];
			else {
				SCCC(di, 1) = 0x28;
				pSccCh->TXCT = WAIT_TXCLOSE;
			}
		}
		if (state & 1)		SCCC(di, 1) = 0x10;	// reset ext/status int
		//SCCC(di, 1) = 0x38;
	} while (state != 0);
}
*/