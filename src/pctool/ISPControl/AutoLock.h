#pragma once

class CriticalSection
{
public:
	CriticalSection(void);
	virtual ~CriticalSection(void);

	void lock();

	void unlock();

private:
	CRITICAL_SECTION m_cCriSec;
};

class CAutoLock
{
public:
	CAutoLock( CriticalSection * pCS );
	~CAutoLock(void);

private:
	CriticalSection * m_lock;
};
