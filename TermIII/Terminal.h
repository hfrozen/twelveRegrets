// Terminal.h : main header file for the TERMINAL application
//

#if !defined(AFX_TERMINAL_H__8C434A25_5FE4_11D5_83EC_00C0262A4671__INCLUDED_)
#define AFX_TERMINAL_H__8C434A25_5FE4_11D5_83EC_00C0262A4671__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define SHIFT_ATT	1
#define CONTROL_ATT	2
#define ALT_ATT		4

typedef struct tagMACRO {
	UINT	nAtt;
	UINT	nKey;
	CString	strContent;
} MACRO;
#define MAX_MACRO 100

/////////////////////////////////////////////////////////////////////////////
// CTerminalApp:
// See Terminal.cpp for the implementation of this class
//

class CTerminalApp : public CWinApp
{
public:
	CTerminalApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerminalApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTerminalApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERMINAL_H__8C434A25_5FE4_11D5_83EC_00C0262A4671__INCLUDED_)
