// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Refer32.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "cmsis_device.h"
#include "cortexm/ExceptionHandlers.h"

#include "Gpio.h"
#include "Uart.h"
#include "Fpgam.h"
#include "Pwm.h"
#include "diag/Trace.h"

#include "HostCmd.h"
#include "BusCmd.h"

#define	VERSION					1.0f
#define TIMER_FREQUENCY_HZ		1000u

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//extern SRAM_HandleTypeDef g_hSram;

IDDRAW	g_id;

struct {
	WORD	wDelay;
	WORD	wTick;
	bool	bTick;
} g_timer;
#define	TIME_TICK		500

struct {
	BYTE	cMax;
	BYTE	cCur;
	BYTE	cScan;
} g_scanInps;

WORD	g_wProjNo;
LIMB	g_lm;
HOST	g_host;
//LOCAL	g_local;
WORD	g_wShape;

extern UARTHND	g_bus;
//extern UARTHND	g_uart4;

void Delayms(WORD w);
void SystemClock_Config(void);

int main(int argc, char* argv[])
{
	g_wProjNo = 1;		// line 7, 16 cabin
	memset(&g_id, 0, sizeof(g_id));
	memset(&g_timer, 0, sizeof(g_timer));
	memset(&g_scanInps, 0, sizeof(g_scanInps));
	memset(&g_lm, 0, sizeof(g_lm));
	memset(&g_host, 0, sizeof(g_host));
	g_wShape = (1 << SHAPE_DEBUGSOCKET);

	HAL_Init();
	SystemClock_Config();
	InitialGpio();
	InitialMon();

	SysTick_Config(SystemCoreClock / TIMER_FREQUENCY_HZ);

	while (!(g_wShape & (1 << SHAPE_AWAKE)));		// wait stabilize address & role

	switch (g_id.cur.role.a) {
	case ROLE_TR :
		printf("=== Tes32 traffic controller version %.1f", VERSION);
		break;
	case ROLE_SC :
		printf("=== Tes32 8ch. communication controller version %.1f", VERSION);
		break;
	case ROLE_DI :
		printf("=== Tes32 32ch. digital input controller version %.1f", VERSION);
		break;
	case ROLE_DO :
		printf("=== Tes32 32ch. digital output controller version %.1f", VERSION);
		break;
	case ROLE_PO :
		printf("=== Tes32 pwm output controller version %.1f", VERSION);
		break;
	case ROLE_PI :
		printf("=== Tes32 pwm input controller version %.1f", VERSION);
		break;
	default :
		while (true) {
			printf("??? unknown function selector ???\r\n");
			Delayms((WORD)1000);
		}
		break;
	}
	printf(" --- %s %s ===\r\n", __DATE__, __TIME__);
	printf("address %d.\r\n", g_id.cur.address.a);
	printf("SystemCoreClock %ld\r\n", SystemCoreClock);

	if (g_id.cur.role.a != ROLE_SC)	InitialBus();

	if (g_id.cur.role.a == ROLE_TR || g_id.cur.role.a == ROLE_SC) {
		BYTE iip = 0;
		if (g_id.cur.role.a == ROLE_SC)	iip = 3;
		InitialHost(iip + g_id.cur.address.a);
		if (g_id.cur.role.a == ROLE_SC) {
			InitialFpgam();
			InitialDefault();
		}
		else {
			g_wShape |= (1 << SHAPE_SCANINGINPS);
			g_scanInps.cMax = 32;	// default length for di
			g_scanInps.cCur = g_scanInps.cScan = 0;
		}
		printf("SHAPE 0x%04x\r\n", g_wShape);
	}
	else if (g_id.cur.role.a == ROLE_PO)	InitialPwmOut();
	else if (g_id.cur.role.a == ROLE_PI)	InitialPwmIn();
	else if (g_id.cur.role.a == ROLE_DO)	InitialD32Out();
	else	InitialD32In();		// di

	while (true) {
		if (g_id.cur.role.a == ROLE_TR || g_id.cur.role.a == ROLE_SC) {
			ScanHost();							// A1:
			if (g_host.r.i > 0)	ActionHost();	// A2:
			if (g_id.cur.role.a == ROLE_TR) {
				if (g_bus.buf.wait == 0) {
					while (g_bus.buf.r.i > 0) {	// bus.r -> host.t
						// A4:
						// TR:	g_bus.buf.r.s에 뭔가가 있다면 노드에서 온 응답이므로...
						BCHEAD bch = AbstractCmd(g_bus.buf.r.s, g_bus.buf.r.i, false);	// continue
						if (DEBUGTRBUS())	printf("A4-0 %d %d\r\n", g_bus.buf.r.i, bch.leng);
						if (bch.p != NULL) {
							if ((g_wShape & (1 << SHAPE_SCANINGINPS)) && bch.leng > 0 &&
								// A4-1:
								// TR:	초기에는 입력 보드의 수를 가늠하므로 cScan에 저장을 하고...
								(bch.cmd.a == BUSCMD_DIRREPORT || bch.cmd.a == BUSCMD_DICREPORT)) {
								g_scanInps.cScan = g_scanInps.cCur;		// cScan always has node no. that responds
							}
							else if (bch.leng > 0) {
								if (g_wShape & (1 << SHAPE_CONNECTCLIENT)) {
									if (DEBUGTRBUS())	printf("A4-2\r\n");
									// A4-2:
									// TR:	노드에서 온 각 프레임을 PC로 전송한다(g_host.t.s)-END
									if (bch.cmd.a != BUSCMD_DICREPORT && bch.cmd.a != BUSCMD_DOCREPORT &&
										bch.cmd.a != BUSCMD_POCREPORT && bch.cmd.a != BUSCMD_PICREPORT) {
										memcpy(g_host.t.s, bch.p, bch.leng + 1);
										DWORD dw = PrintSH(g_host.t.s, bch.leng + 1);	// host -> PC
										if (DEBUGSOCKET())	printf("send sock %02X %d(%ld).\r\n", bch.cmd.a, bch.leng, dw);
									}
								}
							}
							if (g_bus.buf.r.i > (bch.left + bch.leng + 1)) {
								ShiftCmd(g_bus.buf.r.s, bch.left + bch.leng + 1);
								if (g_bus.buf.r.i > (bch.left + bch.leng + 1))	g_bus.buf.r.i -= (bch.left + bch.leng + 1);
								else	g_bus.buf.r.i = 0;
							}
							else	g_bus.buf.r.i = 0;
						}
						else	g_bus.buf.r.i = 0;
						Delayms((WORD)2);
					}
					if (g_bus.buf.t.i > 0) {	// bus.t -> node
						// A3-1:
						// TR:	PC에서 온 명령이 g_bus.buf.t.s에 있다면 버스를 통해 노드로 전송한다. 전송후 g_bus.buf.t.i를 다음 명령으로 이동한다.
						int leng = (int)(g_bus.buf.t.s[0] + 1);
						PrintSB(g_bus.buf.t.s, leng);		// ROLE_TR
						g_bus.buf.wait = TIME_CYCLEOFF;
						if (DEBUGTRTX() && g_bus.buf.t.s[1] != BUSCMD_DICREPORT) {
							printf("TB");
							for (int n = 0; n < leng; n ++)
								printf(" %02x", g_bus.buf.t.s[n]);
							printf("\r\n");
						}
						ShiftCmd(g_bus.buf.t.s, leng);
						if (g_bus.buf.t.i > leng)	g_bus.buf.t.i -= leng;
						else	g_bus.buf.t.i = 0;
					}
					else if (g_scanInps.cMax > 0) {
						if (g_scanInps.cCur >= g_scanInps.cMax) {
							if (g_wShape & (1 << SHAPE_SCANINGINPS)) {
								// A3-2:
								// TR:	초기에는 입력 보드의 숫자를 모르기에 연결된 입력 보드의 수를 알기위해 max를 32로하고
								//		응답을 하든 말든 (주소)32까지 응답 요구를 보낸다. 응답이 있으면 cScan에는 응답이
								//		발생한 cCur로 업뎃하여 마지막에는 응답이 있는 보드의 마지막 주소가 남게된다.
								//		만일 입력 보드가 발견되지 않으면(응답이 없으면) cScan은 0이 되어 다음 사이클부터는
								//		입력 요구를 하지 않을 것이다.
								g_scanInps.cMax = g_scanInps.cScan;
								g_wShape &= ~(1 << SHAPE_SCANINGINPS);
								printf("DI length is %d.\r\n", (int)g_scanInps.cMax);
							}
							g_scanInps.cCur = 0;
						}
						if (g_scanInps.cMax > 0) {
							// A3-3:
							// TR:	PC에서 온 명령이 없다면 입력 요구를 하게된다(입력 보드가 있으면).
							//PrintFB(2, BUSCMD_DICREPORT, (BYTE)(0x40 | g_scanInps.cCur ++));
							//g_bus.buf.wait = TIME_CYCLEOFF;
							g_bus.buf.t.s[g_bus.buf.t.i ++] = 2;
							g_bus.buf.t.s[g_bus.buf.t.i ++] = BUSCMD_DICREPORT;
							g_bus.buf.t.s[g_bus.buf.t.i ++] = (BYTE)(0x40 | g_scanInps.cCur ++);
						}
					}
				}
			}
			else {	// ROLE_SC
				SccCtrl();
			}
		}
		else	DpioCtrl();	// ROLE_DI, ROLE_DO, ROLE_PI, ROLE_PO
	}
}

void Delayms(WORD w)
{
	g_timer.wDelay = w;
	while (g_timer.wDelay != 0)	-- g_timer.wDelay;
}

void SystemClock_Config(void)
{
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitTypeDef osci = { 0 };
	osci.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osci.HSEState = RCC_HSE_ON;
	osci.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	osci.PLL.PLLM = (HSE_VALUE/1000000u);
	osci.PLL.PLLN = 336;
	osci.PLL.PLLP = RCC_PLLP_DIV4;		//2; /* 168 MHz */
	osci.PLL.PLLQ = 7; /* To make USB work. */
	osci.PLL.PLLState = RCC_PLL_ON;
	HAL_RCC_OscConfig(&osci);

	RCC_ClkInitTypeDef clki = { 0 };
	clki.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	clki.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clki.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clki.APB1CLKDivider = RCC_HCLK_DIV4;
	clki.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&clki, FLASH_LATENCY_5);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	HAL_NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIORITY, SYSTICK_SUBPRIORITY);
}

void HAL_SYSTICK_Callback (void)
{
	if (!(g_wShape & (1 << SHAPE_AWAKE))) {
		PIXEL32 tmp;
		tmp.a = (DWORD)GPIOC->IDR;
		g_id.cur.role.b.b3 = tmp.b.b9;
		g_id.cur.role.b.b2 = tmp.b.b8;
		g_id.cur.role.b.b1 = tmp.b.b7;
		g_id.cur.role.b.b0 = tmp.b.b6;
		tmp.a = (DWORD)~GPIOB->IDR;
		g_id.cur.address.b.b4 = tmp.b.b15;
		g_id.cur.address.b.b3 = tmp.b.b14;
		g_id.cur.address.b.b2 = tmp.b.b10;
		tmp.a = (DWORD)~GPIOG->IDR;
		g_id.cur.address.b.b1 = tmp.b.b7;
		g_id.cur.address.b.b0 = tmp.b.b6;
		if (g_id.cur.role.a == g_id.buf.role.a && g_id.cur.address.a == g_id.buf.address.a) {
			if (-- g_id.deb == 0)	g_wShape |= (1 << SHAPE_AWAKE);
		}
		else {
			g_id.buf.role.a = g_id.cur.role.a;
			g_id.buf.address.a = g_id.cur.address.a;
			g_id.deb = DEB_IDENTITY;
		}
	}
	else {
		if (g_id.cur.role.a == ROLE_SC)	IntervalFpgam();
		else if (g_id.cur.role.a == ROLE_DI || g_id.cur.role.a == ROLE_DO)	IntervalGpio();
		else if (g_id.cur.role.a == ROLE_PI || g_id.cur.role.a == ROLE_PO)	IntervalPwm();
		IntervalUart();
	}
	if (g_timer.wDelay != 0)	-- g_timer.wDelay;
	if (++ g_timer.wTick >= 500) {
		g_timer.wTick = 0;		// watch dog trigger
		g_timer.bTick ^= true;
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, g_timer.bTick ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

#pragma GCC diagnostic pop
