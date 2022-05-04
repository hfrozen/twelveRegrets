/*
 * CPwm.cpp
 *
 *  Created on: 2010. 12. 16
 *      Author: Che
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <Printz.h>

#include "CPwm.h"

CPwm::CPwm()
{
}

CPwm::~CPwm()
{
}

void CPwm::ClearBuf()
{
	for (UCURV n = 0; n < SIZE_PWMBUF; n ++)
		c_nBuf[n] = 0;
}

void CPwm::PrintMfError(int ch, int res, PSZ msg)
{
	if (res < MFB_SUCCESS)
		MSGLOG("%s%d(%d-%s).\r\n", msg, ch, res, strerror(errno));
	else	MSGLOG("%s%d(%d).\r\n", msg, ch, res);
}

int CPwm::Open()
{
	/*MFSTATUS status;
	memset(&status, 0, sizeof(MFSTATUS));
	int res;
	if ((res = mfOpen(_PWMIN, &status)) != MFB_SUCCESS) {
		PrintMfError(_PWMIN, res, (PSZ)"[PWM]ERROR:Can not open pwm port");
		return res;
	}
	mfEnableChannel(_PWMIN, TRUE);
	mfEnableChannel(_PWMIN + 1, TRUE);
	mfEnableChannel(_PWMIN + 2, TRUE);
	mfEnableChannel(_PWMIN + 3, TRUE);
	memset(&status, 0, sizeof(MFSTATUS));
	if ((res = mfOpen(_PWMOUT, &status)) != MFB_SUCCESS) {
		PrintMfError(_PWMIN, res, (PSZ)"[PWM]ERROR:Can not open pwm port");
		return res;
	}
	mfEnableChannel(_PWMOUT, TRUE);
	mfEnableChannel(_PWMOUT + 1, TRUE);
	mfEnableChannel(_PWMOUT + 2, TRUE);
	mfEnableChannel(_PWMOUT + 3, TRUE);*/
	return 0;
}

int CPwm::Close()
{
	int resa, resb;
	if ((resa = mfClose(_PWMIN)) != MFB_SUCCESS)
		PrintMfError(_PWMIN, resa, (PSZ)"[PWM]ERROR:Can not close pwm port");
	if ((resb = mfClose(_PWMOUT)) != MFB_SUCCESS)
		PrintMfError(_PWMOUT, resb, (PSZ)"[PWM]ERROR:Can not close pwm port");
	if (resa != MFB_SUCCESS)	return resa;
	else if (resb != MFB_SUCCESS)	return resb;
	return 0;
}

int CPwm::Get(DWORD* pV)
{
	ClearBuf();
	int res;
	int length = 4;
	if ((res = mfRead(_PWMIN, (char*)&c_nBuf[0], &length)) != MFB_SUCCESS) {
		PrintMfError(_PWMIN, res, (PSZ)"[PWM]ERROR:Can not read pwm port");
		return res;
	}
	//MFSTATUS status;
	//memset(&status, 0, sizeof(MFSTATUS));
	//status.counter.init = 0;
	//mfSetStatus(_PWMIN, &status);
	WORD high = MAKEWORD(c_nBuf[0], c_nBuf[1]);
	WORD low = MAKEWORD(c_nBuf[2], c_nBuf[3]);
	*pV = MAKEDWORD(high, low);
	ClearBuf();
	length = 4;
	mfWrite(_PWMIN, (char*)&c_nBuf[0], &length);
	return 0;
}

int CPwm::Put(DWORD* pV)
{
	MFSTATUS status;
	memset(&status, 0, sizeof(MFSTATUS));
	status.pwm.level = HIGHWORD(*pV);
	status.pwm.period = LOWWORD(*pV);
	//MSGLOG("[PWM]Put high=%d period=%d\n", status.pwm.level, status.pwm.period);
	int res;
	if ((res = mfSetStatus(_PWMOUT, &status)) != MFB_SUCCESS) {
		PrintMfError(_PWMOUT, res, (PSZ)"[PWM]ERROR:Can not write pwm port");
		return res;
	}
	memset(&status, 0, sizeof(MFSTATUS));
	if ((res = mfGetStatus(_PWMOUT, &status)) != MFB_SUCCESS) {
		PrintMfError(_PWMOUT, res, (PSZ)"[PWM]ERROR:Can not feedback pwm port");
		return res;
	}
	//MSGLOG("[PWM]Feedback high=%d period=%d\n", status.pwm.level, status.pwm.period);
	return 0;
}
