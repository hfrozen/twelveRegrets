; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=COpen
LastTemplate=CEdit
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Termial.h"

ClassCount=10
Class1=CTermialApp
Class2=CTerminalDlg
Class3=CAboutDlg

ResourceCount=11
Resource1=IDR_MENU_TERMINAL
Resource2=IDR_MAINFRAME
Resource3=IDD_TERMIAL_DIALOG
Resource4=IDD_CSUM_DIALOG
Class4=COpen
Resource5=IDD_MACRO_DIALOG
Class5=CCtrlChar
Resource6=IDD_OPEN_DIALOG
Class6=CSum
Resource7=IDD_TERMINAL_DIALOG
Class7=CPort
Resource8=IDD_CTRLCHAR_DIALOG
Resource9=IDD_ABOUTBOX
Class8=CMacroEdit
Resource10=IDD_NEW_MACRO_DIALOG
Class9=CNewMacro
Class10=CViewEdit
Resource11=IDR_ACCELERATOR

[CLS:CTermialApp]
Type=0
HeaderFile=Termial.h
ImplementationFile=Termial.cpp
Filter=N

[CLS:CAboutDlg]
Type=0
HeaderFile=TermialDlg.h
ImplementationFile=TermialDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_TERMIAL_DIALOG]
Type=1
Class=CTerminalDlg
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDC_EDIT_TX,edit,1350631424
Control3=IDC_STATIC_RX,static,1342312448

[MNU:IDR_MENU_TERMINAL]
Type=1
Class=?
Command1=IDC_OPEN
Command2=ID_APP_EXIT
Command3=IDC_CONTROL_CHAR
Command4=IDC_CHKSUM
Command5=IDC_CLEAR_RX
Command6=IDC_MACRO_EDIT
CommandCount=6

[DLG:IDD_OPEN_DIALOG]
Type=1
Class=COpen
ControlCount=25
Control1=IDC_COMBO_PORT,combobox,1344340226
Control2=IDC_COMBO_BPS,combobox,1344340226
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_RADIO_DATA7,button,1342308361
Control6=IDC_RADIO_DATA8,button,1342177289
Control7=IDC_RADIO_EVEN,button,1342308361
Control8=IDC_RADIO_ODD,button,1342177289
Control9=IDC_RADIO_NON,button,1342177289
Control10=IDC_RADIO_STOP1,button,1342308361
Control11=IDC_RADIO_STOP2,button,1342177289
Control12=IDC_RADIO_DISABLE_DTR,button,1342308361
Control13=IDC_RADIO_ENABLE_DTR,button,1342177289
Control14=IDC_RADIO_HANDSHAKE_DTR,button,1342177289
Control15=IDC_RADIO_DISABLE_RTS,button,1342308361
Control16=IDC_RADIO_ENABLE_RTS,button,1342177289
Control17=IDC_RADIO_HANDSHAKE_RTS,button,1342177289
Control18=IDC_RADIO_TOGGLE_RTS,button,1342177289
Control19=IDC_STATIC,button,1342177287
Control20=IDC_STATIC,button,1342177287
Control21=IDC_STATIC,button,1342177287
Control22=IDC_STATIC,button,1342177287
Control23=IDC_STATIC,button,1342177287
Control24=IDC_STATIC,button,1342177287
Control25=IDC_STATIC,button,1342177287

[CLS:COpen]
Type=0
HeaderFile=Open.h
ImplementationFile=Open.cpp
BaseClass=CDialog
Filter=D
LastObject=COpen
VirtualFilter=dWC

[DLG:IDD_CTRLCHAR_DIALOG]
Type=1
Class=CCtrlChar
ControlCount=18
Control1=IDC_EDIT_SOH,edit,1350631552
Control2=IDC_EDIT_STX,edit,1350631552
Control3=IDC_EDIT_ETX,edit,1350631552
Control4=IDC_EDIT_EOT,edit,1350631552
Control5=IDC_EDIT_ENQ,edit,1350631552
Control6=IDC_EDIT_ACK,edit,1350631552
Control7=IDC_EDIT_NAK,edit,1350631552
Control8=IDOK,button,1342242817
Control9=IDCANCEL,button,1342242816
Control10=IDC_STATIC,static,1342308864
Control11=IDC_STATIC,static,1342308864
Control12=IDC_STATIC,static,1342308864
Control13=IDC_STATIC,static,1342308864
Control14=IDC_STATIC,static,1342308864
Control15=IDC_STATIC,static,1342308864
Control16=IDC_STATIC,static,1342308864
Control17=IDC_STATIC,static,1342308864
Control18=IDC_EDIT_SUM,edit,1350631552

[CLS:CCtrlChar]
Type=0
HeaderFile=CtrlChar.h
ImplementationFile=CtrlChar.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CCtrlChar

[DLG:IDD_CSUM_DIALOG]
Type=1
Class=CSum
ControlCount=13
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_RADIO_ADDSUM,button,1342308361
Control4=IDC_RADIO_XORSUM,button,1342177289
Control5=IDC_RADIO_XORP1SUM,button,1342177289
Control6=IDC_RADIO_XORM1SUM,button,1342177289
Control7=IDC_RADIO_ASCIISUM,button,1342308361
Control8=IDC_RADIO_BINSUM,button,1342177289
Control9=IDC_RADIO_BYTESUM,button,1342308361
Control10=IDC_RADIO_WORDSUM,button,1342177289
Control11=IDC_STATIC,button,1342177287
Control12=IDC_STATIC,button,1342177287
Control13=IDC_STATIC,button,1342177287

[CLS:CSum]
Type=0
HeaderFile=Sum.h
ImplementationFile=Sum.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSum

[ACL:IDR_ACCELERATOR]
Type=1
Class=?
Command1=IDC_CONTROL_CHAR
Command2=IDC_MACRO_EDIT
Command3=IDC_OPEN
Command4=IDC_CHKSUM
Command5=ID_APP_EXIT
CommandCount=5

[DLG:IDD_TERMINAL_DIALOG]
Type=1
Class=CTerminalDlg
ControlCount=4
Control1=IDC_EDIT_TX,edit,1350631424
Control2=IDOK,button,1342242817
Control3=IDC_EDIT_RX,edit,1352665156
Control4=IDC_STATIC,static,1342308865

[CLS:CTerminalDlg]
Type=0
HeaderFile=terminaldlg.h
ImplementationFile=terminaldlg.cpp
BaseClass=CDialog
LastObject=CTerminalDlg
Filter=D
VirtualFilter=dWC

[DLG:IDD_MACRO_DIALOG]
Type=1
Class=CMacroEdit
ControlCount=4
Control1=IDC_MACRO_NEW,button,1342242816
Control2=IDC_MACRO_DEL,button,1342242816
Control3=IDC_MACRO_END,button,1342242817
Control4=IDC_MACRO_LIST,SysListView32,1350631425

[CLS:CMacroEdit]
Type=0
HeaderFile=MacroEdit.h
ImplementationFile=MacroEdit.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CMacroEdit

[DLG:IDD_NEW_MACRO_DIALOG]
Type=1
Class=CNewMacro
ControlCount=6
Control1=IDC_MACRO_NAME_EDIT,edit,1350633600
Control2=IDC_MACRO_STRING_EDIT,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_STATIC,static,1342308864
Control6=IDC_STATIC,static,1342308864

[CLS:CNewMacro]
Type=0
HeaderFile=NewMacro.h
ImplementationFile=NewMacro.cpp
BaseClass=CDialog
Filter=D
LastObject=CNewMacro
VirtualFilter=dWC

[CLS:CViewEdit]
Type=0
HeaderFile=ViewEdit.h
ImplementationFile=ViewEdit.cpp
BaseClass=CEdit
Filter=W
LastObject=CViewEdit
VirtualFilter=WC

