#include "StdAfx.h"
#include "VideoRender.h"
#include "Mmsystem.h"

#define	TIME_ACCURACY	20 //10ms

enum Event_en {
	EVENT_EXECUTE,
	EVENT_STOP,
	EVENT_CNT
};

typedef enum overlay_en
{
	OVERLAY_LINE = 0,
	OVERLAY_RECT,
	OVERLAY_MAX
}OVERLAY;

class COverlay
{
public:
	COverlay(D3DCOLOR color, BYTE opacity, OVERLAY type){
		m_opacity = opacity;
		m_color = (color & 0x00FFFFFF) | (opacity << 24);
		m_type = type;
	}

	virtual ~COverlay(void){}

	virtual int SetOverlayType(OVERLAY enOverlayType) {
		m_type = enOverlayType;
		return 0;
	}
	
	virtual OVERLAY GetOverlayType() {
		return m_type;
	}

	virtual int Draw(D3D9RenderImpl * pDrawDevice, SHORT key) = 0;

protected:
	OVERLAY	m_type;
	D3DCOLOR m_color;
	BYTE m_opacity;
};

class CLineOverlay : public COverlay
{
public:
	CLineOverlay(POINT P1, POINT P2, INT width, D3DCOLOR color, BYTE opacity)
	: COverlay(color, opacity, OVERLAY_LINE)
	, m_PStart(P1), m_PEnd(P2), m_width(width)
	{
	}

	virtual ~CLineOverlay(){}

	virtual int Draw(D3D9RenderImpl * pDrawDevice, SHORT key) {
		return pDrawDevice->DrawLine(key, m_PStart, m_PEnd, m_width, m_color, m_opacity);
	}

protected:
	POINT	m_PStart, m_PEnd;
	INT	m_width;
};

class CRectOverlay : public COverlay
{
public:
	CRectOverlay(RECT rect, INT width, D3DCOLOR color, BYTE opacity) 
	: COverlay(color, opacity, OVERLAY_RECT)
	, m_rect(rect), m_width(width)
	{
	}

	virtual ~CRectOverlay(){}
	
	virtual int Draw(D3D9RenderImpl * pDrawDevice, SHORT key) {
		return pDrawDevice->DrawRectangle(key, m_rect, m_width, m_color, m_opacity);
	}

protected:
	RECT m_rect;
	INT	m_width;
};

CVideoRender * CVideoRender::createNew()
{
	return new CVideoRender();
}

CVideoRender::CVideoRender(void)
: m_pD3DRender(NULL), m_pD3DDeleteRender(NULL), m_pAudioRender(NULL), /*m_nTimerID(0), m_nTimeAccuracy(1)
, m_bHasTimeEvent(FALSE),*/m_bIsThread(FALSE), m_bIsRendering(FALSE), m_nDFreamCount(0)
, m_SyncClock(NULL), m_hDisplayWindow(0), m_nVHeight(0), m_nVWidth(0), m_nFpsOneSec(0)
, m_nRecodeFrames(0), m_nLastMsSinceStart(0), m_mode(Fill), m_bIsLate(FALSE)
, m_nLateCnt(0), m_nLateRender(0), m_bNeedJudge(TRUE), m_nLastRenderTime(0)
{
	m_ThrowFrameRate = 0;
	m_ShowFrameRate = 0;
	m_nServerStreamFps = 30;
	m_IsD3dOpen = FALSE;
	memset(&m_sfPresentationTime, 0, sizeof(struct timeval));
}

CVideoRender::~CVideoRender(void)
{
	CloseRender();
}

int CVideoRender::OpenRender(HWND hDisplayWindow)
{
	m_TimeClock.ReInit();
	m_nRecodeFrames = 0;
	m_nDFreamCount = 0;
	m_nFpsOneSec = 0;
	m_nLastRenderTime = 0;
	m_ThrowFrameRate = 0;
	m_ShowFrameRate = 0;
	m_IsD3dOpen = FALSE;

	m_pD3DRender = new D3D9RenderImpl();
	if (NULL == m_pD3DRender) return -1;
	
	m_hDisplayWindow = hDisplayWindow;
	HRESULT hr = m_pD3DRender->Initialize( hDisplayWindow );
	if (FAILED(hr)) {	
		//AfxMessageBox( L"no support D3D9\n", 0, 0 );
		return -2;
	}
	
	m_IsD3dOpen = TRUE;

	m_pD3DRender->RegisterDeviceLostCallback(onDeviceLost, onDeviceReset, (void *)this);
	
	m_pD3DRender->Clear(D3DCOLOR_ARGB(0xFF, 0, 0, 0));

	m_bIsThread = TRUE;
	if (m_bIsThread) {
		for ( int i = 0; i < EVENT_CNT; ++i )
			m_hEvent[i] = ::CreateEvent( NULL, FALSE, FALSE, NULL );

		Start();
	}
	
	/*
	TIMECAPS tc;
	m_bHasTimeEvent = FALSE;
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR) {
		m_nTimeAccuracy = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
		timeBeginPeriod(m_nTimeAccuracy);
		m_bHasTimeEvent = TRUE;
	}*/

	return 0;
}

int CVideoRender::SetServerStreamFps(int nServerStreamFps)
{
	m_nServerStreamFps = nServerStreamFps;
	return 0;
}

int CVideoRender::Reset()
{
	m_TimeClock.ReInit();
	m_RenderClock.ReInit();

	memset(&m_sfPresentationTime, 0, sizeof(struct timeval));
	FreeDeque();
	
	m_nFpsOneSec = 0;
	m_nRecodeFrames = 0;
	m_nLastMsSinceStart = 0;
	m_bIsLate = FALSE;
	m_nLateCnt = 0;
	m_nLateRender = 0;
	m_bNeedJudge = TRUE;
	m_ThrowFrameRate = 0;
	m_ShowFrameRate = 0;

	if (m_SyncClock) {
		m_SyncClock->ReInit();
	}

	if (m_pAudioRender){
		CAutoLock lock(&m_csForARender);
		m_pAudioRender = NULL;
	}

	return 0;
}

BOOL CVideoRender::IsRendering()
{
	return m_bIsRendering;
}

void CVideoRender::AddOverlay(SHORT key, COverlay * pOverlay)
{
	map<SHORT, COverlay*>::iterator iter = m_OverlayMap.find(key);
	if (iter != m_OverlayMap.end()) delete iter->second;
	m_OverlayMap[key] = pOverlay;
}

int CVideoRender::DrawLine(SHORT key, POINT p1, POINT p2, FLOAT width, D3DCOLOR color, BYTE opacity)
{
	ASSERT(m_pD3DRender);
	
	HRESULT hr = E_FAIL;

	CAutoLock lock(&m_csForRender);
	hr = m_pD3DRender->DrawLine(key, p1, p2, width, color, opacity);
	if (FAILED(hr))	return -1;
	
	COverlay * pOverlay = new CLineOverlay(p1, p2, width, color, opacity);
	if (pOverlay == NULL) {
		fprintf(stderr, "%s:%s out of memory!\n", __FILE__, __FUNCTION__);
		return -1;
	}
	
	AddOverlay(key, pOverlay);

	return 0;
}

int CVideoRender::DrawRectangle(SHORT key, RECT rectangle, FLOAT width, D3DCOLOR color, BYTE opacity)
{
	ASSERT(m_pD3DRender);
	
	HRESULT hr = E_FAIL;

	CAutoLock lock(&m_csForRender);
	hr = m_pD3DRender->DrawRectangle(key, rectangle, width, color, opacity);
	if (FAILED(hr)) return -1;

	COverlay * pOverlay = new CRectOverlay(rectangle, width, color, opacity);
	if (pOverlay == NULL) {
		fprintf(stderr, "%s:%s out of memory!\n", __FILE__, __FUNCTION__);
		return -1;
	}

	AddOverlay(key, pOverlay);

	return 0;
}

int CVideoRender::SendVideo( PBYTE pY, unsigned int nYlineSize, PBYTE pU, unsigned int nUlineSize, 
					   PBYTE pV, unsigned int nVlineSize, struct timeval sPresentationTime, 
					   unsigned int nVideoHeight, unsigned int nVideoWidth )
{
	if (!m_bIsRendering) {
		m_nVHeight = nVideoHeight;
		m_nVWidth = nVideoWidth;
		m_bIsRendering = TRUE;
		
		if (m_IsD3dOpen) {
			CAutoLock lock(&m_csForRender);
			m_pD3DRender->CreateVideoSurface(nVideoWidth, nVideoHeight);
			m_pD3DRender->SetDisplayMode(m_mode);
		}
	}
	
	if (!m_bIsThread) {
		Render(pY, nYlineSize, pU, nUlineSize, pV, nVlineSize);
		return 0;
	}
	
	{
		CAutoLock lock(&m_cs);
		if (m_dePic.size() > 200) {
			//fprintf(stderr, "WARN::render video may be too slow! size = %d\n", m_dePic.size());
			return -1;
		}

		if (!m_ShowFrameRate) {//根据队列中帧的多少决定丢帧的策略。
			if (m_dePic.size() > 8 && m_dePic.size() < 15) {
				m_ThrowFrameRate = 1;
				m_ShowFrameRate = 2;
			}else if (m_dePic.size() >= 15 && m_dePic.size() < 40) {
				m_ThrowFrameRate = 1;
				m_ShowFrameRate = 1;
			}else if (m_dePic.size() >= 40 && m_dePic.size() < 60) {
				m_ThrowFrameRate = 2;
				m_ShowFrameRate = 1;
			}else if (m_dePic.size() >= 60) {
				m_ThrowFrameRate = 3;
				m_ShowFrameRate = 1;
			}else {
				m_ThrowFrameRate = 0;
				m_ShowFrameRate = 0;
			}
		}
	}

	if (m_ThrowFrameRate > 0) {
		--m_ThrowFrameRate;
		return 0;
	}

	PICYUVDATA * pstYuvData = NULL;

	try{
		pstYuvData = new PICYUVDATA;
		if (NULL == pstYuvData) {
			fprintf(stderr, "can't alloc memory for video Render\n");
			return -1;
		}

		ZeroMemory(pstYuvData, sizeof(PICYUVDATA));
		
		pstYuvData->pYData = new uint8_t[nYlineSize * nVideoHeight];
		if (NULL == pstYuvData->pYData) {
			fprintf(stderr, "can't alloc memory for video Render Y data\n");
			delete pstYuvData;
			return -1;
		}

		memcpy(pstYuvData->pYData, pY, nYlineSize * nVideoHeight);
		pstYuvData->nYlineSize = nYlineSize;

		pstYuvData->pUData = new uint8_t[nUlineSize * nVideoHeight >> 1];
		if (NULL == pstYuvData->pUData) {
			fprintf(stderr, "can't alloc memory for video Render U data\n");
			delete[] pstYuvData->pYData;
			delete pstYuvData;
			return -1;
		}

		memcpy(pstYuvData->pUData, pU, nUlineSize * nVideoHeight >> 1);
		pstYuvData->nUlineSize = nUlineSize;

		pstYuvData->pVData = new uint8_t[nVlineSize * nVideoHeight >> 1];
		if (NULL == pstYuvData->pVData) {
			fprintf(stderr, "can't alloc memory for video Render V data\n");
			delete[] pstYuvData->pYData;
			delete[] pstYuvData->pUData;
			delete pstYuvData;
			return -1;
		}

		memcpy(pstYuvData->pVData, pV, nVlineSize * nVideoHeight >> 1);
		pstYuvData->nVlineSize = nVlineSize;

		pstYuvData->sPresentationTime = sPresentationTime;

	}catch (CMemoryException * pEx) {
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		AfxMessageBox(sz);
		pEx->Delete();
		FreeData(pstYuvData);
		if (pstYuvData) delete pstYuvData;

		return -1;
	}
	
	if (m_ShowFrameRate)
		--m_ShowFrameRate;

	{
		CAutoLock lock(&m_cs);
		m_dePic.push_back(pstYuvData);
	}
	
	/*
	if ((m_sfPresentationTime.tv_sec == 0) && (m_sfPresentationTime.tv_usec == 0)) InitTimer(m_nTimeAccuracy);
	*/

	::SetEvent( m_hEvent[EVENT_EXECUTE] );

	return 0;
}

int CVideoRender::setClock(CClock * pClock)
{
	m_SyncClock = pClock;
	return 0;
}

int CVideoRender::FullScreen(BOOL bIsFullScreen, FULLSCREENMESSAGE pMessageCallBack, void * pClassParam)
{
	CAutoLock lock(&m_csForRender);
	
	return m_pD3DRender->FullScreen(bIsFullScreen, 0, pMessageCallBack, pClassParam);
}

int CVideoRender::SetAudioRender(CAudioRender * pAudioRender)
{
	CAutoLock lock(&m_csForARender);
	m_pAudioRender = pAudioRender;
	return 0;
}

int CVideoRender::SetFillMode(FillModeMe mode)
{
	m_mode = mode;
	return 0;
}

int CVideoRender::getFpsAverage(float & fFps)
{
	uint64_t nStartTime = 0;
	m_TimeClock.GetMsSinceStart(nStartTime);
	if (!nStartTime) {
		fFps = 0.0;
		return 0;
	}

	double startSecond = ((double)nStartTime / (double)1000);
	fFps = (double)m_nDFreamCount / startSecond;

	return 0;
}

UINT CVideoRender::getFpsOneSec()
{
	return m_nFpsOneSec;
}

void CVideoRender::onDeviceLost(void * pParam)
{
	//onDeviceReset(pParam);
}

void CVideoRender::onDeviceReset(void * pParam)
{
	CVideoRender * Render = (CVideoRender*)pParam;
	if (!(Render->IsRendering())) return;
	Render->m_pD3DDeleteRender = Render->m_pD3DRender;
}

void CVideoRender::Run()
{
	int iIndex = 0;

	while( true ) {
		
		iIndex = WaitForMultipleObjects( EVENT_CNT, m_hEvent, FALSE, TIME_ACCURACY );
		if ( iIndex - WAIT_OBJECT_0 == EVENT_EXECUTE ) {
			if (ThreadRender() < 0) return;
		}else if ( iIndex - WAIT_OBJECT_0 == EVENT_STOP ) {
			break;
		}else {
			if (ThreadRender() < 0) return;
		}
	}
}

void CVideoRender::CalcOneSecFps()
{
	uint64_t nMs = 0;
	m_TimeClock.GetMsSinceStart(nMs);

	uint64_t diff = nMs - m_nLastMsSinceStart;
	if (diff >= 1000 && diff < 1030) {
		m_nFpsOneSec = m_nRecodeFrames;
	}else if (diff >= 1030) {
		m_nFpsOneSec = (UINT)((double)(m_nRecodeFrames) / (double)((double)(diff) / (double)(1000)));
	}else {
		return;
	}

#ifdef WARN_ERROR_OUT
	if (m_nFpsOneSec < 25) {
		int Size = 0;
		{
			CAutoLock lock( &m_cs );
			Size = m_dePic.size();
		}
		fprintf(stderr, "WARN::####Fps = %d, In %lldms, render buffer size= %d\n", m_nFpsOneSec, diff, Size);
	}
#endif

	m_nRecodeFrames = 0;
	m_nLastMsSinceStart = nMs;
}

#define LOCAL_ALOCK_BEGIN( cs ) { \
		CAutoLock lock( cs );
#define LOCAL_ALOCK_END	}

#define AUDIO_VIDEO_TIME_DIFF_MAX		150
#define THROW_FRAME_RATE				1	//display 1 frames after throw 1 frame.
#define RESTART_SYNC_CLOCK_CNT			10	//after ten time frame late, we will restart the sync clock.

int CVideoRender::ThreadRender()
{
	BOOL bIsLateAvail = TRUE;
	PICYUVDATA * pYuvData = NULL;
	
	LOCAL_ALOCK_BEGIN(&m_cs)
	if (m_dePic.empty()){
		CalcOneSecFps();
		return 0;
	}
	pYuvData = m_dePic.front();
	LOCAL_ALOCK_END;

	if (m_nServerStreamFps == 0) m_nServerStreamFps = 30;

	int nMinRenderDistance = (1000 / m_nServerStreamFps) - 5;

	if ((m_sfPresentationTime.tv_sec == 0) && (m_sfPresentationTime.tv_usec == 0)) {
		m_TimeClock.start(m_sfPresentationTime); // time start with 0
		m_RenderClock.start(m_sfPresentationTime);
		m_sfPresentationTime = pYuvData->sPresentationTime;
		if (m_SyncClock)
			m_SyncClock->start(m_sfPresentationTime);
	}

	if (pYuvData->sPresentationTime.tv_sec < m_sfPresentationTime.tv_sec){
		fprintf(stderr, "last video is time > current video time in second!\n");
		goto end;
	}else {
		if (pYuvData->sPresentationTime.tv_sec == m_sfPresentationTime.tv_sec) {
			if (pYuvData->sPresentationTime.tv_usec < m_sfPresentationTime.tv_usec) {
				fprintf(stderr, "last video is time > current video time in ns!\n");
				goto end;
			}
		}
	}
	
	if (!m_bNeedJudge) goto render;

	if (m_SyncClock) {//使用同步时钟进行音视频同步，请不要与使用Audio时间戳同步的方法混用。
		uint64_t nSyncMs = 0, nFrameMs = 0;
		int ret = 0;

		m_SyncClock->GetMsSinceStart(nSyncMs);
		ret = m_SyncClock->convertTime2MsSinceStart(pYuvData->sPresentationTime, nFrameMs);
		if (ret < 0) {
			fprintf(stderr, "the frame is presentation time is illegal!\n");
			goto end;
		}

		if ((LONGLONG)(nSyncMs) < (LONGLONG)(nFrameMs - 10)) { //wait to presentation time 
			m_bIsLate = FALSE;
			return 0;
		}
		
		if ((LONGLONG)(nSyncMs) > (LONGLONG)(nFrameMs + 60)) { //this frame is too late to display.
			if (m_bIsLate) ++m_nLateCnt;
			m_bIsLate = TRUE;

			if (m_nLateCnt >= RESTART_SYNC_CLOCK_CNT) {
				//m_SyncClock->ReInit();
				//m_SyncClock->start(pYuvData->sPresentationTime);
				m_SyncClock->ReStart(pYuvData->sPresentationTime);
				m_nLateCnt = 0;
			}

			fprintf(stderr, "sync clock!this video frame is too late to display! %lld, %lld\n", nFrameMs, nSyncMs);
			goto end;
		}else {
			m_bIsLate = FALSE;
			m_nLateCnt = 0;
		}
	}
	
	{//使用Audio的Render时间戳进行音视频同步，请不要与使用同步时钟的方法进行混用
		CAutoLock lock(&m_csForARender);
		if (m_pAudioRender) {
			uint64_t nAudioRenderMs = 0, nMs = 0;
			m_pAudioRender->GetRenderTime(nAudioRenderMs);
			if (nAudioRenderMs == 0) goto render;

			nMs = (uint64_t)(pYuvData->sPresentationTime.tv_sec) * (uint64_t)1000;
			nMs += (uint64_t)(pYuvData->sPresentationTime.tv_usec) / (uint64_t)1000;

			if (nMs + AUDIO_VIDEO_TIME_DIFF_MAX < nAudioRenderMs){//this frame is too late to display.
				m_bIsLate = TRUE;
				fprintf(stderr, "sync Audio!this video frame is too late to display! %lld, %lld\n", nMs, nAudioRenderMs);
				goto end;
			}else {
				m_bIsLate = FALSE;
			}

			if (nMs - AUDIO_VIDEO_TIME_DIFF_MAX > nAudioRenderMs) {//wait to presentation time 
				//fprintf(stderr, "sync Audio!this video will wait to presentation time! %lld, %lld\n", nMs, nAudioRenderMs);
				CalcOneSecFps();
				return 0;
			}
		}
	}

	if (m_pD3DDeleteRender) {
		m_pD3DDeleteRender = NULL;
		if (ReOpenD3d() < 0) return -1;
	}
	
	if (!m_SyncClock) {//使用Audio的Render时间戳进行同步时，还需要在播放时按照采集帧率进行播放。解决播放时画面快慢不均的问题。
		uint64_t nNowTime = 0;
		m_RenderClock.GetMsSinceStart(nNowTime);
		
		if (m_nLastRenderTime && (nNowTime - m_nLastRenderTime < nMinRenderDistance)) {
			return 0;
		}

		m_nLastRenderTime = nNowTime;
	}
	
render:
	if (m_bIsLate) ++m_nLateRender;

	if (m_nLateRender >= THROW_FRAME_RATE) {
		m_bNeedJudge = TRUE;
		m_nLateRender = 0;
		bIsLateAvail = FALSE;
	}

	Render(pYuvData->pYData, pYuvData->nYlineSize, pYuvData->pUData, pYuvData->nUlineSize, 
			pYuvData->pVData, pYuvData->nVlineSize);
	
	++m_nRecodeFrames;
	++m_nDFreamCount;

	CalcOneSecFps();

end:
	if (bIsLateAvail && m_bIsLate) {
		m_bNeedJudge = FALSE;
	}

	LOCAL_ALOCK_BEGIN(&m_cs)
	if (!m_dePic.empty())
		m_dePic.pop_front();
	LOCAL_ALOCK_END;
	
	if (pYuvData) {
		FreeData(pYuvData);
		delete pYuvData;
	}

	CalcOneSecFps();
	
	return 0;
}

int CVideoRender::ReOpenD3d()
{
	CAutoLock lock(&m_csForRender);
	if (m_pD3DRender) {
		m_pD3DRender->ReleaseInstance();
		m_pD3DRender = NULL;
	}
	
	m_pD3DRender = new D3D9RenderImpl();
	if (NULL == m_pD3DRender) return -1;


	HRESULT hr = m_pD3DRender->Initialize(m_hDisplayWindow);
	if (FAILED(hr))	return -1;

	m_pD3DRender->RegisterDeviceLostCallback(onDeviceLost, onDeviceReset, (void *)this);
		
	m_pD3DRender->CreateVideoSurface(m_nVWidth, m_nVHeight);

	m_pD3DRender->SetDisplayMode(m_mode);

	map<SHORT, COverlay*>::iterator iter = m_OverlayMap.begin();
	for(; iter != m_OverlayMap.end(); ++iter) {
		iter->second->Draw(m_pD3DRender, iter->first);
	}

	return 0;
}

int CVideoRender::Render(uint8_t * pY, UINT nYLineSize, uint8_t * pU, UINT nULineSize, uint8_t * pV, UINT nVLineSize)
{
	if (!m_IsD3dOpen) return -1;

	CAutoLock lock(&m_csForRender);
#if 0
	LARGE_INTEGER stfreq;
	QueryPerformanceFrequency(&stfreq);
	static double m_dfFreq = (double)stfreq.QuadPart;

	LARGE_INTEGER startTime;
	LARGE_INTEGER nowTime;
	LONGLONG nStartTime = 0;
	LONGLONG nNowTime = 0;

	QueryPerformanceCounter(&startTime);
	nStartTime = (LONGLONG)startTime.QuadPart;
#endif

	m_pD3DRender->Display(pY, nYLineSize, pV, nVLineSize, pU, nULineSize);
#if 0
	QueryPerformanceCounter(&nowTime);
	nNowTime = (LONGLONG)nowTime.QuadPart;

	double dfMinus = (double)(nNowTime - nStartTime);
	LONGLONG nMs = (uint64_t)((dfMinus / m_dfFreq) * 1000);
	fprintf(stderr, "diif = %lld\n", nMs);
#endif

	return 0;
}

int CVideoRender::CloseRender()
{
	if (m_bIsThread) {
		::SetEvent( m_hEvent[EVENT_STOP] );
		Join();
		for ( int i = 0; i < EVENT_CNT; ++i )
			::CloseHandle( m_hEvent[i] );
		FreeDeque();
	}
	
	if (m_pD3DRender) {
		if (m_IsD3dOpen)
			m_pD3DRender->Clear(D3DCOLOR_ARGB(0xFF, 0, 0, 0));
		delete m_pD3DRender;
	}
	m_pD3DRender = NULL;

	m_pD3DDeleteRender = NULL;
	
	/*
	if (m_bHasTimeEvent) {
		timeKillEvent(m_nTimerID);
		timeEndPeriod(m_nTimeAccuracy);
	}

	m_bHasTimeEvent = FALSE;
	*/

	m_bIsRendering = FALSE;
	if (m_hDisplayWindow)  {
		RECT rect = {0};
		::GetWindowRect(m_hDisplayWindow, &rect);
		int nWidth = rect.right - rect.left;
		int nHeight = rect.bottom - rect.top;

		rect.left = 0;
		rect.top = 0;
		rect.right = nWidth;
		rect.bottom = nHeight;

		::InvalidateRect(m_hDisplayWindow, &rect, TRUE);
	}

	return 0;
}

int CVideoRender::FreeData(PICYUVDATA * pstYuvData)
{
	if (pstYuvData == NULL) return 0;
	
	if (pstYuvData->pYData) delete[] pstYuvData->pYData;
	pstYuvData->pYData = NULL;

	if (pstYuvData->pUData) delete[] pstYuvData->pUData;
	pstYuvData->pUData = NULL;

	if (pstYuvData->pVData) delete[] pstYuvData->pVData;
	pstYuvData->pVData = NULL;
	
	return 0;
}

int CVideoRender::FreeDeque()
{
	CAutoLock lock(&m_cs);

	if (m_dePic.empty()) return 0;

	deque<PICYUVDATA *>::iterator iter = m_dePic.begin();
	for (; iter != m_dePic.end(); ++iter) {
		FreeData(*iter);
		delete *iter;
	}

	m_dePic.clear();
	return 0;
}

/*
int CVideoRender::InitTimer(UINT nDelay)
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

void CVideoRender::OnTimer(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CVideoRender * Render = (CVideoRender*)dwUser;
	Render->TimeRender();
}

int CVideoRender::TimeRender()
{
	PICYUVDATA * pYuvData = NULL;
	
	LOCAL_ALOCK_BEGIN(&m_cs)
	if (m_dePic.empty()) goto end;
	pYuvData = m_dePic.front();
	LOCAL_ALOCK_END;

	if ((m_sfPresentationTime.tv_sec == 0) && (m_sfPresentationTime.tv_usec == 0)) {
		m_sfPresentationTime = pYuvData->sPresentationTime;
		m_SyncClock->start(m_sfPresentationTime);
	}
	
	uint64_t nSyncMs = 0, nFrameMs = 0;
	int ret = 0;

	m_SyncClock->GetMsSinceStart(nSyncMs);
	ret = m_SyncClock->convertTime2MsSinceStart(pYuvData->sPresentationTime, nFrameMs);
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
	
	if (m_pD3DDeleteRender) {
		m_pD3DDeleteRender->ReleaseInstance();
		m_pD3DDeleteRender = NULL;
	}

	Render(pYuvData->pYData, pYuvData->nYlineSize, pYuvData->pUData, pYuvData->nUlineSize, 
			pYuvData->pVData, pYuvData->nVlineSize);

	++m_nDFreamCount;

end:
	LOCAL_ALOCK_BEGIN(&m_cs)
	if (!m_dePic.empty())
		m_dePic.pop_front();
	LOCAL_ALOCK_END;
	
	if (pYuvData) {
		FreeData(pYuvData);
		delete pYuvData;
	}

	InitTimer(m_nTimeAccuracy);
	return 0;
}
*/