#include "StdAfx.h"
#include "BasePage.h"

CBasePage::CBasePage(void)
: m_nID(-1), m_pMessageWnd(NULL)
{
}

CBasePage::~CBasePage(void)
{
}

int CBasePage::SetPageID(int nPageId)
{
	m_nID = nPageId;
	return 0;
}

int CBasePage::GetPageID(int & nPageId)
{
	nPageId = m_nID;
	return 0;
}

int CBasePage::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	return 0;
}

int CBasePage::GetPageInfoStAll(int & nPageID, void * pPageInfoSt, int & nStlen)
{
	return 0;
}

int CBasePage::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	return 0;
}

int CBasePage::SetPageInfoStAll(void * pPageInfoSt, int nStLen)
{
	return 0;
}

BOOL CBasePage::GetPageEnable()
{
	return FALSE;
}

int CBasePage::SendPageMessage(CWnd * pWnd, int nPageMsg, int nFlag, unsigned short nMessage)
{
	if (NULL == pWnd || !IsWindow(pWnd->GetSafeHwnd())) return -1;
	
	::SendMessage(pWnd->GetSafeHwnd(), nPageMsg, nFlag,  MAKEMESSAGELP(m_nID, nMessage));
	return 0;
}

int CBasePage::SetMessageWindow(CWnd * pWnd)
{
	m_pMessageWnd = pWnd;
	return 0;
}

int CBasePage::Clear()
{
	return 0;
}
