
// Watcher.h : PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.

#define	COLOR_NAMEBTN		RGB(240, 240, 240)
#define	COLOR_NAMETEXT		RGB(0, 0, 255)
#define	COLOR_BKGND			RGB(255, 255, 255)
#define COLOR_TEXT			RGB(96, 96, 96)
#define	COLOR_INVBKGND		RGB(18, 116, 220)
#define COLOR_INVTEXT		RGB(192, 192, 192)
#define	COLOR_TEXTHIGH		RGB(0, 0, 0)
#define	COLOR_INVTEXTHIGH	RGB(255, 255, 255)

// CWatcherApp:
// 이 클래스의 구현에 대해서는 Watcher.cpp을 참조하십시오.
//

class CWatcherApp : public CWinApp
{
public:
	CWatcherApp();

// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.

	DECLARE_MESSAGE_MAP()
};

extern CWatcherApp theApp;