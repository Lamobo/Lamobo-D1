// ListCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include "ListCtrlEx.h"
#include "ListCellEdit.h"

#include "../BurnTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListCtrlEx

static int m_nIndex;
static int m_nSubItem;
static bool m_bHit;
static int m_nInterval;

#define IDC_PROPCMBBOX   1500

#define IDC_BUTTON3                     32888
#define IDC_BUTTON4                     32889
#define IDC_EDITCELL                    32890

extern CBurnToolApp theApp;

CListCtrlEx::CListCtrlEx()
: m_ItemIndex(0), m_SubItemIndex(0)
{
	m_nIndex = -1;//二级
	m_nSubItem = -1;//一级
	m_bHit = FALSE;//
	m_nInterval = 0;//

    for(int i = 0; i < T_POP_ITEM_NUM; i++)
    {
        m_pop_item[i].SubItemType = T_POP_NULL;
    }
}

CListCtrlEx::~CListCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlEx)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_CBN_KILLFOCUS(IDC_PROPCMBBOX, OnKillfocusCmbBox)
	ON_CBN_SELCHANGE(IDC_PROPCMBBOX, OnSelchangeCmbBox)
	ON_COMMAND_RANGE(IMENU_ITEM_ID,IMENU_ITEM_ID+100, OnMenuSelect)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlEx message handlers

void CListCtrlEx::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	
	// Take the default processing unless we set this to something else below.
	*pResult = CDRF_DODEFAULT;
	
	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.
	
	if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		// This is the notification message for an item.  We'll request
		// notifications before each subitem's prepaint stage.
		
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	
}

BOOL CListCtrlEx::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pMsg->message == WM_KEYDOWN )
	{
		if(pMsg->wParam == VK_RETURN)
			return TRUE;
	}
	
	return CListCtrl::PreTranslateMessage(pMsg);
}

bool CListCtrlEx::SetSubItemPopItem(int nSubItem, T_POP_TYPE type, CString str)
{
    if(nSubItem >= T_POP_ITEM_NUM)
        return false;

    m_pop_item[nSubItem].SubItemType = type;//菜单类型
    m_pop_item[nSubItem].ItemString = str;//字符串

    return true;
}


void CListCtrlEx::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CString strIndex;
	
	CListCtrl::OnLButtonDblClk(nFlags, point);

	int nItem=0, nSubItem=0;
	int rowHeight=0, rowCnt=0;
	CRect rect;
//	CConfigDlg *pDlg = (CConfigDlg *)GetParent();
	
	
    if ((nItem = HitTestEx (point, nSubItem)) != -1)
    {
		if (nSubItem != 0)
		{
            m_nIndex = nItem;
			m_nSubItem = nSubItem;
			
            if(m_pop_item[nSubItem].SubItemType == T_POP_MENU)//T_POP_MENU
            {

				CMenu menu;
				CString str;
                int i = 0, i2 = 0, index = 0;
                
				menu.CreatePopupMenu();

                while ((i2 = m_pop_item[nSubItem].ItemString.Find('|',i)) != -1)
				{
                    BOOL bRet=menu.AppendMenu(MF_STRING, IMENU_ITEM_ID + index, 
                        m_pop_item[nSubItem].ItemString.Mid(i, i2-i));

                    index++;
					i=i2+1;
				}
                

				CPoint ptScreen(point);
				ClientToScreen(&ptScreen);//客户端
				
				menu.TrackPopupMenu(TPM_LEFTBUTTON , ptScreen.x, ptScreen.y, this);
				return;

            }
            else if(m_pop_item[nSubItem].SubItemType == T_POP_CMBBOX)//T_POP_CMBBOX
            {
                CRect Rect;
				GetItemRect (nItem, &Rect, LVIR_BOUNDS);
                
                for(int i = 0; i < nSubItem; i++)
                {
                    Rect.left += GetColumnWidth(i);//获取长度
                }

				m_cmbBox_nItem = nItem;
				m_cmbBox_nSubItem = nSubItem;

				
				if(!m_cmbBox)
				{
					Rect.bottom += 100;
					m_cmbBox.Create(WS_VSCROLL | CBS_AUTOHSCROLL | CBS_DROPDOWNLIST | 
                        CBS_NOINTEGRALHEIGHT | WS_VISIBLE | WS_CHILD | WS_BORDER,
						Rect,this,IDC_PROPCMBBOX);
					//m_cmbBox.SetFont(&m_SSerif8Font);
				}

				CString cmbItems = m_pop_item[nSubItem].ItemString;
				TCHAR lBoxSelText[255];

				GetItemText(nItem, nSubItem, lBoxSelText, 255);
			
				m_cmbBox.ResetContent();//重启内容

				int i2, strlength = 0;

				i=0;

				while ((i2=cmbItems.Find('|',i)) != -1)
				{
                    if(strlength < (int)_tcslen(cmbItems.Mid(i,i2-i)))
                        strlength = _tcslen(cmbItems.Mid(i,i2-i));

					m_cmbBox.AddString(cmbItems.Mid(i,i2-i));
					i=i2+1;
				}

				CDC* dc = GetDC();
				TEXTMETRIC tm_p;

				GetTextMetrics(dc->m_hDC, &tm_p);// 调用GetTextMetrics函数获取字符大小相关信息。

				LONG iLFx = tm_p.tmAveCharWidth;
				LONG iCFx = (tm_p.tmPitchAndFamily & 1 ? 3 : 2) * iLFx / 2;

				ReleaseDC (dc) ;

                strlength *= iCFx;

                strlength += GetSystemMetrics(SM_CXVSCROLL);//SM_CXVSCROLL


                if(strlength < GetColumnWidth(nSubItem))//GetColumnWidth
                    strlength = GetColumnWidth(nSubItem);

                Rect.right = Rect.left + strlength;

                if (m_cmbBox)
					m_cmbBox.MoveWindow(Rect);//移动
			
				m_cmbBox.ShowWindow(SW_SHOW);//显示
				m_cmbBox.SetFocus();//设置
			
				//跳到当前值
				int j = m_cmbBox.FindStringExact(0,lBoxSelText);
				if (j != CB_ERR)
					m_cmbBox.SetCurSel(j);
				else
					m_cmbBox.SetCurSel(0);
            }
            else
				EditSubItem(nItem, nSubItem);//编辑
		}
		return;
    }
	
	rowCnt = GetItemCount();//个数
	if ( rowCnt== 0)
	{
		strIndex.Format(_T("%d"), rowCnt + 1);
		InsertItem(rowCnt, strIndex);
		//EditSubItem(0, 1);
	}
	else
	{
		GetItemRect (0, &rect, LVIR_BOUNDS);
		rowHeight = rect.Height();//高
		rect.top += rowHeight*rowCnt;//上
		rect.bottom += rowHeight*rowCnt;//下
		if (rect.PtInRect(point))
		{
			strIndex.Format(_T("%d"), rowCnt + 1);
			InsertItem(rowCnt, strIndex);
			//EditSubItem(rowCnt, 1);
		}
	}
	
}


void CListCtrlEx::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default

	if (GetFocus() != this) SetFocus();//获取和设置焦点
	
	
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CListCtrlEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	if (GetFocus() != this) SetFocus();
	
	
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CListCtrlEx::OnSize(UINT nType, int cx, int cy) 
{
	CListCtrl::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

void CListCtrlEx::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	LV_DISPINFO *plvDispInfo = (LV_DISPINFO *)pNMHDR;
    LV_ITEM	*plvItem = &plvDispInfo->item;
//	CDlgConfig * pDlg = (CDlgConfig *)GetParent();
	
    if (plvItem->pszText != NULL)
    {
		SetItemText (plvItem->iItem, plvItem->iSubItem, plvItem->pszText);//设置显示威者
    }
	
	*pResult = 0;
}

void CListCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	int k;
	if (nChar == VK_DELETE)
	{
//		CConfigDlg * pDlg = (CConfigDlg *)GetParent();
//		::SendMessage(pDlg->GetSafeHwnd(), WM_COMMAND, IDC_BUTTON4, 0);
		if((k = GetNextItem(-1, LVNI_SELECTED)) != -1)
		{
			DeleteItem(k);//删除
		}
	}
	
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

int CListCtrlEx::HitTestEx (CPoint& Point, int& nSubItem)
{
	nSubItem = 0;
	int ColumnNum = 0;
    int Row = HitTest (Point, NULL);
	
    // Make sure that the ListView is in LVS_REPORT
    if ((GetWindowLong (m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
		return Row;
	
    // Get the top and bottom row visible
    Row = GetTopIndex();
    int Bottom = Row + GetCountPerPage();//每页多少个
    if (Bottom > GetItemCount())
		Bottom = GetItemCount();//个数
    
    // Get the number of columns
    CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();
	
    // Loop through the visible rows
    for(; Row <= Bottom; Row++)
    {
		// Get bounding rect of item and check whether point falls in it.
		CRect Rect;
		GetItemRect (Row, &Rect, LVIR_BOUNDS);
		if (Rect.PtInRect (Point))
		{
			// Now find the column
			for (ColumnNum = 0; ColumnNum < nColumnCount; ColumnNum++)
			{
				int ColWidth = GetColumnWidth (ColumnNum);//GetColumnWidth
				if (Point.x >= Rect.left && Point.x <= (Rect.left + ColWidth))
				{
					nSubItem = ColumnNum;
					return Row;
				}
				Rect.left += ColWidth;
			}
		}
    }
	
    return -1;
}

CEdit* CListCtrlEx::EditSubItem(int nItem, int nSubItem)
{
    // The returned pointer should not be saved
	
    // Make sure that the item is visible
    if (!EnsureVisible (nItem, TRUE)) return NULL;
	
    // Make sure that nCol is valid
    CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();//获取的个数
    if (nSubItem >= nColumnCount || GetColumnWidth (nSubItem) < 5)
		return NULL;
	
    // Get the column offset
    int Offset = 0;
    for (int iColumn = 0; iColumn < nSubItem; iColumn++)
		Offset += GetColumnWidth (iColumn);//偏移
	
    CRect Rect;
    GetItemRect (nItem, &Rect, LVIR_BOUNDS);
	
    // Now scroll if we need to expose the column
    CRect ClientRect;
    GetClientRect (&ClientRect);
    if (Offset + Rect.left < 0 || Offset + Rect.left > ClientRect.right)
    {
		CSize Size;
		if (Offset + Rect.left > 0)
			Size.cx = -(Offset - Rect.left);//大小
		else
			Size.cx = Offset - Rect.left;//大小
		Size.cy = 0;
		Scroll (Size);
		Rect.left -= Size.cx;
    }
	
    // Get nSubItem alignment
    LV_COLUMN lvCol;
    lvCol.mask = LVCF_FMT;
    GetColumn (nSubItem, &lvCol);
    DWORD dwStyle;
    if ((lvCol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
		dwStyle = ES_LEFT;
    else if ((lvCol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
		dwStyle = ES_RIGHT;
    else dwStyle = ES_CENTER;
	
    Rect.left += Offset+4;
    Rect.right = Rect.left + GetColumnWidth (nSubItem) - 3;
    if (Rect.right > ClientRect.right)
		Rect.right = ClientRect.right;
    dwStyle |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
    CEdit *pEdit = new CListCellEdit (nItem, nSubItem, GetItemText (nItem, nSubItem));
    pEdit->Create (dwStyle, Rect, this, IDC_EDITCELL);
	//check to see if the length of input messsage is out of restrict
	pEdit->SetLimitText(255);
	/*
	switch (nSubItem)
	{
	case 1:				//phone book user name
		pEdit->SetLimitText(14);
		break;
	case 2:				//work phone
	case 3:				//home phone
	case 4:				//mobile phone
	case 5:				//fax
		pEdit->SetLimitText(20);
		break;
	case 6:				//email
		pEdit->SetLimitText(40);
		break;
	}
	*/
	
	
	m_ItemIndex = nItem;//二级
	m_SubItemIndex = nSubItem;//一级
	
    return pEdit;
}

int CListCtrlEx::GetItemIndex()
{
	return m_ItemIndex;//m_ItemIndex
}

int CListCtrlEx::GetSubItemIndex()
{
	return m_SubItemIndex;//m_SubItemIndex
}

void CListCtrlEx::MoveNextItem()
{
    // Get the number of columns
	CString strIndex;
    CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();
	
	m_SubItemIndex++;
	if (m_SubItemIndex > nColumnCount-1)
	{
		m_SubItemIndex = 1;
		m_ItemIndex++;
	}

	if (m_ItemIndex >= GetItemCount())//获取单菜的个数
	{
		strIndex.Format(_T("%d"), GetItemCount() + 1);
		InsertItem(GetItemCount(), strIndex);//插入菜单
	}

	
	EditSubItem(m_ItemIndex, m_SubItemIndex);//编辑菜单
}

void CListCtrlEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	int nItem=0, nSubItem=0;
	int rowHeight=0, rowCnt=0;
	CRect rect;
//	CConfigDlg *pDlg = (CConfigDlg *)GetParent();
	TCHAR text[255];

	LVCOLUMN col;
	
	col.mask = LVCF_TEXT ;
	col.cchTextMax = 255;
	col.pszText = text;
	
	
    if ((nItem = HitTestEx (point, nSubItem)) != -1)
    {
		if (nSubItem != 0)
		{		
			GetColumn(nSubItem, &col);

			if(_tcscmp(text, theApp.GetString(IDS_DOWNLOAD_PC_PATH)) == 0)//比较
			{
				if(m_nIndex != nItem)
				{
					m_nIndex = nItem;
					m_nSubItem = nSubItem;
					m_nInterval = GetTickCount();//计次数
				}
				else
				{
					if(m_bHit)
					{
						if(GetTickCount() - m_nInterval > GetDoubleClickTime())
						{
							m_nIndex = -1;
							m_nSubItem = -1;
							m_bHit = FALSE;
							EditSubItem(nItem, nSubItem);//编辑
							return;
						}

					}
					else
					{
						m_bHit = TRUE;
					}
				}
			}
		}
	}
	
	CListCtrl::OnLButtonDown(nFlags, point);
}

void CListCtrlEx::OnKillfocusCmbBox() 
{
	m_cmbBox.ShowWindow(SW_HIDE);//是否显示

	Invalidate();
}

void CListCtrlEx::OnSelchangeCmbBox()
{
	CString selStr;
	if (m_cmbBox)
	{
		m_cmbBox.GetLBText(m_cmbBox.GetCurSel(),selStr);
		SetItemText(m_cmbBox_nItem, m_cmbBox_nSubItem, selStr);	
	}
}

void CListCtrlEx::OnMenuSelect(UINT nID)
{
	TCHAR Current_DirectoryBuffer[MAX_PATH];
	CString sPath;
	CRect rect;
	int nPos;
	
	TCHAR text[255];
	
	CString strIndex;

//	CConfigDlg *pDlg = (CConfigDlg *)GetParent();

	GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);//获取文件名
	sPath.ReleaseBuffer();
	nPos=sPath.ReverseFind ('\\');
    sPath=sPath.Left(nPos+1);
//	stprintf(Current_DirectoryBuffer, _T("%s"), sPath);
	_tcscpy(Current_DirectoryBuffer, sPath);

	LVCOLUMN col;
	
	col.mask = LVCF_TEXT ;
	col.cchTextMax = 255;
	col.pszText = text;

	int i=nID-IMENU_ITEM_ID;

	if(i == 0)//file
	{
		extern HINSTANCE _hInstance;
			
		OPENFILENAME ofn;
		TCHAR pstrFileName[260] = {0}, pstrTitleName[260] = {0};
		
		TCHAR szFilter[] =	TEXT ("BIN Files (*.bin;*.nb0)\0*.bin;*.nb0\0")  \
			TEXT ("All Files (*.*)\0*.*\0\0") ;//支持显示的后缀名
		
		memset ( &ofn, 0, sizeof ( ofn ) );
		
		ofn.lStructSize       = sizeof (OPENFILENAME) ;
		ofn.hInstance         = _hInstance ;
		ofn.hwndOwner         = GetSafeHwnd();
		ofn.lpstrFilter       = szFilter;
		ofn.nMaxFile          = MAX_PATH ;
	//	ofn.nMaxFileTitle     = MAX_PATH ;
		ofn.lpstrDefExt       = TEXT ("bin") ;//后缀名
		ofn.lpstrFile         = pstrFileName ;
	//	ofn.lpstrFileTitle    = pstrTitleName ;
		ofn.Flags             = OFN_FILEMUSTEXIST; 
		

		if(GetOpenFileName (&ofn))//获取文件名
		{
			TCHAR * relative_path = pstrFileName;

			if((relative_path = _tcsstr(pstrFileName, Current_DirectoryBuffer)) != NULL)
			{
				relative_path = pstrFileName + _tcslen(Current_DirectoryBuffer);
				SetItemText(m_nIndex, m_nSubItem, relative_path);//设置路径	
			}
			else
				SetItemText(m_nIndex, m_nSubItem, pstrFileName);//设置路径		
		}
		else
		{
			return;
		}
	}
	else
	{
		extern HINSTANCE _hInstance;


		BROWSEINFO pbi;
		
		memset ( &pbi, 0, sizeof ( pbi ));//清0

		pbi.hwndOwner = GetSafeHwnd();
		pbi.pidlRoot = NULL;
		pbi.lpszTitle = __T("请选择资源树目录路径");
		pbi.ulFlags = BIF_RETURNONLYFSDIRS ;

		LPITEMIDLIST pidl;
		
		if((pidl = SHBrowseForFolder (&pbi)) != NULL)//浏览文件夹
		{
			TCHAR path[MAX_PATH];
			TCHAR *relative_path;

			SHGetPathFromIDList ( pidl, path );
       

			// free memory used
			IMalloc * imalloc = 0;
			if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
			{
				imalloc->Free ( pidl );//释放
				imalloc->Release ( );//释放
			}

			if((relative_path = _tcsstr(path, Current_DirectoryBuffer)) != NULL)//路径
			{
				relative_path = path + _tcslen(Current_DirectoryBuffer);
				SetItemText(m_nIndex, m_nSubItem, relative_path);
			}
			else
				SetItemText(m_nIndex, m_nSubItem, path);	
		}
		else
		{
			return;
		}
	}
}

