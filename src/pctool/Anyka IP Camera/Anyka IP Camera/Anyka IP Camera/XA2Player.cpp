#include "StdAfx.h"
#include "XA2Player.h"
#include "AudioStream.h"

XA2Player::XA2Player(void)
	: m_soundVoice(NULL)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	HRESULT hr = XAudio2Create(&m_xaudio2, 0);
	if (FAILED(hr) || m_xaudio2 == NULL) { //无法创建XAudio2，有可能是PC上声卡或声卡驱动不支持该特性。
		m_ds = new CStreamAudio();//使用DirectSound进行音频播放。
		return;
	}

	m_ds = NULL;

	UINT32 deviceCount;
	hr = m_xaudio2->GetDeviceCount(&deviceCount);
 
	XAUDIO2_DEVICE_DETAILS deviceDetails;
	for (unsigned int i = 0; i < deviceCount; i++)
	{
		m_xaudio2->GetDeviceDetails(i, &deviceDetails);	
		DeviceDetails* dev = new DeviceDetails();
		wmemcpy(dev->DeviceID, deviceDetails.DeviceID, 256);
		wmemcpy(dev->DisplayName, deviceDetails.DisplayName, 256);
		dev->Index = i;
		m_devices.push_back(dev);
	}
}

void XA2Player::ReleaseInstance()
{
	delete this;
}

XA2Player::~XA2Player(void)
{
	if (m_ds) {
		delete ((CStreamAudio*)m_ds);
		return;
	}

	if(m_soundVoice)
	{
		m_soundVoice->Stop();
	}
	m_xaudio2.Release();
}

HRESULT XA2Player::Initialize(const WAVEFORMATEX* format, int index, bool isMusicPlayback)
{
	if (NULL == m_xaudio2 && m_ds == NULL) {
		AfxMessageBox(L"Cannot play the audio, this problem is due to DX version on your PC is too low, or the sound card driver does not support the new features of the DX!", 0, 0);
		return E_FAIL;
	}

	if (m_ds) { //使用的是DirectSound
		HWND hWnd = GetForegroundWindow();
		if (hWnd == NULL)
			hWnd = GetDesktopWindow();
		
		if (!hWnd) {
			AfxMessageBox(L"Cannot play the audio, can not obtain coordinate window to DirectSound!", 0, 0);
			return E_FAIL;
		}

		if (!((CStreamAudio*)m_ds)->Open(hWnd, (CAudioStreamHandler*)this, format)) {
			AfxMessageBox(L"Cannot play the audio，can not open DirectSound!", 0, 0);
			return E_FAIL;
		}

		if (m_ds && !(((CStreamAudio*)m_ds)->IsPlay())) {
			((CStreamAudio*)m_ds)->CtrlStream(TRUE);
		}

		fprintf(stderr, "WARN! XA2Player user direct sound to play the audio!\n");

		return S_OK;
	}

	//使用XAudio2
	memcpy(&m_format, format, sizeof(WAVEFORMATEX));
	UINT32 flags = isMusicPlayback == true ? XAUDIO2_VOICE_MUSIC : 0;
	HRESULT hr = m_xaudio2->CreateMasteringVoice(&m_masteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, index);
	hr = m_xaudio2->CreateSourceVoice(&m_soundVoice, format, flags, XAUDIO2_DEFAULT_FREQ_RATIO);
	if (FAILED(hr)) {
		AfxMessageBox( L"no support XAudio\n", 0, 0 );
		return E_FAIL;
	}
	return m_soundVoice->Start(0);
}


HRESULT XA2Player::QueueSamples(BYTE* buffer, UINT32 size)
{
	if(!buffer)
	{
		return E_POINTER;
	}
	
	CAutoLock * plock = NULL;
	if (m_ds)
		plock = new CAutoLock(&m_csForDs);

	XAUDIO2_BUFFER* buf = new XAUDIO2_BUFFER();
	ZeroMemory(buf, sizeof(XAUDIO2_BUFFER));

	buf->pAudioData = new BYTE[size];
	memcpy((void*)buf->pAudioData, buffer, size);
	buf->AudioBytes = size;
	m_playQueue.push_back(buf);

	if (m_ds && plock)
		delete plock;

	return S_OK;
}

HRESULT XA2Player::PlayQueuedSamples()
{
	if (m_ds) return S_OK;

	XAUDIO2_VOICE_STATE VoiceState;
    m_soundVoice->GetState(&VoiceState);

	while (m_releaseQueue.size() > VoiceState.BuffersQueued) 
	{
        XAUDIO2_BUFFER* tmp = m_releaseQueue.front();
        m_releaseQueue.pop_front();
        if(tmp != NULL)
		{
			delete tmp->pAudioData;
			delete tmp;  
		}
    }

	while (!m_playQueue.empty())
	{
		XAUDIO2_BUFFER* tmp = m_playQueue.front();
        m_releaseQueue.push_back(tmp);
        m_soundVoice->SubmitSourceBuffer(tmp);
        m_playQueue.pop_front();
		//VoiceState.BuffersQueued++;
    }
		
	return S_OK;
}

HRESULT XA2Player::PlaySamples(BYTE* buffer, UINT32 size)
{
	if (m_ds) {
		QueueSamples(buffer, size);
		return S_OK;
	}

	XAUDIO2_BUFFER tmp = {0};
	tmp.pAudioData = buffer;
	tmp.AudioBytes = size;

	return m_soundVoice->SubmitSourceBuffer(&tmp);
}

float XA2Player::GetVolume()
{
	if (m_ds)  return 0.0; // now directsound no support

	float vol;
	m_soundVoice->GetVolume(&vol);
	return vol;
}
	
HRESULT XA2Player::SetVolume(float volume)
{
	if (m_ds)  return S_OK; // now directsound no support

	return m_soundVoice->SetVolume(volume);
}

void XA2Player::GetChannelVolumes(int channels, float* volumes)
{
	if (m_ds) return;// now directsound no support

	m_soundVoice->GetChannelVolumes(channels, volumes);
}
	
HRESULT XA2Player::SetChannelVolumes(int channels, const float* volumes)
{
	if (m_ds) return S_OK;// now directsound no support
	return m_soundVoice->SetChannelVolumes(channels, volumes);
}

HRESULT XA2Player::FlushBuffers()
{
	if (m_ds) {
		CAutoLock lock(&m_csForDs);
		while (!m_playQueue.empty())
		{
			XAUDIO2_BUFFER* tmp = m_playQueue.front();
			m_playQueue.pop_front();
			if(tmp != NULL)
			{
				delete tmp->pAudioData;
				delete tmp;  
			}
		} 

		return S_OK;
	}

	if (!m_soundVoice) return S_OK;

	while (!m_releaseQueue.empty()) 
	{
        XAUDIO2_BUFFER* tmp = m_releaseQueue.front();
        m_releaseQueue.pop_front();
        if(tmp != NULL)
		{
			delete tmp->pAudioData;
			delete tmp;  
		}        
    }

	while (!m_playQueue.empty())
	{
		XAUDIO2_BUFFER* tmp = m_playQueue.front();
		m_playQueue.pop_front();
		if(tmp != NULL)
		{
			delete tmp->pAudioData;
			delete tmp;  
		}
    }   
	
	return m_soundVoice->FlushSourceBuffers();
}

HRESULT XA2Player::GetCurrentQueuedCnt(unsigned int & nCnt)
{
	if (m_ds) {
		nCnt = m_playQueue.size();
		return S_OK;
	}

	nCnt = 0;
	if(!m_soundVoice) return E_FAIL;
	
	XAUDIO2_VOICE_STATE VoiceState;
    m_soundVoice->GetState(&VoiceState);

	nCnt = VoiceState.BuffersQueued;

	return S_OK;
}

//使用m_playQueue中的数据填充DirectSound的播放buffer.
void XA2Player::AdoStreamData(unsigned char * pBuffer, DWORD & nBufferLen)
{
	CAutoLock lock(&m_csForDs);

	if (m_playQueue.empty()) {
		nBufferLen = 0;
		return;
	}

	if (pBuffer == NULL) { //just get current stream data len 
		DWORD nLen = 0;
		for (unsigned int i = 0; i < m_playQueue.size(); ++i) { 
			nLen += m_playQueue[i]->AudioBytes;
			if (nBufferLen < nLen) break;
		}
		
		nBufferLen = nLen;
		return;
	}

	int nLen = nBufferLen;
	while (!m_playQueue.empty() && nLen) {
		XAUDIO2_BUFFER* tmp = m_playQueue.front();

		if (nLen >= (DWORD)(tmp->AudioBytes - tmp->PlayBegin)) {
			memcpy(pBuffer, tmp->pAudioData + tmp->PlayBegin, tmp->AudioBytes - tmp->PlayBegin);
			pBuffer += tmp->AudioBytes - tmp->PlayBegin;
			nLen -= tmp->AudioBytes - tmp->PlayBegin;
			m_playQueue.pop_front();
			if(tmp != NULL) {
				delete tmp->pAudioData;
				delete tmp;  
			}  
		}else {
			memcpy(pBuffer, tmp->pAudioData + tmp->PlayBegin, nLen);
			tmp->PlayBegin += nLen;
			pBuffer += nLen;
			nLen = 0;
		}
	}
}
