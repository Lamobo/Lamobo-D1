// BurnTool.h : main header file for the BURNTOOL application
//

#if !defined(AFX_BURNTOOL_H__839CE2DD_9FA7_4A96_B2F7_69B47A50814E__INCLUDED_)
#define AFX_BURNTOOL_H__839CE2DD_9FA7_4A96_B2F7_69B47A50814E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "Config.h"
#include "Lang.h"
#include "Trace.h"

#define PRODUCE_VERSION_CKECK_OFFSET	32
#define PRODUCE_VERSION_DATA_OFFSET		36

#define VER_CHECK0			'B'
#define VER_CHECK1			'U'
#define VER_CHECK2			'R'
#define VER_CHECK3			'N'

#define MAIN_VERSION		 5
#define SUB_VERSION0		 0
#define SUB_VERSION1		 42
#define SUB_VERSION2		 0

#define CONFIG_FILE_NAME _T("config.txt")
#define CONFIG_ADDR_FILE_NAME _T("config_addr.txt")
/////////////////////////////////////////////////////////////////////////////
// CBurnToolApp:
// See BurnTool.cpp for the implementation of this class
//

BOOL BurnThread(UINT nID);
void BurnProgress(UINT nID, UINT nDatLen);  
UINT config_ram_param(T_RAM_REG *ram);

class CBurnToolApp : public CWinApp
{
public:
	CBurnToolApp();

public:
	UINT  m_config_index;
    UINT  m_config_file_cnt;
    TCHAR m_config_file[10][30];

	CLang m_lang;
	TCHAR m_path[MAX_PATH+1];
	char  m_path_asc[MAX_PATH+1];
    TCHAR m_Open_prop[MAX_PATH];

	TCHAR *ConvertAbsolutePath(LPCTSTR path);

	char *ConvertAbsolutePath_ASC(LPCTSTR path);

	CString GetString(UINT strID);
    UINT GetConfigFiles();
    BOOL IsConfigFile(unsigned short *fileName);

protected:
    BOOL  WhetherAppOpen();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBurnToolApp)
	public:
	virtual BOOL InitInstance();
  
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CBurnToolApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BURNTOOL_H__839CE2DD_9FA7_4A96_B2F7_69B47A50814E__INCLUDED_)
