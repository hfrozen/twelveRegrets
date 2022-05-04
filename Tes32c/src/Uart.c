// Uart.c
#include <string.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "Refer32.h"
#include "Fpgam.h"
#include "Uart.h"

UARTHND	g_bus;
UARTHND	g_mon;

int nCmdLength = 0;
BYTE cCmds[10][32];
BYTE cRepeat[SIZE_ENVELOPE];

extern IDDRAW	g_id;

const WORD crc16Table[256] = {
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

WORD Crc161d0f(BYTE* p, int nLeng)
{
	WORD crc = 0x1d0f;
	for (int n = 0; n < nLeng; n ++) {
		WORD ti = ((crc >> 8) ^ *p ++) & 0xff;
		crc = (crc << 8) ^ crc16Table[ti];
	}
	return crc;
}

WORD Crc16ffff(BYTE* p, int nLeng)
{
	WORD crc = 0xffff;
	for (int n = 0; n < nLeng; n ++) {
		crc ^= (unsigned short)*p ++ << 8;
		for (int m = 0; m < 8; m ++)
			crc = crc << 1 ^ (crc & 0x8000 ? 0x1021 : 0);
	}
	return crc;
}

void PrintFB(int leng, ...)
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
	PrintSB(buf, m);
}

void SplitBusCmd(BYTE* pCmd)
{
	memset(cCmds, 0, 10 * 32);
	nCmdLength = 0;
	int n = 0;
	bool bText = *pCmd == ' ' ? false : true;
	while (*pCmd != 0) {
		if (*pCmd != ' ') {
			bText = true;
			char ch = *pCmd;
			if (ch >= 0x41 && ch <= 0x5a)	ch |= 0x20;
			cCmds[nCmdLength][n ++] = ch;
		}
		else {
			if (bText) {
				bText = false;
				++ nCmdLength;
				n = 0;
			}
		}
		++ pCmd;
	}
	if (bText)	++ nCmdLength;
}

BYTE atoh(BYTE hex)
{
	if (hex < '0')	return 0;
	hex -= '0';
	if (hex > 9)	hex -= 7;
	return hex;
}

WORD strToHex(BYTE* pHex)
{
	WORD hex = 0;
	while (*pHex != 0) {
		hex <<= 4;
		hex |= (atoh(*pHex ++) & 0xf);
	}
	return hex;
}

void UserCmd(BYTE* pCmd)
{
	if (*pCmd == 'r' && *(pCmd + 1) == 0)
		memcpy(pCmd, cRepeat, SIZE_ENVELOPE);
	else	memcpy(cRepeat, pCmd, SIZE_ENVELOPE);

	SplitBusCmd(pCmd);
	if (nCmdLength >= 2 && !memcmp(cCmds[0], "fr", 2)) {
		WORD id = strToHex(cCmds[1]);
		if (id < 6) {
			printf("FPGA read test\r\n");
			uint16_t vert;
			while (1) {
				vert = PREG(id, PROFS_XLIVECODE);
			}
		}

	}
	else if (nCmdLength >= 2 && !memcmp(cCmds[0], "fw", 2)) {
		int id = strToHex(cCmds[1]);
		if (id < 6) {
			printf("FPGA write test\r\n");
			while (1) {
				PREG(id, PROFS_XLIVECODE) = 0xffff;
			}
		}
	}
	else if (nCmdLength >= 3 && !memcmp(cCmds[0], "fc", 2)) {
		int id = strToHex(cCmds[1]);
		if (id < 6) {
			WORD w = strToHex(cCmds[2]);
			PREG(id, PROFS_MANUALCTRL) = w;
			WORD v = PREG(id, PROFS_MANUALCTRL);
			printf("ch%d manual ctrl 0x%04x - 0x%04x\r\n", id, w, v);
		}
	}
	/*if (nCmdLength >= 1 && !memcmp(cCmds[0], "fi", 2)) {
		ResetFpgam();
		InitialPorts();
		printf("fpga initial ok.\r\n");
	}
	else if (nCmdLength >= 2 && !memcmp(cCmds[0], "fs", 2)) {
		WORD pn = strToHex(cCmds[1]);
		if (pn < 4) {
			SendPort(pn, (uint8_t*)g_testSend, SIZE_TESTSEND);
			printf("fpga test frame send.\r\n");
		}
		else	printf("fpga send - but port range over %d!\r\n", pn);
	}
	else if (nCmdLength >= 2 && !memcmp(cCmds[0], "fr", 2)) {
		WORD a16 = strToHex(cCmds[1]);
		if (a16 < 0x1000) {
			uint16_t d16 = DREG(a16);
			printf("fpga read 0x%04x = 0x%04x.\r\n", a16, d16);
		}
		else	printf("fpga read - but address range over 0x%04x!\r\n", a16);
	}
	else if (nCmdLength >= 3 && !memcmp(cCmds[0], "fw", 2)) {
		WORD a16 = strToHex(cCmds[1]);
		WORD d16 = strToHex(cCmds[2]);
		if (a16 < 0x1000) {
			DREG(a16) = d16;
			printf("fpga write 0x%04x = 0x%04x.\r\n", a16, d16);
		}
		else	printf("fpga write - but address range over 0x%04x!\r\n", a16);
	}
	else	printf("%s?\r\n", pCmd);*/
}

void InitialBus()
{
	memset(&g_bus.buf, 0, sizeof(TRXSTREAM));

	__GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpio = { 0 };
	// rts0		- PA12
	gpio.Pin = GPIO_PIN_12;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &gpio);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);	// low -> receive enable

	__HAL_RCC_USART1_CLK_ENABLE();
	g_bus.hu.Instance = USART1;
	g_bus.hu.Init.BaudRate = 115200;
	g_bus.hu.Init.WordLength = UART_WORDLENGTH_8B;
	g_bus.hu.Init.StopBits = UART_STOPBITS_1;
	g_bus.hu.Init.Parity = UART_PARITY_NONE;
	g_bus.hu.Init.Mode = UART_MODE_TX_RX;
	g_bus.hu.Init.HwFlowCtl = UART_HWCONTROL_RTS;
	g_bus.hu.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&g_bus.hu);
	//__HAL_UART_HWCONTROL_RTS_ENABLE(&g_bus.hu);

	__HAL_UART_FLUSH_DRREGISTER(&g_bus.hu);
	HAL_UART_Receive_DMA(&g_bus.hu, (uint8_t*)&g_bus.buf.c, 1);
	RtsCtrl(false);
}

void InitialMon()
{
	memset(&g_mon.buf, 0, sizeof(TRXSTREAM));
	__HAL_RCC_UART4_CLK_ENABLE();
	g_mon.hu.Instance = UART4;
	g_mon.hu.Init.BaudRate = 115200;
	g_mon.hu.Init.WordLength = UART_WORDLENGTH_8B;
	g_mon.hu.Init.StopBits = UART_STOPBITS_1;
	g_mon.hu.Init.Parity = UART_PARITY_NONE;
	g_mon.hu.Init.Mode = UART_MODE_TX_RX;
	g_mon.hu.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	g_mon.hu.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&g_mon.hu);

	__HAL_UART_FLUSH_DRREGISTER(&g_mon.hu);
	HAL_UART_Receive_DMA(&g_mon.hu, (uint8_t*)&g_mon.buf.c, 1);
}

void PrintSB(BYTE* p, int leng)
{
	RtsCtrl(true);
	for (int n = 0; n < 100; n ++);
	HAL_UART_Transmit(&g_bus.hu, (uint8_t*)p, leng, 5);
	while (!(g_bus.hu.Instance->SR & UART_FLAG_TC));
	for (int n = 0; n < 100; n ++);
	RtsCtrl(false);
}

void RtsCtrl(bool bState)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, bState ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//void PutB(char ch)
//{
//	HAL_UART_Transmit(&g_bus.hu, (uint8_t*)&ch, 1, 5);
//}
//
void PrintM(char* p)
{
	HAL_UART_Transmit(&g_mon.hu, (uint8_t*)p, strlen(p), 5);
}

void PutChar(char ch)
{
	HAL_UART_Transmit(&g_mon.hu, (uint8_t*)&ch, 1, 5);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* phUart)
{
	if (phUart->Instance == USART1) {
	}
	else if (phUart->Instance == UART4) {
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* phUart)
{
	if (phUart->Instance == USART1) {		// bus
		__HAL_UART_FLUSH_DRREGISTER(&g_bus.hu);
		// A4:
		// TR:	각 노드로 부터 오는 응답들을 g_bus.buf.r.s에 저장한다.
		//		응답 프레임의 끝을 알 수 없으므로 일정 시간(TIME_RXOFF)동안 전송이 없으면 프레임의 끝으로 본다.
		if (g_bus.buf.r.i < SIZE_ENVELOPE) {
			g_bus.buf.r.s[g_bus.buf.r.i ++] = g_bus.buf.c;
			if (g_id.cur.role.a != ROLE_TR && g_id.cur.role.a != ROLE_SC)
				g_bus.buf.wait = TIME_RXOFF;
		}
	}
	else if (phUart->Instance == UART4) {		// monitor
		__HAL_UART_FLUSH_DRREGISTER(phUart);
		PutChar(g_mon.buf.c);
		if (g_mon.buf.c == 8 && g_mon.buf.r.i > 0) {
			printf(" \b");
			-- g_mon.buf.r.i;
		}
		else if (g_mon.buf.c == '\n' || g_mon.buf.c == '\r') {
			printf("\r\n");
			g_mon.buf.r.s[g_mon.buf.r.i] = 0;
			UserCmd(g_mon.buf.r.s);
			g_mon.buf.r.i = 0;
		}
		else {
			g_mon.buf.r.s[g_mon.buf.r.i ++] = g_mon.buf.c;
			if (g_mon.buf.r.i > (SIZE_ENVELOPE - 4))	g_mon.buf.r.i = 0;
		}
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* phUart)
{
	if (phUart->Instance == USART1)	PrintM("[ERR] usart1!\r\n");
	else if (phUart->Instance == UART4)	PrintM("[ERR] uart4!\r\n");
	else	PrintM("[ERR] uart?\r\n");
}

void IntervalUart()
{
	if (g_bus.buf.wait != 0)	-- g_bus.buf.wait;
	if (g_mon.buf.wait != 0)	-- g_mon.buf.wait;
}
