#pragma once

#include "InterfaceClass.h"
#include "DirectSoundAecCapture.h"
#include "NetCtrl.h"
#include "ServerSearch.h"
#include "FfmpegEnvoy.h"

class CNetTalk : public IAudioCaptureEvent, public IAudioSink
{
public:
	CNetTalk(void);
	virtual ~CNetTalk(void);
	
	int Talk(IServer * pIServer, TALKKICKOUTCB pTalkKickOutCB = NULL, void * pCBParam = NULL);

	int StopTalk(BOOL bSendInform = TRUE);

	BOOL IsTalk();

	IServer * GetTalkServer();

	int GetSendAudioSocketIp(unsigned long & ulIpAddr, unsigned short & usPort);

	virtual long OnDSCapturedData(unsigned char* data, unsigned long size, unsigned long samplerate);

	virtual int SendAudio(PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime, 
				  unsigned int nAudioChannels, unsigned int nAudioSampleRate, unsigned int nAudioFmt);
private:
	IServer *				m_pIServer;
	CNetCtrl				m_NetCtrl;
	CDirectSoundAecCapture	m_DSCapture;
	CFfmpegEnvoy *			m_AudioEncode;
	BOOL					m_bIsTalk;
	BOOL					m_bIsCreate;
};
