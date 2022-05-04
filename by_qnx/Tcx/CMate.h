/*
 * CMate.h
 *
 *  Created on: 2012. 6. 13
 *      Author: Che
 */

#ifndef CMATE_H_
#define CMATE_H_

#include <CAnt.h>
#include <Draft.h>

class CMate: public CAnt
{
public:
	CMate();
	virtual ~CMate();

protected:
	PTCREFERENCE	c_pTcRef;
	PTCDOZ		c_pDoz;
	PTCPERT		c_pSend;
	PTCPERT		c_pTake;

private:
	PVOID	c_pParent;

	WORD	c_wProgress;
	DWORD	c_dwFilesFromBusSize;
	WORD	c_wFilesFromBusLength;
	DWORD	c_dwFilesFromBus[SIZE_FILESUNDERDAY];

	void	DirectoryToPert();
	void	DirectoryFromPert();
	void	FileToPert();
	void	FileFromPert();

public:
	void	InitialModule(PVOID pVoid);

	void	RequestDir();
	BOOL	Research();
	void	ContinueDirectoryToPert(BOOL bValid);
	void	ContinueFileToPert(BOOL bValid);
	void	ContinueFileFromPert(BOOL bValid);
	void	PassiveClose();
	void	ActiveClose();
	WORD	GetProgress()	{ return c_wProgress; }
};

#endif /* CMATE_H_ */
