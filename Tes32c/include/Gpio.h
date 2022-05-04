// gpio.h
#pragma once
#include <stdbool.h>
#include <sys/types.h>
#include "Refer32.h"

void InitialGpio(void);
void InitialD32In(void);
DWORD GetD32In();
void InitialD32Out(void);
//void PutDoutpBI(BYTE bi, bool bState);
void PutD32Out(DWORD dw);
DWORD GetD32Out();
void IntervalGpio(void);
