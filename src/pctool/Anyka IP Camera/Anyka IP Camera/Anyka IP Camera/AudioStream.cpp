#include "StdAfx.h"
#include "AudioStream.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif
#ifndef MAX
#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )
#endif

CStreamAudio::CStreamAudio(void)
{
	m_pDS = NULL ;        // DirectSound component
	m_pDSBuf = NULL ;   // Sound Buffer object
	m_pDSNotify = NULL ;  // Notification object
	m_hNotifyEvent = NULL ; 
	ZeroMemory(&m_wfxOutput, sizeof(m_wfxOutput)) ; // Wave format of output 
	m_wfxOutput.wFormatTag = WAVE_FORMAT_PCM ; 
	m_dwPlayBufSize = 0;  //play loop buffer size 
	m_dwNextPlayOffset = 0; //offset in loop buffer 
	m_dwNotifySize = 0;  //notify pos when loop buffer need to emit the event
	m_bPlaying = FALSE; 
	m_pThread = NULL;
}

CStreamAudio::~CStreamAudio(void)
{
	FreeDirectSound() ; 
	CoUninitialize() ; 
}

HRESULT CStreamAudio::InitDirectSound(HWND hWnd, const WAVEFORMATEX * wfx)
{
	if(FAILED(DirectSoundCreate8(NULL, &m_pDS, NULL))) {
		//MessageBox(NULL, L"Unable to create DirectSound object", "Error", MB_OK);
		return S_FALSE;
	}

	// sets the cooperative level of the application for this sound device
	m_pDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);

	// use preset output wave format
	SetWavFormat(&m_wfxOutput);

	if (!wfx) {
		memcpy(&m_wfxOutput, wfx, sizeof(WAVEFORMATEX));
	}

	return S_OK ; 
}

HRESULT CStreamAudio::FreeDirectSound()
{
 // make sure the thread gone 
	m_bPlaying = FALSE; 
	if (m_pThread)
		::WaitForSingleObject(m_pThread->m_hThread, 500);
	// stop sound play 
	if(m_pDSBuf) m_pDSBuf->Stop();

	// Release the notify event handles
	if(m_hNotifyEvent) {
		CloseHandle(m_hNotifyEvent) ; 
		m_hNotifyEvent = NULL ; 
	}
	// Release DirectSound objects
	SAFE_RELEASE(m_pDSBuf) ; 
	SAFE_RELEASE(m_pDS) ; 
	return S_OK ; 
}

BOOL CStreamAudio::Open(HWND hWnd, CAudioStreamHandler * stream_handler, const WAVEFORMATEX * wfx)
{
	HRESULT hr ; 
	m_stream_handler = stream_handler ; 
	hr = InitDirectSound(hWnd, wfx) ; 
	return (FAILED(hr)) ? FALSE : TRUE ; 
}

BOOL CStreamAudio::Close()
{
	HRESULT hr ; 
	hr = FreeDirectSound() ; 
	return (FAILED(hr)) ? FALSE : TRUE ; 
}

UINT CStreamAudio::notify_stream_thd(LPVOID data) 
{
	CStreamAudio * psmado = static_cast<CStreamAudio *>(data) ; 
	DWORD dwResult = 0 ; 
	DWORD Num = 0 ;

	while(psmado->m_bPlaying) {
		// Wait for a message
		dwResult = MsgWaitForMultipleObjects(1, &psmado->m_hNotifyEvent, 
				FALSE, INFINITE, QS_ALLEVENTS);
		// Get notification
		switch(dwResult) {
		case WAIT_OBJECT_0:
			psmado->LoadStreamData();
			break ; 
		default:
			break ; 
		}
	}

	AfxEndThread(0, TRUE) ; 
	return 0 ; 
}

IDirectSoundBuffer8 * CStreamAudio::CreateStreamBuffer(IDirectSound8* pDS, WAVEFORMATEX * wfx)
{
	IDirectSoundBuffer *  pDSB = NULL ;
	IDirectSoundBuffer8 * pDSBuffer = NULL ;
	DSBUFFERDESC dsbd;
	// calculate play buffer size 
	// Set the notification size
	//m_dwNotifySize = MAX( 1024, wfx->nAvgBytesPerSec / 8 );
	m_dwNotifySize = wfx->nAvgBytesPerSec / NUM_REC_NOTIFICATIONS; 
	m_dwNotifySize -= (m_dwNotifySize % wfx->nBlockAlign); 

	// Set the buffer sizes 
	m_dwPlayBufSize = m_dwNotifySize * NUM_REC_NOTIFICATIONS;

	// create the sound buffer using the header data
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);

	// set DSBCAPS_GLOBALFOCUS to make sure event if the software lose focus could still
	// play sound as well
	dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;
	dsbd.dwBufferBytes = m_dwPlayBufSize ;
	dsbd.lpwfxFormat = wfx ;

	if(FAILED(pDS->CreateSoundBuffer(&dsbd, &pDSB, NULL))) return NULL;

	// get newer interface
	if(FAILED(pDSB->QueryInterface(IID_IDirectSoundBuffer8, (void**)(&pDSBuffer)))) {
		SAFE_RELEASE(pDSB) ; 
		return NULL;
	}

	// return the interface
	return pDSBuffer;
}

BOOL CStreamAudio::LoadStreamData() 
{
	///////////////////////
	HRESULT hr;
	VOID*   pvStreamData1    = NULL;
	DWORD   dwStreamLength1 = 0 ;
	VOID*   pvStreamData2   = NULL;
	DWORD   dwStreamLength2 = 0 ;
	DWORD   dwWritePos = 0 ;
	DWORD   dwPlayPos = 0 ;
	LONG lLockSize = 0 ;
	BOOL bMute = FALSE;

	if(FAILED(hr = m_pDSBuf->GetCurrentPosition(&dwPlayPos, &dwWritePos)))
		return S_FALSE; 
	
	/*lLockSize = dwWritePos - m_dwNextPlayOffset;

	if(lLockSize < 0)	lLockSize += m_dwPlayBufSize;

	// Block align lock size so that we are always write on a boundary
	lLockSize -= (lLockSize % m_dwNotifySize);

	if(lLockSize == 0) return S_FALSE;
	
	DWORD tmpLen = lLockSize;
	// test if can read the tmplen data from stream
	if(m_stream_handler) m_stream_handler->AdoStreamData(NULL, tmpLen);
	if (tmpLen < lLockSize) return FALSE;*/
	
	
	lLockSize = m_dwNotifySize;
	DWORD tmpLen = lLockSize;
	if(m_stream_handler) m_stream_handler->AdoStreamData(NULL, tmpLen);
	if ((LONG)tmpLen < lLockSize) bMute = TRUE;

	// lock the sound buffer at position specified
	if(FAILED(m_pDSBuf->Lock(dwWritePos, lLockSize,
							&pvStreamData1, &dwStreamLength1, 
							&pvStreamData2, &dwStreamLength2, 0L))) {
		return FALSE;
	}

	memset(pvStreamData1, 0, dwStreamLength1);
	// read in the data
	if(!bMute && m_stream_handler) m_stream_handler->AdoStreamData((BYTE *)pvStreamData1, dwStreamLength1);

	// Move the capture offset along
	m_dwNextPlayOffset += dwStreamLength1; 
	m_dwNextPlayOffset %= m_dwPlayBufSize; // Circular buffer
	if(pvStreamData2 != NULL) {
		memset(pvStreamData2, 0, dwStreamLength2);
		if(!bMute && m_stream_handler) m_stream_handler->AdoStreamData((BYTE *)pvStreamData2, dwStreamLength2) ; 
		
		// Move the capture offset along
		m_dwNextPlayOffset += dwStreamLength2; 
		m_dwNextPlayOffset %= m_dwPlayBufSize; // Circular buffer
	}
	// unlock it
	m_pDSBuf->Unlock(pvStreamData1, dwStreamLength1, pvStreamData2, dwStreamLength2) ;
	// return a success
	return TRUE;
}

BOOL CStreamAudio::CtrlStream(BOOL bPlaying)
{
	HRESULT hr ; 
	int i;
	m_bPlaying = bPlaying ; 
	if(m_bPlaying) {
		// Create a 2 second buffer to stream in wave
		m_pDSBuf = CreateStreamBuffer(m_pDS, &m_wfxOutput) ; 

		if(m_pDSBuf == NULL) return FALSE ; 

		// Create the notification interface
		if(FAILED(m_pDSBuf->QueryInterface(IID_IDirectSoundNotify8, (void**)(&m_pDSNotify)))) 
			return FALSE ;

		// create auto notify event 
		m_hNotifyEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		// Setup the notification positions
		for( i = 0; i < NUM_REC_NOTIFICATIONS; i++ ) {
			m_aPosNotify[i].dwOffset = (m_dwNotifySize * i) + m_dwNotifySize - 1;
			m_aPosNotify[i].hEventNotify = m_hNotifyEvent;             
		}

		// Tell DirectSound when to notify us. the notification will come in the from 
		// of signaled events that are handled in WinMain()
		if( FAILED( hr = m_pDSNotify->SetNotificationPositions(NUM_REC_NOTIFICATIONS, m_aPosNotify)))
			return S_FALSE ;

		m_dwNextPlayOffset = 0 ; 

		// Fill buffer with some sound
		LoadStreamData() ; 
		// Play sound looping
		m_pDSBuf->SetCurrentPosition(0);
		m_pDSBuf->SetVolume(DSBVOLUME_MAX);
		m_pDSBuf->Play(0,0,DSBPLAY_LOOPING);
		// create notify event recv thread 
		m_pThread = AfxBeginThread(CStreamAudio::notify_stream_thd, (LPVOID)(this)) ;
	} else {
		// stop play 
		// make sure the thread gone 
		if (m_pThread)
			::WaitForSingleObject(m_pThread->m_hThread, 500);

		// stop sound play 
		if(m_pDSBuf) m_pDSBuf->Stop();
		// Release the notify event handles
		if(m_hNotifyEvent) {
			CloseHandle(m_hNotifyEvent) ; 
			m_hNotifyEvent = NULL ; 
		}
		// Release DirectSound objects
		SAFE_RELEASE(m_pDSBuf) ;  
	}
	return TRUE ; 
}

BOOL CStreamAudio::SetWavFormat(WAVEFORMATEX * wfx)
{
	// get the default capture wave formate 
	ZeroMemory(wfx, sizeof(WAVEFORMATEX)) ; 
	wfx->wFormatTag = WAVE_FORMAT_PCM;
	// 8KHz, 16 bits PCM, Mono
	wfx->nSamplesPerSec = 8000 ; 
	wfx->wBitsPerSample = 16 ; 
	wfx->nChannels  = 1 ;
	wfx->nBlockAlign = wfx->nChannels * ( wfx->wBitsPerSample / 8 ) ; 
	wfx->nAvgBytesPerSec = wfx->nBlockAlign * wfx->nSamplesPerSec;
	return TRUE ; 
}