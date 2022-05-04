/**
 * @file	Infer.h
 * @brief	모니터에 변수들을 나타내기위한 매크로.
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include <stdio.h>

#define	PRIVACY_CONTAINER()\
	private:\
	int		Contain(bool, void*)


#define	PUBLICY_CONTAINER()\
	public:\
	int		Contain(bool, void*)


#define	ENTRY_CONTAINER(curClass)\
int curClass::Contain(bool b, void* p)\
{\
	bool bSpoon = b;\
	char* pSpoon = (char*)p;\
	int spoon = 0;

#define	ISCOOP(nestc)\
	int _i##nestc = nestc.Contain(bSpoon, pSpoon);\
	pSpoon += _i##nestc;\
	spoon += _i##nestc;

#define	PSCOOP(nestc)\
	int _i##nestc = nestc->Contain(bSpoon, pSpoon);\
	pSpoon += _i##nestc;\
	spoon += _i##nestc;

#define	SCOOP(pVoid, leng, szName)\
	if (bSpoon) {\
		sprintf(pSpoon, "%s,%d,", szName, leng);\
		while (*pSpoon != '\0') {\
			++ pSpoon;\
			++ spoon;\
		}\
	}\
	else {\
		memcpy(pSpoon, pVoid, leng);\
		pSpoon += leng;\
		spoon += leng;\
	}

#define	EXIT_CONTAINER()\
	return spoon;\
}

// uuuum.. strcpy -> strncpy, strncpy((char*)p, s, strlen(s));	// what is strlen's meaning
#define	_POUR(p, x, s)\
	do {\
		strcpy((char*)p, s);\
		p += strlen(s);\
		x += strlen(s);\
	} while (0)

#define	_DASH(p, x, s, l)\
	do {\
		memcpy(p, s, l);\
		p += l;\
		x += l;\
	} while (0)
