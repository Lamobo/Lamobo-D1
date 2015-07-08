#pragma once
#include <mmsystem.h>
#include <dsound.h>
#include "IRenderable.h"

#define NUM_REC_NOTIFICATIONS  20

class CStreamAudio
{
protected:
	IDirectSound8 *   m_pDS;        // DirectSound component
	IDirectSoundBuffer8 * m_pDSBuf;   // Sound Buffer object
	IDirectSoundNotify8 * m_pDSNotify;  // Notification object
	WAVEFORMATEX   m_wfxOutput ; // Wave format of output 

	// some codes from capture audio 
	DSBPOSITIONNOTIFY     m_aPosNotify[NUM_REC_NOTIFICATIONS + 1]; //notify flag array 
	DWORD        m_dwPlayBufSize;  //play loop buffer size 
	DWORD        m_dwNextPlayOffset;//offset in loop buffer 
	DWORD        m_dwNotifySize;  //notify pos when loop buffer need to emit the event
	CAudioStreamHandler* m_stream_handler ; // caller stream buffer filler
	public:
	BOOL     m_bPlaying ;
	HANDLE     m_hNotifyEvent;   //notify event
	CWinThread * m_pThread;
	BOOL     LoadStreamData();

public:
	static UINT notify_stream_thd(LPVOID data) ; 

protected:
	HRESULT InitDirectSound(HWND hWnd, const WAVEFORMATEX * wfx); 
	HRESULT FreeDirectSound(); 
	IDirectSoundBuffer8 *CreateStreamBuffer(IDirectSound8* pDS, WAVEFORMATEX* wfx) ; 
	BOOL SetWavFormat(WAVEFORMATEX * wfx);

public:
	CStreamAudio(void);
	~CStreamAudio(void);
	BOOL Open(HWND hWnd, CAudioStreamHandler * stream_handler, const WAVEFORMATEX * wfx); 
	BOOL Close(); 
	BOOL CtrlStream(BOOL bPlaying); 
	BOOL IsPlay() { return m_bPlaying; };
};
