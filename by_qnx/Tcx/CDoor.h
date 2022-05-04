/*
 * CDoor.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CDOOR_H_
#define CDOOR_H_

#include <Draft.h>

class CDoor
{
public:
	CDoor();
	virtual ~CDoor();

protected:
	PVOID		c_pParent;
	PTCREFERENCE	c_pTcRef;
	PTCDOZ		c_pDoz;

private:
	enum  {
		DOORCMD_NON,
		DOORCMD_ERROR,
		DOORCMD_LEFT,
		DOORCMD_RIGHT
	};
#define	DOORCMD_OPEN	TRUE
#define	DOORCMD_CLOSE	FALSE

	void	ShotL(BYTE nCid, WORD wMsg);
	WORD*	GetTimer();
	void	Mode();
	void	Lamp(BYTE lp, BYTE state);
	void	Move(BOOL bDir, BOOL bCmd);
	void	ReOpen(BOOL cmd);
	BOOL	OpenCmd(BOOL bDir);
	void	CloseCmd(BOOL bDir);
	UCURV	ScanOpenCmd(BOOL bClosing);
	UCURV	ScanCloseCmd();
	void	CollectDoorState();
	BOOL	ScanDoorState();
	BOOL	ScanDoorAtOpen();
	BOOL	ScanDoorAtClose(BOOL bClosed);
	void	ScanOppositeDoorOpenCmd(BOOL bRight);
	BYTE	ScanOppositeOpenedDoor(BOOL bRight);
	void	DoorBypassMap(UCURV nCcid, BYTE* pDoor);
	void	DoorBypassMap(UCURV nCcid, BYTE* pDoor, BOOL state);
	void	DoorAnnals();
	UCURV	DecisionDoorByDir(UCURV ddir);
	void	ByEmergency();
	void	ByNormal();

public:
	void	InitialModule(PVOID pVoid);
	BYTE	BitIndex(BYTE bits, BOOL state);
	WORD	XchAtZero(BYTE nCid, WORD wMsg);
	void	Shut();
	void	Control();
};

#endif /* CDOOR_H_ */
