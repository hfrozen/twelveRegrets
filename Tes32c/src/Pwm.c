//	Pwm.c
//#include <string.h>
//#include <stdbool.h>
//#include <stdlib.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_tim_ex.h"
#include "Refer32.h"
#include "Pwm.h"

//	pwm out
//	ch.00:	PD14:	AF2/TIM4 -CH3,	PWM
//	ch.01:	PD15:	AF2/TIM4 -CH4,	PWM
//	ch.02:	PD12:	AF2/TIM4 -CH1,	AO
//	ch.03:	PD13:	AF2/TIM4 -CH2,	AO
//	ch.04:	PE13:	AF1/TIM1 -CH3,	4~20mA(CLAMP)
//	ch.05:	PE14:	AF1/TIM1 -CH4,	4~20mA(CLAMP)
//	ch.06:	PE09:	AF1/TIM1 -CH1,	4~20mA
//	ch.07:	PE11:	AF1/TIM1 -CH2,	4~20mA
//	ch.08:	PB00:	AF1/TIM1 -CH2N,	AF2/TIM3-CH3,	AF3/TIM8-CH2N,	4~20mA
//	ch.09:	PB01:	AF1/TIM1 -CH3N,	AF2/TIM1-CH4,	AF3/TIM8-CH3N,	4~20mA
//	ch.10:	PF09:	AF9/TIM14-CH1,	4~20mA
//	ch.11:	PF08:	AF9/TIM13-CH1,	4~20mA
//	ch.12:	PF07:	AF3/TIM11-CH1,	CLAMP HIGH V
//	ch.13:	PF06:	AF3/TIM10-CH1,	CLAMP LOW V
//	ch.14:	PE06:	AF3/TIM9 -CH2,	not used
//	ch.15:	PE05:	AF3/TIM9 -CH1,	not used

//	PB00,	PB01
//	PD12,	PD13,	PD14,	PD15
//	PE09,	PE11,	PE13,	PE14
//	PF06,	PF07,	PF08,	PF09

const PWMFABRIC g_pwmOutFabric[PWMOUT_LENGTH] = {
		{ "TIM4-CH3",  TIM4,  TIM_CHANNEL_3 },
		{ "TIM4-CH4",  TIM4,  TIM_CHANNEL_4 },
		{ "TIM4-CH1",  TIM4,  TIM_CHANNEL_1 },
		{ "TIM4-CH2",  TIM4,  TIM_CHANNEL_2 },
		{ "TIM1-CH3",  TIM1,  TIM_CHANNEL_3 },
		{ "TIM1-CH4",  TIM1,  TIM_CHANNEL_4 },
		{ "TIM1-CH1",  TIM1,  TIM_CHANNEL_1 },
		{ "TIM1-CH2",  TIM1,  TIM_CHANNEL_2 },

//		{ "TIM8-CH3",  TIM8,  TIM_CHANNEL_2 },
//		{ "TIM8-CH3",  TIM8,  TIM_CHANNEL_3 },
//		{ "TIM14-CH3", TIM14, TIM_CHANNEL_1 },
//		{ "TIM13-CH3", TIM13, TIM_CHANNEL_1 },
//		{ "TIM11-CH3", TIM11, TIM_CHANNEL_1 },
//		{ "TIM10-CH3", TIM10, TIM_CHANNEL_1 },
//		{ "TIM9-CH3",  TIM9,  TIM_CHANNEL_2 },
//		{ "TIM9-CH3",  TIM9,  TIM_CHANNEL_1 }
};

//	pwm in
//	ch.00:	PE05:	AF3/TIM9 -CH1
//	ch.01:	PF06:	AF3/TIM10 -CH1
//	ch.02:	PF07:	AF3/TIM11 -CH1
//	ch.03:	PF08:	AF9/TIM13 -CH1
//	ch.04:	PF09:	AF9/TIM14 -CH1
//	ch.05:	PE09:	AF1/TIM1 -CH1
//
//	adc in
//	ch.00:	PF03:	AF
//	ch.01:	PF04:	AF
//	ch.02:	PF05:	AF
//	ch.03:	PF10:	AF
//const PWMFABRIC	g_pwmInFabric[PWMIN_LENGTH] = {
//		{ "TIM9-CH1",  TIM9,  TIM_CHANNEL_1 },
//		{ "TIM1-CH1",  TIM1,  TIM_CHANNEL_1 }
//};

#define	PRESCALE_PWMOUT		83
#define	PERIOD_500HZ		2000
TIM_HandleTypeDef	g_hTims[PWMOUT_LENGTH];
PWMDUTY		g_duty[PWMIN_LENGTH] = { 0 };

void InitialWidth(int ch, DWORD dwWidth, bool bStart)
{
	TIM_OC_InitTypeDef oit = { 0 };
	if (g_hTims[ch].Instance == TIM1 || g_hTims[ch].Instance == TIM8) {
		oit.OCMode = TIM_OCMODE_PWM2;
		oit.Pulse = g_duty[ch].dwWidth = dwWidth;
		oit.OCPolarity = TIM_OCPOLARITY_LOW;
		oit.OCNPolarity = TIM_OCNPOLARITY_LOW;
		oit.OCFastMode = TIM_OCFAST_DISABLE;
		oit.OCIdleState = TIM_OCIDLESTATE_SET;
		oit.OCNIdleState = TIM_OCNIDLESTATE_SET;
	}
	else {
		oit.OCMode = TIM_OCMODE_PWM1;
		oit.Pulse = g_duty[ch].dwWidth = dwWidth;
		oit.OCPolarity = TIM_OCPOLARITY_HIGH;
		oit.OCFastMode = TIM_OCFAST_DISABLE;
	}
	HAL_TIM_PWM_ConfigChannel(&g_hTims[ch], &oit, g_pwmOutFabric[ch].dwCh);
	if (bStart)	HAL_TIM_PWM_Start(&g_hTims[ch], g_pwmOutFabric[ch].dwCh);
}

void InitialPeriod(int ch, DWORD dwPeriod, DWORD dwWidth)
{
	TIM_MasterConfigTypeDef mct = { 0 };
	g_hTims[ch].Instance = g_pwmOutFabric[ch].pHt;
	g_hTims[ch].Init.Prescaler = PRESCALE_PWMOUT;			// 1us
	g_hTims[ch].Init.CounterMode = TIM_COUNTERMODE_UP;
	g_hTims[ch].Init.Period = g_duty[ch].dwPeriod = dwPeriod;
	g_hTims[ch].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_PWM_Init(&g_hTims[ch]);
	mct.MasterOutputTrigger = TIM_TRGO_RESET;
	mct.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&g_hTims[ch], &mct);

	InitialWidth(ch, dwWidth, false);

	HAL_TIM_Base_Start(&g_hTims[ch]);
	HAL_TIM_PWM_Start(&g_hTims[ch], g_pwmOutFabric[ch].dwCh);
}

void InitialPwmOut()
{
	memset(&g_hTims, 0, sizeof(TIM_HandleTypeDef) * PWMOUT_LENGTH);
	for (int n = 0; n < PWMOUT_LENGTH; n ++)
		InitialPeriod(n, (DWORD)PERIOD_500HZ, (DWORD)0);

	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &gpio);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

	__HAL_RCC_GPIOE_CLK_ENABLE();
	gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6;
	HAL_GPIO_Init(GPIOE, &gpio);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);

	__HAL_RCC_GPIOF_CLK_ENABLE();
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	HAL_GPIO_Init(GPIOF, &gpio);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
}

void SetPwmOut(int ch, DWORD dwPeriod, DWORD dwWidth)
{
	if (ch < PWMOUT_LENGTH) {
		if (dwWidth == 0)	InitialPeriod(ch, PERIOD_500HZ, 0);
		else if (dwWidth < dwPeriod) {
			DWORD dw = dwWidth;
			if (ch < 2)	dw = dwPeriod - dwWidth;
			if (g_duty[ch].dwPeriod != dwPeriod)	InitialPeriod(ch, dwPeriod, dw);
			else	InitialWidth(ch, dw, true);
		}
		else {
			dwWidth = dwPeriod / 2;
			InitialPeriod(ch, dwPeriod, dwWidth);
		}
	}
}

void InitialPwmIn()
{
	memset(&g_duty, 0, sizeof(PWMDUTY) * PWMIN_LENGTH);
	memset(&g_hTims, 0, sizeof(TIM_HandleTypeDef) * PWMOUT_LENGTH);

	TIM_ClockConfigTypeDef cct = { 0 };
	TIM_MasterConfigTypeDef mct = { 0 };
	TIM_IC_InitTypeDef iit = { 0 };

	g_hTims[0].Instance = TIM9;
	g_hTims[0].Init.Prescaler = 167;		//0;
	g_hTims[0].Init.CounterMode = TIM_COUNTERMODE_UP;
	g_hTims[0].Init.Period = 0xffff;
	g_hTims[0].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&g_hTims[0]);

	cct.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&g_hTims[0], &cct);

	HAL_TIM_IC_Init(&g_hTims[0]);

	iit.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	iit.ICSelection = TIM_ICSELECTION_DIRECTTI;
	iit.ICPrescaler = TIM_ICPSC_DIV1;
	iit.ICFilter = 0;
	HAL_TIM_IC_ConfigChannel(&g_hTims[0], &iit, TIM_CHANNEL_1);

	iit.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	iit.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	HAL_TIM_IC_ConfigChannel(&g_hTims[0], &iit, TIM_CHANNEL_2);


	HAL_TIM_IC_Start_IT(&g_hTims[0], TIM_CHANNEL_1);
	HAL_TIM_IC_Start(&g_hTims[0], TIM_CHANNEL_2);

	g_hTims[1].Instance = TIM1;
	g_hTims[1].Init.Prescaler = 167;		//0;
	g_hTims[1].Init.CounterMode = TIM_COUNTERMODE_UP;
	g_hTims[1].Init.Period = 0xffff;
	g_hTims[1].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	g_hTims[1].Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&g_hTims[1]);

	cct.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&g_hTims[1], &cct);

	HAL_TIM_IC_Init(&g_hTims[1]);

	mct.MasterOutputTrigger = TIM_TRGO_RESET;
	mct.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&g_hTims[1], &mct);

	iit.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	iit.ICSelection = TIM_ICSELECTION_DIRECTTI;
	iit.ICPrescaler = TIM_ICPSC_DIV1;
	iit.ICFilter = 0;
	HAL_TIM_IC_ConfigChannel(&g_hTims[1], &iit, TIM_CHANNEL_1);

	iit.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	iit.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	HAL_TIM_IC_ConfigChannel(&g_hTims[1], &iit, TIM_CHANNEL_2);

	HAL_TIM_IC_Start_IT(&g_hTims[1], TIM_CHANNEL_1);
	HAL_TIM_IC_Start(&g_hTims[1], TIM_CHANNEL_2);

	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOF, &gpio);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
}

void GetPwmIn(int ch, DWORD* pdwPeriod, DWORD* pdwWidth)
{
	if (ch < PWMIN_LENGTH) {
		*pdwPeriod = g_duty[ch].dwPeriod;
		*pdwWidth = g_duty[ch].dwWidth;
	}
	else {
		*pdwPeriod = 0;
		*pdwWidth = 0;
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* phTim)
{
	if (phTim->Instance == TIM9 || phTim->Instance == TIM1) {
		int n = phTim->Instance == TIM9 ? 0 : 1;
		g_duty[n].dwPeriod = HAL_TIM_ReadCapturedValue(phTim, TIM_CHANNEL_1);
		g_duty[n].dwWidth = HAL_TIM_ReadCapturedValue(phTim, TIM_CHANNEL_2);
		g_duty[n].wTimer = 0;
		__HAL_TIM_SetCounter(phTim, 0);
		//if (g_duty[n].dwPeriod != 0 || g_duty[n].dwWidth != 0)
		//	printf("ch%d %ld %ld\r\n", n, g_duty[n].dwPeriod, g_duty[n].dwWidth);
	}
}

void IntervalPwm()
{
	for (int n = 0; n < PWMIN_LENGTH; n ++) {
		if ((g_duty[n].dwPeriod > 0 || g_duty[n].dwWidth > 0) &&
			g_duty[n].wTimer < TIME_ZEROPERIOD && ++ g_duty[n].wTimer >= TIME_ZEROPERIOD) {
			memset(&g_duty, 0, sizeof(PWMDUTY) * PWMIN_LENGTH);
		}
	}
}
