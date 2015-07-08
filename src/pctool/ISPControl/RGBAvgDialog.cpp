// RGBAvgDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "RGBAvgDialog.h"
#include "FFScale.h"

#define WM_UPDATE_RGB		WM_USER + 500

// CRGBAvgDialog 对话框

IMPLEMENT_DYNAMIC(CRGBAvgDialog, CDialog)

CRGBAvgDialog::CRGBAvgDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRGBAvgDialog::IDD, pParent)
{
	m_nDataLen = 0;
	m_pRGBData = NULL;
	m_nHeight = m_nWidth = 0;
}

CRGBAvgDialog::~CRGBAvgDialog()
{
	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_PicBitmap.DeleteObject();
	}

	if (m_pRGBData) delete[] m_pRGBData;
}

void CRGBAvgDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRGBAvgDialog, CDialog)
ON_MESSAGE(WM_UPDATE_RGB, OnUpdateRGB)
ON_WM_PAINT()
END_MESSAGE_MAP()

#define IMAGE_WIDTH		220
#define IMAGE_HEIGHT	150

// CRGBAvgDialog 消息处理程序
BOOL CRGBAvgDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CDC* pDC = GetDC();
	m_MemDC.CreateCompatibleDC(pDC);
	
	m_PicBitmap.CreateCompatibleBitmap(pDC, IMAGE_WIDTH, IMAGE_HEIGHT);
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_PicBitmap);
	ReleaseDC(pDC);
	
	m_nDataLen = 0;
	m_pRGBData = NULL;
	m_nHeight = m_nWidth = 0;
	
	return TRUE;
}

int CRGBAvgDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	return 0;
}

int CRGBAvgDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if (NULL == pPageInfoSt || nStLen <= 0) return -1;

	if (nStLen % 4 != 0) return -2;
	
	CAutoLock lock(&m_cs4RGBData);

	m_nDataLen = nStLen;
	if (m_pRGBData) delete[] m_pRGBData;
	m_pRGBData = NULL;

	m_pRGBData = new BYTE[nStLen];
	memcpy(m_pRGBData, pPageInfoSt, nStLen);
	
	PostMessage(WM_UPDATE_RGB);

	return 0;
}

int CRGBAvgDialog::SetWH(int nWidth, int nHight)
{
	CAutoLock lock(&m_cs4RGBData);
	m_nHeight = nHight;
	m_nWidth = nWidth;

	return 0;
}

void CRGBAvgDialog::CalcRGBAvgAndPrepareBitmap()
{
	CAutoLock lock(&m_cs4RGBData);
	
	double fGAvg = 0.0, fRAvg = 0.0, fBAvg = 0.0;
	double fGRAvg = 0.0, fGBAvg = 0.0;

	for (int index = 0; index < m_nDataLen; index += 4) {
		fBAvg += m_pRGBData[index];
		fGAvg += m_pRGBData[index + 1];
		fRAvg += m_pRGBData[index + 2];
	}

	fRAvg /= (m_nDataLen / 4);
	fGAvg /= (m_nDataLen / 4);
	fBAvg /= (m_nDataLen / 4);

	fGRAvg = fGAvg / fRAvg;
	fGBAvg = fGAvg / fBAvg;
	
	CString strText;
	
	strText.Format(L"%f", fRAvg);
	GetDlgItem(IDC_STATIC_R)->SetWindowText(strText);
	
	strText.Format(L"%f", fGAvg);
	GetDlgItem(IDC_STATIC_G)->SetWindowText(strText);

	strText.Format(L"%f", fBAvg);
	GetDlgItem(IDC_STATIC_B)->SetWindowText(strText);

	strText.Format(L"%f", fGRAvg);
	GetDlgItem(IDC_STATIC_GR)->SetWindowText(strText);

	strText.Format(L"%f", fGBAvg);
	GetDlgItem(IDC_STATIC_GB)->SetWindowText(strText);
	
	CRect rect(300, 70, 300 + IMAGE_WIDTH, 70 + IMAGE_HEIGHT);
	InvalidateRect(&rect, FALSE);
}

LRESULT CRGBAvgDialog::OnUpdateRGB(WPARAM wParam, LPARAM lParam)
{
	CalcRGBAvgAndPrepareBitmap();
	
	return 0;
}

void CRGBAvgDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 __super::OnPaint()

	m_MemDC.FillSolidRect(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GetSysColor(COLOR_3DFACE));
	CBitmap* pOldMemBitmap = m_MemDC.SelectObject(&m_PicBitmap);
	int oldBkMode = m_MemDC.SetBkMode(TRANSPARENT);

	CFFScale ffScale;
	ffScale.SetAttribute(SWS_PF_BGRA, SWS_PF_BGRA);

	BYTE * pRGB = new BYTE[IMAGE_WIDTH * IMAGE_HEIGHT * 4];
	if (!pRGB) return;

	CAutoLock * plock = new CAutoLock(&m_cs4RGBData);
	
	BOOL bRet = ffScale.Scale(m_pRGBData, m_nWidth, m_nHeight, m_nWidth * 4, pRGB, 
							IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_WIDTH * 4);

	delete plock;
	
	m_PicBitmap.SetBitmapBits(IMAGE_WIDTH * IMAGE_HEIGHT * 4, pRGB);
	
	dc.BitBlt(300, 70, IMAGE_WIDTH, IMAGE_HEIGHT, &m_MemDC, 0, 0, SRCCOPY);

	m_MemDC.SelectObject(pOldMemBitmap);

	delete pRGB;
}
