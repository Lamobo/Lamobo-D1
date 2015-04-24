#include <string.h>
#include <pthread.h>
#include "DemuxForLive555.h"
#include "IFileSource.h"
#include "platform_cb.h"
using namespace akmedia;
#include "media_demuxer_lib.h"

#define RET_OK 0
#define RET_ERROR -1
#define RET_NO_BUFFER -2
#define RET_END 1


/*
 * 读取数据回调函数
 * 参数：
 * hFile:指向的文件指针
 * buf:  放置存储的缓冲区
 * size: 读取的数据大小
 * 返回值：
 * 实际读取到的数据大小
 */
static T_S32 libFread(T_S32 hFile, T_pVOID buf, T_S32 size)
{
    IFileSource *dataSource = (IFileSource*)hFile;
    int rtt =dataSource->read(buf, size);
    return rtt; 
}

/*
 * 写数据回调函数
 * 参数：
 * hFile:指向的文件指针
 * buf:  放置存储的缓冲区
 * size: 写入的数据大小
 * 返回值：
 * 总是返回-1
 */
static T_S32 libFwrite(T_S32 hFile, T_pVOID buf, T_S32 size)
{
    return -1;
}

/*
 * 写数据回调函数
 * 参数：
 * hFile:指向的文件指针
 * buf:  放置存储的缓冲区
 * size: 写入的数据大小
 * 返回值：
 * 实际写入的数据大小
 */
static T_S32 libFseek(T_S32 hFile, T_S32 offset, T_S32 whence)
{
    IFileSource *dataSource = (IFileSource*)hFile;
    if (dataSource->seek(offset, whence) < 0) {
        return -1;
    }

    return dataSource->tell();
}

/*
 * 获取文件的大小
 * 参数：
 * hFile:文件句柄
 * 返回值：
 * 文件的大小
 */
static T_S32 libFtell(T_S32 hFile)
{
    IFileSource *dataSource = (IFileSource*)hFile;
    return dataSource->tell();
}

/*
 * 判断文件是否存在
 * 参数：
 * hFile:文件句柄
 * 返回值：
 * 存在返回1，不存在返回0
 */
static T_S32 lnx_fhandle_exist(T_S32 hFile)
{
    if ((void*)hFile == NULL) {
        return 0;
    }
    IFileSource *dataSource = (IFileSource*)hFile;
    if (dataSource->tell() < 0) {
        return 0;
    }

    return 1;
}



/*
 * 初始化Demux库
 * 参数：
 * source[in]:文件源
 * 返回值：
 * 0：初始化成功
 * 其它：初始化失败
 */
static int dmx_init(void** hMedia, IFileSource* source, T_MEDIALIB_DMX_INFO* mediainfo)
{
    T_MEDIALIB_DMX_OPEN_INPUT open_input;
    T_MEDIALIB_DMX_OPEN_OUTPUT open_output;
    memset(&open_input, 0, sizeof(T_MEDIALIB_DMX_OPEN_INPUT));
    memset(&open_output, 0, sizeof(T_MEDIALIB_DMX_OPEN_OUTPUT));
    open_input.m_hMediaSource = (T_S32)source;
    open_input.m_CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
    open_input.m_CBFunc.m_FunRead = (MEDIALIB_CALLBACK_FUN_READ)libFread;
    open_input.m_CBFunc.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)libFwrite;
    open_input.m_CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)libFseek;
    open_input.m_CBFunc.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)libFtell;
    open_input.m_CBFunc.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
    open_input.m_CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
    open_input.m_CBFunc.m_FunFileHandleExist = lnx_fhandle_exist;

    // open the dumxer
    *hMedia = MediaLib_Dmx_Open(&open_input, &open_output);
    if (AK_NULL == *hMedia)
    {
        return -1;
    }
    // get media info
    memset(mediainfo, 0, sizeof(T_MEDIALIB_DMX_INFO));
    MediaLib_Dmx_GetInfo(*hMedia, mediainfo);
    // release the info memory
    MediaLib_Dmx_ReleaseInfoMem(*hMedia);
	
    return 0;
}

/*
 * 释放Demux占用的资源
 * 参数：
 * 无
 * 返回值
 * 无
 */
void dmx_deinit(void* hMedia)
{
    //close lib
    if (hMedia != NULL)
    {
        MediaLib_Dmx_Stop(hMedia);
        MediaLib_Dmx_Close(hMedia);
    }
}
static void printMediaInfo(T_MEDIALIB_DMX_INFO* mediainfo)
{
	printf("mediaType:%u\n", (unsigned int)mediainfo->m_MediaType);
	printf("m_bHasVideo:%u\n", (unsigned int)mediainfo->m_bHasVideo);
	printf("m_bHasAudio:%u\n", (unsigned int)mediainfo->m_bHasAudio);
	printf("m_bAllowSeek:%u\n", (unsigned int)mediainfo->m_bAllowSeek);
	printf("m_bSelfRecord:%u\n", (unsigned int)mediainfo->m_bSelfRecord);
	printf("m_ulTotalTime_ms:%u\n", (unsigned int)mediainfo->m_ulTotalTime_ms);

	printf("video\n"); 
	printf("m_VideoDrvType:%u\n", (unsigned int)mediainfo->m_VideoDrvType);
	printf("m_uWidth:%u\n", (unsigned int)mediainfo->m_uWidth);
	printf("m_uHeight:%u\n", (unsigned int)mediainfo->m_uHeight);
	printf("m_uFPS:%u\n", (unsigned int)mediainfo->m_uFPS);
	printf("m_ulVideoBitrate:%u\n", (unsigned int)mediainfo->m_ulVideoBitrate);

	printf("audio\n"); 
	printf("m_ulAudioBitRate:%u\n", (unsigned int)mediainfo->m_ulAudioBitRate);
	printf("m_wFormatTag:%u\n", (unsigned int)mediainfo->m_wFormatTag);
	printf("m_nChannels:%u\n", (unsigned int)mediainfo->m_nChannels);
	printf("m_nSamplesPerSec:%u\n", (unsigned int)mediainfo->m_nSamplesPerSec);
	printf("m_nAvgBytesPerSec:%u\n", (unsigned int)mediainfo->m_nAvgBytesPerSec);
	printf("m_wBitsPerSample:%u\n", (unsigned int)mediainfo->m_wBitsPerSample);
	printf("m_cbSize:%u\n", (unsigned int)mediainfo->m_cbSize);
}

#if 1
static void  dodemex2(void* hMedia)
{
	T_MEDIALIB_DMX_BLKINFO dmxBlockInfo;
	T_U32 streamLen;
	T_U8* streamBuf = new T_U8[200 * 1024];
	//T_U8 streamBuf[200 * 1024];
	int demux_status;
	//goto out1;
	while(1)
  {
  	if (MediaLib_Dmx_GetNextBlockInfo(hMedia, &dmxBlockInfo) == AK_FALSE)
		{
			//error
			printf("MediaLib_Dmx_GetNextBlockInfo error\n");
			break;
		}

		streamLen = dmxBlockInfo.m_ulBlkLen;
		//printf("streamLen:%u\n", streamLen);
		switch (dmxBlockInfo.m_eBlkType)
		{
			case T_eMEDIALIB_BLKTYPE_VIDEO:
				if (streamLen != 0)
				{
					MediaLib_Dmx_GetVideoFrame(hMedia, streamBuf, &streamLen);
				}
				//printf("video size:%u\n", streamLen);
			break;
			case T_eMEDIALIB_BLKTYPE_AUDIO:
				if (streamLen != 0)
				{
					MediaLib_Dmx_GetAudioData(hMedia, streamBuf, streamLen);
				}
				//printf("audio size:%u\n", streamLen);
			break;
			default:
				printf("unknow !!!\n");
				break;
			}

			demux_status = MediaLib_Dmx_GetStatus(hMedia);
			if (demux_status == MEDIALIB_DMX_END || demux_status == MEDIALIB_DMX_ERR)
			{
				printf("error or end\n");
				break;
			}
			printf("demux_status=%d\n", demux_status);
  }
 //out1:
  printf("end11\n");
  delete streamBuf;
  printf("end22\n");
  streamBuf = NULL;
}
#endif
void DemuxTest(const char* filename)
{
	void * hmedia = NULL;
	T_MEDIALIB_DMX_INFO mediaInfo;
	
	
	IFileSource * ifile = new CFileSource(filename);
  if(ifile == NULL)
  	return;
  	
  dmx_init(&hmedia, ifile, &mediaInfo);
  printMediaInfo(&mediaInfo);
    
  unsigned int buffLen = MediaLib_Dmx_GetFirstVideoSize(hmedia);
  printf("first video size:%u\n", (unsigned int)buffLen);	
  unsigned char* buff = new unsigned char[buffLen];
  
  if(AK_FALSE == MediaLib_Dmx_GetFirstVideo(hmedia, buff, (T_U32*)&buffLen))
  {
  	printf("MediaLib_Dmx_GetFirstVideo error\n");	
  }
 
  delete buff;
  buff = NULL;
  
  MediaLib_Dmx_Start(hmedia, 0);
 
  #if 1
  dodemex2(hmedia);
  #endif
  
  dmx_deinit(hmedia);
  delete ifile;
  ifile = NULL;
	printf("6666\n");

}
static void MiToDmi(T_MEDIALIB_DMX_INFO* mi, AKDemuxMediaInfo* dmi)
{
	if(MEDIALIB_MEDIA_AVI == mi->m_MediaType)
		dmi->m_MediaType = AKDEMUX_MEDIA_AVI;
	else if(MEDIALIB_MEDIA_MP4 == mi->m_MediaType)
		dmi->m_MediaType = AKDEMUX_MEDIA_MP4;
	else
		dmi->m_MediaType = AKDEMUX_MEDIA_UNKNOW;
		
	dmi->m_bHasVideo = mi->m_bHasVideo;
	dmi->m_bHasAudio = mi->m_bHasAudio;
	dmi->m_bAllowSeek = mi->m_bAllowSeek;
	dmi->m_bSelfRecord = mi->m_bSelfRecord;
	dmi->m_ulTotalTime_ms = mi->m_ulTotalTime_ms;

	//video
	//printf("mi->m_VideoDrvType=%u\n", mi->m_VideoDrvType);
	if(VIDEO_DRV_H264 == mi->m_VideoDrvType)
		dmi->m_VideoDrvType = AKDEMUX_VIDEO_H264;
	else if(VIDEO_DRV_MJPEG == mi->m_VideoDrvType)
		dmi->m_VideoDrvType = AKDEMUX_VIDEO_MJPEG;
	else
		dmi->m_VideoDrvType = AKDEMUX_AV_UNKNOW;
	//printf("dmi->m_VideoDrvType=%u\n", dmi->m_VideoDrvType);
	dmi->m_uWidth = mi->m_uWidth;
	dmi->m_uHeight = mi->m_uHeight;
	dmi->m_uFPS = mi->m_uFPS;
	dmi->m_ulVideoBitrate = mi->m_ulVideoBitrate;
	
	//audio
	if(_SD_MEDIA_TYPE_AAC == mi->m_AudioType)
		dmi->m_AudioType = AKDEMUX_AUDIO_AAC;
	else
		dmi->m_AudioType = AKDEMUX_AV_UNKNOW;
		
	dmi->m_ulAudioBitRate = mi->m_ulAudioBitRate;
	dmi->m_wFormatTag = mi->m_wFormatTag;
	dmi->m_nChannels = mi->m_nChannels;
	dmi->m_nSamplesPerSec = mi->m_nSamplesPerSec;
	dmi->m_nAvgBytesPerSec = mi->m_nAvgBytesPerSec;
	dmi->m_nBlockAlign = mi->m_nBlockAlign;
	dmi->m_wBitsPerSample = mi->m_wBitsPerSample;
	dmi->m_cbSize = mi->m_cbSize;
	if(dmi->m_cbSize > 0)
		memcpy(dmi->m_szData, mi->m_szData, dmi->m_cbSize);
}
static int DemuxForLiveInit(void **hMedia, IFileSource* fs, AKDemuxMediaInfo* dmi)
{
	T_MEDIALIB_DMX_INFO mediaInfo;
	dmx_init(hMedia, fs, &mediaInfo);
	//printMediaInfo(&mediaInfo);
	MiToDmi(&mediaInfo, dmi);
	//printf("*******************\n");
	//printMediaInfo(&mediaInfo);

	//printf("dmi->fps=%u, mi-fps:%u\n", dmi->m_uFPS,mediaInfo.m_uFPS);
	unsigned int buffLen = MediaLib_Dmx_GetFirstVideoSize(*hMedia);
  //printf("first video size:%u\n", (unsigned int)buffLen);	
  unsigned char* buff = new unsigned char[buffLen];
  
  if(AK_FALSE == MediaLib_Dmx_GetFirstVideo(*hMedia, buff, (T_U32*)&buffLen))
  {
  	printf("MediaLib_Dmx_GetFirstVideo error\n");	
  }
 	//printf("after first video size:%u\n", (unsigned int)buffLen);	
  delete buff;
  buff = NULL;
  
  MediaLib_Dmx_Start(*hMedia, 0);
	return 0;	
}
static int DemuxForLiveDeinit(void* hMedia)
{
	dmx_deinit(hMedia);
	return 0;	
}
static int DemuxForLiveGetVideoData(void* hMedia, void* buf, unsigned* size, int nNeedIFramecount)
{
	*size = MediaLib_Dmx_GetVideoFrameSize(hMedia);
	
	//demuxVideoData(*size, buf, hMedia);
	return 0;	
}
static int DemuxForLiveGetAudioData(void* hMedia, void* buf, unsigned* size, int nNeedIFramecount)
{
	*size = MediaLib_Dmx_GetAudioDataSize(hMedia);
	//demuxAudioData(*size, buf, hMedia);
	return 0;	
}

static int DemuxForLiveGetAVData(void* hMedia, void* buf, unsigned* size, int nNeedIFramecount, int* ntype)
{
	T_MEDIALIB_DMX_BLKINFO dmxBlockInfo;
	if (MediaLib_Dmx_GetNextBlockInfo(hMedia, &dmxBlockInfo) == AK_FALSE)
	{
			//error
			printf("MediaLib_Dmx_GetNextBlockInfo error\n");
			return -1;
	}

	*size = dmxBlockInfo.m_ulBlkLen;
	switch (dmxBlockInfo.m_eBlkType)
	{
			case T_eMEDIALIB_BLKTYPE_VIDEO:
			*ntype = 0;
			//printf("before size=%u\n", *size);
			if (*size != 0)
			{
				MediaLib_Dmx_GetVideoFrame(hMedia, (T_U8*)buf, (T_U32*)size);
			}
			//printf("0x%x video size:%u\n", hMedia, *size);
			//*size -= 8;
			break;
			case T_eMEDIALIB_BLKTYPE_AUDIO:
			*ntype = 1;
			if (*size != 0)
			{
				MediaLib_Dmx_GetAudioData(hMedia, (T_U8*)buf, (T_U32)*size);
			}
			
			//printf("0x%x audio size:%u\n", hMedia, *size);
			if(*size == 4096)
				*size = 0;
			break;
			default:
			printf("unknow type\n");
			*ntype = -1;
			break;
	}

	T_U32 demux_status = MediaLib_Dmx_GetStatus(hMedia);
	if (demux_status == MEDIALIB_DMX_END || demux_status == MEDIALIB_DMX_ERR)
	{
		printf("end or error\n");
		return -2;
	}	
  return 0;
}
void DemuxForLiveSetCallBack()
{
	AKDemuxIFCallBack callback;
	callback.initFunc = DemuxForLiveInit;
	callback.deinitFunc = DemuxForLiveDeinit;
	callback.getVideoData = DemuxForLiveGetVideoData;
	callback.getAudioData = DemuxForLiveGetAudioData;
	callback.getAVData = DemuxForLiveGetAVData;
	setCallBack(callback);
}
