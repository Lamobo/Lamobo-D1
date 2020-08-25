/*
 * ffmpeg_demux.h
 *
 *  Created on: 2011-12-9
 *      Author: Liang Guangwei
 */

#ifndef AK_DEMUX_H_
#define AK_DEMUX_H_
#include <liveMedia.hh>
#include "AKDemuxedElementaryStream.hh"
#include "AKMJPEGDemuxedElementaryStream.hh"
class SavedData; // forward
//class AKDemuxedElementaryStream;
//struct AVPacket;
#include "AKDemuxInterface.h"
class AKDemux: public Medium {
public:
    static AKDemux* CreateNew(UsageEnvironment& env, char const* filename,
            Boolean reclaim_last_es_dies);
private:
    AKDemux(UsageEnvironment& env, char const *filename,
            Boolean reclaim_last_es_dies);
    virtual ~AKDemux();

public:
    Boolean InitAK();
    Boolean deinitAK();
    /*AKDemuxedElementaryStream*/FramedSource* NewElementaryStream(u_int8_t streamIdTag,
            char const* mine_type, unsigned duration, int isMjpeg = 0);

    // similar to FramedSource::getNextFrame(), except that it also
    // takes a stream id tag as parameter.
    void GetNextFrame(u_int8_t stream_id, unsigned char* to, unsigned maxsize,
            FramedSource::afterGettingFunc* AfterGettingFunc,
            void* after_getting_client_data,
            FramedSource::onCloseFunc* OnCloseFunc, void* on_close_client_ata);

    // similar to FramedSource::stopGettingFrames(), except that it also
    // takes a stream id tag as parameter.
    void StopGettingFrames(u_int8_t stream_id_tag);

    // This should be called (on ourself) if the source is discovered
    // to be closed (i.e., no longer readable)
    static void HandleClosure(void* client_data);

    // should be called before any 'seek' on the underlying source
    void FlushInput();

private:
    void RegisterReadInterest(u_int8_t stream_id_tag, unsigned char* to,
            unsigned maxsize, FramedSource::afterGettingFunc* AfterGettingFunc,
            void* after_getting_client_data,
            FramedSource::onCloseFunc* OnCloseFunc, void* on_close_client_data);

    Boolean UseSavedData(u_int8_t stream_id_tag, unsigned char* to,
            unsigned maxsize, FramedSource::afterGettingFunc* AfterGettingFunc,
            void* after_getting_client_data);

    static void ContinueReadProcessing(void* client_data, unsigned char* ptr,
            unsigned size, struct timeval presentation_time);
    void ContinueReadProcessing();

    int16_t Parse();

    int ParseH264ExtraDataInMp4(int stream_id);

    int ReadOneFrame(/*AVPacket*/void* packet, /*boolean*/Boolean &has_extra_data);

    int CopyData(void* dst, int dst_max_size, const void* src, int src_size);

    int SaveData(int stream_id, unsigned char* srcbuf, int size,
            /*boolean*/Boolean reuse_buf = False);

private:
    friend class AKDemuxedElementaryStream;
    friend class AKMJPEGDemuxedElementaryStream;
    void NoteElementaryStreamDeletion();
private:
    char const *filename_;
   // struct AVFormatContext *format_ctx_;

    // A descriptor for each possible stream id tag:
    typedef struct OutputDescriptor {
        // input parameters
        unsigned char* to;
        unsigned max_size;
        FramedSource::afterGettingFunc* AfterGettingFunc;
        void* after_getting_client_data;
        FramedSource::onCloseFunc* OnCloseFunc;
        void* on_close_client_data;

        // output parameters
        unsigned frame_size;
        struct timeval presentation_time;

        SavedData* saved_data_head;
        SavedData* saved_data_tail;
        unsigned saved_data_total_size;

        // status parameters
        Boolean is_potentially_readable;
        Boolean is_currently_active;
        Boolean is_currently_awaiting_data;

        int data_counts; //this should be delete

    } OutputDescriptor_t;
    OutputDescriptor_t output_[256];

    unsigned num_pending_reads_;
    Boolean have_undelivered_data_;

    Boolean reclaim_last_es_dies_; //whether delete self when last es dies
    int num_out_es_;
    
    void* hMedia_;
    IFileSource* fs_;
    
    AKDemuxMediaInfo mediainfo_;
    DemuxPacket packet_;
};

#endif /* FFMPEG_DEMUX_H_ */
