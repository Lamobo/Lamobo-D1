/*
 * FfmpegMJPEGDemuxedElementaryStream.h
 *
 *  Created on: 2011-12-8
 *      Author: Liang Guangwei
 */

#ifndef AKMJPEGDEMUXEDELEMENTARYSTREAM_H_
#define AKMJPEGDEMUXEDELEMENTARYSTREAM_H_
#include <liveMedia.hh>
//#include "AKDemux.hh"
class AKDemux;
class AKMJPEGDemuxedElementaryStream: public JPEGVideoSource {
public:
   static AKMJPEGDemuxedElementaryStream* CreateNew(
            UsageEnvironment& env,
            u_int8_t stream_id,
            AKDemux& demux,
            char const* mine_type,
            unsigned duration, int width, int height);
   void setWidth(int nWidth)
	{
		m_nWidth = nWidth;
	};
	void setHeight(int nHeight)
	{
		m_nHeight = nHeight;
	};
	
	virtual u_int8_t type()
	{
		return m_nType; 
	};
	virtual u_int8_t qFactor()
	{
		return m_nQFactor;
	};
	virtual u_int8_t width()
	{
		return m_nWidth / 8;	
	};
	virtual u_int8_t height()
	{
		return m_nHeight / 8;
	};
private:
    virtual ~AKMJPEGDemuxedElementaryStream();
    AKMJPEGDemuxedElementaryStream(
            UsageEnvironment& env,
            u_int8_t stream_id,
            AKDemux& demux,
            char const* mine_type,
            unsigned duration, int width, int height);
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
 
		static void getNextFrame(void* ptr);
		void getNextFrame1();
private:
    AKDemux &ak_demux_;
    u_int8_t stream_id_;
    char const* mine_type_;
    unsigned duration_;
    
    int m_nWidth;
		int m_nHeight;
		int m_nType;
		int m_nQFactor;
};

#endif /* FFMPEGMJPEGDemuxedELEMENTARYSTREAM_H_ */
