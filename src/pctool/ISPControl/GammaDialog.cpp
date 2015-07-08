// GammaDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "GammaDialog.h"
#include <math.h>
#include <gdiplus.h>
#include <Gdiplusgraphics.h>

using namespace Gdiplus;

#define PI	3.1415926
#define CONTROL_POINT_APART		1

class Vector
{
public:
	double _x, _y;

	Vector(double x, double y)
	{
		_x = x; _y = y;
	};

	Vector(PointF pt)
	{
		_x = pt.X;
		_y = pt.Y;
	};

	Vector(PointF st, PointF end)
	{
		_x = end.X - st.X;
		_y = end.Y - st.Y;
	};

	Vector(Point pt)
	{
		_x = pt.X;
		_y = pt.Y;
	};

	Vector(Point st, Point end)
	{
		_x = end.X - st.X;
		_y = end.Y - st.Y;
	};

	double Magnitude()
	{
		return sqrt(_x * _x + _y * _y);
	};

	friend Vector operator +(Vector & V1, Vector & V2)
	{
		return Vector(V1._x + V2._x, V1._y + V2._y);
	};

	friend Vector operator -(Vector & V1, Vector & V2)
	{
		return Vector(V1._x - V2._x, V1._y - V2._y);
	};

	friend Vector operator -(Vector & V1)
	{
		return Vector(-V1._x, -V1._y);
	};

	friend Vector operator *(double c, Vector & V)
	{
		return Vector(c * V._x, c * V._y);
	};

	friend Vector operator *(Vector & V, double c)
	{
		return Vector(c * V._x, c * V._y);
	};

	friend Vector operator /(Vector & V, double c)
	{
		return Vector(V._x / c, V._y / c);
	};

    // A * B =|A|.|B|.sin(angle AOB)
    double CrossProduct(Vector & V)
    {
		return _x * V._y - V._x * _y;
    };

    // A. B=|A|.|B|.cos(angle AOB)
    double DotProduct(Vector & V)
    {
		return _x * V._x + _y * V._y;
    };

    static bool IsClockwise(PointF pt1, PointF pt2, PointF pt3)
    {
        Vector V21(pt2, pt1);
        Vector v23(pt2, pt3);
        return V21.CrossProduct(v23) < 0; // sin(angle pt1 pt2 pt3) > 0, 0<angle pt1 pt2 pt3 <180
    };

	static bool IsCCW(PointF pt1, PointF pt2, PointF pt3)
    {
        Vector V21(pt2, pt1);
        Vector v23(pt2, pt3);
        return V21.CrossProduct(v23) > 0;  // sin(angle pt2 pt1 pt3) < 0, 180<angle pt2 pt1 pt3 <360
    };
	
	static double DistancePointLine(PointF pt, PointF lnA, PointF lnB)
    {
        Vector v1(lnA, lnB);
        Vector v2(lnA, pt);
        v1 = v1 / v1.Magnitude();
        return abs(v2.CrossProduct(v1));
    }

	void Rotate(int Degree)
    {
        double radian = Degree * PI / 180.0;
        double dsin = sin(radian);
        double dcos = cos(radian);
        double nx = _x * dcos - _y * dsin;
        double ny = _x * dsin + _y * dcos;
        _x = nx;
        _y = ny;
    }

	Point ToPoint()
    {
        return Point((int)_x, (int)_y);
    }

	PointF ToPointF()
    {
        return PointF((float)_x, (float)_y);
    }
};

class Spline
{
public:
	Spline()
	{
		precision = 0.1;
		isXcalibrated = TRUE;
	};

	void setPrecision(double value)
	{
		precision = value;
	};

	double getPrecision()
	{
		return precision;
	};

	void setXaxisCalibrated(BOOL value)
	{
		isXcalibrated = TRUE;
	};

	void DataPoint(vector<CPoint> & vecPoints)
	{
		dataPoint.clear();

		for (unsigned int i = 0; i < vecPoints.size(); ++i) {
			dataPoint.push_back(Vector(Point(vecPoints[i].x, vecPoints[i].y)));
		}
	};

	void DataPoint(vector<Point> & vecPoints)
	{
		dataPoint.clear();

		for (unsigned int i = 0; i < vecPoints.size(); ++i) {
			dataPoint.push_back(Vector(vecPoints[i]));
		}
	}

	void DataPointF(vector<PointF> & vecPointFs)
	{
		dataPoint.clear();

		for (unsigned int i = 0; i < vecPointFs.size(); ++i) {
			dataPoint.push_back(Vector(vecPointFs[i]));
		}
	};

	void ListDataPoint(vector<CPoint> & listPoints)
	{
		dataPoint.clear();
		
		vector<CPoint>::iterator iter = listPoints.begin();
		for (; iter != listPoints.end(); ++iter) {
			dataPoint.push_back(Vector(Point(iter->x, iter->y)));
		}
	};
	
	void ListDataPointF(vector<PointF> & listPointFs)
	{
		dataPoint.clear();
		
		vector<PointF>::iterator iter = listPointFs.begin();
		for (; iter != listPointFs.end(); ++iter) {
			dataPoint.push_back(Vector(*iter));
		}
	};

	void SplinePoint(vector<Point> & splinePoints)
	{
		getSplinePoints();
		splinePoints.clear();

		vector<Vector>::iterator iter = splinePoint.begin();
		for (; iter != splinePoint.end(); ++iter) {
			splinePoints.push_back(iter->ToPoint());
		}
	};

	void SplinePointF(vector<PointF> & splinePointFs)
	{
		getSplinePoints();
		splinePointFs.clear();

		vector<Vector>::iterator iter = splinePoint.begin();

		for (; iter != splinePoint.end(); ++iter) {
			splinePointFs.push_back(iter->ToPointF());
		}
	};

private:

	void getSplinePoints()
	{
		splinePoint.clear();
		if (dataPoint.size() == 1) splinePoint.push_back(dataPoint[0]);

		if (dataPoint.size() == 2) {
			int n = 1;

			if(isXcalibrated)
				n=(int)((dataPoint[1]._x - dataPoint[0]._x) / precision);
			else n = (int)((dataPoint[1]._y - dataPoint[0]._y) / precision);

			if (n == 0) n = 1;
			if (n < 0) n = -n;

			for (int j = 0; j < n; j++) {
				double t = (double)j / (double)n;
				splinePoint.push_back((1 - t) * dataPoint[0] + t * dataPoint[1]);
			}
		}

		if (dataPoint.size() > 2) {
			getControlPoints();

			//draw bezier curves using Bernstein Polynomials
			for (unsigned int i = 0; i < controlPoint.size() - 1; i++) {
				Vector b1 = controlPoint[i] * 2.0 / 3.0 + controlPoint[i + 1] / 3.0;
				Vector b2 = controlPoint[i] / 3.0 + controlPoint[i + 1] * 2.0 / 3.0;

				int n = 1;
				if(isXcalibrated)
					n=(int)((dataPoint[i + 1]._x - dataPoint[i]._x) / precision);
				else n = (int)((dataPoint[i + 1]._y - dataPoint[i]._y) / precision);

				if (n == 0) n = 1;
				if (n < 0) n = -n;

				for (int j = 0; j < n; j++ )
				{
					double t = (double)j / (double)n;
					Vector v = (1 - t) * (1 - t) * (1 - t) * dataPoint[i] + 3 * (1 - t) * (1 - t) * t * b1 +
						3 * (1 - t) * t * t * b2 + t * t * t * dataPoint[i + 1];

					splinePoint.push_back(v);
				}
			}
		}
	};

	void getControlPoints()
	{
		controlPoint.clear();

		if (!dataPoint.empty() && dataPoint.size() == 3) {
			controlPoint.push_back(dataPoint[0]);
            controlPoint.push_back((6 * dataPoint[1] - dataPoint[0] - dataPoint[2]) / 4);
			controlPoint.push_back(dataPoint[2]);
        }

		if (!dataPoint.empty() && dataPoint.size() > 3) {
			unsigned int n = dataPoint.size();
			double * diag = new double[n]; // tridiagonal matrix a(i , i)
			double * sub = new double[n]; // tridiagonal matrix a(i , i-1)
			double * sup = new double[n]; // tridiagonal matrix a(i , i+1)

			for (unsigned int i = 0; i < n; ++i) {
				controlPoint.push_back(dataPoint[i]);
				diag[i] = 4;
				sub[i] = 1;
				sup[i] = 1;
			}

			controlPoint[1] = 6 * controlPoint[1] - controlPoint[0];
			controlPoint[n - 2] = 6 * controlPoint[n - 2] - controlPoint[n - 1];

			for (unsigned int i = 2; i < n - 2; i++)
				controlPoint[i] = 6 * controlPoint[i];

			// Gaussian elimination fron row 1 to n-2
			for (unsigned int i = 2; i < n - 1; i++) {
				sub[i] = sub[i] / diag[i - 1];
				diag[i] = diag[i] - sub[i] * sup[i - 1];
				controlPoint[i] = controlPoint[i] - sub[i] * controlPoint[i - 1];
			}

			controlPoint[n - 2] = controlPoint[n - 2] / diag[n - 2];

			for (int i = n - 3; i >0; i--)
				controlPoint[i] = (controlPoint[i] - sup[i] * controlPoint[i + 1]) / diag[i];

			delete[] diag;
			delete[] sub;
			delete[] sup;
		}
	};

private:
	vector<Vector> dataPoint;
	vector<Vector> controlPoint;
	vector<Vector> splinePoint;
	double precision;
	BOOL isXcalibrated;
};


// CGammaDialog 对话框

IMPLEMENT_DYNAMIC(CGammaDialog, CDialog)

CGammaDialog::CGammaDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGammaDialog::IDD, pParent)
{
	m_drag = FALSE;
	m_bGammaEnable = FALSE;
	ZeroMemory(m_level, sizeof(m_level));
	m_moveflag = -1;
	m_bUseFildLoadCurve = FALSE;

	GdiplusStartupInput gdiplusStartupInput;
	
    // Initialize GDI+.
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

CGammaDialog::~CGammaDialog()
{
	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}

	GdiplusShutdown(m_gdiplusToken);
}

void CGammaDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_GAMMA, m_GammaCheck);
	DDX_Control(pDX, IDC_CHECK_GAMMA2, m_FileLoadCheck);
}


BEGIN_MESSAGE_MAP(CGammaDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_GAMMA, &CGammaDialog::OnBnClickedCheckGamma)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON1, &CGammaDialog::OnBnClickedButton1)
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_BUTTON2, &CGammaDialog::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_CHECK_GAMMA2, &CGammaDialog::OnBnClickedCheckGamma2)
END_MESSAGE_MAP()


// CGammaDialog 消息处理程序

#define CURVE_WINDOW_HEIGHT		256
#define CURVE_WINDOW_WIDTH		256

BOOL CGammaDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CDC* pDC = GetDC();

	m_MemDC.CreateCompatibleDC(pDC);
	
	m_MemBitmap.CreateCompatibleBitmap(pDC, 500, 500);
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_MemBitmap);
	ReleaseDC(pDC);

	CRect ChectRect;
	m_GammaCheck.GetWindowRect(ChectRect);
	ScreenToClient(&ChectRect);

	m_CurveRect.left = ChectRect.left + ChectRect.Width();
	m_CurveRect.right = m_CurveRect.left + CURVE_WINDOW_HEIGHT;
	m_CurveRect.top = ChectRect.bottom + 20;
	m_CurveRect.bottom = m_CurveRect.top + CURVE_WINDOW_WIDTH;
	
	m_keyPts.clear();
	m_keyPts.push_back(CPoint(0, m_CurveRect.Height()));
	m_keyPts.push_back(CPoint(m_CurveRect.Width(), 0));

	m_CurveFrameRect.left = m_CurveRect.left - 2;
	m_CurveFrameRect.right = m_CurveRect.right + 2;
	m_CurveFrameRect.top = m_CurveRect.top - 2;
	m_CurveFrameRect.bottom = m_CurveRect.bottom + 2;

	m_handCursor = ::LoadCursor(NULL, IDC_HAND);

	//init text postion
	CWnd * pCoordText1 = GetDlgItem(IDC_STATIC_COORD1);
	CWnd * pCoordText2 = GetDlgItem(IDC_STATIC_COORD2);
	CWnd * pCoordText3 = GetDlgItem(IDC_STATIC_COORD3);
	
	CRect TextRect;
	pCoordText1->GetClientRect(TextRect);
	pCoordText1->MoveWindow(m_CurveFrameRect.left - TextRect.Width() / 2, 
		m_CurveFrameRect.bottom, TextRect.Width(), TextRect.Height());

	pCoordText2->GetClientRect(TextRect);
	pCoordText2->MoveWindow(m_CurveFrameRect.right - TextRect.Width() / 2, 
		m_CurveFrameRect.bottom, TextRect.Width(), TextRect.Height());

	pCoordText3->GetClientRect(TextRect);
	pCoordText3->MoveWindow(m_CurveFrameRect.left - TextRect.Width() - 2, 
		m_CurveFrameRect.top - TextRect.Height() / 2, TextRect.Width(), TextRect.Height());

	//init postion x, y out put static text
	CWnd * pXText1 = GetDlgItem(IDC_STATIC_X);
	CWnd * pYText2 = GetDlgItem(IDC_STATIC_Y);
	CWnd * pCoordText4 = GetDlgItem(IDC_STATIC_COORD4);
	CWnd * pCoordText5 = GetDlgItem(IDC_STATIC_COORD5);
	
	pXText1->GetClientRect(TextRect);
	pXText1->MoveWindow(m_CurveFrameRect.right + 20, m_CurveFrameRect.top + 20, TextRect.Width(), TextRect.Height());
	pYText2->MoveWindow(m_CurveFrameRect.right + 20, m_CurveFrameRect.top + TextRect.Height() + 22, 
						TextRect.Width(), TextRect.Height());
	
	pCoordText4->MoveWindow(m_CurveFrameRect.right + TextRect.Width() + 22, m_CurveFrameRect.top + 20, 30, TextRect.Height());
	pCoordText5->MoveWindow(m_CurveFrameRect.right + TextRect.Width() + 22, 
							m_CurveFrameRect.top + TextRect.Height() + 22, 30, TextRect.Height());

	return TRUE;
}

void CGammaDialog::OnBnClickedCheckGamma()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bGammaEnable = m_GammaCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_GAMMA, m_bGammaEnable);
}

void CGammaDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码

	if (bShow) {
		m_GammaCheck.SetCheck(m_bGammaEnable);
		if (!m_bUseFildLoadCurve) {
			((CButton *)GetDlgItem(IDC_BUTTON2))->EnableWindow(FALSE);
		}else {
			((CButton *)GetDlgItem(IDC_BUTTON2))->EnableWindow(TRUE);
		}
	}
}

#define CURVE_LINE_COLOR		Color(0, 0, 0)		//BLACK
#define CONTROL_POINT_COLOR		Color(255, 0, 0)	//Red
#define BASE_LINE_COLOR			Color(150, 150, 150)

void CGammaDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 __super::OnPaint()

	//draw frame
	dc.DrawEdge(m_CurveFrameRect, EDGE_BUMP, BF_RECT);
	
	m_MemDC.FillSolidRect(0, 0, m_CurveRect.Width(), m_CurveRect.Height(), RGB(189, 252, 201));
	int oldBkMode = m_MemDC.SetBkMode(TRANSPARENT);

	CBrush brush(RGB(255, 0, 0));
	CBrush* pOldBrush;

	pOldBrush = m_MemDC.SelectObject(&brush);

	CPen pen(PS_SOLID, 4, RGB(255, 0, 0));
	CPen* pOldPen = m_MemDC.SelectObject(&pen);
	Graphics graphics(m_MemDC);

	//draw base line
	Pen gdiBaseLinePen(BASE_LINE_COLOR);
	graphics.DrawLine(&gdiBaseLinePen, Point(0, 255), Point(255, 0));

	// draw curve
	vector<Point> splinePoints;
	Spline spline;
	Pen gdiCurvePen(CURVE_LINE_COLOR);
	Point * Points = NULL;
	int nPoints = 0;

	if (!m_bUseFildLoadCurve) {
		graphics.DrawLine(&gdiCurvePen, Point(0, m_keyPts.front().y), Point(m_keyPts.front().x, m_keyPts.front().y));
		graphics.DrawLine(&gdiCurvePen, Point(m_CurveRect.Width(), m_keyPts.back().y), Point(m_keyPts.back().x, m_keyPts.back().y));
		
		spline.ListDataPoint(m_keyPts);
		spline.setPrecision(2);
		spline.SplinePoint(splinePoints);
		nPoints = splinePoints.size();
		Points = new Point[nPoints];
		vector<Point>::iterator iter = splinePoints.begin();
		for (int i = 0; iter != splinePoints.end(); ++iter, ++i)
			Points[i] = *iter;
	}else {
		nPoints = m_FileLoadPtsY.size();
		Points = new Point[nPoints];
		for (int i = 0; i < m_FileLoadPtsY.size(); ++i) {
			Points[i].X = i;
			Points[i].Y = 255 - m_FileLoadPtsY[i];
		}
	}

	graphics.DrawLines(&gdiCurvePen, Points, nPoints);

	if (!m_bUseFildLoadCurve)
		graphics.DrawLine(&gdiCurvePen, Point(m_keyPts.back().x, m_keyPts.back().y), Points[splinePoints.size() - 1]);
	
	delete[] Points;
	Points = NULL;

	//draw control point
	if (!m_bUseFildLoadCurve){
		SolidBrush gdiControlPosBrush(CONTROL_POINT_COLOR);
		vector<CPoint>::iterator iter2 = m_keyPts.begin();
		for (; iter2 != m_keyPts.end(); ++iter2) {
			Point points[4] = {Point(iter2->x, iter2->y - 3), Point(iter2->x - 3, iter2->y), 
							   Point(iter2->x, iter2->y + 3), Point(iter2->x + 3, iter2->y)};
			graphics.FillPolygon(&gdiControlPosBrush, points, 4);
		}
	}

	m_MemDC.SelectObject(pOldBrush);
	brush.DeleteObject();
	m_MemDC.SelectObject(pOldPen);
	pen.DeleteObject();
	m_MemDC.SetBkMode(oldBkMode);
	
	dc.BitBlt(m_CurveRect.left, m_CurveRect.top, m_CurveRect.Width(), m_CurveRect.Height(), &m_MemDC, 0, 0, SRCCOPY);
}

void CGammaDialog::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	__super::OnClose();
}

void CGammaDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_CurveRect.PtInRect(point)) {
		__super::OnMouseMove(nFlags, point);
		m_drag = FALSE;
		return;
	}

	CPoint InPoint;
	InPoint.x = point.x - m_CurveRect.left;
	InPoint.y = point.y - m_CurveRect.top;
	
	if (m_drag && m_moveflag > 0 && (unsigned int)(m_moveflag) < m_keyPts.size() - 1) {
		if (InPoint.x > m_keyPts[m_moveflag - 1].x + CONTROL_POINT_APART && 
			InPoint.x < m_keyPts[m_moveflag + 1].x - CONTROL_POINT_APART)
			m_keyPts[m_moveflag] = InPoint;
		else {
			vector<CPoint>::iterator iterRemove = m_keyPts.begin() + m_moveflag;
			m_keyPts.erase(iterRemove);
			m_moveflag = -1;
			m_drag = FALSE;
		}
	}

	if (m_drag && m_moveflag == 0 && InPoint.x < m_keyPts[1].x - CONTROL_POINT_APART)
		m_keyPts[0] = InPoint;

    if (m_drag && m_moveflag == m_keyPts.size() - 1 && 
		InPoint.x > m_keyPts[m_keyPts.size() - 2].x + CONTROL_POINT_APART)
        m_keyPts[m_moveflag] = InPoint;

	InvalidateRect(m_CurveRect, FALSE);
	
	if (m_drag && m_moveflag >= 0 && (unsigned int)(m_moveflag) < m_keyPts.size()) {
		CString strCoord;
		CWnd * pCoordText4 = GetDlgItem(IDC_STATIC_COORD4);
		CWnd * pCoordText5 = GetDlgItem(IDC_STATIC_COORD5);
		
		strCoord.Format(L"%d", InPoint.x);
		pCoordText4->SetWindowTextW(strCoord);
		strCoord.Format(L"%d", 255 - InPoint.y);
		pCoordText5->SetWindowTextW(strCoord);
	}
	
	__super::OnMouseMove(nFlags, point);
}

void CGammaDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_drag)
		m_drag = FALSE;

	__super::OnLButtonUp(nFlags, point);
}

void CGammaDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_CurveRect.PtInRect(point)) {
		__super::OnLButtonDown(nFlags, point);
		return;
	}

	if (m_bUseFildLoadCurve) {
		__super::OnLButtonDown(nFlags, point);
		return;
	}
	
	GetDlgItem(IDC_BUTTON1)->SetFocus();

	CPoint InPoint;
	InPoint.x = point.x - m_CurveRect.left;
	InPoint.y = point.y - m_CurveRect.top;

	for (unsigned int i = 1; i < m_keyPts.size(); ++i)
	{
		if (InPoint.x > m_keyPts[i - 1].x + CONTROL_POINT_APART && InPoint.y > 0 && 
			InPoint.x < m_keyPts[i].x - CONTROL_POINT_APART && InPoint.y < m_CurveRect.Height())
		{
			vector<CPoint>::iterator iterInsert = m_keyPts.begin() + i;
			m_keyPts.insert(iterInsert, InPoint);
			m_drag = TRUE;
			m_moveflag = i;
			::SetCursor(m_handCursor);
			InvalidateRect(m_CurveRect, FALSE);
		}
	}
	
	vector<CPoint>::iterator iter = m_keyPts.begin();
	CRect rect(InPoint.x - 20, InPoint.y - 20, InPoint.x + 20, InPoint.y + 20);
	for (int i = 0; iter != m_keyPts.end(); ++iter, ++i) {
		if (rect.PtInRect(*iter)) {
			m_drag = TRUE;
			m_moveflag = i;
		}
	}
	
	CString strCoord;
	CWnd * pCoordText4 = GetDlgItem(IDC_STATIC_COORD4);
	CWnd * pCoordText5 = GetDlgItem(IDC_STATIC_COORD5);
	
	strCoord.Format(L"%d", InPoint.x);
	pCoordText4->SetWindowTextW(strCoord);
	strCoord.Format(L"%d", 255 - InPoint.y);
	pCoordText5->SetWindowTextW(strCoord);

	__super::OnLButtonDown(nFlags, point);
}

BOOL CGammaDialog::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_drag) {
		::SetCursor(m_handCursor);
		return TRUE;
	}

	return __super::OnSetCursor(pWnd, nHitTest, message);
}

int CGammaDialog::SetGammaEnable(BOOL bEnable)
{
	m_bGammaEnable = bEnable;
	return 0;
}

int CGammaDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if (m_keyPts.size() * sizeof(POINT) > nStLen) return -1;
	
	nStLen = m_keyPts.size() * sizeof(POINT);
	nPageID = m_nID;

	BYTE * pos = (BYTE *)pPageInfoSt;
	
	POINT point = {0, 0};
	for (unsigned int i = 0; i < m_keyPts.size(); ++i, pos += sizeof(POINT)) {
		point.x = m_keyPts[i].x;
		point.y = m_keyPts[i].y;
		memcpy(pos, &point, sizeof(POINT));
	}

	return 0;
}

int CGammaDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || ((nStLen % sizeof(POINT)) != 0)) return -1;
	
	POINT point = {0, 0};
	BYTE * pos = (BYTE *)pPageInfoSt;

	m_keyPts.clear();
	for (int i = 0; i < nStLen; i += sizeof(POINT)) {
		point = *((POINT *)pos);
		m_keyPts.push_back(CPoint(point));
		pos += sizeof(POINT);
	}

	m_bGammaEnable = TRUE;
	m_GammaCheck.SetCheck(m_bGammaEnable);

	OnBnClickedCheckGamma();

	InvalidateRect(m_CurveRect, FALSE);

	return 0;
}

int CGammaDialog::GetPageInfoStIndex(int & nPageID, void * pPageInfoSt, int & nStLen, int nIndex)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(GAMMACALC)) || 
		(nIndex > 1) || (nIndex < 0) ) return -1;

	GetImageLevel();

	GAMMACALC stGammaCalc;
	ZeroMemory(&stGammaCalc, sizeof(GAMMACALC));

	BYTE * p = NULL;
	
	stGammaCalc.enable = m_bGammaEnable;
	stGammaCalc.type = ISP_CID_GAMMA;
	stGammaCalc.sync = nIndex;

	p = (BYTE *)(stGammaCalc.gamma);
	for (int i = 0; i < 128; ++i) {
		p[i] = m_level[nIndex * 128 + i];
	}

	memcpy(pPageInfoSt, &stGammaCalc, sizeof(GAMMACALC));

	nPageID = m_nID;
	nStLen = sizeof(GAMMACALC);

	return 0;
}

int CGammaDialog::Clear()
{
	m_bGammaEnable = FALSE;
	m_GammaCheck.SetCheck(0);
	ZeroMemory(m_level, sizeof(m_level));

	m_drag = FALSE;
	m_moveflag = -1;

	m_keyPts.clear();

	return 0;
}

void CGammaDialog::GetImageLevel()
{
	if (m_bUseFildLoadCurve) {
		for (unsigned int i = 0; i < m_FileLoadPtsY.size(); ++i)
			m_level[i] = m_FileLoadPtsY[i];

		return;
	}

	vector<Point> pts;
	pts.resize(m_keyPts.size());

	for (unsigned int i = 0; i < m_keyPts.size(); ++i) {
		pts[i].X = m_keyPts[i].x * 255 / m_CurveRect.Width();
		pts[i].Y = 255 - m_keyPts[i].y * 255 / m_CurveRect.Height();
	}

	for (int i = 0; i < pts[0].X; ++i)
		m_level[i] = (BYTE)pts[0].Y;

	for (int i = pts[m_keyPts.size() - 1].X; i < 256; ++i)
		m_level[i] = (BYTE)pts[m_keyPts.size() - 1].Y;

	Spline spline;
	spline.DataPoint(pts);
	spline.setPrecision(1.0);

	vector<Point> spt;
	spline.SplinePoint(spt);

	for (unsigned int i = 0; i < spt.size(); ++i)
	{
		int n = spt[i].Y;
		if (n < 0) n = 0;
		if (n > 255) n = 255;
		m_level[pts[0].X + i] = (BYTE)n;
	}
}

BOOL CGammaDialog::PreTranslateMessage(MSG * pMsg)
{
	// TODO: 在此添加控件通知处理程序代码
	if (pMsg->message == WM_KEYDOWN) {
		SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CGammaDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_GAMMA, 0);
}

void CGammaDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_moveflag < 0 || m_moveflag >= m_keyPts.size()) {
		__super::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}
	
	CPoint point = m_keyPts[m_moveflag];
	BOOL bMove = FALSE;

	switch(nChar) {
	case VK_LEFT:
		point.x -= 1;
		bMove = TRUE;
		break;
	case VK_RIGHT:
		point.x += 1;
		bMove = TRUE;
		break;
	case VK_UP:
		point.y -= 1;
		bMove = TRUE;
		break;
	case VK_DOWN:
		point.y += 1;
		bMove = TRUE;
		break;
	default:
		bMove = FALSE;
		break;
	}
	
	if (bMove) {
		BOOL bTempDrag = m_drag;
		m_drag = TRUE;

		point.x += m_CurveRect.left;
		point.y += m_CurveRect.top;

		OnMouseMove(nFlags, point);
		m_drag = bTempDrag;
	}

	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

#define NUMBER_SEPARATOR			','
#define STRING_SEPARATOR			0
#define READ_FILE_BUFFER_LEN		100
#define NUMBER_CHAR_BEGIN			'0'
#define NUMBER_CHAR_END				'9'

void CGammaDialog::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	USES_CONVERSION;

	CString strFile, strError;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"all (*.*)|*.*||", NULL);

	if (dlg.DoModal() == IDOK) {
		strFile = dlg.GetPathName();
	}else return;

	CFileException fileException;
	CFile file;
	if (!file.Open(strFile, CFile::modeRead | CFile::shareExclusive, &fileException)) {
		strError.Format(L"Error %u! Unable to open file%s!", fileException.m_cause, strFile);
		AfxMessageBox(strError, 0, 0);
		return;
	}
	
	m_FileLoadPtsY.clear();

	char Buf[READ_FILE_BUFFER_LEN] = {0}, aNumber[6] = {0}, *pWhere = NULL;
	UINT nlen = 0, nStepReadLen = READ_FILE_BUFFER_LEN - 1, nIndex = 0, nNumberIndex = 0;
	while ((m_FileLoadPtsY.size() < 256) && ((nlen = file.Read(Buf, nStepReadLen)) > 0)) {
		Buf[nlen] = STRING_SEPARATOR;
		nIndex = 0;

		while (nIndex < nlen) {

			while((nNumberIndex == 0) && 
				  (Buf[nIndex] < NUMBER_CHAR_BEGIN || Buf[nIndex] > NUMBER_CHAR_END) && (nIndex < nlen)) 
				++nIndex;

			if (nIndex >= nlen) break;
			
			while((Buf[nIndex] >= NUMBER_CHAR_BEGIN && Buf[nIndex] <= NUMBER_CHAR_END) && 
				  (nIndex < nlen) && (nNumberIndex < 5)) {
				aNumber[nNumberIndex] = Buf[nIndex];
				++nNumberIndex;
				++nIndex;
			}

			if ((nIndex >= nlen) && (nNumberIndex < (5 - 1))) break;
			
			if ((nNumberIndex == 5)) {
				while((Buf[nIndex] >= NUMBER_CHAR_BEGIN && Buf[nIndex] <= NUMBER_CHAR_END) && (nIndex < nlen))
					++nIndex;

				if (nIndex >= nlen) break;
			}

			m_FileLoadPtsY.push_back(atoi(aNumber));
			nNumberIndex = 0;
		}
	}

	if (nNumberIndex != 0) {
		m_FileLoadPtsY.push_back(atoi(aNumber));
		nNumberIndex = 0;
	}
	
	file.Close();

	InvalidateRect(m_CurveRect, FALSE);
}

void CGammaDialog::OnBnClickedCheckGamma2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bUseFildLoadCurve = m_FileLoadCheck.GetCheck();
	if (!m_bUseFildLoadCurve) {
		((CButton *)GetDlgItem(IDC_BUTTON2))->EnableWindow(FALSE);
	}else {
		((CButton *)GetDlgItem(IDC_BUTTON2))->EnableWindow(TRUE);
	}

	InvalidateRect(m_CurveRect, FALSE);
}
