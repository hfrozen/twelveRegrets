// stm32f4xx_it.h
#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

void SysTick_Handler(void);
void DMA2_Stream2_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);

#ifdef	__cplusplus
}
#endif
