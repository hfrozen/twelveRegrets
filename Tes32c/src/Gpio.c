// gpio.c
#include "stm32f4xx_hal.h"
#include "Gpio.h"

// PE00(b30)/PE01(b31)/PE02(b29)/PE03(b28)	-	PE04(b27)/PE05(b26)/PE06(b25)/PE07(b08)
// PE08(b07)/PE09(b06)/PE10(b05)/PE11(b04) -	PE12(b03)/PE13(b02)/PE14(b01)/PE15(b00)
// PF00(b24)/PF01(b23)/PF02(b22)/PF03(b21) -	PF04(b20)/PF05(b19)/PF06(b18)/PF07(b17)
// PF08(b16)/PF09(b15)/PF10(b14)/PF11(b13)	-	PF12(b12)/PF13(b11)/PF14(b10)/PF15(b09)
const DWORD dwDinMap[32] = {
	0x40000000,		// b30
	0x80000000,		// b31
	0x20000000,		// b29
	0x10000000,		// b28
	0x08000000,		// b27
	0x04000000,		// b26
	0x02000000,		// b25
	0x00000100,		// b08

	0x00000080,		// b07
	0x00000040,		// b06
	0x00000020,		// b05
	0x00000010,		// b04
	0x00000008,		// b03
	0x00000004,		// b02
	0x00000002,		// b01
	0x00000001,		// b00

	0x01000000,		// b24
	0x00800000,		// b23
	0x00400000,		// b22
	0x00200000,		// b21
	0x00100000,		// b20
	0x00080000,		// b19
	0x00040000,		// b18
	0x00020000,		// b17

	0x00010000,		// b16
	0x00008000,		// b15
	0x00004000,		// b14
	0x00002000,		// b13
	0x00001000,		// b12
	0x00000800,		// b11
	0x00000400,		// b10
	0x00000200,		// b09
};

// PE15(b00)/PE14(b01)/PE13(b02)/PE12(b03)	-	PE11(b04)/PE10(b05)/PE09(b06)/PE08(b07)
// PE07(b08)/PF15(b09)/PF14(b10)/PF13(b11)	-	PF12(b12)/PF11(b13)/PF10(b14)/PF09(b15)
// PF08(b16)/PF07(b17)/PF06(b18)/PF05(b19)	-	PF04(b20)/PF03(b21)/PF02(b22)/PF01(b23)
// PF00(b24)/PE06(b25)/PE05(b26)/PE04(b27)	-	PE03(b28)/PE02(b29)/PE00(b30)/PE01(b31)

const DWORD dwDoutMap[32] = {
	0x00008000,		// pe15
	0x00004000,		// pe14
	0x00002000,		// pe13
	0x00001000,		// pe12
	0x00000800,		// pe11
	0x00000400,		// pe10
	0x00000200,		// pe09
	0x00000100,		// pe08

	0x00000080,		// pe07
	0x80000000,		// pf15
	0x40000000,		// pf14
	0x20000000,		// pf13
	0x10000000,		// pf12
	0x08000000,		// pf11
	0x04000000,		// pf10
	0x02000000,		// pf09

	0x01000000,		// pf08
	0x00800000,		// pf07
	0x00400000,		// pf06
	0x00200000,		// pf05
	0x00100000,		// pf04
	0x00080000,		// pf03
	0x00040000,		// pf02
	0x00020000,		// pf01

	0x00010000,		// pf00
	0x00000040,		// pe06
	0x00000020,		// pe05
	0x00000010,		// pe04
	0x00000008,		// pe03
	0x00000004,		// pe02
	0x00000001,		// pe00
	0x00000002,		// pe01
};

typedef struct _tagDINBUFFER {
	_QUAD	cur;
	_QUAD	buf;
	_QUAD	stab;
	BYTE	deb;
} DINBUFFER;
DINBUFFER	din;
#define	DEB_DIN		20

void InitialGpio(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };
	// card function input	- PC6(b0)/PC7(b1)/PC8(b2)/PC9(b3)
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOC, &gpio);

	// card address input	- PG6(b0)/PG7(b1)/PB10(b2)/PB14(b3)/PB15(b4)
	gpio.Pin = GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &gpio);
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOG, &gpio);

	// watch dog			- PB6
	gpio.Pin = GPIO_PIN_6;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &gpio);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCO_DIV1);
}

// for din
void InitialD32In(void)
{
	memset(&din, 0, sizeof(din));

	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
			GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOE, &gpio);
	HAL_GPIO_Init(GPIOF, &gpio);
}

DWORD GetD32In()
{
	DWORD dwInp = din.stab.dw;
	DWORD dwRes = 0;
	for (int n = 0; n < 32; n ++)
		if (dwInp & (1 << n))	dwRes |= dwDinMap[n];

	return dwRes;
}

void InitialD32Out(void)
{
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();

	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
			GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOE, &gpio);
	HAL_GPIO_Init(GPIOF, &gpio);
	PutD32Out(0);
}

//void PutDoutpBI(BYTE bi, bool bState)
//{
//	DWORD dw = dwDoutMap[bi];
//	if (dw & 0xffff) {
//		if (!bState)	dw <<= 16;
//		GPIOE->BSRR = dw;
//	}
//	else {
//		if (bState)	dw >>= 16;
//		GPIOF->BSRR = dw;
//	}
//}
//
void PutD32Out(DWORD dw)
{
	_QUAD dout;
	dout.w[0] = dout.w[1] = 0;
	for (int n = 0; n < 32; n ++)
		if (dw & (1 << n))	dout.dw |= dwDoutMap[n];

	GPIOE->ODR = (DWORD)(~dout.w[0] & 0xffff);
	GPIOF->ODR = (DWORD)(~dout.w[1] & 0xffff);
}

void IntervalGpio(void)
{
	din.cur.w[0] = (WORD)(~GPIOE->IDR);
	din.cur.w[1] = (WORD)(~GPIOF->IDR);
	if (din.cur.dw != din.buf.dw) {
		din.buf.dw = din.cur.dw;
		din.deb = DEB_DIN;
	}
	else if (din.deb != 0 && -- din.deb == 0)
		din.stab.dw = din.buf.dw;
}
