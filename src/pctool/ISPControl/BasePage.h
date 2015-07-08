#pragma once

#define WM_ENABLE_CHANGE	WM_USER + 200
#define WM_SUBMISSION		WM_USER + 201
#define WM_GET_ISP_INFO		WM_USER + 202

#define MAX_PAGE_STRUCT_LEN	300

#define MAKEMESSAGELP(id, msg)		((id << 16) | (msg))
#define GETMESSAGEID(x)				((x) >> 16)
#define GETMESSAGEINFO(x)			((x) & 0xFFFF)

enum EnableChangeFlag_en
{
	ECT_BB = 0,
	ECT_LENS,
	ECT_DEMOSAIC,
	ECT_RGBFILTER,
	ECT_UVFILTER,
	ECT_DEFECTPIXEL,
	ECT_WB,
	ECT_AWB,
	ECT_CCORRECT,
	ECT_GAMMA,
	ECT_BENHANCE,
	ECT_SATURATION,
	ECT_SPECEFFEICT,
	ECT_REGISTER,
	ECT_NUMBER
};

class CBasePage
{
public:
	CBasePage(void);
	virtual ~CBasePage(void);
	
	virtual int SetPageID(int nPageId);
	virtual int GetPageID(int & nPageId);
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int GetPageInfoStAll(int & nPageID, void * pPageInfoSt, int & nStlen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	virtual int SetPageInfoStAll(void * pPageInfoSt, int nStLen);
	virtual BOOL GetPageEnable();
	virtual int SendPageMessage(CWnd * pWnd, int nPageMsg, int nFlag, unsigned short nMessage);
	virtual int SetMessageWindow(CWnd * pWnd);
	virtual int Clear();

protected:
	int m_nID;
	CWnd * m_pMessageWnd;
};
