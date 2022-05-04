// Uart.h
#pragma once

#include <stdbool.h>
#include <stdarg.h>
#include "Refer32.h"

WORD Crc161d0f(BYTE* p, int nLeng);
WORD Crc16ffff(BYTE* p, int nLeng);

void PrintFB(int leng, ...);		// flexibility to bus
void InitialBus(void);
void InitialMon(void);
void PrintSB(BYTE* p, int leng);	// static to bus
void RtsCtrl(bool bState);
//void PutB(char ch);
void PrintM(char* p);
void PutChar(char ch);
void IntervalUart();
