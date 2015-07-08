# Microsoft Developer Studio Project File - Name="BurnTool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=BurnTool - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "BurnTool.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BurnTool.mak" CFG="BurnTool - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BurnTool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "BurnTool - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BurnTool - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\CtrlSource" /I ".\fslib" /I ".\unicode" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 transc.lib USBTransc.lib fslib\fat32fs.lib fslib\mount.lib Unicode\unicode.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "BurnTool - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\CtrlSource" /I ".\fslib" /I ".\unicode" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "UNICODE" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 transc_debug.lib USBTransc_debug.lib Fslib\fat32fs.lib Fslib\mount.lib Unicode\unicode.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "BurnTool - Win32 Release"
# Name "BurnTool - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AKFS.cpp
# End Source File
# Begin Source File

SOURCE=.\Burn.cpp
# End Source File
# Begin Source File

SOURCE=.\BurnTool.cpp
# End Source File
# Begin Source File

SOURCE=.\BurnTool.rc
# End Source File
# Begin Source File

SOURCE=.\BurnToolDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\BurnToolView.cpp
# End Source File
# Begin Source File

SOURCE=.\CheckExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Config.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\CoolTabCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgImageCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgPassword.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgPasswordChange.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSpiImageCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode\eng_dataconvert.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\Lang.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\ListCellEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\ListCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\logFile.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MyEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\nand_list.cpp
# End Source File
# Begin Source File

SOURCE=.\PageBaseband.cpp
# End Source File
# Begin Source File

SOURCE=.\PageDownload.cpp
# End Source File
# Begin Source File

SOURCE=.\PageFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\PageGeneral.cpp
# End Source File
# Begin Source File

SOURCE=.\PageHardware.cpp
# End Source File
# Begin Source File

SOURCE=.\PageNandflash.cpp
# End Source File
# Begin Source File

SOURCE=.\PageSpiflash.cpp
# End Source File
# Begin Source File

SOURCE=.\PageSpiNandflash.cpp
# End Source File
# Begin Source File

SOURCE=.\ramconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\scbarg.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\sizecbar.cpp
# End Source File
# Begin Source File

SOURCE=.\SpiImageCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\TreePropSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\TreePropSheetPgFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\TreePropSheetPgFrameDef.cpp
# End Source File
# Begin Source File

SOURCE=.\Update.cpp
# End Source File
# Begin Source File

SOURCE=.\UpdateBase.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\VisualStylesXP.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AKFS.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\anyka_types.h
# End Source File
# Begin Source File

SOURCE=.\Burn.h
# End Source File
# Begin Source File

SOURCE=.\BurnTool.h
# End Source File
# Begin Source File

SOURCE=.\BurnToolDoc.h
# End Source File
# Begin Source File

SOURCE=.\BurnToolView.h
# End Source File
# Begin Source File

SOURCE=.\CheckExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\Config.h
# End Source File
# Begin Source File

SOURCE=.\DlgConfig.h
# End Source File
# Begin Source File

SOURCE=.\DlgImageCreate.h
# End Source File
# Begin Source File

SOURCE=.\DlgPassword.h
# End Source File
# Begin Source File

SOURCE=.\DlgPasswordChange.h
# End Source File
# Begin Source File

SOURCE=.\DlgSpiImageCreate.h
# End Source File
# Begin Source File

SOURCE=.\Unicode\eng_dataconvert.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\fha.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\fha_asa.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\file.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\fs.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\fsa.h
# End Source File
# Begin Source File

SOURCE=.\IFWD_DownloadDll.h
# End Source File
# Begin Source File

SOURCE=.\IFWD_std_types.h
# End Source File
# Begin Source File

SOURCE=.\ImageCreate.h
# End Source File
# Begin Source File

SOURCE=.\Lang.h
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\ListCellEdit.h
# End Source File
# Begin Source File

SOURCE=.\CtrlSource\ListCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\logFile.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\medium.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\mtdlib.h
# End Source File
# Begin Source File

SOURCE=.\Fslib\nandflash.h
# End Source File
# Begin Source File

SOURCE=.\PageBaseband.h
# End Source File
# Begin Source File

SOURCE=.\PageDownload.h
# End Source File
# Begin Source File

SOURCE=.\PageFormat.h
# End Source File
# Begin Source File

SOURCE=.\PageGeneral.h
# End Source File
# Begin Source File

SOURCE=.\PageHardware.h
# End Source File
# Begin Source File

SOURCE=.\PageNandflash.h
# End Source File
# Begin Source File

SOURCE=.\PageSpiflash.h
# End Source File
# Begin Source File

SOURCE=.\PageSpiNandflash.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SpiImageCreate.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\transc.h
# End Source File
# Begin Source File

SOURCE=.\Unicode\unicode_api.h
# End Source File
# Begin Source File

SOURCE=.\Update.h
# End Source File
# Begin Source File

SOURCE=.\UpdateBase.h
# End Source File
# Begin Source File

SOURCE=.\USBTransc.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\BurnTool.ico
# End Source File
# Begin Source File

SOURCE=.\res\BurnTool.rc2
# End Source File
# Begin Source File

SOURCE=.\res\BurnToolDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Client0.ico
# End Source File
# Begin Source File

SOURCE=.\res\Client1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Client2.ico
# End Source File
# Begin Source File

SOURCE=.\res\Client3.ico
# End Source File
# Begin Source File

SOURCE=.\res\export.ico
# End Source File
# Begin Source File

SOURCE=.\res\format.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\import.ico
# End Source File
# Begin Source File

SOURCE=".\res\Key manager.ico"
# End Source File
# Begin Source File

SOURCE=.\res\Left.ico
# End Source File
# Begin Source File

SOURCE=.\res\Preferences.ico
# End Source File
# Begin Source File

SOURCE=.\res\Right.ico
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\IFWD_DownloadDll.lib
# End Source File
# Begin Source File

SOURCE=.\transc.lib
# End Source File
# Begin Source File

SOURCE=.\USBTransc.lib
# End Source File
# Begin Source File

SOURCE=.\Fslib\mount.lib
# End Source File
# Begin Source File

SOURCE=.\Fslib\fat32fs.lib
# End Source File
# Begin Source File

SOURCE=.\Unicode\unicode.lib
# End Source File
# Begin Source File

SOURCE=.\Fslib\EXNFTL\mtdlib.lib
# End Source File
# Begin Source File

SOURCE=.\Fslib\fsa.lib
# End Source File
# Begin Source File

SOURCE=.\Fslib\fha.lib
# End Source File
# End Target
# End Project
