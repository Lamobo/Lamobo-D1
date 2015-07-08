#pragma once

#include "XA2Player.h"
#include "InterfaceClass.h"
#include "AutoLock.h"
#include "Clock.h"
#include "ThreadBase.h"
#include <deque>
using namespace std;

typedef struct PCMData_st
{
	uint8_t *		pPcm;
	unsigned int	nDataLen;

	struct timeval	sPresentationTime;
}PCMDATA;

class CAudioRender : public IAudioSink, protected CThreadBase
{
public:
	static CAudioRender * createNew();
	virtual ~CAudioRender(void);

	int OpenRender();

	int CloseRender();

	int setClock(CClock * pClock);

	int GetRenderTime(uint64_t & nAudioRenderMs);

	virtual int SendAudio( PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime, 
						   unsigned int nAudioChannels, unsigned int nAudioSampleRate, unsigned int nAudioFmt);

protected:
	CAudioRender(void);
	
	/*
	static void PASCAL OnTimer(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);
	int InitTimer(UINT nDelay);
	int TimeRender();
	*/

	virtual void Run();
	
	int ThreadRender();
	int Render(uint8_t * pPcmData, UINT nDataLen);

	int FreeData(PCMDATA * pstPcmData);
	int FreeDeque();

private:
	XA2Player *				m_aPlayer;
	deque<PCMDATA *>		m_dePCM;
	CClock	*				m_SyncClock;
	
	HANDLE					m_hEvent[2];
	BOOL					m_bIsThread;
	/*
	UINT					m_nTimerID;
	UINT					m_nTimeAccuracy;
	BOOL					m_bHasTimeEvent;
	*/

	BOOL					m_bIsRendering;
	BOOL					m_bIsXAudioCanWork;
	uint64_t				m_nAudioRenderMs;

	CriticalSection			m_cs;
	CriticalSection			m_csForRender;
	CriticalSection			m_csForRenderMs;

	struct timeval			m_sfPresentationTime;
};
