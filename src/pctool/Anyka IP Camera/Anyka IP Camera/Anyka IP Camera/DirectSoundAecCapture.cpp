#include "stdafx.h"
#include "DirectSoundAecCapture.h"

BOOL CALLBACK DSoundEnum(LPGUID lpGUID,
                         LPCSTR lpszDesc,
                         LPCSTR lpszDrvName,
                         LPVOID lpContext)
{
    CDirectSoundAecCapture* pThis = (CDirectSoundAecCapture*)lpContext;
    DSoundDeviceInfo info;
    info.name = lpszDrvName;
    info.desc = lpszDesc;

    if (lpGUID)
        info.guid = *lpGUID;
    else
        info.guid = GUID_NULL;
 
    pThis->m_allRenderDevices.push_back(info);
    return TRUE;
}

BOOL CALLBACK DSoundCapEnum(LPGUID lpGUID,
                            LPCSTR lpszDesc,
                            LPCSTR lpszDrvName,
                            LPVOID lpContext)
{
    CDirectSoundAecCapture* pThis = (CDirectSoundAecCapture*)lpContext;
    DSoundDeviceInfo info;
    info.name = lpszDrvName;
    info.desc = lpszDesc;

    if (lpGUID)
        info.guid = *lpGUID;
    else
        info.guid = GUID_NULL;

    pThis->m_allCaptureDevices.push_back(info);
    return TRUE;
}

CDirectSoundAecCapture::CDirectSoundAecCapture(void)
{
	m_pDSFD = NULL;
	m_pDSCBuffer8 = NULL;
	m_pDSBuffer8 = NULL;

    m_captureIndex = -1;
    m_renderIndex = -1;
	m_samplerate = 0;

	m_NotifySize = 0;
	m_captureBufferSize = 0;
	m_pCaptureDataTemp = NULL;
	m_pEvent = NULL;
	m_runThreadFlag = false;
	m_bUseDefault = FALSE;
	
    EnumDevices();
}

CDirectSoundAecCapture::~CDirectSoundAecCapture(void)
{
	if(m_pEvent)
	{
		CloseHandle(m_pEvent);
		m_pEvent = NULL;
	}

	if(m_pCaptureDataTemp)
	{
		free(m_pCaptureDataTemp);
		m_pCaptureDataTemp = NULL;
	}

	if (m_pDSCBuffer8) m_pDSCBuffer8->Release();
	if (m_pDSBuffer8) m_pDSBuffer8->Release();
	if (m_pDSFD) m_pDSFD->Release();
}

HRESULT CDirectSoundAecCapture::EnumDevices()
{
    if (FAILED(DirectSoundEnumerateA(DSoundEnum, (LPVOID)this)))
    {
        return E_FAIL;
    }

    if (FAILED(DirectSoundCaptureEnumerateA(DSoundCapEnum, (LPVOID)this)))
    {
        return E_FAIL;
    }

    return S_OK;
}

long CDirectSoundAecCapture::GetCaptureDeviceCount()
{
    return m_allCaptureDevices.size();
}

long CDirectSoundAecCapture::GetCaptureDeviceName(long index,char* device)
{
    if ((size_t)index >= m_allCaptureDevices.size())
    {
        return -1;
    }
    strcpy(device, m_allCaptureDevices[index].desc.c_str());
    return 0;
}

long CDirectSoundAecCapture::SetCaptureDeviceByName(char* device)
{
    long index = 0;
    long count = m_allCaptureDevices.size();
    while (index < count)
    {
        if (m_allCaptureDevices[index].desc == device)
        {
            break;
        }
        index++;
    }
    if (index < count)
    {
        m_captureIndex = index;
    }
    return 0;
}

long CDirectSoundAecCapture::SetCaptureDeviceByID(long device)
{
    if ((size_t)device >= m_allCaptureDevices.size())
    {
        return -1;
    }
    m_captureIndex = device;
    return 0;
}

long CDirectSoundAecCapture::GetRenderDeviceCount()
{
    return m_allRenderDevices.size();
}

long CDirectSoundAecCapture::GetRenderDeviceName(long index,char* device)
{
    if ((size_t)index >= m_allRenderDevices.size())
    {
        return -1;
    }
    strcpy(device,m_allRenderDevices[index].desc.c_str());
    return 0;
}

long CDirectSoundAecCapture::SetRenderDeviceByName(char* device)
{
    long index = 0;
    long count = m_allRenderDevices.size();
    while (index < count)
    {
        if (m_allRenderDevices[index].desc == device)
        {
            break;
        }
        index++;
    }
    if (index < count)
    {
        m_renderIndex = index;
    }
    return 0;
}

long CDirectSoundAecCapture::SetRenderDeviceByID(long device)
{
    if ((size_t)device >= m_allRenderDevices.size())
    {
        return -1;
    }
    m_captureIndex = device;
    return 0;
}

long CDirectSoundAecCapture::UseDefault(BOOL bDefault)
{
	m_bUseDefault = bDefault;
	return 0;
}

HRESULT CDirectSoundAecCapture::Create(WAVEFORMATEX & wfxOut)
{
    const GUID * pguidCapture = &(GUID_NULL);
    const GUID * pguidRender = &(GUID_NULL);
	
	if (m_bUseDefault) {
		pguidCapture = NULL;
		pguidRender = NULL;
	}
	else {
		if (m_captureIndex > 0)
			pguidCapture = &(m_allCaptureDevices[m_captureIndex].guid);
		if (m_renderIndex > 0)
			pguidRender = &(m_allRenderDevices[m_renderIndex].guid);
	}

	m_samplerate = wfxOut.nSamplesPerSec;
	m_captureBufferSize = wfxOut.nAvgBytesPerSec; //one sec buffer size

	if (m_pCaptureDataTemp) delete[] m_pCaptureDataTemp;

	m_pCaptureDataTemp = new unsigned char[m_captureBufferSize];
	if (m_pCaptureDataTemp == NULL) {
		fprintf(stderr, "%s::%s::%s::out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	m_NotifySize = (wfxOut.nAvgBytesPerSec / 1000) * START_THRESHOLD;

    DSCEFFECTDESC dsced[2];
    ZeroMemory( &dsced[0], sizeof( DSCEFFECTDESC )*2 );
    dsced[0].dwSize				= sizeof(DSCEFFECTDESC);
    dsced[0].dwFlags            = DSCFX_LOCSOFTWARE;
    dsced[0].guidDSCFXClass     = GUID_DSCFX_CLASS_AEC;
    dsced[0].guidDSCFXInstance  = GUID_DSCFX_MS_AEC;
    dsced[0].dwReserved1        = 0;
    dsced[0].dwReserved2        = 0;
    dsced[1].dwSize             = sizeof(DSCEFFECTDESC);
    dsced[1].dwFlags            = DSCFX_LOCSOFTWARE;
    dsced[1].guidDSCFXClass     = GUID_DSCFX_CLASS_NS;
    dsced[1].guidDSCFXInstance  = GUID_DSCFX_MS_NS;
    dsced[1].dwReserved1        = 0;
    dsced[1].dwReserved2        = 0;

	DSCBUFFERDESC dscbd;
	ZeroMemory(&dscbd,sizeof(DSCBUFFERDESC));
	dscbd.dwSize				= sizeof(DSCBUFFERDESC);
	dscbd.dwFlags				= DSCBCAPS_CTRLFX;
	dscbd.dwBufferBytes			= m_captureBufferSize;
	dscbd.dwReserved            = 0;
	dscbd.lpwfxFormat			= &wfxOut;
	dscbd.dwFXCount				= 2;
	dscbd.lpDSCFXDesc			= dsced;

	DSBUFFERDESC dsobd;
	ZeroMemory(&dsobd,sizeof(DSBUFFERDESC));
	dsobd.dwSize				= sizeof(DSBUFFERDESC);
	dsobd.dwFlags				= DSBCAPS_CTRLFX;
	dsobd.dwBufferBytes			= m_captureBufferSize;
	dsobd.lpwfxFormat			= &wfxOut;
	dsobd.dwReserved			= 0;
	dsobd.dwFlags 				= DSBCAPS_GETCURRENTPOSITION2 
								| DSBCAPS_GLOBALFOCUS
								| DSBCAPS_CTRLPOSITIONNOTIFY 
								| DSBCAPS_LOCSOFTWARE 
								| DSBCAPS_CTRLFREQUENCY;


	HWND hWnd = GetForegroundWindow();
	if (hWnd == NULL)
		hWnd = GetDesktopWindow();

	HRESULT hr = DirectSoundFullDuplexCreate8(pguidCapture, pguidRender, &dscbd, &dsobd, hWnd,
		DSSCL_PRIORITY, &m_pDSFD, &m_pDSCBuffer8, &m_pDSBuffer8, NULL);
	if (FAILED(hr))
	{
		dscbd.dwFlags				= 0;
		dscbd.dwFXCount				= 0;
		dscbd.lpDSCFXDesc			= NULL;
		hr = DirectSoundFullDuplexCreate8(pguidCapture, pguidRender, &dscbd, &dsobd, hWnd,
			DSSCL_PRIORITY, &m_pDSFD, &m_pDSCBuffer8, &m_pDSBuffer8, NULL);
		if (FAILED(hr))
		{
			dsobd.dwFlags &= ~DSBCAPS_CTRLFREQUENCY;
			hr = DirectSoundFullDuplexCreate8(pguidCapture, pguidRender, &dscbd, &dsobd, hWnd,
				DSSCL_PRIORITY, &m_pDSFD, &m_pDSCBuffer8, &m_pDSBuffer8, NULL);
		}
	}
	if (FAILED(hr))
	{
		return -1;
	}

	CComPtr<IDirectSoundCaptureFXAec8> pAec = NULL;
	hr = m_pDSCBuffer8->GetObjectInPath(GUID_DSCFX_CLASS_AEC, 0, IID_IDirectSoundCaptureFXAec8, (LPVOID*)&pAec);
	if(pAec)
	{
		DSCFXAec aecMode = {TRUE,TRUE,DSCFX_AEC_MODE_FULL_DUPLEX };
		hr = pAec->SetAllParameters(&aecMode);
	}
	
	m_pEvent = CreateEvent( NULL, FALSE, FALSE,NULL );
	CComPtr<IDirectSoundNotify8> pNotify;
	hr = m_pDSCBuffer8->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&pNotify);

	for (long index = 0; index < NOTIFY_CNT; index++)
	{
		m_dsPosNotify[index].dwOffset = index * m_NotifySize + m_NotifySize - 1;
		m_dsPosNotify[index].hEventNotify = m_pEvent;
	}

	hr = pNotify->SetNotificationPositions(NOTIFY_CNT, m_dsPosNotify);
	m_dwNextCapOffset = 0;
    return hr;
}

long CDirectSoundAecCapture::Start(void)
{
	if (m_runThreadFlag) return 0;

	//m_dwNextCapOffset = 0;
	m_runThreadFlag = true;
	
	CThreadBase::Start();
	return 0;
}

long CDirectSoundAecCapture::Stop(void)
{
	if (!m_runThreadFlag) return 0;
	
	m_runThreadFlag = false;
	CThreadBase::Join();
	//m_dwNextCapOffset = 0;
	m_pDSCBuffer8->Stop();

	return 0;
}

long CDirectSoundAecCapture::SetEvent(IAudioCaptureEvent *receiver)
{
	_event = receiver;
	return 0;
}

void CDirectSoundAecCapture::Run()
{
	if( NULL == m_pDSCBuffer8 ) return;

	Sleep(700);
	m_pDSCBuffer8->Start(DSCBSTART_LOOPING);

	HRESULT hr;
	VOID*   pbCaptureData    = NULL;
	DWORD   dwCaptureLength;
	VOID*   pbCaptureData2   = NULL;
	DWORD   dwCaptureLength2;
	DWORD   dwReadPos;
	DWORD   dwCapturePos;
	unsigned long capFrameSize = 0;
	while(m_runThreadFlag)
	{
		WaitForSingleObject(m_pEvent, INFINITE);
		if(!m_runThreadFlag)
		{
			break;
		}

		capFrameSize = 0;
		pbCaptureData    = NULL;
		pbCaptureData2   = NULL;
		
		LONG lLockSize; 
		

		if(FAILED(hr = m_pDSCBuffer8->GetCurrentPosition(&dwCapturePos, &dwReadPos)))
			continue; 
		lLockSize = dwReadPos - m_dwNextCapOffset;
		if( lLockSize < 0 )
			lLockSize += m_captureBufferSize; 

		lLockSize -= (lLockSize % m_NotifySize); 
		if( lLockSize == 0 )
			continue; 

		if( FAILED( hr = m_pDSCBuffer8->Lock(m_dwNextCapOffset, lLockSize,
			&pbCaptureData, &dwCaptureLength, 
			&pbCaptureData2, &dwCaptureLength2, 0L)))
			continue ; 

		if(_event) {
			memcpy(m_pCaptureDataTemp, pbCaptureData, dwCaptureLength); 
			capFrameSize = dwCaptureLength;
		}

		m_dwNextCapOffset += dwCaptureLength; 
		m_dwNextCapOffset %= m_captureBufferSize;
		if( pbCaptureData2 != NULL ) {
			if(_event) {
				memcpy(m_pCaptureDataTemp + capFrameSize, pbCaptureData2, dwCaptureLength2); 
				capFrameSize += dwCaptureLength2;
			} 
			m_dwNextCapOffset += dwCaptureLength2; 
			m_dwNextCapOffset %= m_captureBufferSize; 
		} 
		m_pDSCBuffer8->Unlock(pbCaptureData, dwCaptureLength, pbCaptureData2, dwCaptureLength2); 
		if(_event)
		{
			_event->OnDSCapturedData(m_pCaptureDataTemp, capFrameSize, m_samplerate);
		}
	}
}
