// Scc.h

#ifndef SCC_H_
#define SCC_H_

#include "ReferTes.h"

#define SCC_BASE		0x2000

enum  { CHB = 0, CHA };

#define	SCCC(di, ci)	*((volatile BYTE*)(SCC_BASE + (di << 8) + (ci == 0 ? CHA : CHB)))
#define	SCCD(di, ci)	*((volatile BYTE*)(SCC_BASE + (di << 8) + (ci == 0 ? CHA : CHB) + 2))

#define	WR5CMD_SYNC		0x69		// b0:crc enable
#define	WR5CMD_ASYNC	0x68

#define EnableTxSync(id)\
	do {\
		SCCC(((id) >> 1), ((id) & 1)) = 5;\
		SCCC(((id) >> 1), ((id) & 1)) = WR5CMD_SYNC;\
	} while (0)

#define DisableTxSync(id)\
	do {\
		SCCC(((id) >> 1), ((id) & 1)) = 5;\
		SCCC(((id) >> 1), ((id) & 1)) = WR5CMD_SYNC | 2;\
	} while (0)

#define EnableTxAsync(id)\
	do {\
		SCCC(((id) >> 1), ((id) & 1)) = 5;\
		SCCC(((id) >> 1), ((id) & 1)) = WR5CMD_ASYNC;\
	} while (0)

#define DisableTxAsync(id)\
	do {\
		SCCC(((id) >> 1), ((id) & 1)) = 5;\
		SCCC(((id) >> 1), ((id) & 1)) = WR5CMD_ASYNC | 2;\
	} while (0)

WORD	Crc161d0f(BYTE* p, BYTE leng);
WORD	Crc16ffff(BYTE* p, BYTE leng);
void	InitialChannels(BYTE cid);
void	InitialScc(BYTE did);
//BOOL	SccRxFull(BYTE id);
SCCCHS*	GetSccBuf(BYTE id);
void	SendScc(BYTE id);
void	ScanScc();
BYTE	GetSendLeng(BYTE id);
//WORD	GetDeviceAddress(BYTE did);
//void	SccISR(BYTE id);

#endif /* SCC_H_ */