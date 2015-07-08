#pragma once

#include "ThreadBase.h"
#include "InterfaceClass.h"
#include <MMReg.h>
#include <InitGuid.h>
#include <dsound.h>

#include <vector>
#include <string>
using namespace std;

#define START_THRESHOLD		10
#define NOTIFY_CNT			1000 / START_THRESHOLD

struct DSoundDeviceInfo
{
	string name;
	GUID   guid;
	string desc;
};

class CDirectSoundAecCapture : CThreadBase
{
public:
	CDirectSoundAecCapture(void);
	~CDirectSoundAecCapture(void);

	long GetCaptureDeviceCount();
	long GetCaptureDeviceName(long index,char* device);
	long SetCaptureDeviceByName(char* device);
	long SetCaptureDeviceByID(long device);

	long GetRenderDeviceCount();
	long GetRenderDeviceName(long index,char* device);
	long SetRenderDeviceByName(char* device);
	long SetRenderDeviceByID(long device);
	
	long UseDefault(BOOL bDefault = TRUE);

	HRESULT Create(WAVEFORMATEX & wfxOut);

	long GetCaptureData(unsigned char* data,/*in out*/unsigned long* size);

	long SetEvent(IAudioCaptureEvent *receiver);
	long Start(void);
	long Stop(void);

private:
	HRESULT EnumDevices();

protected:
	void						Run();

public:
	vector<DSoundDeviceInfo>	m_allRenderDevices;
	vector<DSoundDeviceInfo>	m_allCaptureDevices;

private:
	long						m_captureIndex;
	long						m_renderIndex;
	long						m_samplerate;

	LPDIRECTSOUNDFULLDUPLEX		m_pDSFD;
	LPDIRECTSOUNDCAPTUREBUFFER8 m_pDSCBuffer8;
	LPDIRECTSOUNDBUFFER8		m_pDSBuffer8;

	HANDLE						m_pCapThread;
	HANDLE						m_pEvent;
	long						m_NotifySize;
	long						m_captureBufferSize;
	unsigned char*				m_pCaptureDataTemp;
	DSBPOSITIONNOTIFY			m_dsPosNotify[NOTIFY_CNT+1];

	
	IAudioCaptureEvent			*_event;
	bool						m_runThreadFlag;
	DWORD						m_dwNextCapOffset;
	BOOL						m_bUseDefault;
};
