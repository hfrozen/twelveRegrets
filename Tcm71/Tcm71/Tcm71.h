// Tcm71.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CTcm71App:
// �� Ŭ������ ������ ���ؼ��� Tcm71.cpp�� �����Ͻʽÿ�.
//

class CTcm71App : public CWinApp
{
public:
	CTcm71App();

// �������Դϴ�.
	public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CTcm71App theApp;