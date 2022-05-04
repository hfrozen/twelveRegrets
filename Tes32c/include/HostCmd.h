/*
 * HostCmd.h
 *
 *  Created on: 2019. 3. 19.
 *      Author: SWCho
 */
#pragma once

void Shift(BYTE* p, int sn);
void InitialHost(BYTE iip);
void ScanHost(void);
void ActionHost(void);
void PrintSDR(int id);
DWORD PrintSH(BYTE* p, int leng);	// static to host
void PrintFH(int leng, ...);		// flexibility to host
