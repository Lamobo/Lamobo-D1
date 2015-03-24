/*
 * FfmpegDemuxedElementaryStream.h
 *
 *  Created on: 2011-12-8
 *      Author: Liang Guangwei
 */

#ifndef AKDEMUXEDELEMENTARYSTREAM_H_
#define AKDEMUXEDELEMENTARYSTREAM_H_
#include <liveMedia.hh>
//#include "AKDemux.hh"
class AKDemux;
class AKDemuxedElementaryStream: public FramedSource {
public:
   static AKDemuxedElementaryStream* CreateNew(
            UsageEnvironment& env,
            u_int8_t stream_id,
            AKDemux& demux,
            char const* mine_type,
            unsigned duration);
private:
    virtual ~AKDemuxedElementaryStream();
    AKDemuxedElementaryStream(
            UsageEnvironment& env,
            u_int8_t stream_id,
            AKDemux& demux,
            char const* mine_type,
            unsigned duration);
    virtual unsigned maxFrameSize() const;

protected:  //redefined virtual functions
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();
    virtual char const* MIMEtype() const;

private:
    static void AfterGettingFrame(void* client_data,
                  unsigned frame_size, unsigned num_truncated_bytes,
                  struct timeval presentation_time,
                  unsigned duration_in_microseconds);

    void AfterGettingFrame1(unsigned frame_size, unsigned num_truncated_bytes,
                struct timeval presentation_time,
                unsigned duration_in_microseconds);
private:
    AKDemux &ak_demux_;
    u_int8_t stream_id_;
    char const* mine_type_;
    unsigned duration_;
};

#endif /* FFMPEGDEMUXEDELEMENTARYSTREAM_H_ */
