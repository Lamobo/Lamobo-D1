/*
 * FfmpegDemux.h
 *
 *  Created on: 2011-12-8
 *  Author: Liang Guangwei
 *
 *  use ffmpeg to demux file
 */

#ifndef AK_SERVER_DEMUX_H_
#define AK_Server_DEMUX_H_
#include <liveMedia.hh>
//#include "AKDemuxedElementaryStream.hh"
//#include "AKServerMediaSubsession.hh"
//#include "AKDemux.hh"
#include "AKDemuxInterface.h"

#define MAX_STREAM_NUM 5

class AKDemuxedElementaryStream;
class AKDemux;

struct StreamInfo {
    char const* mine_type;
    unsigned int duration;
    unsigned char* extra_data;
    int extra_data_size;
    unsigned int codec_id;

    //audio parameter
    unsigned int sample_rate;
    unsigned int channels;
};

class AKServerDemux: public Medium{
public:
    static AKServerDemux* CreateNew(UsageEnvironment& env, char const* filename,
            Boolean reuse_source);

private:
    virtual ~AKServerDemux();
    AKServerDemux(UsageEnvironment& env, char const* filename,
            Boolean reuse_source);

public:
    /*AKDemuxedElementaryStream*/FramedSource* NewElementaryStream(
            unsigned client_session_id, u_int8_t stream_id, int isMjpeg = 0);

    //
    //the follow tow functions called in class FfmpegDemux
    //
    AKDemuxedElementaryStream* NewAudioStream();
    AKDemuxedElementaryStream* NewVideoStream();

    ServerMediaSubsession* NewAudioServerMediaSubsession();
    ServerMediaSubsession* NewVideoServerMediaSubsession();

    char const* MIMEtype(int stream_id);
    const StreamInfo* GetStreamInfo(int stream_id);
private:
    ServerMediaSubsession* NewServerMediaSubsession(unsigned int type);
    //detecte stream tags from file
    Boolean DetectedStream();

private:
    FramedSource *input_source_;
    Boolean reuse_source_;
    char const* filename_;

    AKDemux *session0_demux_;
    AKDemux *last_created_demux_;
    unsigned last_client_session_id_;

    int video_stream_id_;
    int audio_stream_id_;
    
    StreamInfo stream_[MAX_STREAM_NUM];
};

#endif /* FFMPEGDEMUX_H_ */
