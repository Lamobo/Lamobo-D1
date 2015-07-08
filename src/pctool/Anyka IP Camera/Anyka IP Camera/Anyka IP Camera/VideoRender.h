#pragma once
#include <deque>
#include <map>
#include "InterfaceClass.h"
#include "D3D9RenderImpl.h"
#include "Clock.h"
#include "ThreadBase.h"
#include "AudioRender.h"
using namespace std;

class COverlay;

typedef struct PicYUVData_st
{
	uint8_t *		pYData;
	unsigned int	nYlineSize;

	uint8_t *		pUData;
	unsigned int	nUlineSize;

	uint8_t *		pVData;
	unsigned int	nVlineSize;

	struct timeval	sPresentationTime;
}PICYUVDATA;

class CVideoRender : public IVideoSink, protected CThreadBase
{
public:
	static CVideoRender * createNew();
	virtual ~CVideoRender(void);
	
	int OpenRender(HWND hDisplayWindow);

	int SetServerStreamFps(int nServerStreamFps);

	int Reset();

	int CloseRender();
	
	BOOL IsRendering();

	int DrawLine(SHORT key, POINT p1, POINT p2, FLOAT width = 2, D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255), BYTE opacity = 255);

	int DrawRectangle(SHORT key, RECT rectangle, FLOAT width, D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255), BYTE opacity = 255);

	virtual int SendVideo(PBYTE pY, unsigned int nYlineSize, PBYTE pU, unsigned int nUlineSize, 
					   PBYTE pV, unsigned int nVlineSize, struct timeval sPresentationTime, 
					   unsigned int nVideoHeight, unsigned int nVideoWidth);

	int setClock(CClock * pClock);

	int SetAudioRender(CAudioRender * pAudioRender);

	int getFpsAverage(float & fFps);

	UINT getFpsOneSec();

	int SetFillMode(FillModeMe mode);
	
	//FullScreen must call by a UI thread, because it will create a full screen window. if it do not call by a
	//UI thread the full screen window will unable to handle any message, it will blocked and can't restorable.
	int FullScreen(BOOL bIsFullScreen, FULLSCREENMESSAGE pMessageCallBack, void * pClassParam);

protected:
	CVideoRender(void);
	
	/*
	static void PASCAL OnTimer(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);
	int InitTimer(UINT nDelay);
	int TimeRender();
	*/

	virtual void Run();

	static void PASCAL onDeviceLost(void * pParam);

	static void PASCAL onDeviceReset(void * pParam);
	
	int ThreadRender();
	int Render(uint8_t * pY, UINT nYLineSize, uint8_t * pU, UINT nULineSize, uint8_t * pV, UINT nVLineSize);
	
	int FreeData(PICYUVDATA * pstYuvData);
	int FreeDeque();

	void CalcOneSecFps();

	void AddOverlay(SHORT key, COverlay * pOverlay);

	int ReOpenD3d();

private:
	D3D9RenderImpl *		m_pD3DRender, *	m_pD3DDeleteRender;
	deque<PICYUVDATA *>		m_dePic;
	CAudioRender *			m_pAudioRender;

	HANDLE					m_hEvent[2];

	/*
	UINT					m_nTimerID;
	UINT					m_nTimeAccuracy;
	BOOL					m_bHasTimeEvent;
	*/
	
	CriticalSection			m_cs, m_csForRender, m_csForARender;

	BOOL					m_bIsRendering,	m_bIsLate, m_bNeedJudge, m_bIsThread, m_IsD3dOpen;

	CClock					m_TimeClock, m_RenderClock, * m_SyncClock;
	struct timeval			m_sfPresentationTime;
	LONGLONG				m_nDFreamCount;

	HWND					m_hDisplayWindow;
	UINT					m_nVHeight, m_nVWidth, m_nFpsOneSec, m_nRecodeFrames, 
							m_nLateCnt, m_nLateRender, m_nServerStreamFps, 
							m_ThrowFrameRate, m_ShowFrameRate;

	uint64_t				m_nLastMsSinceStart, m_nLastRenderTime;
	
	FillModeMe				m_mode;

	map<SHORT, COverlay*>	m_OverlayMap;
};
