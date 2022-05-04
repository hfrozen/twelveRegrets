// Tes5.c

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "Tes5.h"
#include "Socket.h"
#include "Scc.h"

BYTE	cTesState;
BYTE	cDebug;
BYTE	cCid;
#define	DEB_ADDR	20
#define	CTYPE()		(HINIBB(cCid))
#define	CADDR()		(LONIBB(cCid))

BYTE	cSockIntFlag[MAX_SOCKET];
WORD	wSockRxiDeb[MAX_SOCKET];

BYTE	cTick;
#define	TIME_TICK		100

BYTE	cTock;
#define	TIME_TOCK		10

LIMB	lm;
HOST	host;
LOCAL	local;
MONC	monc;
TIMES	time;

BYTE	cTraceLength;	// lsb:siv,	msb:v3f
BYTE	cCare;
_FLAGS	fl[4];
//BYTE	cEnf;
BYTE	cCrcType;

WORD	wDelay;

//BYTE	cAtoBuf[SIZE_SCCBUF];
//BYTE	cV3fBuf[SIZE_SCCBUF];
//BYTE	cSivBuf[SIZE_SCCBUF];
BYTE	cMultiBuf[SIZE_SCCBUF];		// ato, siv, v3f is never one unit!!!

static int PutChar(char c, FILE* stream);
static FILE TesStdOut = FDEV_SETUP_STREAM(PutChar, NULL, _FDEV_SETUP_WRITE);

const FBYTE szMac[] =			{ 0x00, 0x08, 0xdc, 0x06, 0x02, 0x01 };
const FBYTE szHelloA[] =		{ "Traffic unit of Train electric Signal Simulation System ver2.1  " };
const FBYTE szHelloB[] =		{ "Communication unit of Train electric Signal Simulation System ver2.1  " };
const FBYTE szHelloc[] =		{ "In/Out unit of Train electric Signal Simulation System ver2.1  " };

const FBYTE cmdLength[2][5][16] = {
	// send
	{{	0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	},	// 00 unit:common
	{	0x01, 0x06, 0x06, 0x02, 0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	},	// 10 unit:comm.
	{	0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	},	// 20 unit:in
	{	0x02, 0x02, 0x02, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	},	// 30 unit:out
	{	0x02, 0x03, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	}},	// 40 unit:pwm
	// receive
	{{	0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	},	// 00 unit:common
	{	0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	},	// 10 unit:comm.
	{	0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	},	// 20 unit:in
	{	0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	},	// 30 unit:out
	{	0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	}}	// 40 unit:pwm
};

int main(void) __attribute__((OS_main));
int main(void)
{
	cTesState = 0;
	cDebug = DEBUG_MONITCOMMA | DEBUG_MONITSOCK;	// DEBUG_MONITCOMMA | DEBUG_MONITCOMM;	//  | DEBUG_MONITSOCK;
	memset((PVOID)&cDleBuf, 0, SIZE_DLEBUF);
	memset((PVOID)&lm, 0, sizeof(lm));
	memset((PVOID)&local, 0, sizeof(local));
	memset((PVOID)&host, 0, sizeof(host));
	memset((PVOID)&monc, 0, sizeof(monc));
	memset((PVOID)&time, 0, sizeof(time));
	cCare = 0;
	memset((PVOID)&fl, 0, sizeof(_FLAGS) * 4);
	memset((PVOID)cMultiBuf, 0, SIZE_SCCBUF);
	cTraceLength = 0;

	lm.inp.uDeb[0] = DEB_ADDR;
	cTick = TIME_TICK;
	cTock = TIME_TOCK;
	cCrcType = 0;

	PORTB = 0xfe;	// PB0=tdir=0, local bus dir
	DDRB = 0xff;
	PORTD = 0xff;
	DDRD = 8;
	PORTE = 0xff;
	DDRE = 6;
	PORTF = 0xff;
	DDRF = 0;
	PORTG = 0xff;
	DDRG = 0xf0;
	stdout = &TesStdOut;

	ReloadTimer();
	TCCR0 = (1 << CS02) | (1 << CS00);		// 1 / 18.432MHz * 128 = 6.9444...us (* 144 = 1ms)
	TIFR = (1 << TOV0);
	TIMSK = (1 << TOIE0);
	EIMSK = 0;
	sei();

	while (!(cTesState & TESSTATE_AWAKE));	// high=address, low=type
	cli();
	InitialDev();
	sei();

	if (CTYPE() == CTYPE_TRAFFIC || CTYPE() == CTYPE_COMM) {
		SoftResetSock();
		DWORD dwAddr = DEF_SOCKSIP + CADDR();		// 80 ~	// 50~65
		if (CTYPE() == CTYPE_COMM)	dwAddr += 3;	// 83 ~	// 66~81
		SetSrcIpAddress(dwAddr);
		SetGatewayAddress(DEF_SOCKGATE);
		SetSubnetMaskAddress(DEF_SOCKMASK);
		for (BYTE n = 0; n < 6; n ++)
			host.t.s[n] = pgm_read_byte(&szMac[n]);
		//host.t.s[4] += CADDR() + 2;
		host.t.s[5] += CADDR();
		if (CTYPE() == CTYPE_COMM)	host.t.s[5] += 16;
		SetSrcHwAddress(host.t.s);
		SetSockIntMask(0xff);
	}

	PRINTT();
	if (CTYPE() == CTYPE_TRAFFIC || CTYPE() == CTYPE_COMM) {
		if (CTYPE() == CTYPE_TRAFFIC)	printf_P((const char*)szHelloA);
		else	printf_P((const char*)szHelloB);
		printf_P(PSTR(" IP ["));
		PrintIP(GetSrcIpAddress());
		printf_P(PSTR("]\r\n"));
		if (CTYPE() == CTYPE_TRAFFIC) {
			cTesState |= TESSTATE_SCANINB;
			local.cMaxInb = 16;
			local.cCurInb = 0;
			local.cScanInb = 0;
		}
		else {
			InitialChannels(CADDR());
			lm.scc.cState = 0;
		}
		Delay(20);
	}
	else {
		printf_P((const char*)szHelloc);
		printf_P(PSTR("Status %d - %d.\r\n"), CTYPE(), CADDR());
	}

	BYTE socksted = 0xff;
	while (TRUE) {
		if (CTYPE() == CTYPE_TRAFFIC || CTYPE() == CTYPE_COMM) {
			BYTE sockst = GetSockStatus(SOCK_SERVER);
			if (sockst != socksted) {
				socksted = sockst;
				if (SockM())	TRACE(PSTR("CSS %02X.\r\n"), sockst);
			}
			int len;
			switch (sockst) {
			case SOCKSTATUS_ESTABLISHED :
				if (!(cTesState & TESSTATE_CONNECTCLIENT)) {
					if (cSockIntFlag[SOCK_SERVER] & SOCKINTR_CONNECT)
						cSockIntFlag[SOCK_SERVER] &= ~SOCKINTR_CONNECT;
					cTesState |= TESSTATE_CONNECTCLIENT;
					TRACE(PSTR("Client ["));
					PrintIP(GetSockDestIpAddress(SOCK_SERVER));
					NTRACE(PSTR("] connected.\r\n"));
				}
				if ((len = GetSockRecvedSize(SOCK_SERVER)) > 0) {
					if (len > (SIZE_ENVELOPE - host.r.i))	len = SIZE_ENVELOPE - host.r.i;
					len = RecvSock(SOCK_SERVER, &host.r.s[host.r.i], len);
					host.r.i += len;
					if (SockM())	TRACE(PSTR("GetS %d\r\n"), len);
				}
				break;
			case SOCKSTATUS_CLOSEWAIT :
				cTesState &= ~TESSTATE_CONNECTCLIENT;
				DisconnectSock(SOCK_SERVER);
				TRACE(PSTR("Client close.\r\n"));
				break;
			case SOCKSTATUS_CLOSED :
				if (SockM())	TRACE(PSTR("Ready Open...\r\n"));
				cTesState &= ~TESSTATE_CONNECTCLIENT;
				OpenSock(SOCK_SERVER, SOCKMODE_TCP, DEF_SOCKLPT, SOCKMODE_NDACK);	//0);
				if (!ListenSock(SOCK_SERVER)) {
					CloseSock(SOCK_SERVER);
					TRACE(PSTR("Listen failure!\r\n"));
				}
				else {
					if (SockM())	TRACE(PSTR("Listen...\r\n"));
				}
				break;
			default :	break;
			}
			if (host.r.i > 0)	HostCommand();
			if (CTYPE() == CTYPE_TRAFFIC) {
				if (local.cWaitResp == 0) {
					while (local.r.i > 0) {
						HCH hch = AbstractCmd(local.r.s, local.r.i, FALSE);		// continue
						if (hch.p != NULL) {
							if ((cTesState & TESSTATE_SCANINB) && hch.leng > 0 &&
								(hch.cmd.a == HCMD_DIRREPORT || hch.cmd.a == HCMD_DICREPORT)) {
								local.cScanInb = local.cCurInb;
							}
							else if (hch.leng > 0 && (hch.cmd.a == HCMD_SCSREPORT || hch.cmd.a == HCMD_SCRREPORT ||
								hch.cmd.a == HCMD_DIRREPORT || hch.cmd.a == HCMD_DORREPORT || hch.cmd.a == HCMD_PMRREPORT)) {
								if (cTesState & TESSTATE_CONNECTCLIENT) {
									memcpy((PVOID)host.t.s, (PVOID)(hch.p), hch.leng + 1);
									WORD w = SendSock(SOCK_SERVER, host.t.s, (WORD)(hch.leng + 1));
									TRACE(PSTR("send sock %02X %d(%d).\r\n"), hch.cmd.a, hch.leng, w);
								}
							}
							if (local.r.i > (hch.left + hch.leng + 1)) {
								Shift(local.r.s, hch.left + hch.leng + 1);
								local.r.i -= (hch.left + hch.leng + 1);
							}
							else	local.r.i = 0;
						}
						else	local.r.i = 0;
						Delay(2);
					}
					if (local.t.i > 0) {
						TRACE(PSTR("b\r\n"));
						BYTE leng = local.t.s[0] + 1;
						PutsLB(local.t.s, leng);
						local.cWaitResp = TIME_LOCALWAITRESPOND;
						Shift(local.t.s, leng);
						local.t.i -= leng;
					}
					else if (local.cMaxInb > 0) {
						if (local.cCurInb >= local.cMaxInb) {
							TRACE(PSTR("c\r\n"));
							if (cTesState & TESSTATE_SCANINB) {
								local.cMaxInb = local.cScanInb;
								cTesState &= ~TESSTATE_SCANINB;
								TRACE(PSTR("Input board length is %d.\r\n"), local.cMaxInb);
							}
							local.cCurInb = 0;
						}
						if (local.cMaxInb > 0) {
							TRACE(PSTR("d\r\n"));
							local.t.s[0] = 2;
							local.t.s[1] = 0x22;
							local.t.s[2] = 0x50 | local.cCurInb ++;
							PutsLB(local.t.s, 3);
							local.cWaitResp = TIME_LOCALWAITRESPOND;
						}
					}
				}
				else	TRACE(PSTR("e\r\n"));
			}
			else {
				//T2FALL();
				if (cCare != 0) {
					for (BYTE n = 0; n < 4; n ++) {
						if (cCare & (3 << (n << 1))) {
							cCare &= ~(3 << (n << 1));
							InitialScc(n);
							if (ComaM())	TRACE(PSTR("IC#%d\r\n"), n);
						}
					}
				}
				if (lm.scc.cState != 0) {
					for (BYTE n = 0; n < 8; n ++) {
						if (lm.scc.cState & (1 << n)) {
							//if (ComaM()) {
							//	if (lm.scc.pt[n].cf.tp.b.mode == SCCMODE_SYNC) {
							//		if (lm.scc.pt[n].RXI != 0)
							//			TRACE(PSTR("rc%d-%d %02X!\r\n"), lm.scc.pt[n].RXI, n, lm.scc.cState);
							//	}
							//	else	TRACE(PSTR("tc%d-%d %02X.\r\n"), lm.scc.pt[n].RXI, n, lm.scc.cState);
							//}

							if (lm.scc.pt[n].RXI > 6) {
								BOOL bMatch = FALSE;
								BYTE cDisPos = 0x10;	// cf.att.b.dr0 - general position
								if (lm.scc.pt[n].cf.tp.b.mode != SCCMODE_ASYNCCRC) {
									if (lm.scc.pt[n].cf.tp.b.mode == SCCMODE_SYNC) {
										if ((lm.scc.pt[n].cf.att.b.id == SDID_DCUL || lm.scc.pt[n].cf.att.b.id == SDID_DCUR) &&
											lm.scc.pt[n].RXB[0][0] == lm.scc.pt[n].cf.addr.c[0]) {
											bMatch = TRUE;
											cDisPos <<= (lm.scc.pt[n].RXB[0][1] - lm.scc.pt[n].cf.addr.c[1]);
											cDisPos &= 0xf0;
										}
										else if (lm.scc.pt[n].RXB[0][0] == lm.scc.pt[n].cf.addr.c[0] &&
											lm.scc.pt[n].RXB[0][1] == lm.scc.pt[n].cf.addr.c[1])	bMatch = TRUE;
									}
									else	bMatch = TRUE;	// SCCMODE_ASYNC or SCCMODE_ASYNCBCC
								}
								else {						// SCCMODE_ASYNCCRC
									BYTE res = DleDecoder(lm.scc.pt[n].RXB[0], lm.scc.pt[n].RXI);
									if (res == 0) {
										memcpy(lm.scc.pt[n].RXB[0], cDleBuf, 11);
										bMatch = TRUE;
									}
									else	TRACE(PSTR("ARE %d\r\n"), res);
								}
								if (bMatch) {
									//if (ComaM())	TRACE(PSTR("Receive ch.%d - %d.\r\n"), n, lm.scc.pt[n].RXI);
									if (!(lm.scc.pt[n].cf.att.a & cDisPos))	PreSend(n);
									if (lm.scc.pt[n].cf.tp.b.rrm) {
										BOOL bCmp = TRUE;
										BYTE si = 0;
										BYTE leng = 0;
										switch (lm.scc.pt[n].cf.att.b.id) {
										case SDID_ATO :		si = 2;		leng = 7;	break;
										case SDID_PAU :
										case SDID_PIS :		si = 4;		leng = 14;	break;
										case SDID_TRS :		si = 4;		leng = 3;	break;
										case SDID_SIV :		si = 4;		leng = 2;	break;
										case SDID_V3F :		si = 10;	leng = 12;	break;
										case SDID_ECU :		si = 4;		leng = 2;	break;
										case SDID_HVAC :	si = 4;		leng = 8;	break;
										case SDID_DCUL :
										case SDID_DCUR :	si = 4;		leng = 2;	break;
										default :	bCmp = FALSE;	break;
										}
										if (bCmp) {
											if (memcmp(&lm.scc.pt[n].RXB[1][si], &lm.scc.pt[n].RXB[0][si], leng)) {
												memcpy(&lm.scc.pt[n].RXB[1][si], &lm.scc.pt[n].RXB[0][si], leng);
												RecvReport(n);
											}
										}
									}
								}
							}
							else if (ComaM()) {
								InitialScc(n >> 1);
								TRACE(PSTR("rlech.%d(%d)\r\n"), n, lm.scc.pt[n].RXI);
							}
							cli();
							lm.scc.pt[n].RXI = 0;
							lm.scc.cState &= ~(1 << n);
							sei();
						}
					}
				}
				//T2RISE();
			}
		}	// TRAFFIC, COMM end
		else {	// other
			if (local.r.i > 0 && local.cWaitResp == 0) {
				HCH hch = AbstractCmd(local.r.s, local.r.i, TRUE);	// continue
				if (hch.p == NULL)	local.r.i = 0;
				else {
					if (LONIBB(hch.cid) == LONIBB(cCid)) {	// && pHch->cmd.b.h == HINIBB(uCid)) {
						switch (CTYPE()) {
						case CTYPE_INP :
							switch (hch.cmd.a) {
							case HCMD_DIRESET :
								lm.inp.uVrf[0] = lm.inp.uVrf[1] = lm.inp.uVrf[2] = 0;
								break;
							case HCMD_DIRREPORT :
								InpReport();
								break;
							case HCMD_DICREPORT :
								if (lm.inp.uVal[0] != lm.inp.uVrf[0] || lm.inp.uVal[1] != lm.inp.uVrf[1] ||
									lm.inp.uVal[2] != lm.inp.uVrf[2])
									InpReport();
								else	InpNullReport();
								break;
							default :	break;
							}
							break;
						case CTYPE_OUTP :
							switch (hch.cmd.a) {
							case HCMD_DORESET :
								lm.outp.uVal[0] = lm.outp.uVal[1] = lm.outp.uVal[2] =
								lm.outp.uVrf[0] = lm.outp.uVrf[1] = lm.outp.uVrf[2] = 0;
								Output();
								break;
							case HCMD_DORREPORT :
								OutpReport();
								break;
							case HCMD_DOCREPORT :
								if (lm.outp.uVal[0] != lm.outp.uVrf[0] ||
									lm.outp.uVal[1] != lm.outp.uVrf[1] ||
									lm.outp.uVal[2] != lm.outp.uVrf[2])
									OutpReport();
								else	OutpNullReport();
								break;
							case HCMD_DOABSPUT :
								lm.outp.uVal[2] = hch.p[3];
								lm.outp.uVal[1] = hch.p[4];
								lm.outp.uVal[0] = hch.p[5];
								Output();
								break;
							case HCMD_DOSELPUT :
								lm.outp.uRef[0] = hch.p[3];
								lm.outp.uRef[1] = hch.p[4];
								lm.outp.uRef[2] = lm.outp.uRef[0] / 8;
								if (lm.outp.uRef[2] < 3) {
									lm.outp.uRef[0] %= 8;
									lm.outp.uVal[lm.outp.uRef[2]] &= ~(1 << lm.outp.uRef[0]);
									if (lm.outp.uRef[1] != 0)
										lm.outp.uVal[lm.outp.uRef[2]] |= (1 << lm.outp.uRef[0]);
									Output();
								}
								break;
							default :	break;
							}
							break;
						case CTYPE_PWM :
							switch (hch.cmd.a) {
							case HCMD_PMRESET :
								break;
							case HCMD_PMRREPORT :
								if (hch.p[3] < 4)	PwmReport(hch.p[3]);
								break;
							case HCMD_PMCREPORT :
								if (hch.p[3] < 4) {
									if (lm.pwm.wVal[hch.p[3]] != lm.pwm.wVrf[hch.p[3]])
										PwmReport(hch.p[3]);
									else	PwmNullReport(hch.p[3]);
								}
								break;
							case HCMD_PMABSPUT :
								if (hch.p[3] < 4) {
									lm.pwm.v = MAKEWORD(hch.p[5], hch.p[4]);
									if ((hch.p[3] > 0 && lm.pwm.v <= 1000) || hch.p[3] == 0) {
										lm.pwm.wVal[hch.p[3]] = lm.pwm.v;
										PwmCtrl(hch.p[3]);
									}
								}
								else if (hch.p[3] == 4) {	// relay
									if (hch.p[4] == 0 && (hch.p[5] & 0xf0) == 0) {
										PORTA = (PINA & 0xf0);
										PORTA |= Turnup((hch.p[5] << 4) & 0xf0);
									}
								}
								break;
							default :	break;
							}
							break;
						default :	break;
						}
					}	// address match
					if (local.r.i > (hch.left + hch.leng + 1)) {
						Shift(local.r.s, hch.left + hch.leng + 1);
						local.r.i -= (hch.left + hch.leng + 1);
					}
					else	local.r.i = 0;
				}	// pHch != NULL
			}	// local.r.i > 0 && local.cWaitResp == 0
		}	// other card

		if (cTesState & TESSTATE_MONIT) {
			cTesState &= ~TESSTATE_MONIT;
			Monit();
			monc.r.i = 0;
		}
	}
}

void HostCommand(void)
{
	HCH hch = AbstractCmd(host.r.s, host.r.i, TRUE);	// continue
	if (hch.p == NULL)	host.r.i = 0;
	else {
		if (CTYPE() == CTYPE_TRAFFIC) {
			memcpy((PVOID)&local.t.s[local.t.i], (PVOID)(hch.p), hch.leng + 1);
			local.t.i += (hch.leng + 1);
			if ((hch.cmd.a >= HCMD_DORESET && hch.cmd.a <= HCMD_DOSELPUT) ||
				(hch.cmd.a >= HCMD_PMRESET && hch.cmd.a <= HCMD_PMABSPUT))
				RespSock(CADDR());
		}
		else if (CTYPE() == CTYPE_COMM) {
			BYTE ch = LONIBB(hch.cid);
			if (ch < 8) {
				if (CommM())	TRACE(PSTR("Comm. command %X ch.%d\r\n"), hch.cmd.a, ch);
				switch (hch.cmd.a) {
				case HCMD_SCRESET :
					break;
				case HCMD_SCINIT :
				case HCMD_SCCONFIG :
					//lm.scc.pt[ch].cf.addr = MAKEWORD(hch.p[3], hch.p[4]);
					lm.scc.pt[ch].cf.att.a &= 0xf;
					lm.scc.pt[ch].cf.att.a |= (hch.p[5] & 0xf0);
					lm.scc.pt[ch].cf.tp.a &= 0x3f;
					lm.scc.pt[ch].cf.tp.a |= (hch.p[6] & 0xc0);
					if (hch.cmd.a == HCMD_SCINIT && (ch & 1)) {
						InitialScc(ch >> 1);
						if (ComaM())	TRACE(PSTR("Initial ch.%d & ch.%d\r\n"), ch & 0xfe, ch);
					}
					//StatusReport(ch, hch.cmd.a);
					//RespSock(ch);
					break;
				case HCMD_SCSREPORT :
					StatusReport(ch, hch.cmd.a);
					break;
				case HCMD_SCSEND :
				case HCMD_SCRESVSEND :
					if (hch.leng > 2) {
						if (lm.scc.pt[ch].cf.att.b.id == SDID_SIV) {
							memcpy((PVOID)cMultiBuf, (PVOID)(hch.p + 3), hch.leng - 2);
							cTraceLength &= 0xf0;
							cTraceLength |= ((cMultiBuf[15] >> 4) & 0xf);
							TRACE(PSTR("siv tl %d\r\n"), cTraceLength & 0xf);
						}
						else if (lm.scc.pt[ch].cf.att.b.id == SDID_V3F) {
							memcpy((PVOID)cMultiBuf, (PVOID)(hch.p + 3), hch.leng - 2);
							cTraceLength &= 0xf;
							cTraceLength |= (cMultiBuf[33] & 0xf0);
							TRACE(PSTR("v3f tl %d\r\n"), (cTraceLength >> 4) & 0xf);
						}
						else	memcpy((PVOID)lm.scc.pt[ch].TXB[1], (PVOID)(hch.p + 3), hch.leng - 2);
						if (hch.cmd.a == HCMD_SCSEND) {
							PreSend(ch);
							if (ComaM())	TRACE(PSTR("Send %d byte to ch.%d\r\n"), hch.leng - 2, ch);
						}
					}
					else	TRACE(PSTR("Send data empty at ch.%d.\r\n"), ch);
					break;
				case HCMD_SCRREPORT :
					RecvReport(ch);
					break;
				default :
					break;
				}
			}
			else {
				if (CommM())	TRACE(PSTR("Ch.(%d)is out!\r\n"), ch);
			}
		}
		if (host.r.i > (hch.left + hch.leng + 1)) {
			Shift(host.r.s, hch.left + hch.leng + 1);
			host.r.i -= (hch.left + hch.leng + 1);
		}
		else	host.r.i = 0;
	}
}

HCH AbstractCmd(BYTE* p, BYTE leng, BOOL cmd)
{
	HCH hch;
	BYTE i;
	for (i = 0; i < leng; i ++) {
		hch.left = i;
		hch.p = &p[i];
		hch.leng = p[i];
		hch.cmd.a = p[i + 1];
		hch.cid = p[i + 2];
		if (hch.cmd.a > 0x4f)	continue;
		BYTE rlen = pgm_read_byte((WORD)&cmdLength[cmd ? 0 : 1][hch.cmd.b.h][hch.cmd.b.l]);
		if (rlen != 0xff && rlen != hch.leng)	continue;
		if (i + hch.leng > leng)	continue;
		if (hch.leng > 1 && HINIBB(hch.cid) != 5 && HINIBB(hch.cid) != 0xa)	continue;
		if (CommM())	TRACE(PSTR("Abstract result leng=%02X, cmd=%02X, cid=%02x\r\n"), hch.leng, hch.cmd.a, hch.cid);
		return hch;
	}
	hch.p = NULL;
	hch.leng = 0;
	if (CommM())	TRACE(PSTR("Abstract failure!(%d-%d)\r\n"), i, leng);
	return hch;
}

void Shift(BYTE* p, int sn)
{
	int n = 0;
	for ( ; sn < SIZE_ENVELOPE; n ++, sn ++)	p[n] = p[sn];
	for ( ; n < SIZE_ENVELOPE; n ++)	p[n] = 0;
}

void PrintIP(DWORD dwIp)
{
	NTRACE(PSTR("%d.%d.%d.%d"), (int)((dwIp >> 24) & 0xff), (int)((dwIp >> 16) & 0xff),
											(int)((dwIp >> 8) & 0xff), (int)(dwIp & 0xff));
	//sprintf((char*)host.t.s, "%d.%d.%d.%d", (int)((dwIp >> 24) & 0xff), (int)((dwIp >> 16) & 0xff),
	//										(int)((dwIp >> 8) & 0xff), (int)(dwIp & 0xff));
	//printf((char*)host.t.s);
}

void RespSock(BYTE cId)
{
	//sprintf((char*)host.t.s, "CH%02d OK!\r\n", cId);
	//if (cTesState & TESSTATE_CONNECTCLIENT)	SendSock(SOCK_SERVER, host.t.s, strlen((char*)host.t.s));
	//printf((char*)host.t.s);
}

void PreSend(BYTE cId)
{
	SCCCHS* pSccCh = GetSccBuf(cId);
	BYTE cTrace = 0;
	//if (pSccCh->RXB[0][3] == DEVTRACE_REQ)
	//	TRACE(PSTR("TRC %d %d\r\n"), pSccCh->cf.att.b.id, pSccCh->RXB[0][3]);
	if ((pSccCh->cf.att.b.id == SDID_SIV || pSccCh->cf.att.b.id == SDID_V3F) && pSccCh->RXB[0][3] == DEVTRACE_REQ) {
		WORD wPageMax;
		if (pSccCh->cf.att.b.id == SDID_SIV) {
			cTrace = cTraceLength & 0xf;
			wPageMax = 81;
		}
		else {
			cTrace = (cTraceLength >> 4) & 0xf;
			wPageMax = 81;
		}
		WORD wPageID = (((WORD)pSccCh->RXB[0][5] << 8) & 0xff00) | ((WORD)pSccCh->RXB[0][6] & 0xff);
		if (cTrace > 0 && pSccCh->RXB[0][4] < cTrace && wPageID < wPageMax) {
			pSccCh->TXB[0][0] = pSccCh->cf.addr.c[0];
			pSccCh->TXB[0][1] = pSccCh->cf.addr.c[1];
			pSccCh->TXB[0][2] = DEVCHAR_CTL;
			pSccCh->TXB[0][3] = DEVTRACE_ANS;
			pSccCh->TXB[0][4] = pSccCh->RXB[0][4];	// trace chapter
			pSccCh->TXB[0][5] = pSccCh->RXB[0][5];	// trace page
			pSccCh->TXB[0][6] = pSccCh->RXB[0][6];
			for (BYTE n = 0; n < 50; n ++) {
				WORD w = wPageID * 50 + n;
				pSccCh->TXB[0][n * 2 + 7] = (BYTE)(w & 0xff);
				pSccCh->TXB[0][n * 2 + 8] = (BYTE)((w >> 8) & 0xff);
			}
			pSccCh->TXR = 107;
			SendScc(cId);
		}
		else	cTrace = 0;
	}

	if (cTrace == 0) {
		pSccCh->TXR = GetSendLeng(pSccCh->cf.att.b.id);
		if (pSccCh->cf.att.b.id == SDID_ATO)
			memcpy(&pSccCh->TXB[0][2], &pSccCh->TXB[1][2], SIZE_SCCBUF - 2);
		else if (pSccCh->cf.att.b.id == SDID_SIV)	memcpy(pSccCh->TXB[0], cMultiBuf, SIZE_SCCBUF);
		else if (pSccCh->cf.att.b.id == SDID_V3F)	memcpy(pSccCh->TXB[0], cMultiBuf, SIZE_SCCBUF);
		else	memcpy(pSccCh->TXB[0], pSccCh->TXB[1], SIZE_SCCBUF);
		// TXB[0]는 인터럽트에 의해 출력되므로 Socket에 의해 바뀌어선 안된다.

		if (pSccCh->cf.att.b.id == SDID_ATO && !pSccCh->cf.tp.b.cci) {
			++ pSccCh->TXB[0][0];
			if (memcmp(cMultiBuf, &pSccCh->TXB[0][2], pSccCh->TXR - 4))	++ pSccCh->TXB[0][1];
			memcpy(cMultiBuf, &pSccCh->TXB[0][2], pSccCh->TXR - 4);
		}

		if (pSccCh->cf.tp.b.mode == SCCMODE_ASYNCCRC) {
			BYTE leng = DleEncoder(pSccCh->TXB[0], pSccCh->TXR - 2);
			memcpy(pSccCh->TXB[0], cDleBuf, leng);
			pSccCh->TXR = leng;
		}
		else {
			if (pSccCh->cf.att.b.id == SDID_DCUL || pSccCh->cf.att.b.id == SDID_DCUR) {
				pSccCh->TXB[0][0] = pSccCh->cf.addr.c[0];
				pSccCh->TXB[0][1] = pSccCh->RXB[0][1];
			}
			else {
				pSccCh->TXB[0][0] = pSccCh->cf.addr.c[0];
				pSccCh->TXB[0][1] = pSccCh->cf.addr.c[1];
			}
			pSccCh->TXB[0][2] = DEVCHAR_CTL;
			pSccCh->TXB[0][3] = DEVFLOW_ANS;
		}
		SendScc(cId);
	}
}

void CidReport(void)
{
	local.t.s[0] = 2;
	local.t.s[1] = 2;
	local.t.s[2] = cCid;
	PutsLB(local.t.s, 3);
	if (SockM())	TRACE(PSTR("\tCID is %02u %02u.\r\n"), CTYPE(), CADDR());
}

void StatusReport(BYTE ch, BYTE cmd)
{
	host.t.s[0] = 6;
	host.t.s[1] = cmd;
	host.t.s[2] = 0xa0 | ch;
	host.t.s[3] = lm.scc.pt[ch].cf.addr.c[0];
	host.t.s[4] = lm.scc.pt[ch].cf.addr.c[1];
	host.t.s[5] = lm.scc.pt[ch].cf.att.a;
	host.t.s[6] = lm.scc.pt[ch].cf.tp.a;
	if (cTesState & TESSTATE_CONNECTCLIENT)	SendSock(SOCK_SERVER, host.t.s, 7);
}

void RecvReport(BYTE ch)
{
	memcpy((PVOID)&host.t.s[3], (PVOID)lm.scc.pt[ch].RXB[0], lm.scc.pt[ch].RXI);
	host.t.s[0] = lm.scc.pt[ch].RXI + 2;
	host.t.s[1] = HCMD_SCRREPORT;
	host.t.s[2] = 0xa0 | ch;
	if (cTesState & TESSTATE_CONNECTCLIENT)	SendSock(SOCK_SERVER, host.t.s, (WORD)(host.t.s[0] + 1));
}

void InpReport(void)
{
	local.t.s[0] = 5;
	local.t.s[1] = 0x21;
	local.t.s[2] = 0xa0 | LONIBB(cCid);
	local.t.s[3] = lm.inp.uVal[2];
	local.t.s[4] = lm.inp.uVal[1];
	local.t.s[5] = lm.inp.uVal[0];
	PutsLB(local.t.s, 6);
	for (int n = 0; n < 3; n ++)	lm.inp.uVrf[n] = lm.inp.uVal[n];
}

void InpNullReport(void)
{
	local.t.s[0] = 2;
	local.t.s[1] = 0x22;
	local.t.s[2] = 0xa0 | LONIBB(cCid);
	PutsLB(local.t.s, 3);
}

void Output(void)
{
	PORTA = lm.outp.uVal[0];
	PORTC = lm.outp.uVal[1];
	BYTE c = lm.outp.uVal[2];
	PORTB = (PINB & 0xf) | (c & 0xf0);
	PORTE = (PINE & 0xf) | ((c << 4) & 0xf0);
}

void OutpReport(void)
{
	local.t.s[0] = 5;
	local.t.s[1] = 0x31;
	local.t.s[2] = 0xa0 | LONIBB(cCid);
	local.t.s[3] = lm.outp.uVal[2];
	local.t.s[4] = lm.outp.uVal[1];
	local.t.s[5] = lm.outp.uVal[0];
	PutsLB(local.t.s, 6);
	for (int n = 0; n < 3; n ++)	lm.outp.uVrf[n] = lm.outp.uVal[n];
}

void OutpNullReport(void)
{
	local.t.s[0] = 2;
	local.t.s[1] = 0x32;
	local.t.s[2] = 0xa0 | LONIBB(cCid);
	PutsLB(local.t.s, 3);
}

void PwmCtrl(BYTE ch)
{
	if (ch < 4) {
		//cli();
		if (ch == 0) {
			ICR1 = lm.pwm.wVal[ch];
			OCR1A = lm.pwm.wVal[ch] >> 1;
			TCCR1B = DEF_TCCR1B | (1 << CS12);
		}
		else {
			WORD v = (WORD)((DWORD)4608 * (DWORD)lm.pwm.wVal[ch] / (DWORD)1000);
			switch (ch) {
			case 1 :	OCR3C = v;	break;
			case 2 :	OCR3A = v;	break;
			default :	OCR3B = v;	break;
			}
			TCCR3B = DEF_TCCR3B | (1 << CS31);
		}
		//sei();
		RespSock(0);
	}
}

void PwmReport(BYTE ch)
{
	local.t.s[0] = 5;
	local.t.s[1] = 0x41;
	local.t.s[2] = 0xa0 | LONIBB(cCid);
	local.t.s[3] = ch;
	local.t.s[4] = (BYTE)(lm.pwm.wVal[ch] >> 8);
	local.t.s[5] = (BYTE)(lm.pwm.wVal[ch] & 0xff);
	PutsLB(local.t.s, 6);
	lm.pwm.wVrf[ch] = lm.pwm.wVal[ch];
}

void PwmNullReport(BYTE ch)
{
	local.t.s[0] = 3;
	local.t.s[1] = 0x42;
	local.t.s[2] = 0xa0 | LONIBB(cCid);
	local.t.s[3] = ch;
	PutsLB(local.t.s, 4);
}

void InvalidCommand(BYTE* p)
{
	TRACE(PSTR("\t%02X Invalid command!.\r"), cCid);
}

BYTE DleEncoder(BYTE* pSrc, BYTE cLength)
{
	BYTE* pDest = &cDleBuf[2];
	BYTE append = 0;
	for (BYTE c = 0; c < cLength; c ++) {
		if (*pSrc == 0x10) {
			*pDest ++ = 0x10;
			++ append;
		}
		*pDest ++ = *pSrc ++;
	}
	cDleBuf[0] = 0x10;
	cDleBuf[1] = 2;
	*pDest ++ = 0x10;
	*pDest ++ = 3;
	cLength += (append + 4);
	WORD crc;
	if (cCrcType == 1)	crc = Crc16ffff(cDleBuf, cLength);
	else	crc = Crc161d0f(cDleBuf, cLength);
	*pDest ++ = HIBYTE(crc);
	*pDest = LOBYTE(crc);
	return cLength + 2;
}

BYTE DleDecoder(BYTE* pDest, BYTE cLength)
{
	BYTE* pBkup = pDest;	// for calc crc...
	BOOL bDle = FALSE;
	int bi = -1;
	WORD crc;
	//TRACE(PSTR("DD %d\r\n"), cLength);
	for (BYTE c = 0; c < cLength; c ++) {
		BYTE ch = *pDest;
		if (bDle && (ch == 2 || ch == 3 || ch == 0x10)) {
			switch (ch) {
			case 2 :
				if (bi < 0)	bi = 0;
				else {
					//TRACE(PSTR("DDE %d\r\n"), bi);
					return 1;
				}
				break;
			case 3 :
				if (bi != 11)	return 3;	// without crc
				crc = Crc16ffff(pBkup, (c + 1));
				if (HIBYTE(crc) == *(pDest + 1) && LOBYTE(crc) == *(pDest + 2)) {
					cCrcType = 1;
					return 0;
				}
				crc = Crc161d0f(pBkup, (c + 1));
				if (HIBYTE(crc) == *(pDest + 1) && LOBYTE(crc) == *(pDest + 2)) {
					cCrcType = 0;
					return 0;
				}
				//TRACE(PSTR("DDE %d %04X %02X%02X\r\n"), c, crc, *(pDest + 1), *(pDest + 2));
				return 4;
				break;
			default :
				if (bi < 0)	return 2;
				cDleBuf[bi ++] = ch;
				break;
			}
			bDle = FALSE;
		}
		else if (!bDle && ch == 0x10)	bDle = TRUE;
		else {
			if (bi < 0)	return 2;
			cDleBuf[bi ++] = ch;
		}
		++ pDest;
		if (bi > SIZE_DLEBUF)	return 5;
	}
	return 6;
}

void PutsLB(BYTE *p, BYTE leng)
{
	while (local.cWaitResp > 0);
	PORTB |= 1;		// dir to tx
	while (leng > 0) {
		PutCharLB(*p ++);
		-- leng;
	}
	while (!(UCSR1A & (1 << TXC1)));
	for (WORD w = 0; w < 300; w ++);	// less than 1ms
	PORTB &= 0xfe;
}

void PutCharLB(BYTE c)
{
	while (!(UCSR1A & (1 << UDRE1)));
	UDR1 = c;
}

static int PutChar(char c, FILE* stream)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
	return 0;
}

void SendByte(BYTE c)
{
	SendChar(ctoa(c >> 4));
	SendChar(ctoa(c & 0xf));
}

void SendChar(BYTE c)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

BYTE ctoa(BYTE c)
{
	c += '0';
	if (c > '9')	c += 7;
	return c;
}

BYTE Turnup(BYTE c)
{
	BYTE tc = 0;
	if (c & 1)	tc |= 0x80;
	if (c & 2)	tc |= 0x40;
	if (c & 4)	tc |= 0x20;
	if (c & 8)	tc |= 0x10;
	if (c & 0x10)	tc |= 8;
	if (c & 0x20)	tc |= 4;
	if (c & 0x40)	tc |= 2;
	if (c & 0x80)	tc |= 1;
	return tc;
}

void Delay(WORD delay)
{
	cli();
	wDelay = delay;
	sei();
	while (wDelay > 0);
}

void InitialDev(void)
{
	if (CTYPE() == CTYPE_TRAFFIC || CTYPE() == CTYPE_COMM) {
		MCUCR = 0xc0;
		XMCRA = 0x48;
	}
	else if (CTYPE() == CTYPE_INP) {
		MCUCR = 0;
		PORTA = PORTC = 0xff;
		DDRA = DDRC = 0;
		PORTB = 0xfe;
		DDRB = 0xf;
		PORTE = 0xff;
		DDRE = 6;
	}
	else if (CTYPE() == CTYPE_OUTP) {
		MCUCR = 0;
		PORTA = PORTC = 0;
		DDRA = DDRC = 0xff;
		PORTB = 0xe;
		DDRB = 0xff;
		PORTE = 0xf;
		DDRE = 0xf6;
	}
	else {
		MCUCR = 0;
		PORTA = 0;
		DDRA = 0xff;
		PORTB = 0xe;
		DDRB = 0xff;
		PORTE = 0xc6;
		DDRE = 0xfe;
	}

	UBRR0H = HIBYTE(BPS115200);
	UBRR0L = LOBYTE(BPS115200);
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	if (CTYPE() != CTYPE_COMM) {
		UBRR1H = HIBYTE(BPS115200);
		UBRR1L = LOBYTE(BPS115200);
		UCSR1B = (1 << RXCIE1) | (1 << RXEN1) | (1 << TXEN1);
		UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
	}

	EIMSK = 0;
	if (CTYPE() == CTYPE_TRAFFIC || CTYPE() == CTYPE_COMM) {
		// falling edge external int.
		EICRA = (1 << ISC31) | (1 << ISC21) | (1 << ISC11) | (1 << ISC01);
		EICRB = (1 << ISC71) | (1 << ISC61) | (1 << ISC51) | (1 << ISC41);
		if (CTYPE() == CTYPE_COMM) {
			EIMSK  = (1 << INT7) | (1 << INT6) | (1 << INT5) | (1 << INT4) | (1 << INT0);
			EIFR  = (1 << INTF7) | (1 << INTF6) | (1 << INTF5) | (1 << INTF4) | (1 << INTF0);
		}
		else {
			EIMSK = (1 << INT0);
			EIFR = (1 << INTF0);
		}
	}
	else {
		if (CTYPE() == CTYPE_PWM) {
			TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1 << COM1B0) |
					(1 << COM1C1) | (1 << COM1C0) | (1 << WGM11);
			TCCR1B = DEF_TCCR1B | (1 << CS12);
			TCCR3A = (1 << COM3A1) | (1 << COM3A0) | (1 << COM3B1) | (1 << COM3B0) |
					(1 << COM3C1) | (1 << COM3C0) | (1 << WGM31);
			TCCR3B = DEF_TCCR3B | (1 << CS31);
			// tacho spec = 100p/1turn = 2.7M(860mm)/1turn = 27mm/1p
			// 100Km/h = 27.78m/s = 1028p/s = 1028Hz = 0.972ms
			// 1 / 18432000 * 256 * 65535 = 0.91s = 1.098Hz = 106.8m/h
			// 1 / 18432000 * 256 * 70 = 0.972ms = 1028.8Hz, 65535 ~ 70
			// tacho spec. = 90p/1turn = 2.7M(860mm)/1turn = 30mm/1p
			// 100Km/h = 27.778m/s = 926p/s = 926Hz = 1.080ms
			// 1.08 * 72(1 / 18432000 * 256) = 77.76
			// 926 * 3.1415 * 860 / 90 = 27797mm/s = 27.797m/s -> * 3600 = 100.0692Km/h
			TCNT1 = 0;
			ICR1 = 0;
			OCR1A = OCR1B = OCR1C = 0;

			TCNT3 = 0;
			ICR3 = 4608;			// 1 / 18432000 * 8 * 4608 = 0.002s = 500Hz
			OCR3A = OCR3B = OCR3C = 0;
		}
	}
}

void Monit(void)
{
	monc.r.s[monc.r.i] = 0;
	if (!strcmp_P((char*)monc.r.s, PSTR("~RESET\n"))) {
		printf_P(PSTR("RESET...\r\n"));
		while(1);	// wait reset by watchdog
	}
	else if (monc.r.s[0] == '~' && (monc.r.s[1] == 'F' || monc.r.s[1] == 'f') &&
		(monc.r.s[2] >= '0' && monc.r.s[2] < '4') && monc.r.s[3] == '\r') {
		BYTE n = monc.r.s[2] - '0';
		//printf_P(PSTR("fl#%d, enf=%02X\r\n"), n, cEnf);
		printf_P(PSTR("fl#%d\r\n"), n);
		printf_P(PSTR("%d-"), fl[n].i);
		for (BYTE m = 0; m < 64; m ++)
			printf_P(PSTR("%02X "), fl[n].b[m]);
		printf_P(PSTR("\r\n"));
	}
	else if (monc.r.s[0] == '~' && (monc.r.s[1] == 'I' || monc.r.s[1] == 'i') &&
		(monc.r.s[2] == 'S' || monc.r.s[2] == 's') && monc.r.s[3] == '\r') {
		BYTE eim = EIMSK;
		BYTE eif = EIFR;
		BYTE eia = EICRA;
		BYTE eib = EICRB;
		printf_P(PSTR("EIMSK=%02X, EIFR=%02X, EICRA=%02X, EICRB=%02X\r\n"), eim, eif, eia, eib);
	}
	else if (monc.r.s[0] == '~' && (monc.r.s[1] == 'I' || monc.r.s[1] == 'i') &&
		(monc.r.s[2] >= '0' && monc.r.s[2] < '4') && monc.r.s[3] == '\r') {
		BYTE id = monc.r.s[2] - '0';
		printf_P(PSTR("Initial Scc #%d\r\n"), id);
		InitialScc(id);
	}
	else if (monc.r.s[0] == '~' && (monc.r.s[1] == 'S' || monc.r.s[1] == 's') &&
		(monc.r.s[2] == 'T' || monc.r.s[2] == 't') && monc.r.s[3] == '\r') {
		printf_P(PSTR("state %02X\r\n"), lm.scc.cState);
	}
	else	printf_P(PSTR("?\r\n"));
}

// ===== ISR =====
ISR(TIMER0_OVF_vect)
{
	ReloadTimer();
	//T1FALL();
	if (!(cTesState & TESSTATE_AWAKE)) {	// capture card address when wakeup
		lm.inp.uCur[0] = ((~PING << 4) & 0x80) | (~PIND & 0x70) | (PINF & 0xf);
		if (lm.inp.uCur[0] == lm.inp.uBuf[0]) {
			if (-- lm.inp.uDeb[0] == 0) {
				cCid = ((lm.inp.uBuf[0] >> 4) & 0xf) | ((lm.inp.uBuf[0] << 4) & 0xf0);
				cTesState |= TESSTATE_AWAKE;
			}
		}
		else {
			lm.inp.uBuf[0] = lm.inp.uCur[0];
			lm.inp.uDeb[0] = DEB_ADDR;
		}
	}
	else {		// normal timer interrupt
		if (local.cWaitResp > 0)	-- local.cWaitResp;
		if (wDelay > 0)	-- wDelay;

		if (CTYPE() == CTYPE_COMM)	ScanScc();
		else if (CTYPE() == CTYPE_INP) {
			lm.inp.uCur[0] = Turnup(~PINA);
			lm.inp.uCur[1] = Turnup(~PINC);
			lm.inp.uCur[2] = Turnup(((~PINE & 0xf0) >> 4) | (~PINB & 0xf0));
			for (BYTE n = 0; n < 3; n ++) {
				if (lm.inp.uCur[n] != lm.inp.uBuf[n]) {
					lm.inp.uBuf[n] = lm.inp.uCur[n];
					lm.inp.uDeb[n] = DEB_INP;
				}
				else if (lm.inp.uDeb[n] > 0 && -- lm.inp.uDeb[n] == 0)
					lm.inp.uVal[n] = lm.inp.uBuf[n];
			}
		}
	}
	if (++ time.ms >= 1000) {
		time.ms = 0;
		if (++ time.sec >= 60) {
			time.sec = 0;
			if (++ time.min >= 60) {
				time.min = 0;
				++ time.hour;
			}
		}
	}
	if (-- cTick == 0) {
		cTick = TIME_TICK;
		PORTG ^= 0x10;
		if (-- cTock == 0) {
			cTock = TIME_TOCK;
		}
	}
	//T1RISE();
}

ISR(USART0_RX_vect)
{
	BYTE ch = UDR0;
	if (ch == '\r' || ch == '\n') { SendChar('\r'); SendChar('\n'); }
	else	SendChar(ch);

	if (monc.r.i == 0) {
		if (ch == '~')	monc.r.s[monc.r.i ++] = ch;
	}
	else {
		if (monc.r.i < SIZE_ENVELOPE) {
			monc.r.s[monc.r.i ++] = ch;
			if (ch == '\r' || ch == '\n')
				cTesState |= TESSTATE_MONIT;
		}
		else	monc.r.i = 0;
	}
}

ISR(USART1_RX_vect)
{
	BYTE ch = UDR1;
	if (local.r.i < SIZE_ENVELOPE) {
		local.r.s[local.r.i ++] = ch;
		local.cWaitResp = TIME_LOCALRXCONTINUE;
	}
}

ISR(INT0_vect)
{
	SockISR();
}

/*ISR(INT4_vect)
{
	SccISR(0);
}

ISR(INT5_vect)
{
	SccISR(1);
}

ISR(INT6_vect)
{
	SccISR(2);
}

ISR(INT7_vect)
{
	SccISR(3);
}
*/