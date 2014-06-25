/*
 * ffmpeg_MJPEG_server_media_subession.h
 *
 *  Created on: 2011-12-27
 *      Author: Liang Guangwei
 */

#ifndef AK_MJPEG_SERVER_MEDIA_SUBESSION_H_
#define AK_MJPEG_SERVER_MEDIA_SUBESSION_H_
#include <liveMedia.hh>
//#include "AKDemux.hh"
//#include "AKServerDemux.hh"
//#include "AKDemuxedElementaryStream.hh"

class AKServerDemux;
class AKMJPEGServerMediaSubsession: public FileServerMediaSubsession{
public:
    static AKMJPEGServerMediaSubsession* CreateNew(
            AKServerDemux& demux, u_int8_t stream_id, Boolean reuse_source);
private:
    AKMJPEGServerMediaSubsession(AKServerDemux& demux, u_int8_t stream_id, Boolean reuse_source);
    virtual ~AKMJPEGServerMediaSubsession();

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

#endif /* FFMPEG_MJPEG_SERVER_MEDIA_SUBESSION_H_ */
