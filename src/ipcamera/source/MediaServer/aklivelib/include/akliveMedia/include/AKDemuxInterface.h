#ifndef __AK_DEMUX_INTERFACE_H
#define __AK_DEMUX_INTERFACE_H
#include "IFileSource.h"
using namespace akmedia;

typedef enum
{
	AKDEMUX_MEDIA_UNKNOW,
	AKDEMUX_MEDIA_AVI,
	AKDEMUX_MEDIA_MP4
}AKDemuxMediaType;

typedef enum
{
	AKDEMUX_AV_UNKNOW,
	AKDEMUX_VIDEO_H264,
	AKDEMUX_VIDEO_MJPEG,	
	AKDEMUX_AUDIO_AAC
}AKDemuxAVType;

#define AKDEMUX_INFO_EX_SIZE 128
typedef struct
{
	AKDemuxMediaType m_MediaType;
	bool m_bHasVideo;
	bool m_bHasAudio;
	bool m_bAllowSeek;
	bool m_bSelfRecord;
	unsigned int m_ulTotalTime_ms;

	//video
	AKDemuxAVType m_VideoDrvType;
	unsigned short m_uWidth;
	unsigned short m_uHeight;
	unsigned short m_uFPS;
	unsigned int m_ulVideoBitrate;
	//audio
	AKDemuxAVType m_AudioType;
	unsigned int m_ulAudioBitRate;
	unsigned short m_wFormatTag;
	unsigned short m_nChannels;
	unsigned int m_nSamplesPerSec;
	unsigned int m_nAvgBytesPerSec;
	unsigned short m_nBlockAlign;
	unsigned short m_wBitsPerSample;
	unsigned short m_cbSize;
	unsigned char	m_szData[AKDEMUX_INFO_EX_SIZE];
}AKDemuxMediaInfo;

typedef int (*AKDemuxIFInitFunc)(void **hMedia, IFileSource* fs, AKDemuxMediaInfo* dmi);
typedef int (*AKDemuxIFDeinitFunc)(void* hMedia);
typedef int (*AKDemuxIFGetVideoData)(void* hMedia, void* buf, unsigned* size, int nNeedIFramecount);
typedef int (*AKDemuxIFGetAudioData)(void* hMedia, void* buf, unsigned* size, int nNeedIFramecount);
typedef int (*AKDemuxIFGetAVData)(void* hMedia, void* buf, unsigned* size, int nNeedIFramecount, int* ntype);
typedef struct
{
	AKDemuxIFInitFunc initFunc;
	AKDemuxIFDeinitFunc deinitFunc;
	AKDemuxIFGetVideoData getVideoData;//not used now
	AKDemuxIFGetAudioData getAudioData;//now used now
	AKDemuxIFGetAVData getAVData;
}AKDemuxIFCallBack;
void setCallBack(AKDemuxIFCallBack callback);
AKDemuxIFCallBack* getCallBack();

#define MAX_DMX_PACKET_SIZE 200 * 1024
typedef struct
{
	unsigned char data[MAX_DMX_PACKET_SIZE];
	unsigned size;
	int type;
}DemuxPacket;
#endif

