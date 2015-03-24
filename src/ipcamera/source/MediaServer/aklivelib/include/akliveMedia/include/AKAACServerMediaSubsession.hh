/*
 * ffmpeg_AAC_server_media_subession.h
 *
 *  Created on: 2011-12-27
 *      Author: Liang Guangwei
 */

#ifndef AK_AAC_SERVER_MEDIA_SUBESSION_H_
#define AK_AAC_SERVER_MEDIA_SUBESSION_H_
#include <liveMedia.hh>
//#include "AKDemux.hh"
//#include "AKServerDemux.hh"
//#include "AKDemuxedElementaryStream.hh"
class AKServerDemux;
class AKAACServerMediaSubsession: public FileServerMediaSubsession{
public:
    static AKAACServerMediaSubsession* CreateNew(
            AKServerDemux& demux, u_int8_t stream_id, Boolean reuse_source);
private:
    AKAACServerMediaSubsession(AKServerDemux& demux, u_int8_t stream_id, Boolean reuse_source);
    virtual ~AKAACServerMediaSubsession();

protected:  //redefined virtual functions

    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
            unsigned& estBitrate);
            
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
            FramedSource* inputSource);

private:
    AKServerDemux& ak_demux_;
    u_int8_t stream_id_;
    char* config_str_;
};

#endif /* FFMPEG_AAC_SERVER_MEDIA_SUBESSION_H_ */
