// DlgImageCreate.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "DlgImageCreate.h"
#include "Burn.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CConfig theConfig;
extern CBurnToolApp theApp;


/////////////////////////////////////////////////////////////////////////////
// DlgImageCreate dialog


DlgImageCreate::DlgImageCreate(CWnd* pParent /*=NULL*/)
	: CDialog(DlgImageCreate::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgImageCreate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DlgImageCreate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgImageCreate)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgImageCreate, CDialog)
	//{{AFX_MSG_MAP(DlgImageCreate)
	ON_BN_CLICKED(IDCREATE, OnCreate)
	ON_BN_CLICKED(IDC_BUTTON_DEST_BROWSER, OnButtonDestBrowser)
	ON_BN_CLICKED(IDC_BUTTON_SOURCE_BROWSER, OnButtonSourceBrowser)
	ON_CBN_EDITCHANGE(IDC_COMBO_PAGESIZE, OnEditchangeComboPagesize)
	ON_BN_CLICKED(IDC_STATIC_USED_TIME, OnStaticUsedTime)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO_PAGESIZE, OnSelchangeComboPagesize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgImageCreate message handlers

HANDLE offline_image_event;
UINT   g_offline_img_stat;

BOOL DlgImageCreate::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    CString str;

	SetupDisplay();//初始化一些值
	
    defaultConfig();  //离线镜像制作的默认值
	
    // 填充参数
    SetDlgItemText(IDC_EDIT_SOURCE_FOLDER, source_path);//源文件路径
    SetDlgItemText(IDC_EDIT_DEST_FOLDER, dest_path);//目标路径
    SetDlgItemText(IDC_EDIT_IMAGE_NAME, name);//镜像名
	
    str.Format(_T("%d"), sector_size);//
    SetDlgItemText(IDC_EDIT_SECTORSIZE,str); //扇区大小
	
    str.Format(_T("%d"), page_size);
    SetDlgItemText(IDC_COMBO_PAGESIZE,str);//页大小
	
    str.Format(_T("%d"), capacity);
    SetDlgItemText(IDC_EDIT_CAPACITY,str); //空量大小
    
    ctrl_time = 0;
    progCtrl = (CProgressCtrl *)GetDlgItem(IDC_PROGRESS_MAKING);
    progCtrl->SetRange(0,100);
    progCtrl->SetPos(0);
    progCtrl->SetStep(1);
	ExitFlag = TRUE;
	
    return TRUE;
}

void DlgImageCreate::SetupDisplay()
{
	CString str;

	str = theApp.GetString(IDS_SOURCE_FOLDER);
	GetDlgItem(IDC_STATIC_SOURCE_FOLDER)->SetWindowText(str); //源文件夹

	str = theApp.GetString(IDS_DEST_FOLDER);
	GetDlgItem(IDC_STATIC_DEST_FOLDER)->SetWindowText(str);//目标文件夹

	str = theApp.GetString(IDS_BROWSE_SOURCE_FOLDER);
	GetDlgItem(IDC_BUTTON_SOURCE_BROWSER)->SetWindowText(str);//浏览源文件夹

	str = theApp.GetString(IDS_BROWSE_DEST_FOLDER);
	GetDlgItem(IDC_BUTTON_DEST_BROWSER)->SetWindowText(str);//浏览目标文件夹

	str = theApp.GetString(IDS_IMAGE_FILE_NAME);
	GetDlgItem(IDC_STATIC_IMAGE_NAME)->SetWindowText(str);//镜像文件名

	str = theApp.GetString(IDS_TOTAL_CAPACITY);
	GetDlgItem(IDC_STATIC_CAPACITY)->SetWindowText(str);//容量大小

	
	str = theApp.GetString(IDS_PAGE_SIZE);
	GetDlgItem(IDC_STATIC_PAGESIZE)->SetWindowText(str);//页大小

	str = theApp.GetString(IDS_SECTOR_SIZE);
	GetDlgItem(IDC_STATIC_SECTORSIZE)->SetWindowText(str);//扇区大小

	str = theApp.GetString(IDS_MAKE_INFORMATION);
	GetDlgItem(IDC_MAKE_INFORMATION)->SetWindowText(str);//制作信息

	str = theApp.GetString(IDS_READY_MAKE);
	GetDlgItem(IDC_CURENT_INFO)->SetWindowText(str);//准备制作

	str = theApp.GetString(IDS_MAKE_BEGIN);
	GetDlgItem(IDCREATE)->SetWindowText(str);//开始

	str = theApp.GetString(IDS_MAKE_END);
	GetDlgItem(IDCANCEL)->SetWindowText(str);//完成

	str = theApp.GetString(IDS_MAKE_IMAGE_TITLE);	
	SetWindowText(str);
}





void DlgImageCreate::OnCreate() 
{
	// TODO: Add your control notification handler code here
	HANDLE tHandle = INVALID_HANDLE_VALUE;
    DWORD tid = 0;
	DWORD fileCnt = 0;
    
    if (get_config_data())
    {
        if ((page_size >= sector_size) && (page_size % 512 == 0))
        {
			SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_GET_FOLDER_INFO));
			prog_file_total_size = 0;
			//获取源文件夹
			if  (!GetFileInfoInDir(source_path, &fileCnt, &prog_file_total_size, NULL))
			{
				MessageBox(theApp.GetString(IDS_GET_FOLDER_INFO_FAIL));
				return;
			}
			//如果制作的文件大于空量，那么就出错
			if (capacity*1024*1024 < prog_file_total_size)
			{
				MessageBox(theApp.GetString(IDS_TOTAL_CAPACITY_NO_ENOUGH));
				return;
			}
			//进入创建镜像的线程
			SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_MAKING));
			tHandle = CreateThread(NULL, 0, image_create_thread, this, 0, &tid);
			CloseHandle(tHandle);

        }
        else
        {
			//页大小不对
            MessageBox(theApp.GetString(IDS_PARAM_ERROR));
        }
        //tHandle = CreateThread(NULL, 0, offline_image_create, &m_image_create, 0, &tid);
    }

    //CDialog::OnOK();
}
void DlgImageCreate::OnCancel() 
{
	// TODO: Add extra cleanup here
	m_image_create.ExitFlag = TRUE;
    while (!ExitFlag)
    {
        Sleep(10);
    }
	
	CDialog::OnCancel();
}

void DlgImageCreate::OnButtonDestBrowser() 
{
	// TODO: Add your control notification handler code here
	browser_folder(dest_path);
    SetDlgItemText(IDC_EDIT_DEST_FOLDER, dest_path);
	
}

void DlgImageCreate::OnButtonSourceBrowser() 
{
	// TODO: Add your control notification handler code here
	browser_folder(source_path);
    SetDlgItemText(IDC_EDIT_SOURCE_FOLDER, source_path);
	
}


void DlgImageCreate::defaultConfig()
{
    CString  str;
    //默认初始值
    _tcscpy(source_path, _T("D:\\img"));//初始化源文件路径
    _tcscpy(dest_path, _T("D:\\img_output"));//目标路径
    str.Format(_T("imgA"));//镜像名
    _tcscpy(name, str);
    capacity = 1024;//容量大小
    page_size = 512;//页大小
    sector_size = page_size;//扇区大小
}

BOOL DlgImageCreate::get_config_data()
{
    CString get_str;
    
    USES_CONVERSION;

    // source folder;
    GetDlgItemText(IDC_EDIT_SOURCE_FOLDER, get_str);
    if (get_str.IsEmpty())//是否为空
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_SOURCE_FOLDER_EMPTY));
        return FALSE;
    }
    else 
    {
        _tcsncpy(source_path, get_str, get_str.GetLength() + 1); //获取源路径
    }

    // destination folder
    GetDlgItemText(IDC_EDIT_DEST_FOLDER, get_str);
    if (get_str.IsEmpty())//是否为空
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_DEST_FOLDER_EMPTY));
        return FALSE;
    }
    else 
    {
        _tcsncpy( dest_path, get_str, get_str.GetLength() + 1); //目标路径
    }

    // image param
    GetDlgItemText(IDC_EDIT_IMAGE_NAME, get_str);
    if (get_str.IsEmpty())
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_IMAGE_NAME_EMPTY));
        return FALSE;
    }
    else
    {
        _tcsncpy( name, get_str, get_str.GetLength() + 1);//镜像名
    }

	//获取容量
    GetDlgItemText(IDC_EDIT_CAPACITY, get_str);
    if (get_str.IsEmpty())
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_CAPACITY_SIZE_EMPTY));
        return FALSE;
    }
    else
    {
        capacity = atoi(T2A(get_str));//容量
    }
    
    GetDlgItemText(IDC_EDIT_SECTORSIZE,get_str);
    if (get_str.IsEmpty())
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_SECTOR_SIZE_EMPTY));
        return FALSE;
    }
    else
    {
        sector_size = atoi(T2A(get_str));//扇区大小
    }

    GetDlgItemText(IDC_COMBO_PAGESIZE,get_str);
    if (get_str.IsEmpty())
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_PAGE_SIZE_EMPTY));
        return FALSE;
    }
    else
    {
        page_size = atoi(T2A(get_str));//页大小
    }
    
    return TRUE;
}

BOOL DlgImageCreate::browser_folder(TCHAR *folderPath)
{
    BROWSEINFOW opp;
    
    opp.hwndOwner = GetSafeHwnd();
    opp.pidlRoot = NULL;
    opp.pszDisplayName = folderPath;
    opp.lpszTitle = _T("Choose Path");//选择文件夹路径
    opp.ulFlags = BIF_RETURNONLYFSDIRS;
    opp.lpfn = NULL;
    opp.iImage = 0;

    LPITEMIDLIST pIDList = SHBrowseForFolder(&opp); //调用显示选择对话框

    if (pIDList)
    {
        SHGetPathFromIDList(pIDList, folderPath);
        return TRUE;
    }
    else return FALSE;
}

BOOL  DlgImageCreate::SetProgressPos()
{
    //progCtrl->SetPos(10);
    progCtrl->StepIt();

    return TRUE;
}

void  DlgImageCreate::StartTimer()
{
    used_time = 0;
    ctrl_time = SetTimer(1, 1000, NULL);//设置时间
}

void  DlgImageCreate::StopTimer()
{
    KillTimer(ctrl_time);//清空时间
    ctrl_time = 0;
}

DWORD  DlgImageCreate::offline_image_create()//离线制作
{
    UINT i=0, j=0;
    UINT path_len = 0;
    DWORD dwAttr;
    CString str;
    TCHAR  dest_file_path[MAX_PATH];
    TCHAR  img_file_path[MAX_PATH];
    
    USES_CONVERSION;

    if ( FALSE == m_image_create.fslib_init() )//初始化
    {
        return 0;
    }

    path_len = _tcslen(dest_path);//路径长度
    if (0 == path_len)
    {
        return 0;
    }
    else if ( (path_len > 0) && (dest_path[path_len-1] != '\\') )//目标路径
    {
        dest_path[path_len] = '\\';
        dest_path[path_len+1] = 0;
    }
    
    CreateDirectory(dest_path, NULL);   // destination path
    _tcscpy(dest_file_path, dest_path);//目标
    _tcscat(dest_file_path, name);//镜像名
    _tcscat(dest_file_path, _T(".img"));//镜像名的后缀
    if(!m_image_create.img_create(dest_file_path, capacity, sector_size, page_size, theConfig.burn_mode, 'A'))//开始制作镜像
    {
        return 0;
    }

    dwAttr = GetFileAttributes(source_path);//属性
    if(0xFFFFFFFF == dwAttr)
    {
        m_image_create.img_destroy();//销毁
        return 0;
    }

    _tcscpy(img_file_path, _T("A:"));
    if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
        if(!m_image_create.img_add_dir(source_path, T2A(img_file_path)))//加文件夹
        {
            m_image_create.img_destroy();//销毁
            return 0;
        }
    }
    else
    {
        if( !m_image_create.img_add_file(source_path, T2A(img_file_path)) )//加文件
        {
            m_image_create.img_destroy();//销毁
            return 0;
        }
    }
    m_image_create.img_destroy();//销毁
    
    return 1;
}

void DlgImageCreate::display_used_time()
{
    UINT  hour;
    UINT  minite;
    UINT  second;
    UINT  temp;

    hour = used_time/3600;//小时
    temp = used_time%3600;//
    minite = temp/60;//分钟
    second = temp%60;//移

    CString str;
    
    str.Format(_T("%02d:%02d:%02d"),hour,minite,second);//
    SetDlgItemText(IDC_STATIC_USED_TIME, str);//
}

void DlgImageCreate::OnEditchangeComboPagesize() 
{
	// TODO: Add your control notification handler code here
}

DWORD WINAPI DlgImageCreate::image_create_thread(LPVOID para)//在镜制作
{
	DlgImageCreate *pDTD = (DlgImageCreate *)para;
    UINT i=0, j=0;
    UINT path_len = 0;
    DWORD dwAttr;
    CString str;
    TCHAR  dest_file_path[MAX_PATH+1] = {0};
    TCHAR  img_file_path[MAX_PATH+1] = {0};
	BOOL   fs_destroy_flag = FALSE;
	DWORD  ret = 0;

    USES_CONVERSION;

	//pDTD->EnableWindow(FALSE);
    pDTD->GetDlgItem(IDCREATE)->EnableWindow(FALSE);
	pDTD->ExitFlag = FALSE;
  	//pDTD->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    if ( FALSE == pDTD->m_image_create.fslib_init() )//初始化
    {
        goto exit_image_create_thread;
    }

    path_len = _tcslen(pDTD->dest_path);//长度
    if (0 == path_len)
    {
        goto exit_image_create_thread;
    }
    else if ( (path_len > 0) && (pDTD->dest_path[path_len-1] != '\\') )
    {
        pDTD->dest_path[path_len] = '\\';
        pDTD->dest_path[path_len+1] = 0;
    }
    
	pDTD->StartTimer();//开始时间
	fs_destroy_flag = TRUE;

    CreateDirectory(pDTD->dest_path, NULL);   // destination path
    _tcscpy(dest_file_path, pDTD->dest_path);//路径
    _tcscat(dest_file_path, pDTD->name);//名
    _tcscat(dest_file_path, _T(".img"));//后缀名
    if(!pDTD->m_image_create.img_create(dest_file_path, pDTD->capacity*1024, pDTD->sector_size, pDTD->page_size, theConfig.burn_mode, 'A'))
    {
        goto exit_image_create_thread;
    }
	
	
	pDTD->m_image_create.ExitFlag = FALSE;
    dwAttr = GetFileAttributes(pDTD->source_path);//属性
    if(0xFFFFFFFF == dwAttr)
    {
        goto exit_image_create_thread;
    }
	/**A是img_create中定义的**/
    _tcscpy(img_file_path, _T("A:"));
    if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
        if(!pDTD->m_image_create.img_add_dir(pDTD->source_path, T2A(img_file_path)))//加文件夹
        {
            goto exit_image_create_thread;//
        }
    }
    else
    {
        if( !pDTD->m_image_create.img_add_file(pDTD->source_path, T2A(img_file_path)) )//加文件
        {
            goto exit_image_create_thread;//
        }
    }
	
	ret = 1;
exit_image_create_thread:
	if (fs_destroy_flag)
	{
		pDTD->StopTimer();
		FS_UnMountMemDev(0);
		pDTD->m_image_create.img_destroy();//
	}

	if (ret == 1)
	{
		pDTD->SetDlgItemText(IDC_STATIC_PROG_TIME, _T("100%"));//
		pDTD->progCtrl->SetPos(100);//
		pDTD->SetDlgItemText(IDC_CURENT_INFO,theApp.GetString(IDS_SUSSCCE));//
		//MessageBox(_T("sussess!"));
	}
	else
	{
		/*不增加这个判断会死循环*/
		if (!pDTD->m_image_create.ExitFlag)
			pDTD->SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_FAIL));//
		//MessageBox(_T("fail!"));
	}
	
	if (!pDTD->m_image_create.ExitFlag)//
	{
		//pDTD->EnableWindow(TRUE);
		pDTD->GetDlgItem(IDCREATE)->EnableWindow(TRUE);//
  		//pDTD->GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	}
	pDTD->ExitFlag = TRUE;
	return ret;
}

/*
功能：递归查找文件夹中的文件个数和总文件大小，及最小字节的文件
说明：文件夹的内容总容量不能大于4g，要么总文件大小出错的 
作者：luqiliu
输入：pathFloder--文件夹的总路径
输出：FileCnt-----文件的个数 
      FileSize----文件的总容量(unit byte)
      MinSize-----最小的文件
返回：成功找到一个以上的文件返回TRUE， 否则返回FALSE
*/
BOOL DlgImageCreate::GetFileInfoInDir(LPTSTR pathFloder, DWORD *FileCnt, DWORDLONG *FileSize, DWORDLONG *MinSize)
{
	WIN32_FIND_DATA fd;
	HANDLE hSearch;
	TCHAR searchPath[MAX_PATH+1] = {0};
    TCHAR tmpPCPath[MAX_PATH+1] = {0};
    HANDLE hFile;
    DWORD  dwSize;
    CString str;
 
    if (pathFloder == NULL)
    {
        return FALSE;
    }
    
    _tcsncpy(searchPath, pathFloder, MAX_PATH);//查找到的文件夹路径
	_tcscat(searchPath, _T("\\*"));//加//

	hSearch = FindFirstFile(searchPath, &fd);//查第一个
	if(INVALID_HANDLE_VALUE == hSearch)
	{
		return FALSE;
	}
	
	USES_CONVERSION;
	
	do
	{
		if((0 != _tcscmp(fd.cFileName, _T("."))) && (0 != _tcscmp(fd.cFileName, _T(".."))))
		{
			_tcscpy(tmpPCPath, pathFloder);
            _tcscat(tmpPCPath, _T("\\"));
			_tcscat(tmpPCPath, fd.cFileName);

			/*如果镜像，只查找当前的目录*/
            if(MinSize == NULL && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
                if (!GetFileInfoInDir(tmpPCPath, FileCnt, FileSize, MinSize))//获取信息
                {
                    FindClose(hSearch);

                    return FALSE;
                }
			}
            else
            {
	            //add produce length
                hFile = CreateFile(tmpPCPath, GENERIC_READ , FILE_SHARE_READ , NULL , 
					            OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);//创建
	            if(hFile != INVALID_HANDLE_VALUE)
	            {
		            dwSize = GetFileSize(hFile, NULL);//获取大上
                    CloseHandle(hFile);
		            if(dwSize != 0xFFFFFFFF)
		            {
                        if (FileSize != NULL)
                        {
                            *FileSize += dwSize;//增加
                        }
                        if (FileCnt != NULL)
                            (*FileCnt)++;

                        /*为了自动匹配镜像，查找最小字节的镜像文件*/
                        if (MinSize != NULL)
                        {
                            if (*MinSize > dwSize)
                                *MinSize = dwSize;
                        }

                    }
		            else
		            {
                        FindClose(hSearch);
			            str.Format(_T("GetFileInfoInDir Cannot find file: %s"), tmpPCPath);
			            MessageBox(str);
                    
			            return FALSE;
		            }
                }
            }
		}
	}
	while(FindNextFile(hSearch, &fd));//查一下

	FindClose(hSearch);//关闭
    
	return TRUE;	
}

void DlgImageCreate::OnStaticUsedTime() 
{
	// TODO: Add your control notification handler code here	
}


void DlgImageCreate::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	DWORDLONG prog_pos;
    CString str;
    //progCtrl->StepIt();
	prog_pos = (m_image_create.FileSize * 100) / prog_file_total_size;//
	progCtrl->SetPos((int)prog_pos);
    
	if (prog_pos < 10)
	{
		str.Format(_T("%01d%%"), prog_pos);//
		SetDlgItemText(IDC_STATIC_PROG_TIME, str);//
	}
	else if (prog_pos < 100)
	{
		str.Format(_T("%02d%%"), prog_pos);//
		SetDlgItemText(IDC_STATIC_PROG_TIME, str);//
	}
	
    used_time++;
    display_used_time();//


	CDialog::OnTimer(nIDEvent);
}

void DlgImageCreate::OnSelchangeComboPagesize() 
{
	// TODO: Add your control notification handler code here
	CString get_str;
	UINT pagesize = 0;
	
	CComboBox* cbo = (CComboBox*)GetDlgItem(IDC_COMBO_PAGESIZE);//
    USES_CONVERSION;
	cbo->GetLBText(cbo->GetCurSel(), get_str);//
	pagesize = atoi(T2A(get_str));//
	
	if (pagesize > 4096)
	{
		SetDlgItemText(IDC_EDIT_SECTORSIZE, _T("4096"));//
	}
	else
	{
		SetDlgItemText(IDC_EDIT_SECTORSIZE, get_str);	//
	}
}
