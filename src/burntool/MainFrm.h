// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__0BFFC4D2_A974_4C2D_95F4_A230406E9B6D__INCLUDED_)
#define AFX_MAINFRM_H__0BFFC4D2_A974_4C2D_95F4_A230406E9B6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "logFile.h"
#include "ImageCreate.h"
#include "Burn.h"

#define ID_COM_BAR	123
#define ID_TAB_CTL	125

typedef struct 
{
	UINT nID;
	UINT time_count;
}T_BURN_TIME;

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CEdit			*m_pEdit;

	CCoolBar		m_ComBar;
	CToolBar		m_ToolBar;
    CDialogBar		m_DlgBar;
    
	UINT			m_device_num;

	UINT			m_main_timer;
	UINT			m_udisk_timer;
	UINT			m_time_count;

	UINT			download_length_data;
	UINT			download_length_mtd;
	UINT			download_length_udisk;
	UINT			download_length_new_mtd;
	UINT            bin_pos;

	T_BURN_TIME		m_burn_time[32];

	BYTE			m_udisk_state[32];
    
    UINT		    m_cur_link;
    UINT            m_cur_useful;
	UINT			m_total_num;
	UINT			m_total_pass;
    UINT			m_total_fail;

	CImageCreate	m_image_create;

    int				m_update_udisk_cnt;
	BOOL			m_bScan;	
    CLogFile        frmLogfile;

// Operations
public:
	void enable_control();//可用
	void disable_control();//不可用

    void StartTimer();//开始时间
	void StopTimer();//停止时间

	void SetupDisplay();
	void RefreshLangMenu();//刷新

	void StartCountTime(UINT nID);//开始计时
	void StopCountTime(UINT nID);//停止计时

	void ImportUptFile();//导入

	void ScanUDisk();//u变化

	int GetUDisk(char pDiskName[], int len);//获取u盘个数
    
    void RefreshWorkInfo();//
	void set_window_title();
	
	static DWORD WINAPI image_create_thread(LPVOID para);//
	static DWORD WINAPI online_image_create_thread(LPVOID para);//其也
	static DWORD WINAPI online_image_create_thread_snowdirdL(LPVOID para);//10L
    BOOL GetFileInfoInDir(LPTSTR pathFloder, DWORD *FileCnt, DWORD *FileSize, DWORD *MinSize);
  
protected:

//	HDEVNOTIFY m_hDevNotify;
	DoRegisterDeviceInterface();
	void OnMacAddr(int port, BOOL falg);
	void OnSequenceAddr(int port, BOOL falg);
	void terminate_mission(int id, LPCTSTR str);

	void fail_in_module_burn(int id, LPCTSTR str);

  	void onTbtbClrRec();
	
	//void DisplayWorkNO();
	BOOL ClearLogFile(VOID);
	void OnClose();
	void SetConfigMenu();
    
protected:
	BOOL SetupCOM(int com_num);
	BOOL SetupBurn(int device_num);

	void find_resource(LPCTSTR resource_path, LPCTSTR dest_path, BOOL compare);
	void SetupDownload();
	void CountDownloadLength();	
	//void ParamConfig();
	void OnChangeConfig();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	VOID SendCmdToResetUSB(char diskname);
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTbtnStart();
	afx_msg void OnTbtnSetting();
	afx_msg void OnTbtnImport();
	afx_msg void OnTbtnExport();
	afx_msg void OnTbtnFormat();
	afx_msg void OnViewCom();
	afx_msg void OnUpdateViewCom(CCmdUI* pCmdUI);
	afx_msg void OnSettingChangePasswd();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnUpdateSettingChangePasswd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAppHelp(CCmdUI* pCmdUI);
	afx_msg void OnAppHelp();
	afx_msg void OnToolImageMaker();
	afx_msg void OnSpiImageMaker();
	afx_msg void OnBinUpload();
	afx_msg void OnOnlineImageMake();
	afx_msg void OnAutoBurn();
	afx_msg void OnChangeConfig0();
    afx_msg void OnChangeConfig1();
    afx_msg void OnChangeConfig2();
    afx_msg void OnChangeConfig3();
    afx_msg void OnChangeConfig4();
    afx_msg void OnChangeConfig5();
	afx_msg void OnCheckUdiskBurn();
	//}}AFX_MSG
	LRESULT On_Receive(WPARAM wp, LPARAM lp);
	LRESULT On_Message(WPARAM wp, LPARAM lp);
 	LRESULT On_Process(WPARAM wp, LPARAM lp);
    LRESULT On_DeviceArrive(WPARAM wp, LPARAM lp);
    LRESULT On_DeviceRemove(WPARAM wp, LPARAM lp);
	LRESULT On_ModuleBurnMessage(WPARAM wp, LPARAM lp);
    LRESULT On_ModuleBurnProgress(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__0BFFC4D2_A974_4C2D_95F4_A230406E9B6D__INCLUDED_)
