/**
 * @file media_demuxer_lib.h
 * @brief This file provides MP4/3GP/AVI/AKV/RMVB/MKV/mp3/aac/amr/flac/ape... demuxing functions
 * these APIs must cooperate with APIs in video codec and audio codec
 *
 * Copyright (C) 2012 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Su Dan
 * @date 2010-12-1
 * @update date 2012-1-17
 * @version 0.1.0
 * @version ever green group version: x.x
 * @note
	The following is an example to use demuxing APIs
   @code

T_S32 Demux_Media(char* filename)
{
	T_MEDIALIB_DMX_OPEN_INPUT open_input;
	T_MEDIALIB_DMX_OPEN_OUTPUT open_output;
	T_MEDIALIB_DMX_INFO media_info;
	T_U8 streamBuf[960*480*2] = {0};
	T_eMEDIALIB_DMX_STATUS demux_status;
	T_VOID *hMedia;
	T_S32 fid;
	T_U32 streamLen = 0;
	T_S32 pts = 0;

	FILE *fp;
	FILE *fp_a;

	fid = _open(filename, _O_RDONLY | _O_BINARY);
	if(fid <= 0)
	{
		printf("open file failed\r\n");
		return 0;
	}

	memset(&open_input, 0, sizeof(T_MEDIALIB_DMX_OPEN_INPUT));
	open_input.m_hMediaSource = fid;
	open_input.m_CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	open_input.m_CBFunc.m_FunRead = (MEDIALIB_CALLBACK_FUN_READ)_read;
	open_input.m_CBFunc.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)_write;
	open_input.m_CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)_lseek;
	open_input.m_CBFunc.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)_tell;
	open_input.m_CBFunc.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)my_malloc;
	open_input.m_CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
	open_input.m_CBFunc.m_FunFileHandleExist = file_handle_exist;

	hMedia = MediaLib_Dmx_Open(&open_input, &open_output);
	if (AK_NULL == hMedia)
	{
		_close(fid);
		return 0;	
	}

	MediaLib_Dmx_GetInfo(hMedia, &media_info);
	MediaLib_Dmx_ReleaseInfoMem(hMedia);
	MediaLib_Dmx_GetInfo(hMedia, &media_info);

	bSeekFlag = 0;

	streamLen = MediaLib_Dmx_GetFirstVideoSize(hMedia);
	if (MediaLib_Dmx_GetFirstVideo(hMedia, streamBuf, &streamLen) == AK_FALSE)
	{
		MediaLib_Dmx_Close(hMedia);
		_close(fid);
		return 0;
	}

	//here decode first video

	pts = MediaLib_Dmx_Start(hMedia, 0);

	while (1)
	{
		if (!MediaLib_Dmx_CheckAudioEnd(hMedia))
		{
			streamLen = MediaLib_Dmx_GetAudioDataSize(hMedia);
			if (streamLen != 0)
			{
				MediaLib_Dmx_GetAudioData(hMedia, streamBuf, streamLen);
			}

			//here decode audio
		}

		if (!MediaLib_Dmx_CheckVideoEnd(hMedia))
		{
			streamLen = MediaLib_Dmx_GetVideoFrameSize(hMedia);
			if (streamLen != 0)
			{
				MediaLib_Dmx_GetVideoFrame(hMedia, streamBuf, &streamLen);
			}

			//here decode video
		}

		demux_status = MediaLib_Dmx_GetStatus(hMedia);
		if (demux_status == MEDIALIB_DMX_END || demux_status == MEDIALIB_DMX_ERR)
		{
			break;
		}
	}

	if (fp != 0)
	{
		fclose(fp);
	}
	if (fp_a != 0)
	{
		fclose(fp_a);
	}

	MediaLib_Dmx_Close(hMedia);

	_close(fid);

	return 1;
}

*****************instead of the code above in while(1)************************
forward read mode:
	while (1)
	{
		if (MediaLib_Dmx_GetNextBlockInfo(hMedia, &dmxBlockInfo) == AK_FALSE)
		{
			//error
			break;
		}

		streamLen = dmxBlockInfo.m_ulBlkLen;
		switch (dmxBlockInfo.m_eBlkType)
		{
		case T_eMEDIALIB_BLKTYPE_VIDEO:
			if (streamLen != 0)
			{
				MediaLib_Dmx_GetVideoFrame(hMedia, streamBuf, &streamLen);
			}
			break;
		case T_eMEDIALIB_BLKTYPE_AUDIO:
			if (streamLen != 0)
			{
				MediaLib_Dmx_GetAudioData(hMedia, streamBuf, streamLen);
			}
			break;
		default:
			break;
		}

		demux_status = MediaLib_Dmx_GetStatus(hMedia);
		if (demux_status == MEDIALIB_DMX_END || demux_status == MEDIALIB_DMX_ERR)
		{
			break;
		}
	}

mix read mode:
	while (1)
	{
		if (MediaLib_Dmx_GetNextBlockInfo(hMedia, &dmxBlockInfo) == AK_FALSE)
		{
			//error
			break;
		}

		streamLen = dmxBlockInfo.m_ulBlkLen;
		switch (dmxBlockInfo.m_eBlkType)
		{
		case T_eMEDIALIB_BLKTYPE_VIDEO:
			if (streamLen != 0)
			{
				MediaLib_Dmx_GetVideoFrame(hMedia, streamBuf, &streamLen);
			}
			break;
		case T_eMEDIALIB_BLKTYPE_AUDIO:
			if (streamLen != 0)
			{
				MediaLib_Dmx_GetAudioData(hMedia, streamBuf, streamLen);
			}
			break;
		default:
			break;
		}

		if (!MediaLib_Dmx_CheckAudioEnd(hMedia))
		{
			streamLen = MediaLib_Dmx_GetAudioDataSize(hMedia);
			if (streamLen != 0)
			{
				MediaLib_Dmx_GetAudioData(hMedia, streamBuf, streamLen);
			}
		}

		if (!MediaLib_Dmx_CheckVideoEnd(hMedia))
		{
			streamLen = MediaLib_Dmx_GetVideoFrameSize(hMedia);
			if (streamLen != 0)
			{
				MediaLib_Dmx_GetVideoFrame(hMedia, streamBuf, &streamLen);
			}
		}

		demux_status = MediaLib_Dmx_GetStatus(hMedia);
		if (demux_status == MEDIALIB_DMX_END || demux_status == MEDIALIB_DMX_ERR)
		{
			break;
		}
	}

	@endcode

 ***************************************************/
#ifndef _MEDIA_DEMUXER_LIB_H_
#define _MEDIA_DEMUXER_LIB_H_

#include "medialib_struct.h"
#include "video_stream_lib.h"
#include "sdcodec.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	MEDIALIB_DMX_END,
	MEDIALIB_DMX_PLAY,
	MEDIALIB_DMX_PAUSE,
	MEDIALIB_DMX_STOP,
	MEDIALIB_DMX_ERR,
	MEDIALIB_DMX_SEEK,
	MEDIALIB_DMX_FF,
	MEDIALIB_DMX_FR
}T_eMEDIALIB_DMX_STATUS;

typedef struct
{
	T_eMEDIALIB_MEDIA_TYPE	m_MediaType;
	T_S32					m_hMediaSource;
	T_MEDIALIB_CB 			m_CBFunc;
}T_MEDIALIB_DMX_OPEN_INPUT;

typedef struct
{	
	T_eMEDIALIB_STATE		m_State;
}T_MEDIALIB_DMX_OPEN_OUTPUT;

#define MEDIALIB_DMX_INFO_EX_SIZE	24
typedef struct
{
	T_eMEDIALIB_MEDIA_TYPE	m_MediaType;
	T_BOOL	m_bHasVideo;
	T_BOOL	m_bHasAudio;
	T_BOOL	m_bAllowSeek;
	T_BOOL	m_bSelfRecord;
	T_U32	m_ulTotalTime_ms;

	//video
	T_eVIDEO_DRV_TYPE	m_VideoDrvType;
	T_U16	m_uWidth;
	T_U16	m_uHeight;
	T_U16	m_uFPS;
	T_U32	m_ulVideoBitrate;

	//audio
	T_AUDIO_TYPE		m_AudioType;
	T_U32	m_ulAudioBitRate;
	T_U16	m_wFormatTag;
	T_U16	m_nChannels;
	T_U32	m_nSamplesPerSec;
	T_U32	m_nAvgBytesPerSec;
	T_U16	m_nBlockAlign;
	T_U16	m_wBitsPerSample;
	T_U16	m_cbSize;
	T_U8	m_szData[MEDIALIB_DMX_INFO_EX_SIZE];

	T_MEDIALIB_META_INFO	*m_pMetaInfo;
}T_MEDIALIB_DMX_INFO;


/**
 * @brief Open Demuxer, Analysis Media header, and check parameters
 *
 * @author Su_Dan
 * @param	dmx_open_input		[in]	pointer of T_MEDIALIB_DMX_OPEN_INPUT struct
 * @param	dmx_open_output		[out]	pointer of T_MEDIALIB_DMX_OPEN_OUTPUT struct
 * @return T_MEDIALIB_STRUCT
 * @retval	AK_NULL			open failed
 * @retval	other			open ok
 */
T_MEDIALIB_STRUCT MediaLib_Dmx_Open(T_MEDIALIB_DMX_OPEN_INPUT *dmx_open_input, T_MEDIALIB_DMX_OPEN_OUTPUT *dmx_open_output);


/**
 * @brief Close Demuxer
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Close ok
 * @retval	AK_FALSE	Close fail
 */
T_BOOL MediaLib_Dmx_Close(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get ready to read video and audio stream
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	start_pos	[in]	start time, 0 or the retuned value by setpos
 * @return T_S32
 * @retval	< 0		start fail
 * @retval	other	if there is audio, audio start time in millisecond
					if there is no audio, video start time in millisecond
 */
T_S32 MediaLib_Dmx_Start(T_MEDIALIB_STRUCT hMedia, T_U32 start_pos);


/**
 * @brief Resume reading video and audio stream
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Resume ok
 * @retval	AK_FALSE	Resume fail
 */
T_BOOL MediaLib_Dmx_Resume(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Stop reading video and audio stream
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Stop ok
 * @retval	AK_FALSE	Stop fail
 */
T_BOOL MediaLib_Dmx_Stop(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Pause reading video and audio stream
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Pause ok
 * @retval	AK_FALSE	Pause fail
 */
T_BOOL MediaLib_Dmx_Pause(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get current media information
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	pInfo		[out]	pointer of T_MEDIALIB_DMX_INFO struct
 * @return T_BOOL
 * @retval	AK_FALSE	get information fail
 * @retval	AK_TRUE		get information ok
 */
T_BOOL MediaLib_Dmx_GetInfo(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_DMX_INFO *pInfo);


/**
 * @brief Release memory of information to save space
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		release ok
 * @retval	AK_FALSE	release fail
 */
T_BOOL MediaLib_Dmx_ReleaseInfoMem(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get audio seek information
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_AUDIO_SEEK_INFO *
 * @retval	AK_NULL		get fail
 * @retval	other		get ok
 */
T_AUDIO_SEEK_INFO *MediaLib_Dmx_GetAudioSeekInfo(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get status of demuxer
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_eMEDIALIB_DMX_STATUS 
 * @retval	see struct T_eMEDIALIB_DMX_STATUS
 */
T_eMEDIALIB_DMX_STATUS MediaLib_Dmx_GetStatus(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Set movie position
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	msTime		[in]	the movies millisecond
 * @param	bSeekNext	[in]	whether set postion by next key frame, only valid to video
 * @return T_S32
 * @retval	< 0			set fail
 * @retval	other		if there is video, video pts in millisecond
						if no video, audio pts in millisecond
 */
T_S32 MediaLib_Dmx_SetPosition(T_MEDIALIB_STRUCT hMedia, T_U32 uTimeMs, T_BOOL bSeekNext);


/**
 * @brief Reset audio position, to get first audio
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_S32
 * @retval	< 0			set fail
 * @retval	other		audio pts in millisecond
 */
T_S32 MediaLib_Dmx_ResetAudioPos(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get audio data, send audio data to pData, send audio data length is pDataLen
 *
 * @author Su_Dan
 * @param	hMedia		[in]		pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	pData		[out]		audio data
 * @param	uDataLen	[in]		data size, returned by MediaLib_Dmx_GetAudioDataSize
 * @return T_U32
 * @retval	0		no more audio data
 * @retval	other	audio data size
 */
T_U32 MediaLib_Dmx_GetAudioData(T_MEDIALIB_STRUCT hMedia, T_pDATA pData, T_U32 uDataLen);


/**
 * @brief Get audio size for checking the space
 *
 * @author Su_Dan
 * @param	hMedia	[in]		pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_U32
 * @retval	0		no more audio data
 * @retval	other	audio data size
 */
T_U32 MediaLib_Dmx_GetAudioDataSize(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Ready to preview
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	ulMilliSec	[in]	time in millisecond
 * @return T_U8
 * @retval	0	fail
 * @retval	1	first key frame ok
 * @retval	2	about 1/5 pos frame ok
T_U8 MediaLib_Dmx_Preview(T_MEDIALIB_STRUCT hMedia, T_U32 ulMilliSec);
 */


/**
 * @brief Disable reading audio stream
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval	AK_FALSE	fail
 * @retval	AK_TRUE		ok
T_BOOL MediaLib_Dmx_DisableAudio(T_MEDIALIB_STRUCT hMedia);
 */


/**
 * @brief Get status of demuxer
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_S32
 * @retval	< 0		error
 * @retval	other	audio pts
T_S32 MediaLib_Dmx_GetAudioPTS(T_MEDIALIB_STRUCT hMedia);
 */


/**
 * @brief Check whether audio data is end
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		end
 * @retval	AK_FALSE	not end
 */
T_BOOL MediaLib_Dmx_CheckAudioEnd(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Check whether video data is end
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		end
 * @retval	AK_FALSE	not end
 */
T_BOOL MediaLib_Dmx_CheckVideoEnd(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get video data size with stream header and the first key frame data
 *
 * @author Su_Dan
 * @param	hMedia	[in]		pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_U32
 * @retval	0		no more video data
 * @retval	other	video data size
 */
T_U32 MediaLib_Dmx_GetFirstVideoSize(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get video data with stream header and the first key frame data
 *
 * @author Su_Dan
 * @param	hMedia		[in]		pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	pData		[out]		video data
 * @param	pDataLen	[in/out]	data size, returned by MediaLib_Dmx_GetFirstVideoSize
 * @return T_BOOL
 * @retval	AK_TRUE		Get ok
 * @retval	AK_FALSE	Get fail
 */
T_BOOL MediaLib_Dmx_GetFirstVideo(T_MEDIALIB_STRUCT hMedia, T_pDATA pData, T_U32 *pDataLen);


/**
 * @brief Get video size for checking the space
 *
 * @author Su_Dan
 * @param	hMedia	[in]		pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_U32
 * @retval	0		no more video data
 * @retval	other	video data size
 */
T_U32 MediaLib_Dmx_GetVideoFrameSize(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get video data, send video data to pData, send video data length is pDataLen
 *
 * @author Su_Dan
 * @param	hMedia		[in]		pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	pData		[out]		video data
 * @param	pDataLen	[in/out]	data size, returned by MediaLib_Dmx_GetVideoDataSize
 * @return T_BOOL
 * @retval	AK_FALSE	no more video data or get error
 * @retval	AK_TRUE		get ok
 */
T_BOOL MediaLib_Dmx_GetVideoFrame(T_MEDIALIB_STRUCT hMedia, T_pDATA pData, T_U32 *pDataLen);


/**
 * @brief Disable reading video stream
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval 	AK_FALSE	fail
 * @retval 	AK_TRUE		ok
 */
T_BOOL MediaLib_Dmx_DisableVideo(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Disable reading audio stream
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @return T_BOOL
 * @retval 	AK_FALSE	fail
 * @retval 	AK_TRUE		ok
 */
T_BOOL MediaLib_Dmx_DisableAudio(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get next block information
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	dmxBlockInfo	[out]	indicate type and length of the next block, see T_MEDIALIB_DMX_BLKINFO struct
 * @return T_BOOL
 * @retval	AK_TRUE		Get ok
 * @retval	AK_FALSE	Get fail
 */
T_BOOL MediaLib_Dmx_GetNextBlockInfo(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_DMX_BLKINFO *dmxBlockInfo);


/**
 * @brief Switch to fast forward or fast rewind status
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Dmx_Open function
 * @param	video_pts	[in]	current video timestamp for fast forward or rewind
 * @param	dmx_status	[in]	MEDIALIB_DMX_FF: FastForword, MEDIALIB_DMX_FR: Fast Rewind
 * @return T_BOOL
 * @retval	AK_TRUE		switch ok
 * @retval	AK_FALSE	switch fail
 */
T_BOOL MediaLib_Dmx_FF_FR(T_MEDIALIB_STRUCT hMedia, T_U32 video_pts, T_eMEDIALIB_DMX_STATUS dmx_status);

#ifdef __cplusplus
}
#endif

#endif//_MEDIA_DEMUXER_LIB_H_
