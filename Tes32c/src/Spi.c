// Spi.c
#include <string.h>
#include <Uart.h>
#include <Gpio.h>
#include "stm32f4xx_hal.h"
#include "Ethernet/WizConfc.h"
#include "Ethernet/W5500/W5500c.h"

#define EMPTY_SPI_FLAG(__HANDLE__, __FLAG__) ((((__HANDLE__)->Instance->SR) & (__FLAG__)) == 0)

SPI_HandleTypeDef g_hSpi1 = { 0 };

//const
wiz_NetInfo g_netInfo = {
		.mac =	{ 0x00, 0x08, 0xdc, 0x06, 0x03, 0x01 },
		.ip =	{ 193, 168, 0, 160 },
		.sn =	{ 255, 255, 255, 0 },
		.gw =	{ 193, 168, 0, 1 },
		.dns =	{ 8, 8, 8, 8},
		.dhcp =	NETINFO_STATIC
};

uint8_t SpiReadByte(void);
void SpiWriteByte(uint8_t wb);
void SpiSelect(void);
void SpiDeSelect(void);
//void SpiCrisEnter(void);
//void SpiCrisExit(void);

#define	SPICLKMETHODA

void InitialSpi1(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpio = { 0 };				// w5500 reset
	gpio.Pin = GPIO_PIN_14;	// | GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &gpio);
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);

	memset(&g_hSpi1, 0, sizeof(SPI_HandleTypeDef));
	__HAL_RCC_SPI1_CLK_ENABLE();
	g_hSpi1.Instance = SPI1;
	g_hSpi1.Init.Direction = SPI_DIRECTION_2LINES;
	g_hSpi1.Init.Mode = SPI_MODE_MASTER;
	g_hSpi1.Init.DataSize = SPI_DATASIZE_8BIT;
#if	defined(SPICLKMETHODA)
	g_hSpi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	g_hSpi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	g_hSpi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
#else
	g_hSpi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
	g_hSpi1.Init.CLKPhase = SPI_PHASE_2EDGE;
	g_hSpi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
#endif
	g_hSpi1.Init.NSS = SPI_NSS_SOFT;
	g_hSpi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	g_hSpi1.Init.TIMode = SPI_TIMODE_DISABLE;
	g_hSpi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	g_hSpi1.Init.CRCPolynomial = 7;
	HAL_StatusTypeDef status = { 0 };
	if ((status = HAL_SPI_Init(&g_hSpi1)) != HAL_OK)	printf("HAL_SPI_Init() failed!(%d)\r\n", status);
	__HAL_SPI_ENABLE(&g_hSpi1);
}

int InitialW5500(uint8_t iip)
{
	SpiDeSelect();
	//reg_wizchip_cris_cbfunc(SpiCrisEnter, SpiCrisExit);
	reg_wizchip_cs_cbfunc(SpiSelect, SpiDeSelect);
	reg_wizchip_spi_cbfunc(SpiReadByte, SpiWriteByte);

	if (WIZCHIP.if_mode != _WIZCHIP_IO_MODE_SPI_VDM_)
		printf("InitialW5500() wrong if mode!\r\n");
	uint8_t tmp = getVERSIONR();
	printf("w5500 version %02x\r\n", tmp);
	uint8_t sockSize[2][8] = {{ 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 }};
	if (ctlwizchip(CW_INIT_WIZCHIP, (void*)sockSize) != 0) {
		printf("ctrlwizchip(CW_INIT_WIZCHIP) failed!\r\n");
		return -1;
	}
	//wizchip_setnetinfo(&netInfo);

#if 0
	uint32_t tick = HAL_GetTick();
	tmp = 0;
	while ((tmp & PHY_LINK_ON) == 0) {
		if ((HAL_GetTick() - tick) > 2000) {
			printf("Ethernet link timeout (0x%02x)!\r\n", tmp);
			return -1;
		}
		tmp = getPHYCFGR();
	};
#endif
	g_netInfo.ip[3] = 130 + iip;
	g_netInfo.mac[5] += iip;
	ctlnetwork(CN_SET_NETINFO, (void*)&g_netInfo);

	return 0;
}

void NetConfig(void)
{
	wiz_NetInfo ni;
	memset((void*)&ni, 0, sizeof(wiz_NetInfo));
	ctlnetwork(CN_GET_NETINFO, (void*)&ni);
	printf("MAC: %02x-%02x-%02x-%02x-%02x-%02x\r\n", ni.mac[0], ni.mac[1], ni.mac[2], ni.mac[3], ni.mac[4], ni.mac[5]);
	printf("IP: %d.%d.%d.%d\r\n", ni.ip[0], ni.ip[1], ni.ip[2], ni.ip[3]);
	printf("GW: %d.%d.%d.%d\r\n", ni.gw[0], ni.gw[1], ni.gw[2], ni.gw[3]);
	printf("SN: %d.%d.%d.%d\r\n", ni.sn[0], ni.sn[1], ni.sn[2], ni.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", ni.dns[0], ni.dns[1], ni.dns[2], ni.dns[3]);
}

#define SPIMETHODA
#if	defined(SPIMETHODA)
uint8_t SpiReadByte(void)
{
	while (EMPTY_SPI_FLAG(&g_hSpi1, SPI_FLAG_TXE));
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	g_hSpi1.Instance->DR = 0xff;

	while (EMPTY_SPI_FLAG(&g_hSpi1, SPI_FLAG_RXNE));
	uint8_t rb = (uint8_t)g_hSpi1.Instance->DR;
	//printf("rx:%02x\r\n", rb);
	for (int n = 0; n < 10; n ++);
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	return rb;
}

void SpiWriteByte(uint8_t wb)
{
	while (EMPTY_SPI_FLAG(&g_hSpi1, SPI_FLAG_TXE));
	g_hSpi1.Instance->DR = wb;
	//printf("tx:%02x\r\n", wb);
	while (EMPTY_SPI_FLAG(&g_hSpi1, SPI_FLAG_RXNE));
	uint8_t rb = (uint8_t)g_hSpi1.Instance->DR;
	if (0)	printf("%02x", rb);
}

#else
uint8_t SpiReadByte(void)
{
	uint8_t wb = 0xff;	// dummy
	uint8_t rb;
	while (HAL_SPI_GetState(&hSpi1) == HAL_SPI_STATE_RESET);
	HAL_StatusTypeDef status = { 0 };
	if ((status = HAL_SPI_TransmitReceive(&hSpi1, &wb, &rb, 1, 100)) != HAL_OK)
		printf("SpiReadByte()->HAL_SPI_TransmitReceive() failed!(%d)\r\n", status);
	if (hSpi1.ErrorCode != HAL_SPI_ERROR_NONE)
		printf("SpiReadByte()->HAL_SPI_TransmitReceive() failedA!(%ld)\r\n", hSpi1.ErrorCode);
	//printf("spi read %02x\r\n", rb);
	return rb;
}

#define	SPIWRITEMETHODA
void SpiWriteByte(uint8_t wb)
{
	while (HAL_SPI_GetState(&hSpi1) == HAL_SPI_STATE_RESET);
	HAL_StatusTypeDef status = { 0 };
#if	defined(SPIWRITEMETHODA)
	uint8_t rb;
	if ((status = HAL_SPI_TransmitReceive(&hSpi1, &wb, &rb, 1, 100)) != HAL_OK)
		printf("SpiWriteByte()->HAL_SPI_TransmitReceive() failed!(%d)\r\n", status);
	if (hSpi1.ErrorCode != HAL_SPI_ERROR_NONE)
		printf("SpiWriteByte()->HAL_SPI_TransmitReceive() failedA!(%ld)\r\n", hSpi1.ErrorCode);
#else
	if ((status = HAL_SPI_Transmit(&hSpi1, &wb, 1, 100)) != HAL_OK)
		printf("SpiWriteByte()->HAL_SPI_Transmit() failed!(%d)\r\n", status);
	if (hSpi1.ErrorCode != HAL_SPI_ERROR_NONE)
		printf("SpiWriteByte()->HAL_SPI_Transmit() failedA!(%ld)\r\n", hSpi1.ErrorCode);
#endif
	//printf("spi write %02x\r\n", wb);
}
#endif

void SpiSelect(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}

void SpiDeSelect(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

//void SpiCrisEnter(void)
//{
//	__set_PRIMASK(1);
//}
//
//void SpiCrisExit(void)
//{
//	__set_PRIMASK(0);
//}
//
void Reset5500(void)
{
	HAL_Delay(50);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_Delay(50);
}
