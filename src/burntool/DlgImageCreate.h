#if !defined(AFX_DLGIMAGECREATE_H__1558FC69_BF28_41B7_A48C_BF87274EDA99__INCLUDED_)
#define AFX_DLGIMAGECREATE_H__1558FC69_BF28_41B7_A48C_BF87274EDA99__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgImageCreate.h : header file
//

#include "ImageCreate.h"

/////////////////////////////////////////////////////////////////////////////
// DlgImageCreate dialog

class DlgImageCreate : public CDialog
{
// Construction
public:
	DlgImageCreate(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgImageCreate)
	enum { IDD = IDD_DLG_IMAGE_CREATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

public:
    CImageCreate   m_image_create;
    CProgressCtrl  *progCtrl;
    UINT           ctrl_time;
    DWORD           used_time;
	DWORDLONG      prog_file_total_size;
	BOOL ExitFlag;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgImageCreate)
	protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


protected:
	TCHAR   source_path[MAX_PATH+1];
	TCHAR   dest_path[MAX_PATH+1];
	TCHAR   name[40];
	UINT    capacity;
	UINT    page_size;
    UINT    sector_size;


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgImageCreate)
	virtual BOOL OnInitDialog();
	afx_msg void OnCreate();
	virtual void OnCancel();
	afx_msg void OnButtonDestBrowser();
	afx_msg void OnButtonSourceBrowser();
	afx_msg void OnEditchangeComboPagesize();
	afx_msg void OnStaticUsedTime();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeComboPagesize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	static DWORD WINAPI image_create_thread(LPVOID para);		 
protected:
	void  defaultConfig();
	void  SetupDisplay();
    BOOL  get_config_data();
    BOOL  browser_folder(TCHAR *folderPath);
    BOOL  SetProgressPos();
    void  StartTimer();
    void  StopTimer();
    void  display_used_time();
    DWORD offline_image_create();
	BOOL GetFileInfoInDir(LPTSTR pathFloder, DWORD *FileCnt, DWORDLONG *FileSize, DWORDLONG *MinSize);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGIMAGECREATE_H__1558FC69_BF28_41B7_A48C_BF87274EDA99__INCLUDED_)
