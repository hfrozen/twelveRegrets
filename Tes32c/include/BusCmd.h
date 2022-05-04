/*
 * BusCmd.h
 *
 *  Created on: 2019. 3. 19.
 *      Author: SWCho
 */
#pragma once

BCHEAD AbstractCmd(BYTE* p, int leng, bool bDir);
void ShiftCmd(BYTE* p, int shift);
void DpioCtrl();
