/*
 * CProbe.h
 *
 *  Created on: 2010. 12. 16
 *      Author: Che
 */

#ifndef CPROBE_H_
#define CPROBE_H_

#include <Mfb.h>
#include <Define.h>
#include <PcioInfo.h>

#define	SIZE_AIN	SIZE_TCAI
#define	SIZE_DIN	4
#define	SIZE_DOUT	3
#define	SIZE_DOUTEX	4
#define	OUTOF_PROBECH	MFB_DIOB1_ERR + 1

#if defined(_CTCX)
#	define	_PDI	DIOB1_DINA
#	define	_PDO	DIOB0_DOTA

#elif defined(_CCCX)
#	define	_PDI	DIOB0_DINA
#	define	_PDO	DIOB1_DOTA

#else
#	error undefined _CTCX or _CCCX
#endif

class CProbe
{
public:
	CProbe();
	virtual ~CProbe();

private:
#define	SIZE_PROBEBUF	8
	BYTE	c_nBuf[SIZE_PROBEBUF];
	static const int	c_probeChes[];

	void	ClearBuf();
	void	PrintMfError(int ch, int pch, int res, PSZ msg);

public:
	int		Open();
	int		Close();
	int		Geta(UCURV ch, WORD* pV);
	int		Getd(UCURV ch, WORD* pV);
	int		Putd(UCURV ch, WORD* pV);
};

#endif /* CPROBE_H_ */
