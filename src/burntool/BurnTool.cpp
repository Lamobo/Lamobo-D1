// BurnTool.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "BurnTool.h"

#include "MainFrm.h"
#include "BurnToolDoc.h"
#include "BurnToolView.h"
#include "transc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBurnToolApp

BEGIN_MESSAGE_MAP(CBurnToolApp, CWinApp)
	//{{AFX_MSG_MAP(CBurnToolApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBurnToolApp construction

CBurnToolApp::CBurnToolApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBurnToolApp object

CBurnToolApp theApp;
CConfig theConfig;
HINSTANCE _hInstance;
CString strpmid;
CString strrdid;

/////////////////////////////////////////////////////////////////////////////
// CBurnToolApp initialization
BOOL CBurnToolApp::InitInstance()
{
    //BT_Init(8,  BurnThread,  BurnProgress);
	AfxEnableControlContainer();
    
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
   
    if (FALSE == WhetherAppOpen())      // check the app whether had been Open
    {
        return FALSE;
    }

    //theConfig.ReadConfig(CONFIG_FILE_NAME);
	theConfig.read_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, (TCHAR *)&theConfig.g_mac_current_low, theConfig.mac_current_low);
	theConfig.read_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, (TCHAR *)&theConfig.g_sequence_current_low, theConfig.sequence_current_low);

	TCHAR strLang[256];
	UINT  i;
	
	GetConfigFiles();
	
	m_config_index = 0;
	
	if (m_config_file_cnt > 0)
	{
		GetPrivateProfileString(_T("Config"), _T("File Name"), _T(""), strLang, 255, 
			ConvertAbsolutePath(_T("burn.ini")));
		for (i=0; i<m_config_file_cnt; i++)
		{
			if (0 == _tcsncmp(strLang, m_config_file[i], 30))
			{
				m_config_index = i;
				break;
			}
		}
		theConfig.ReadConfig(m_config_file[m_config_index]);
	}
	else
	{
		m_config_file_cnt = 1;	// create default config file;
		_tcsncpy(m_config_file[0], _T("config.txt"), 30);
		theConfig.ReadConfig(m_config_file[m_config_index]);
	}
	
	if (theConfig.device_num > MAX_DEVICE_NUM)
		theConfig.device_num = MAX_DEVICE_NUM;

	BT_Init(theConfig.device_num,  BurnThread,  BurnProgress);
    //get string in certain language
	GetPrivateProfileString(_T("Lang"), _T("Lang"), _T(""), strLang, 255, 
		ConvertAbsolutePath(_T("burn.ini")));

	m_lang.Init(strLang);

	_hInstance = theApp.m_hInstance;
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CBurnToolDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CBurnToolView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	//m_pMainWnd->ShowWindow(SW_SHOW);
	//m_pMainWnd->UpdateWindow();
	
	return TRUE;
}

TCHAR *CBurnToolApp::ConvertAbsolutePath(LPCTSTR path)
{
    CString sPath;
	CString filePath;

    if (path[0] == '\0')
    {
        return NULL;
    }
	else if ((':' == path[1]) || (('\\'==path[0]) && ('\\'==path[1])))
	{
		_tcsncpy(m_path, path, MAX_PATH);
	}
	else
	{
		GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);

		sPath.ReleaseBuffer ();
		int nPos;
		nPos=sPath.ReverseFind ('\\');
		sPath=sPath.Left (nPos+1);

		filePath = sPath + path;

		_tcsncpy(m_path, filePath, MAX_PATH);
	}

	return m_path;
}

char *CBurnToolApp::ConvertAbsolutePath_ASC(LPCTSTR path)
{
    CString sPath;
	CString filePath;

	USES_CONVERSION;

	if(':' == path[1])
	{
		strncpy(m_path_asc, T2A(path), MAX_PATH);
	}
	else
	{
		GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);

		sPath.ReleaseBuffer ();
		int nPos;
		nPos=sPath.ReverseFind ('\\');
		sPath=sPath.Left (nPos+1);

		filePath = sPath + path;

		strncpy(m_path_asc, T2A(filePath), MAX_PATH);
	}

	return m_path_asc;
}

CString CBurnToolApp::GetString(UINT strID)
{
	return m_lang.GetString(strID);
}


BOOL CBurnToolApp::WhetherAppOpen()
{
    TCHAR  initStr[255];

    GetPrivateProfileString(_T("Open"), _T("Open"), _T(""), initStr, 255, 
		ConvertAbsolutePath(_T("burn.ini")));

    if (0 == _tcscmp(initStr, _T("Y")))     // 程序已打开, 激活该程序窗口
    {
        GetPrivateProfileString(_T("Open"), _T("Prop"), NULL, initStr, 255, ConvertAbsolutePath(_T("burn.ini")));

        HWND  hWndPrev = ::GetWindow(::GetDesktopWindow(),GW_CHILD);
        
        while ( ::IsWindow(hWndPrev) )
        {
            if ( ::GetProp(hWndPrev, initStr) )
            {
                if (::IsIconic(hWndPrev))
                {
                    ::ShowWindow(hWndPrev, SW_RESTORE);
                }
                SetForegroundWindow(hWndPrev);
                return FALSE;
            }

            hWndPrev = ::GetWindow(hWndPrev, GW_HWNDNEXT);
        }
        // 没找到窗口
    }
    // else        // 程序没打开, 往初始化配置文件里做已打开标志
    {
        WritePrivateProfileString(_T("Open"),_T("Open"),_T("Y"),ConvertAbsolutePath(_T("burn.ini")));
        
        SYSTEMTIME  InitTime;
        GetLocalTime(&InitTime);
        swprintf(m_Open_prop, _T("%02d%02d%02d%02d%02d"), InitTime.wMonth,InitTime.wDay, InitTime.wHour,InitTime.wMinute,InitTime.wSecond);
        WritePrivateProfileString(_T("Open"),_T("Prop"),m_Open_prop,ConvertAbsolutePath(_T("burn.ini")));
    }

    return TRUE;
}

UINT CBurnToolApp::GetConfigFiles()
{
    WIN32_FIND_DATA  FindFileData; 
    HANDLE hSearch;
    BOOL   fFinish = FALSE;
    UINT   cnt = 0;

    m_config_file_cnt = 0;
    hSearch = FindFirstFile(_T("*.txt"), &FindFileData); 
    if (hSearch == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    while (!fFinish)
    {
        if (IsConfigFile(FindFileData.cFileName))
        {
            _tcsncpy(m_config_file[m_config_file_cnt], FindFileData.cFileName,30);
            m_config_file_cnt++;
        }
        
        if (m_config_file_cnt > 9) fFinish = TRUE;

        if (!FindNextFile(hSearch, &FindFileData)) 
        {
            if (ERROR_NO_MORE_FILES == GetLastError()) 
            { 
     			hSearch = INVALID_HANDLE_VALUE;
                fFinish = TRUE; 
            } 
        }
    }

	
	if (hSearch != INVALID_HANDLE_VALUE)
	    CloseHandle(hSearch);
	
    return m_config_file_cnt;
}

BOOL CBurnToolApp::IsConfigFile(unsigned short *fileName)
{
    UINT name_len = _tcslen(fileName);
    UINT i;
    
    for (i=0; i<name_len-6; i++)
    {
        if( ((fileName[i] == 'c') || (fileName[i] == 'C')) &&
            ((fileName[i+1] == 'o') || (fileName[i+1] == 'O')) &&
            ((fileName[i+2] == 'n') || (fileName[i+2] == 'N')) &&
            ((fileName[i+3] == 'f') || (fileName[i+3] == 'F')) &&
            ((fileName[i+4] == 'i') || (fileName[i+4] == 'I')) &&
            ((fileName[i+5] == 'g') || (fileName[i+5] == 'G')) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CBurnToolApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CBurnToolApp message handlers


BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UINT lib_main_ver, lib_sub_ver1, lib_sub_ver2;
	UINT usb_main_ver, usb_sub_ver;

	BT_GetVersion(&lib_main_ver, &lib_sub_ver1, &lib_sub_ver2);
	BT_GetUSBLibVersion(&usb_main_ver, &usb_sub_ver);
	
	CString strTitle;
	strTitle.Format(_T("Version:BurnTool %d.%d.%02d.%02d, transclib %d.%d.%02d, usblib:%d.%d"), 
		MAIN_VERSION, SUB_VERSION0, SUB_VERSION1, SUB_VERSION2, lib_main_ver, lib_sub_ver1, lib_sub_ver2, usb_main_ver, usb_sub_ver);
	GetDlgItem(IDC_STATIC_VERSION)->SetWindowText(strTitle);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

