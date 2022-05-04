// Refer.h
#pragma once

enum {
	FILETYPE_TROUBLE,
	//FILETYPE_ARMLOG,
	//FILETYPE_LOG,
	FILETYPE_LOGBOOK,
	FILETYPE_INSPECT,
	FILETYPE_MAX
};

enum {
	OPERITEM_HIERARCHY,
	OEPRITEM_TYPE,
	OPERITEM_NAME,
	OPERITEM_FLAG,
	OPERITEM_OFFSET,
	OPERITEM_REF,
	OPERITEM_MAX
};

#define	SIZE_TROUBLEBLOCK	8
#define	SIZE_LOGBOOKBLOCK	504
#define	SIZE_LOGBOOKHEAD	32
#define	SIZE_INSPECTFILE	768

#define	WM_OPENFILE		WM_USER + 22
#define	WM_SELITEM		WM_USER + 24
#define	WM_DROPITEM		WM_USER + 26
#define	WM_PARTSELECT	WM_USER + 28
#define	WM_PARTDELETE	WM_USER + 30
#define	WM_ITEMDELETE	WM_USER + 32

typedef union	_tagDUET {
	BYTE	c[2];
	WORD	w;
} _DUET;

typedef union	_tagQUARTET {
	BYTE	c[4];
	WORD	w[2];
	DWORD	dw;
} _QUARTET;

typedef union	_tagOCTET {
	BYTE	c[8];
	WORD	w[4];
	DWORD	dw[2];
	QWORD	qw;
} _OCTET;

typedef union _tagDBFTIME {
	struct {
		DWORD	year	: 6;	// 0~63 -> 2010~2073
		DWORD	mon		: 4;
		DWORD	day		: 5;
		DWORD	hour	: 5;
		DWORD	min		: 6;
		DWORD	sec		: 6;
	} t;						// 2/4
	DWORD	dw;
} DBFTIME, *PDBFTIME;

typedef union _tagTROUBLEARCHIVE {
	struct {
		QWORD	down	:  1;	// ������..
		QWORD	alarm	:  1;	// type�߿��� ����ڰ� �����ϴ� ������ �� 1�� �ȴ�. �ڴ��� ������ �����.
		QWORD	type	:  4;	// see ALARMTYPE enum, ���� �� ���� ������?
		QWORD	cid		:  4;	// 0~9, ���� ��ȣ
		QWORD	code	: 11;	// ���� �ڵ�, 0x7ff-2047
		QWORD	mon		:  4;	// 0~11, �߻��� ��(1~12)
		QWORD	day		:  5;	// 0~30, ...    ��(1~31)
		QWORD	uhour	:  5;	// 0~23, ...    ��(0~23)
		QWORD	umin	:  6;	// 0~59, ...    ��(0~59)
		QWORD	usec	:  6;	// 0~59, ...    ��(0~59)
		QWORD	dhour	:  5;	// 0~23, �Ҹ��� ��(0~23)
		QWORD	dmin	:  6;	// 0~59, ...    ��(0~59)
		QWORD	dsec	:  6;	// 0~59, ...    ��(0~59)
	} b;
	QWORD	a;
} TROUBLEARCH, *PTROUBLEARCH;

