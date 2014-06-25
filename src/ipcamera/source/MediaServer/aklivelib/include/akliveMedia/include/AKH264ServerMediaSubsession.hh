/*
 * ffmpeg_H264_server_media_subsession.h
 *
 *  Created on: 2011-12-8
 *      Author: Liang Guangwei
 */

#ifndef AK_H264_SERVER_MEDIA_SUBSESSION_H_
#define AK_H264_SERVER_MEDIA_SUBSESSION_H_
#include <liveMedia.hh>
//#include "AKDemux.hh"
//#include "AKServerDemux.hh"
//#include "AKDemuxedElementaryStream.hh"
//though inherit from H264VideoFileServerMediaSubsession, we don't use the function
//about dealing file.
class AKServerDemux;
class AKH264ServerMediaSubsession: public H264VideoFileServerMediaSubsession{
public:
    static AKH264ServerMediaSubsession* CreateNew(
            AKServerDemux& demux, u_int8_t stream_id, Boolean reuse_sourc);

private:
    virtual ~AKH264ServerMediaSubsession();
    AKH264ServerMediaSubsession(
            AKServerDemux& demux, u_int8_t stream_id, Boolean reuse_sourc);

protected:  //redefined virtual functions
    //
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
            unsigned& estBitrate);
private:
    AKServerDemux& ak_demux_;
    u_int8_t stream_id_;

};

#endif /* FFMPEG_H264_VIDEO_SERVER_MEDIA_SUBSESSION_H_ */
