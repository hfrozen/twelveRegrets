#pragma once

#include "ClientSock.h"

#define	WM_NODECLOSE		WM_USER + 10

class CNode : public CDialogEx
{
	DECLARE_DYNAMIC(CNode)

public:
	CNode(CWnd* pParent = NULL);
	virtual ~CNode();

	enum { IDD = IDD_DIALOG_CLIENT };

	CClientSock*	m_pClientSock;

protected:
	CWnd*			m_pParent;
	CRichEditCtrl	m_edit;

	typedef union _tagBCDT {
		struct {
			BYTE	one :	4;
			BYTE	ten :	4;
		} b;
		BYTE a;
	} BCDT;

#pragma pack(push, 1)
	typedef union _tagDBFTIME {
		struct {
			DWORD	year	: 6;	// 0~63 -> 2010~2073
			DWORD	mon		: 4;
			DWORD	day		: 5;
			DWORD	hour	: 5;
			DWORD	min		: 6;
			DWORD	sec		: 6;
		} t;						// 2/4
		DWORD dw;
	} DBFTIME;

#define	SIZE_TODDUMMY	1398		// 1398 + 10 = 1408
	typedef struct _tagTODT {
		BYTE	ctrl;		// 1
		BYTE	cnt;		// 2
		struct {
			BCDT	year;	// 3
			BCDT	mon;	// 4
			BCDT	day;	// 5
			BCDT	hour;	// 6
			BCDT	min;	// 7
			BCDT	sec;	// 8
		} t;
		struct {
			BCDT	th;		// 9
			BCDT	to;		// 10
		} n;
		BYTE	dummy[SIZE_TODDUMMY];
	} TODT;
	TODT	m_todt;
#pragma pack(pop)

	void	Log(CString strLog, COLORREF crText = RGB(0, 0, 0));
	BYTE	ToBin(BYTE c);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClickedButtonClear();
	//afx_msg void OnClickedButtonSend();
	afx_msg LRESULT OnConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
};
