// Tranzx.h
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"

class CTranzxApp : public CWinAppEx
{
public:
	CTranzxApp();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTranzxApp theApp;
