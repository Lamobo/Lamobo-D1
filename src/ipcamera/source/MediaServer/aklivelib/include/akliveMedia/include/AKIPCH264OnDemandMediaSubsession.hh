#ifndef _AKIPC_H264_ONDEMAND_MEDIA_SUBSESSION_
#define _AKIPC_H264_ONDEMAND_MEDIA_SUBSESSION_
#include <liveMedia.hh>
#include "AKIPCH264FramedSource.hh"


class AKIPCH264OnDemandMediaSubsession: public OnDemandServerMediaSubsession
{
private:
	FramedSource* mp_source;
	char* mp_sdp_line;
	RTPSink* mp_dummy_rtpsink;
	char m_done;
	int m_type;
	int findex;
public:

	GetFrameFunc getframefunc;	
	SetLed setledstart;
	SetLed setledexit;

	static AKIPCH264OnDemandMediaSubsession* createNew(UsageEnvironment& env, FramedSource* source, int nType, int vsIndex)
	{
		return new AKIPCH264OnDemandMediaSubsession(env, source, nType, vsIndex);
	}
protected:
	AKIPCH264OnDemandMediaSubsession(UsageEnvironment& env, FramedSource* source,int nType, int index)
			:OnDemandServerMediaSubsession(env, True)
	{
		m_type = nType;
		mp_source = source;
		mp_sdp_line = 0;
		findex = index;
	}
	~AKIPCH264OnDemandMediaSubsession()
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
	virtual const char* getAuxSDPLine(RTPSink *sink, FramedSource* source);
};

#endif
