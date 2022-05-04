// stm32f4xx_it.c
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "Refer32.h"

extern UARTHND	g_bus;
extern UARTHND	g_mon;
extern TIM_HandleTypeDef	g_hTims[PWMOUT_LENGTH];
int		g_aux[2] = { 0 };

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void DMA2_Stream2_IRQHandler(void)
{
	HAL_NVIC_ClearPendingIRQ(DMA2_Stream2_IRQn);
	HAL_DMA_IRQHandler(&g_bus.hd);
}

void DMA1_Stream2_IRQHandler(void)
{
	HAL_NVIC_ClearPendingIRQ(DMA1_Stream2_IRQn);
	HAL_DMA_IRQHandler(&g_mon.hd);
}

void TIM1_BRK_TIM9_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&g_hTims[0]);
}

void TIM1_CC_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&g_hTims[1]);
}

void EXTI0_IRQHandler()
{
	EXTI->PR |= (1 << 0);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler()
{
	EXTI->PR |= (1 << 1);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler()
{
	EXTI->PR |= (1 << 2);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler()
{
	EXTI->PR |= (1 << 3);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_IRQHandler()
{
	EXTI->PR |= (1 << 4);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

void EXTI9_5_IRQHandler()
{
	if (EXTI->PR & (1 << 5)) {
		EXTI->PR |= (1 << 5);
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
	}
	if (EXTI->PR & (1 << 6)) {
		EXTI->PR |= (1 << 6);
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
	}
	if (EXTI->PR & (1 << 7)) {
		EXTI->PR |= (1 << 7);
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
	}
	if (EXTI->PR & (1 << 8)) {
		EXTI->PR |= (1 << 8);
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
	}
	if (EXTI->PR & (1 << 9)) {
		EXTI->PR |= (1 << 9);
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
	}
}
