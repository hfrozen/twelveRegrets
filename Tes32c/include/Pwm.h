/*
 * Pwm.h
 *
 *  Created on: 2019. 4. 2.
 *      Author: SWCho
 */
#pragma once
#include "Refer32.h"

//void InitialWidth(int ch, DWORD dwWidth, bool bStart);
//void InitialPeriod(int ch, DWORD dwPeriod, DWORD dwWidth);
void InitialPwmOut();
void SetPwmOut(int ch, DWORD dwPeriod, DWORD dwWidth);
void InitialPwmIn();
void GetPwmIn(int ch, DWORD* pdwPeriod, DWORD* pdwWidth);
void IntervalPwm();
