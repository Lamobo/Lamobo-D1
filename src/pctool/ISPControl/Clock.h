#pragma once
#include "stdint.h"
#include "AutoLock.h"

class CClock
{
public:
	CClock(void);
	virtual ~CClock(void);
	
	int start(struct timeval sTime);
	int ReStart(struct timeval sTime);

	BOOL IsStart();

	int GetMsSinceStart(uint64_t & nMs);
	
	int convertTime2MsSinceStart(struct timeval sTime, uint64_t & nMs);

	int ReInit();

private:
	BOOL				m_bIsStart;
	double				m_dfFreq;
	LONGLONG			m_nStartTime;
	struct timeval		m_sFirstTime;
	CriticalSection		m_cs;
};
