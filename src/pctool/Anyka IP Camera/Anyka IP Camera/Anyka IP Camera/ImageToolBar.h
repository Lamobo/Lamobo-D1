#pragma once
#include "afxext.h"

#include <gdiplus.h>
#include <Gdiplusgraphics.h>
#include <vector>

using namespace Gdiplus;
using namespace std;

class CImageToolBar :
	public CToolBar
{

DECLARE_DYNAMIC(CImageToolBar)

public:
	CImageToolBar(void);
	~CImageToolBar(void);
	
	enum SET_ERR { SUCCESS = 0, RESOURCE_ERR, TOOBAR_ERR, SIZE_ERR, ERR };

	int SetImage(const char * imgPath, bool IsEnd = false);
	//int SetImage(int sourceId);
	//int SetImage(HBITMAP h_bmp);

	int RefreshImg();

protected:
	vector<Image *> m_pvecImage;
	Image * m_pImage;
	void ColorReplace(CBitmap & bmpImg, COLORREF from, COLORREF to);
	int GetButtonCount();
	void GetButtonImgSize(CSize & size);
	void GetToolImgRect(CRect & imgRect);
	virtual void DrawBarImg(CBitmap & bmpImg);
	virtual void RendHotImg(CBitmap & bmpImg);
	virtual void RendDisableImg(CBitmap & bmpImg);

	DECLARE_MESSAGE_MAP()

public:
	ULONG_PTR	gdiplusToken;
};
