
// Watcher.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.

#define	COLOR_NAMEBTN		RGB(240, 240, 240)
#define	COLOR_NAMETEXT		RGB(0, 0, 255)
#define	COLOR_BKGND			RGB(255, 255, 255)
#define COLOR_TEXT			RGB(96, 96, 96)
#define	COLOR_INVBKGND		RGB(18, 116, 220)
#define COLOR_INVTEXT		RGB(192, 192, 192)
#define	COLOR_TEXTHIGH		RGB(0, 0, 0)
#define	COLOR_INVTEXTHIGH	RGB(255, 255, 255)

// CWatcherApp:
// �� Ŭ������ ������ ���ؼ��� Watcher.cpp�� �����Ͻʽÿ�.
//

class CWatcherApp : public CWinApp
{
public:
	CWatcherApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CWatcherApp theApp;