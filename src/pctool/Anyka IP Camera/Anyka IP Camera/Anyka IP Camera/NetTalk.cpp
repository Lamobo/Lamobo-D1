#include "StdAfx.h"
#include "NetTalk.h"

#define DEST_TALK_PORT	7710

CNetTalk::CNetTalk(void)
: m_pIServer(NULL), m_AudioEncode(NULL), m_bIsTalk(FALSE), m_bIsCreate(FALSE)
{
}

CNetTalk::~CNetTalk(void)
{
	StopTalk();
}

int CNetTalk::Talk(IServer * pIServer, TALKKICKOUTCB pTalkKickOutCB, void * pCBParam)
{
	if (NULL == pIServer) return -1;
	
	if (m_pIServer != NULL) m_pIServer->SetTalkKickOutCallBack(NULL, NULL);

	m_pIServer = pIServer;
	
	m_NetCtrl.CloseNetCtrl();

	char strIp[MAX_IP_LEN] = {0};
	unsigned int nLen = MAX_IP_LEN;

	m_pIServer->GetServerIp(strIp, &nLen);
	m_pIServer->SetTalkKickOutCallBack(pTalkKickOutCB, pCBParam);

	int ret = m_NetCtrl.OpenNetCtrl(DEST_TALK_PORT, 7080, strIp, NET_TYPE_TCP);
	if (ret < 0) {
		fprintf(stderr, "can't connect to server, server ip = %s, port = %d\n", strIp, DEST_TALK_PORT);
		return -1;
	}

	WAVEFORMATEX wfxOut = {0};

	wfxOut.cbSize = 0;
	wfxOut.nChannels = 1;
	wfxOut.nSamplesPerSec = 8000;
	wfxOut.wBitsPerSample = 16;
	wfxOut.wFormatTag = WAVE_FORMAT_PCM;
	wfxOut.nBlockAlign = wfxOut.nChannels * wfxOut.wBitsPerSample / 8;
	wfxOut.nAvgBytesPerSec = wfxOut.nSamplesPerSec * wfxOut.nBlockAlign;
	
	AUDIOPARAMETER stAudioParam = {0};
	stAudioParam.nEncodeType = AUDIO_EN_AAC;
	stAudioParam.nChannels = 1;
	stAudioParam.nSampleRateType = AUDIO_SAMPLERATE_8K;
	stAudioParam.nSampleFmt = AUDIO_SAMPLE_FMT_S16;

	ret = m_pIServer->SendTalk(stAudioParam); //let server know we want to talk to it.
	if (ret < 0) {
		fprintf(stderr, "send talk command to server error! can't do the talk job!\n");
		return -1;
	}

	if (!m_bIsCreate) {
		m_DSCapture.UseDefault();
		m_DSCapture.Create(wfxOut);
		m_DSCapture.SetEvent(this);
		m_bIsCreate = TRUE;
	}
	
	if (m_AudioEncode) delete m_AudioEncode;

	STREAMPARAMETER stStreamParm = {0};

	stStreamParm.nAidioFmt = wfxOut.wBitsPerSample;
	stStreamParm.nAudioChannels = wfxOut.nChannels;
	stStreamParm.nAudioSampleRate = wfxOut.nSamplesPerSec;
	stStreamParm.nBitsRate = 32000; //just use the encoder default bits rate;

	m_AudioEncode = CFfmpegEnvoy::createNew();
	m_AudioEncode->OpenFfmpeg(false, FUN_ENCODER);
	m_AudioEncode->SetEncodeCodecID(AV_CODEC_ID_AAC);
	m_AudioEncode->setStreamParameter(stStreamParm);
	m_AudioEncode->RegisterSink((IAudioSink *)this, SINK_AUDIO);
	
	m_AudioEncode->Start();
	m_DSCapture.Start();

	m_bIsTalk = TRUE;

	return 0;
}

int CNetTalk::StopTalk(BOOL bSendInform)
{
	AUDIOPARAMETER stAudioParam = {0};
	if (!m_pIServer) return 0;
	
	if (bSendInform)
		m_pIServer->SendTalk(stAudioParam); // tell the server finish the talk job

	m_DSCapture.Stop();
	
	if (m_AudioEncode) delete m_AudioEncode;
	m_AudioEncode = NULL;

	m_NetCtrl.CloseNetCtrl();

	m_bIsTalk = FALSE;
	m_pIServer = NULL;

	return 0;
}

int CNetTalk::GetSendAudioSocketIp(unsigned long & ulIpAddr, unsigned short & usPort)
{
	return m_NetCtrl.GetHostSendIpAddr(ulIpAddr, usPort);
}

BOOL CNetTalk::IsTalk()
{
	return m_bIsTalk;
}

IServer * CNetTalk::GetTalkServer()
{
	return m_pIServer;
}

long CNetTalk::OnDSCapturedData(unsigned char* data, unsigned long size, unsigned long samplerate)
{
	if (m_AudioEncode){
		timeval ptime = {0};
		m_AudioEncode->SendData(data, size, ptime, "audio", NULL, NULL);
	}

	return 0;
}

int CNetTalk::SendAudio(PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime, 
						unsigned int nAudioChannels, unsigned int nAudioSampleRate, unsigned int nAudioFmt)
{
	return m_NetCtrl.SendAudioData(pData, nDataLen);
}