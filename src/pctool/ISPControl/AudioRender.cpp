#include "StdAfx.h"
#include "AudioRender.h"

#define	TIME_ACCURACY	20 //20ms

enum Event_en {
	EVENT_EXECUTE,
	EVENT_STOP,
	EVENT_CNT
};

CAudioRender * CAudioRender::createNew()
{
	return new CAudioRender();
}

CAudioRender::CAudioRender(void)
: m_aPlayer(NULL), m_SyncClock(NULL)/*, m_nTimerID(0), m_nTimeAccuracy(0), m_bHasTimeEvent(FALSE)*/
, m_bIsThread(FALSE), m_bIsRendering(FALSE), m_bIsXAudioCanWork(FALSE), m_nAudioRenderMs(0)
{
	memset(&m_sfPresentationTime, 0, sizeof(struct timeval));
}

CAudioRender::~CAudioRender(void)
{
	CloseRender();
}

int CAudioRender::OpenRender()
{
	m_aPlayer = new XA2Player();
	if (NULL == m_aPlayer) return -1;

	memset(&m_sfPresentationTime, 0, sizeof(struct timeval));

	/*
	TIMECAPS tc;
	m_bHasTimeEvent = FALSE;
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR) {
		m_nTimeAccuracy = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
		timeBeginPeriod(m_nTimeAccuracy);
		m_bHasTimeEvent = TRUE;
	}*/

	m_bIsThread = TRUE;
	if (m_bIsThread) {
		for ( int i = 0; i < EVENT_CNT; ++i )
			m_hEvent[i] = ::CreateEvent( NULL, FALSE, FALSE, NULL );

		Start();
	}

	return 0;
}

int CAudioRender::CloseRender()
{
	::SetEvent( m_hEvent[EVENT_STOP] );
	Join();
	
	memset(&m_sfPresentationTime, 0, sizeof(struct timeval));

	for ( int i = 0; i < EVENT_CNT; ++i )
		::CloseHandle( m_hEvent[i] );
	FreeDeque();
	
	if (m_aPlayer) {
		m_aPlayer->FlushBuffers();
		delete m_aPlayer;
		m_aPlayer = NULL;
	}
	
	/*
	if (m_bHasTimeEvent) {
		timeKillEvent(m_nTimerID);
		timeEndPeriod(m_nTimeAccuracy);
	}
	m_bHasTimeEvent = FALSE;
	*/
	m_bIsRendering = FALSE;

	return 0;
}

int CAudioRender::setClock(CClock * pClock)
{
	m_SyncClock = pClock;
	return 0;
}

int CAudioRender::GetRenderTime(uint64_t & nAudioRenderMs)
{
	CAutoLock lock(&m_csForRenderMs);
	nAudioRenderMs = m_nAudioRenderMs;
	return 0;
}

int CAudioRender::SendAudio( PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime, 
						   unsigned int nAudioChannels, unsigned int nAudioSampleRate, unsigned int nAudioFmt)
{
	if (!m_bIsRendering) {
		WAVEFORMATEX stWaveFormat;
		stWaveFormat.nSamplesPerSec = nAudioSampleRate;
		stWaveFormat.nChannels = nAudioChannels;
		stWaveFormat.wBitsPerSample = nAudioFmt;
		stWaveFormat.cbSize = 0;
		stWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
		stWaveFormat.nBlockAlign = nAudioChannels * stWaveFormat.wBitsPerSample / 8;
		stWaveFormat.nAvgBytesPerSec = nAudioSampleRate * stWaveFormat.nBlockAlign;
		
		HRESULT hr = S_OK;
		hr = m_aPlayer->Initialize(&stWaveFormat);
		if (FAILED(hr)) m_bIsXAudioCanWork = FALSE;
		else m_bIsXAudioCanWork = TRUE;

		m_bIsRendering = TRUE;
	}

	if (!m_bIsXAudioCanWork) return -1;
	
	/*
	if (!m_bHasTimeEvent) {
		Render(pData, nDataLen);
		return 0;
	}*/

	if (!m_bIsThread) {
		Render(pData, nDataLen);
		return 0;
	}
	
	{
		CAutoLock lock(&m_cs);
		if (m_dePCM.size() > 200) {
			fprintf(stderr, "WARN::render audio may be too slow! size = %d\n", m_dePCM.size());
			return -1;
		}
	}

	PCMDATA * pstPcmData = new PCMDATA;
	if (NULL == pstPcmData) {
		fprintf(stderr, "can't alloc memory for audio Render\n");
		return -1;
	}
	
	pstPcmData->pPcm = new uint8_t[nDataLen];
	if (NULL == pstPcmData->pPcm) {
		fprintf(stderr, "can't alloc memory for audio Render pcm data\n");
		delete pstPcmData;
		return -1;
	}

	memcpy(pstPcmData->pPcm, pData, nDataLen);

	pstPcmData->nDataLen = nDataLen;
	pstPcmData->sPresentationTime = sPresentationTime;
	
	{
		CAutoLock lock(&m_cs);
		m_dePCM.push_back(pstPcmData);
	}
	
	/*
	if ((m_sfPresentationTime.tv_sec == 0) && (m_sfPresentationTime.tv_usec == 0)) InitTimer(m_nTimeAccuracy);
	*/

	::SetEvent( m_hEvent[EVENT_EXECUTE] );

	return 0;
}

void CAudioRender::Run()
{
	int iIndex = 0;

	while( true ) {
		
		iIndex = WaitForMultipleObjects( EVENT_CNT, m_hEvent, FALSE, TIME_ACCURACY );
		if ( iIndex - WAIT_OBJECT_0 == EVENT_EXECUTE ) {
			ThreadRender();
		}else if ( iIndex - WAIT_OBJECT_0 == EVENT_STOP ) {
			break;
		}else {
			ThreadRender();
		}
	}
}

#define LOCAL_ALOCK_BEGIN( cs ) { \
		CAutoLock lock( cs );
#define LOCAL_ALOCK_END	}

int CAudioRender::ThreadRender()
{
	PCMDATA * pPcmData = NULL;
	
	LOCAL_ALOCK_BEGIN(&m_cs)
	if (m_dePCM.empty()) return 0;
	pPcmData = m_dePCM.front();
	LOCAL_ALOCK_END;

	if ((m_sfPresentationTime.tv_sec == 0) && (m_sfPresentationTime.tv_usec == 0)) {
		m_sfPresentationTime = pPcmData->sPresentationTime;
		m_nAudioRenderMs = 0;
		if (m_SyncClock)
			m_SyncClock->start(m_sfPresentationTime);
	}

	if (pPcmData->sPresentationTime.tv_sec < m_sfPresentationTime.tv_sec){
		fprintf(stderr, "last video is time > current video time in second!\n");
		goto end;
	}else {
		if (pPcmData->sPresentationTime.tv_sec == m_sfPresentationTime.tv_sec) {
			if (pPcmData->sPresentationTime.tv_usec < m_sfPresentationTime.tv_usec) {
				fprintf(stderr, "last video is time > current video time in ns!\n");
				goto end;
			}
		}
	}
	
	if (m_SyncClock) { //使用同步时钟进行音视频同步。
		uint64_t nSyncMs = 0, nFrameMs = 0;
		int ret = 0;

		m_SyncClock->GetMsSinceStart(nSyncMs);
		ret = m_SyncClock->convertTime2MsSinceStart(pPcmData->sPresentationTime, nFrameMs);
		if (ret < 0) {
			fprintf(stderr, "the audio frame is presentation time is illegal!\n");
			goto end;
		}
		
		if ((LONGLONG)(nSyncMs) < (LONGLONG)(nFrameMs - 20)) { //wait to presentation time 
			return 0;
		}
		
		if ((LONGLONG)(nSyncMs) > (LONGLONG)(nFrameMs + 200)) { //this frame is too late to display.
			fprintf(stderr, "sync clock! this Audio frame is too late to play! %lld, %lld\n", nFrameMs, nSyncMs);
			goto end;
		}
	}
	
	Render(pPcmData->pPcm, pPcmData->nDataLen);
	//fprintf(stderr, "render audio: %ds, %dms", pPcmData->sPresentationTime.tv_sec, (pPcmData->sPresentationTime.tv_usec / 1000));
	
	{
		CAutoLock lock(&m_csForRenderMs);
		m_nAudioRenderMs = (uint64_t)(pPcmData->sPresentationTime.tv_sec) * (uint64_t)1000;
		m_nAudioRenderMs += (uint64_t)(pPcmData->sPresentationTime.tv_usec) / (uint64_t)1000;
	}


end:
	LOCAL_ALOCK_BEGIN(&m_cs)
	if (!m_dePCM.empty())
		m_dePCM.pop_front();
	LOCAL_ALOCK_END;
	
	if (pPcmData) {
		FreeData(pPcmData);
		delete pPcmData;
	}

	return 0;
}

int CAudioRender::Render(uint8_t * pPcmData, UINT nDataLen)
{
	unsigned int nCnt = 0;
	m_aPlayer->GetCurrentQueuedCnt(nCnt);

	if (nCnt >= 3) {
		return 0;
	}

	m_aPlayer->QueueSamples(pPcmData, nDataLen);
	m_aPlayer->PlayQueuedSamples();
	return 0;
}

int CAudioRender::FreeData(PCMDATA * pstPcmData)
{
	if (pstPcmData == NULL) return 0;
	
	if (pstPcmData->pPcm) delete[] pstPcmData->pPcm;
	pstPcmData->pPcm = NULL;

	return 0;
}

int CAudioRender::FreeDeque()
{
	if (m_dePCM.empty()) return 0;

	deque<PCMDATA *>::iterator iter = m_dePCM.begin();
	for (; iter != m_dePCM.end(); ++iter) {
		FreeData(*iter);
		delete *iter;
	}

	m_dePCM.clear();
	return 0;
}

/*
void CAudioRender::OnTimer(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CAudioRender * Render = (CAudioRender*)dwUser;
	Render->TimeRender();
}

int CAudioRender::InitTimer(UINT nDelay)
{
	if (!m_bHasTimeEvent) return 0;
	
	if (m_nTimerID) {
		timeKillEvent(m_nTimerID);
		timeEndPeriod(m_nTimeAccuracy);
		m_nTimerID = NULL;
	}

	m_nTimerID = timeSetEvent(nDelay, m_nTimeAccuracy, OnTimer, (DWORD_PTR)this, TIME_ONESHOT);
	return 0;
}

int CAudioRender::TimeRender()
{
	PCMDATA * pPcmData = NULL;
	
	LOCAL_ALOCK_BEGIN(&m_cs)
	if (m_dePCM.empty()) goto end;
	pPcmData = m_dePCM.front();
	LOCAL_ALOCK_END;

	if ((m_sfPresentationTime.tv_sec == 0) && (m_sfPresentationTime.tv_usec == 0)) {
		m_sfPresentationTime = pPcmData->sPresentationTime;
		m_SyncClock->start(m_sfPresentationTime);
	}
	
	uint64_t nSyncMs = 0, nFrameMs = 0;
	int ret = 0;

	m_SyncClock->GetMsSinceStart(nSyncMs);
	ret = m_SyncClock->convertTime2MsSinceStart(pPcmData->sPresentationTime, nFrameMs);
	if (ret < 0) {
		fprintf(stderr, "the frame is presentation time is illegal!\n");
		goto end;
	}
	
	if ((LONGLONG)(nSyncMs) < (LONGLONG)(nFrameMs - 10)) { //wait to presentation time 
		InitTimer(max((nFrameMs - nSyncMs), m_nTimeAccuracy));
		return 0;
	}
	
	if ((LONGLONG)(nSyncMs) > (LONGLONG)(nFrameMs + 100)) { //this frame is too late to display.
		goto end;
	}

	Render(pPcmData->pPcm, pPcmData->nDataLen);

end:
	LOCAL_ALOCK_BEGIN(&m_cs)
	if (!m_dePCM.empty())
		m_dePCM.pop_front();
	LOCAL_ALOCK_END;
	
	if (pPcmData) {
		FreeData(pPcmData);
		delete pPcmData;
	}

	InitTimer(m_nTimeAccuracy);
	return 0;
}
*/