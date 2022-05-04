// Fpgam.h
#pragma once
#include <stdbool.h>

void SccCtrl();
void SendSDA(int pn, bool bResp);
void SendPort(int pn, BYTE* p, WORD len);
int InitialPort(int pn);
void InitialDefault();
void ResetEach(int pn);
void ResetFpgam();
void InitialFpgam();
void IntervalFpgam();
