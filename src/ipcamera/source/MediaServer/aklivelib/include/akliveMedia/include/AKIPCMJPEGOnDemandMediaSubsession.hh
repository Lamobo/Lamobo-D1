#ifndef _AKIPC_MJPEG_ONDEMAND_MEDIA_SUBSESSION_
#define _AKIPC_MJPEG_ONDEMAND_MEDIA_SUBSESSION_
#include <liveMedia.hh>
#include "AKIPCMJPEGFramedSource.hh"


class AKIPCMJPEGOnDemandMediaSubsession: public OnDemandServerMediaSubsession
{
private:
	FramedSource* mp_source;
	char* mp_sdp_line;
	RTPSink* mp_dummy_rtpsink;
	char m_done;
	int m_nWidth;
	int m_nHeight;
public:

	GetFrameFunc getframefunc;	
	SetLed setledstart;
	SetLed setledexit;
	int findex;

	static AKIPCMJPEGOnDemandMediaSubsession* createNew(UsageEnvironment& env, FramedSource* source, int nWidth, int nHeight, int vsIndex)
	{
		return new AKIPCMJPEGOnDemandMediaSubsession(env, source, nWidth, nHeight, vsIndex);
	}
protected:
	AKIPCMJPEGOnDemandMediaSubsession(UsageEnvironment& env, FramedSource* source,int nWidth, int nHeight, int index)
			:OnDemandServerMediaSubsession(env, True)
	{
		mp_source = source;
		mp_sdp_line = 0;
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		findex = index;
	}
	~AKIPCMJPEGOnDemandMediaSubsession()
	{
		if(mp_sdp_line)
			free(mp_sdp_line);
		mp_sdp_line = NULL;
	}

	virtual RTPSink* createNewRTPSink(Groupsock* rtpsock, unsigned char type, FramedSource* source);
	virtual FramedSource* createNewStreamSource(unsigned sid, unsigned& bitrate);
	//virtual char const* sdpLines();

private:
	static void afterPlayingDummy(void* ptr);
	static void chkForAuxSDPLine(void* ptr);
	void chkForAuxSDPLine1();

protected:
};

#endif
