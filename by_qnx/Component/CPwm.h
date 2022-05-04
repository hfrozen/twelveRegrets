/*
 * CPwm.h
 *
 *  Created on: 2010. 12. 16
 *      Author: Che
 */

#ifndef CPWM_H_
#define CPWM_H_

#include <Mfb.h>
#include <Define.h>

#define	_PWMIN		MF_TIMERINA
#define	_PWMOUT		MF_PWMOA

class CPwm
{
public:
	CPwm();
	virtual ~CPwm();

private:
#define	SIZE_PWMBUF		4
	BYTE	c_nBuf[SIZE_PWMBUF];

	void	ClearBuf();
	void	PrintMfError(int ch, int res, PSZ msg);

public:
	int		Open();
	int		Close();
	int		Get(DWORD* pV);
	int		Put(DWORD* pV);
};

#endif /* CPWM_H_ */
