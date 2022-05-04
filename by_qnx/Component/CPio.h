/*
 * CPio.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CPIO_H_
#define CPIO_H_

/*
*/

#include <men/men_typs.h>
#include <men/mdis_api.h>
#include <men/usr_oss.h>
#include <men/z17_drv.h>

#include <Define.h>

#define	LENGTH_GPIO		5

class CPio
{
public:
	CPio();
	virtual ~CPio();

private:
	BOOL	c_bState;
	MDIS_PATH	c_dev[LENGTH_GPIO];
	static const char*	c_device[LENGTH_GPIO];

	typedef struct _tagATTRIBUTE {
		BYTE	init;
		BYTE	attr;
	} ATTRIBUTE;
	static const ATTRIBUTE	c_attr[LENGTH_GPIO];
	static const WORD		c_wCcDeves[3][10];

	BOOL	Close();

public:
	BOOL	Open();
	DWORD	ReadThumb();
	BYTE	Read(BYTE add);
	void	Write(BYTE add, BYTE n);
	BYTE	Swap(BYTE n);
	WORD	GetCcDev(PCARINFO pCar);
	BYTE	GetV3fQuantity(PCARINFO pCar);
	BYTE	GetEcuQuantity(PCARINFO pCar);
	char*	TcNamePlate(char* p);
	char*	CcNamePlate(char* p);
	BOOL	GetState()	{ return c_bState; }
};

#endif /* CPIO_H_ */
