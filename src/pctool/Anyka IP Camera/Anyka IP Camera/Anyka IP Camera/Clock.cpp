#include "StdAfx.h"
#include "Clock.h"

CClock::CClock(void)
: m_bIsStart(FALSE), m_dfFreq(0.0), m_nStartTime(0)
{
	memset(&m_sFirstTime, 0, sizeof(struct timeval));
}

CClock::~CClock(void)
{
}

int CClock::start(struct timeval sTime)
{
	CAutoLock lock(&m_cs);
	if (m_bIsStart) return 0;
	if ((m_sFirstTime.tv_sec != 0) || (m_sFirstTime.tv_usec != 0)) return 0;

	LARGE_INTEGER stfreq;
	QueryPerformanceFrequency(&stfreq);
	m_dfFreq = (double)stfreq.QuadPart;
	
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);
	m_nStartTime = (LONGLONG)startTime.QuadPart;
	m_sFirstTime = sTime;
	
	m_bIsStart = TRUE;

	return 0;
}

int CClock::ReStart(struct timeval sTime)
{
	CAutoLock lock(&m_cs);
	
	if (ReInit() < 0) return -1;
	if (start(sTime) < 0) return -1;

	return 0;
}

BOOL CClock::IsStart()
{
	CAutoLock lock(&m_cs);
	return m_bIsStart;
}

int CClock::GetMsSinceStart(uint64_t & nMs)
{
	CAutoLock lock(&m_cs);
	if (!m_bIsStart) { 
		nMs = 0;
		return 0;
	}

	LARGE_INTEGER sNow;
	LONGLONG nNowTime;
	QueryPerformanceCounter(&sNow);
	nNowTime = (LONGLONG)sNow.QuadPart;
	
	double dfMinus = (double)(nNowTime - m_nStartTime);
	
	nMs = (uint64_t)((dfMinus / m_dfFreq) * 1000);

	return 0;
}

int CClock::convertTime2MsSinceStart(struct timeval sTime, uint64_t & nMs)
{
	CAutoLock lock(&m_cs);
	if (!m_bIsStart) { 
		nMs = 0;
		start(sTime);
		return 0;
	}

	if (sTime.tv_sec < m_sFirstTime.tv_sec) return -1;
	else if (sTime.tv_sec == m_sFirstTime.tv_sec)
		if (sTime.tv_usec < m_sFirstTime.tv_usec) return -1;

	nMs = (sTime.tv_sec - m_sFirstTime.tv_sec) * 1000;
	if (sTime.tv_usec < m_sFirstTime.tv_usec) {
		sTime.tv_usec += 1000000;
		nMs -= 1000;
	}

	nMs += ((sTime.tv_usec - m_sFirstTime.tv_usec)/1000);

	return 0;
}

int CClock::ReInit()
{
	CAutoLock lock(&m_cs);
	m_bIsStart = FALSE;
	m_dfFreq = 0.0;
	m_nStartTime = 0;
	memset(&m_sFirstTime, 0, sizeof(struct timeval));

	return 0;
}
