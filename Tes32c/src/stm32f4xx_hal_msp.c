// stm32f4xx_hal_msp.c
#include "stm32f4xx_hal.h"
#include "Refer32.h"

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif

extern UARTHND	g_bus;
extern UARTHND	g_mon;

void HAL_MspInit(void)
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_MspDeInit(void)
{
}

void HAL_PPP_MspInit(void)
{
}

void HAL_PPP_MspDeInit(void)
{
}

static uint32_t fsmcInit = 0;

static void HAL_FSMC_MspInit(void)
{
	if (fsmcInit)	return;

	fsmcInit = 1;
	__HAL_RCC_FSMC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = GPIO_PIN_9;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOG, &gpio);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_SET);

	gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
			GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
			GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOE, &gpio);

	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
			GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOF, &gpio);

	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOG, &gpio);

	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 |
			GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
			GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOD, &gpio);

	printf("fsmc io config.\r\n");
}

void HAL_SRAM_MspInit(SRAM_HandleTypeDef* phSram)
{
	HAL_FSMC_MspInit();
}

static uint32_t fsmcDeinit = 0;

static void HAL_FSMC_MspDeInit(void)
{
	if (fsmcDeinit)	return;

	fsmcDeinit = 1;
	__HAL_RCC_FSMC_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 |
		GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
			GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
			GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
			GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
			GPIO_PIN_9);
}

void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* phSram)
{
	HAL_FSMC_MspDeInit();
}

//#define	GPIO_SPI_METHODA
void HAL_SPI_MspInit(SPI_HandleTypeDef* phSpi)
{
	if (phSpi->Instance == SPI1) {
		GPIO_InitTypeDef gpio = { 0 };
		gpio.Pin = GPIO_PIN_4;
		gpio.Mode = GPIO_MODE_OUTPUT_PP;
		gpio.Pull = GPIO_NOPULL;
		gpio.Speed = GPIO_SPEED_HIGH;
		HAL_GPIO_Init(GPIOA, &gpio);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);

		gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
		gpio.Mode = GPIO_MODE_AF_PP;
		gpio.Pull = GPIO_NOPULL;
		gpio.Speed = GPIO_SPEED_HIGH;
		gpio.Alternate = GPIO_AF5_SPI1;
		HAL_GPIO_Init(GPIOA, &gpio);
	}
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* phSpi)
{
	if (phSpi->Instance == SPI1) {
		__HAL_RCC_SPI1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
		__HAL_SPI_DISABLE(phSpi);
	}
}

void HAL_UART_MspInit(UART_HandleTypeDef* phUart)
{
	if (phUart->Instance == USART1) {
		__HAL_RCC_GPIOA_CLK_ENABLE();
		GPIO_InitTypeDef gpio = { 0 };
		gpio.Pin = GPIO_PIN_9 | GPIO_PIN_10;
		gpio.Mode = GPIO_MODE_AF_PP;
		gpio.Pull = GPIO_NOPULL;
		gpio.Speed = GPIO_SPEED_HIGH;
		gpio.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &gpio);

		__HAL_RCC_DMA2_CLK_ENABLE();
		g_bus.hd.Instance = DMA2_Stream2;
		g_bus.hd.Init.Channel = DMA_CHANNEL_4;
		g_bus.hd.Init.Direction = DMA_PERIPH_TO_MEMORY;
		g_bus.hd.Init.PeriphInc = DMA_PINC_DISABLE;
		g_bus.hd.Init.MemInc = DMA_MINC_DISABLE;
		g_bus.hd.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		g_bus.hd.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		g_bus.hd.Init.Mode = DMA_CIRCULAR;
		g_bus.hd.Init.Priority = DMA_PRIORITY_LOW;
		g_bus.hd.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&g_bus.hd);

		__HAL_LINKDMA(phUart, hdmarx, g_bus.hd);

		HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, UART_PRIORITY, UART_RX_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
	}
	else if (phUart->Instance == UART4) {
		__HAL_RCC_GPIOC_CLK_ENABLE();
		GPIO_InitTypeDef gpio = { 0 };
		gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		gpio.Mode = GPIO_MODE_AF_PP;
		gpio.Pull = GPIO_NOPULL;
		gpio.Speed = GPIO_SPEED_HIGH;
		gpio.Alternate = GPIO_AF8_UART4;
		HAL_GPIO_Init(GPIOC, &gpio);

		__HAL_RCC_DMA1_CLK_ENABLE();
		g_mon.hd.Instance = DMA1_Stream2;
		g_mon.hd.Init.Channel = DMA_CHANNEL_4;
		g_mon.hd.Init.Direction = DMA_PERIPH_TO_MEMORY;
		g_mon.hd.Init.PeriphInc = DMA_PINC_DISABLE;
		g_mon.hd.Init.MemInc = DMA_MINC_DISABLE;
		g_mon.hd.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		g_mon.hd.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		g_mon.hd.Init.Mode = DMA_CIRCULAR;
		g_mon.hd.Init.Priority = DMA_PRIORITY_LOW;
		g_mon.hd.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&g_mon.hd);

		__HAL_LINKDMA(phUart, hdmarx, g_mon.hd);

		HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, UART_PRIORITY, UART_RX_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* phUart)
{
	if (phUart->Instance == USART1) {
		__HAL_RCC_USART1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
		HAL_DMA_DeInit(phUart->hdmarx);
		HAL_DMA_DeInit(phUart->hdmatx);
	}
	else if (phUart->Instance == UART4) {
		__HAL_RCC_UART5_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11);
		HAL_DMA_DeInit(phUart->hdmarx);
		HAL_DMA_DeInit(phUart->hdmatx);
	}
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* phTim)
{
	GPIO_InitTypeDef gpio = { 0 };
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_HIGH;
	if (phTim->Instance == TIM1) {
		__HAL_RCC_TIM1_CLK_ENABLE();
		__HAL_RCC_GPIOE_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
		gpio.Alternate = GPIO_AF1_TIM1;
		HAL_GPIO_Init(GPIOE, &gpio);
	}
	else if (phTim->Instance == TIM4) {
		__HAL_RCC_TIM4_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
		gpio.Alternate = GPIO_AF2_TIM4;
		HAL_GPIO_Init(GPIOD, &gpio);
	}
	else if (phTim->Instance == TIM8) {
		__HAL_RCC_TIM8_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
		gpio.Alternate = GPIO_AF3_TIM8;
		HAL_GPIO_Init(GPIOB, &gpio);
	}
	else if (phTim->Instance == TIM10) {
		__HAL_RCC_TIM10_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_6;
		gpio.Alternate = GPIO_AF3_TIM10;
		HAL_GPIO_Init(GPIOF, &gpio);
	}
	else if (phTim->Instance == TIM11) {
		__HAL_RCC_TIM11_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_7;
		gpio.Alternate = GPIO_AF3_TIM11;
		HAL_GPIO_Init(GPIOF, &gpio);
	}
	else if (phTim->Instance == TIM13) {
		__HAL_RCC_TIM13_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_8;
		gpio.Alternate = GPIO_AF9_TIM13;
		HAL_GPIO_Init(GPIOF, &gpio);
	}
	else if (phTim->Instance == TIM14) {
		__HAL_RCC_TIM14_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_9;
		gpio.Alternate = GPIO_AF9_TIM14;
		HAL_GPIO_Init(GPIOF, &gpio);
	}
	else {
		printf("HAL_TIM_PWM_MspInit() wrong argument!!!\r\n");
	}
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *phTim)
{
	GPIO_InitTypeDef gpio = { 0 };
	//gpio.Mode = GPIO_MODE_AF_PP;
	//gpio.Pull = GPIO_NOPULL;
	//gpio.Speed = GPIO_SPEED_FREQ_LOW;
	if (phTim->Instance == TIM9) {
		__HAL_RCC_TIM9_CLK_ENABLE();
		__HAL_RCC_GPIOE_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_5;
		gpio.Mode = GPIO_MODE_AF_PP;
		gpio.Pull = GPIO_NOPULL;
		gpio.Speed = GPIO_SPEED_FREQ_LOW;
		gpio.Alternate = GPIO_AF3_TIM9;
		HAL_GPIO_Init(GPIOE, &gpio);

		HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, PWNIN_PRIORITY, PWMIN_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
	}
	else if (phTim->Instance == TIM1) {
		__HAL_RCC_TIM1_CLK_ENABLE();
		__HAL_RCC_GPIOE_CLK_ENABLE();
		gpio.Pin = GPIO_PIN_9;
		gpio.Mode = GPIO_MODE_AF_PP;
		gpio.Pull = GPIO_NOPULL;
		gpio.Speed = GPIO_SPEED_FREQ_LOW;
		gpio.Alternate = GPIO_AF1_TIM1;
		HAL_GPIO_Init(GPIOE, &gpio);

		HAL_NVIC_SetPriority(TIM1_CC_IRQn, PWNIN_PRIORITY, PWMIN_SUBPRIORITY);
		HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
	}
	else {
		printf("HAL_TIM_Base_MspInit() wrong argument!!!\r\n");
	}
}

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif
