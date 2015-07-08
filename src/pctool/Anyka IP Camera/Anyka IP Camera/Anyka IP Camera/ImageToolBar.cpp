#include "StdAfx.h"
#include "ImageToolBar.h"
#include "atlimage.h"

IMPLEMENT_DYNAMIC(CImageToolBar, CToolBar) 

CImageToolBar::CImageToolBar(void)
{
	m_pImage = NULL;
    GdiplusStartupInput gdiplusStartupInput;
 
    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

CImageToolBar::~CImageToolBar(void)
{
	//if (m_pImage!=NULL)
		//delete m_pImage;
	int iSize = m_pvecImage.size();

	for ( int i = 0; i < iSize; ++i ) {
		delete m_pvecImage[i];
	}

	m_pvecImage.clear();

	GdiplusShutdown(gdiplusToken);
}

BEGIN_MESSAGE_MAP(CImageToolBar, CToolBar)
END_MESSAGE_MAP()

// CImageToolBar 消息处理程序 
int CImageToolBar::SetImage(const char * imgPath, bool IsEnd) 
{
	//if (m_pImage!=NULL)
		//delete m_pImage;
	//m_pImage = Image::FromFile(CStringW(imgPath));
	Image * pImage = Image::FromFile(CStringW(imgPath));
	
	//if (m_pImage == NULL)
		//return RESOURCE_ERR;

	if (pImage == NULL)
		return RESOURCE_ERR;

	m_pvecImage.push_back( pImage );
	
	if ( IsEnd )
		return RefreshImg();
	else 
		return SUCCESS;
}

//int CImageToolBar::SetImage(int sourceId) 
//{ 
	//CBitmap  bitmap;

	//if (bitmap.LoadBitmap(sourceId))
	  // return SetImage(bitmap);

	//return RESOURCE_ERR;
//} 

//int CImageToolBar::SetImage(HBITMAP h_bmp)
//{
	//if (m_pImage!=NULL)
	   //delete m_pImage;

	//m_pImage = Bitmap::FromHBITMAP(h_bmp,NULL);

	//if (m_pImage == NULL)
	   //return RESOURCE_ERR;

	//return RefreshImg();
//}

int CImageToolBar::RefreshImg() 
{
	if (m_hWnd == NULL)
	   return TOOBAR_ERR;

	CToolBarCtrl& ToolBarCtrl = GetToolBarCtrl();
	CSize ButtonSize;
	GetButtonImgSize(ButtonSize);
	CBitmap BmpBack;
	CRect BKRect;
	GetToolImgRect(BKRect);
	CDC * pWndDC = GetWindowDC();
	BmpBack.CreateCompatibleBitmap(pWndDC, BKRect.Width(), BKRect.Height());
	ReleaseDC(pWndDC);
	CImageList ImgList;
	int iSize = m_pvecImage.size();

	{ // normal
	   //DrawBarImg(BmpBack);
	   
	   ImgList.Create(ButtonSize.cx, ButtonSize.cy, ILC_COLORDDB | ILC_MASK, 1, 1);

	   for ( int i = 0; i < iSize; ++i ) {
		   m_pImage = m_pvecImage[i];
		   DrawBarImg(BmpBack);
		   ImgList.Add(&BmpBack, RGB(0,0,0) );
	   }

	   ToolBarCtrl.SetImageList(&ImgList);
	   ImgList.Detach();
	}

	{ // hot
	   //DrawBarImg(BmpBack);
	   //RendHotImg(BmpBack);
	   ImgList.Create(ButtonSize.cx, ButtonSize.cy, ILC_COLORDDB | ILC_MASK, 1, 1);
	   //ImgList.Add(&BmpBack, RGB(0,0,0) );

	   for ( int i = 0; i < iSize; ++i ) {
		   m_pImage = m_pvecImage[i];
		   DrawBarImg(BmpBack);
		   RendHotImg(BmpBack);
		   ImgList.Add(&BmpBack, RGB(0,0,0) );
	   }


	   ToolBarCtrl.SetHotImageList(&ImgList);
	   ImgList.Detach();
	}

	{
	   // disable
	  //DrawBarImg(BmpBack);
	   //RendDisableImg(BmpBack);
	   ImgList.Create(ButtonSize.cx, ButtonSize.cy, ILC_COLORDDB | ILC_MASK, 1, 1);
	   //ImgList.Add(&BmpBack, RGB(0,0,0) );

	   for ( int i = 0; i < iSize; ++i ) {
		   m_pImage = m_pvecImage[i];
		   DrawBarImg(BmpBack);
		   RendHotImg(BmpBack);
		   ImgList.Add(&BmpBack, RGB(0,0,0) );
	   }

	   ToolBarCtrl.SetDisabledImageList(&ImgList);
	   ImgList.Detach();
	}

	ReleaseDC(pWndDC);
	return SUCCESS;
}

void CImageToolBar::ColorReplace(CBitmap & bmpImg, COLORREF from, COLORREF to)
{ 
	CImage	img;

	img.Attach(bmpImg);
	COLORREF BKold = img.GetPixel(0,0);
	COLORREF BKnew = GetSysColor(COLOR_BTNFACE);
	int w = img.GetWidth(),
	   h = img.GetHeight();

	int x,y;

	for (x = 0; x < w; ++x)
	{
		for (y = 0; y < h; ++y)
		{
			if (img.GetPixel(x,y) == from)
			 img.SetPixel(x,y,to);
		}
	}

	img.Detach();
}

void CImageToolBar::GetButtonImgSize(CSize & size) 
{ 
	//size = GetToolBarCtrl().GetButtonSize();
	m_sizeImage.cx;
	//size.cx -= 7;
	//size.cy -= 6;
	size.cx = m_sizeImage.cx;
	size.cy = m_sizeImage.cy;
}

int CImageToolBar::GetButtonCount() 
{ 
	int count = GetToolBarCtrl().GetButtonCount();

	for (int i = count - 1; i >= 0; --i)
	{
	   if (TBBS_BUTTON != GetButtonStyle(i))
			--count;
	}

	return count;
}

void CImageToolBar::GetToolImgRect(CRect & imgRect) 
{ 
	CSize size;
	GetButtonImgSize(size);
	imgRect.left = 0;
	imgRect.top = 0;
	imgRect.bottom = size.cy;
	imgRect.right = size.cx; //* GetButtonCount();
}

void CImageToolBar::DrawBarImg(CBitmap & bmpImg) 
{ 
	if (m_pImage==NULL)
	   return;

	CRect	BKRect;
	GetToolImgRect(BKRect);

	CDC	DrawDC,
	*pWndDC = GetWindowDC();

	DrawDC.CreateCompatibleDC(pWndDC);
	DrawDC.SelectObject(&bmpImg);
	ReleaseDC(pWndDC);
	DrawDC.FillRect(&BKRect, &CBrush(GetSysColor(COLOR_BTNFACE) ) );
	Graphics graphics(DrawDC);
	graphics.DrawImage(m_pImage, 0, 0, BKRect.Width(), BKRect.Height());

	if (m_pImage->GetType() < ImageTypeMetafile)
	{
		COLORREF bk = DrawDC.GetPixel(0,0);
		DrawDC.DeleteDC();
		ColorReplace(bmpImg, bk, GetSysColor(COLOR_BTNFACE));
	}
}

void CImageToolBar::RendHotImg(CBitmap & bmpImg) 
{
	CRect BKRect;
	GetToolImgRect(BKRect);
	CDC   DrawDC,
	*pWndDC = GetWindowDC();
	DrawDC.CreateCompatibleDC(pWndDC);
	DrawDC.SelectObject(&bmpImg);
	ReleaseDC(pWndDC);
	unsigned char r,g,b;
	COLORREF color;
	for (int x = 0; x < BKRect.Width(); ++x)
	{
		for (int y = 0; y < BKRect.Height(); ++y)
		{
			color = DrawDC.GetPixel(x,y);
			r = (color>>0)&0x0ff;
			g = (color>> 8)&0x0ff;
			b = (color>>16)&0x0ff;
			r += min(255-r, r*0.15);
			g += min(255-g, g*0.15);
			b += min(255-b, b*0.15);
			DrawDC.SetPixel(x,y,RGB(r,g,b));
		}
	}
}

void CImageToolBar::RendDisableImg(CBitmap & bmpImg) 
{ 
	try{ 
		CRect BKRect;
		GetToolImgRect(BKRect);

		CDC   DrawDC,
		*pWndDC = GetWindowDC();

		DrawDC.CreateCompatibleDC(pWndDC);
		DrawDC.SelectObject(&bmpImg);
		ReleaseDC(pWndDC);
		CImage Img;

		if (!Img.Create(BKRect.Width(), BKRect.Height(), 32))
			return;

		CDC ImgDC;
		ImgDC.Attach(Img.GetDC());
		ImgDC.FillRect(&BKRect, &CBrush(GetSysColor(COLOR_BTNFACE)) );
		ImgDC.Detach();
		Img.AlphaBlend(DrawDC, BKRect, BKRect, 180);
		Img.ReleaseDC();
	}catch(...) 
	{
	}
} 