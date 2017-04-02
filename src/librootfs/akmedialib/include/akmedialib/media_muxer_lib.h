/**
 * @file media_muxer_lib.h
 * @brief This file provides AVI/3GP muxing functions
 * these APIs may cooperate with APIs in video codec and audio codec
 *
 * Copyright (C) 2012 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Su Dan, Xia Jiaquan
 * @date 2012-8-20
 * @update date 2012-9-4
 * @version 0.1.0
 * @version ever green group version: x.x
 * @note
	The following is an example to use muxing APIs
   @code

int media_muxer(char *filename)
{
	T_S32 fid;
	int count;
	T_MEDIALIB_MUX_OPEN_INPUT mux_open_input;
	T_MEDIALIB_MUX_OPEN_OUTPUT mux_open_output;
	T_eMEDIALIB_MUX_STATUS mux_status;
	char press_key;
	int i;
	T_MEDIALIB_MUX_INFO MuxInfo;
	T_VOID *hMedia;
	T_S32 audio_bytes = 0;
	T_S32 video_bytes = 0;
	T_S32 index_fid;
	T_S32 loop_times = 0;
	T_MEDIALIB_MUX_PARAM mux_param;
	T_U32 audio_pos = 0;
	char mux_filename[100];

	fid = FileOpen(filename);
	if(fid <= 0)
	{
		printf("open file failed\r\n");
		return 0;
	}

	index_fid = FileOpen(index_filename);

	memset(&mux_open_input, 0, sizeof(T_MEDIALIB_MUX_OPEN_INPUT));
	mux_open_input.m_MediaRecType	= MEDIALIB_REC_AVI_NORMAL;
	mux_open_input.m_hMediaDest		= fid;
	mux_open_input.m_bCaptureAudio	= 1;
	mux_open_input.m_bNeedSYN		= AK_TRUE;
	mux_open_input.m_bLocalMode		= AK_FALSE;
	mux_open_input.m_bIdxInMem		= AK_FALSE;
	mux_open_input.m_ulIndexMemSize	= 0;
	mux_open_input.m_hIndexFile		= index_fid;

	//for syn
	mux_open_input.m_ulVFifoSize	= 200*1024; //video fifo size
	mux_open_input.m_ulAFifoSize	= 100*1024; //audio fifo size
	mux_open_input.m_ulTimeScale	= 1000;		//time scale

// set video open info
	mux_open_input.m_eVideoType		= MEDIALIB_VIDEO_MJPEG;
	mux_open_input.m_nWidth			= 176;
	mux_open_input.m_nHeight		= 144;
	mux_open_input.m_nFPS			= 10;
	mux_open_input.m_nKeyframeInterval	= 19;
	
// set audio open info
	mux_open_input.m_eAudioType			= MEDIALIB_AUDIO_PCM;
	mux_open_input.m_ulAudioBitrate		= 16000*8;
	mux_open_input.m_ulSamplesPerPack	= 4000;
	mux_open_input.m_nSampleRate		= 8000;
	mux_open_input.m_nChannels			= 1;
	mux_open_input.m_wBitsPerSample		= 16;

	mux_open_input.m_CBFunc.m_FunPrintf	= (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	mux_open_input.m_CBFunc.m_FunMalloc	= (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	mux_open_input.m_CBFunc.m_FunFree	= (MEDIALIB_CALLBACK_FUN_FREE)free;
	mux_open_input.m_CBFunc.m_FunRead	= (MEDIALIB_CALLBACK_FUN_READ)_read;
	mux_open_input.m_CBFunc.m_FunSeek	= (MEDIALIB_CALLBACK_FUN_SEEK)_lseek;
	mux_open_input.m_CBFunc.m_FunTell	= (MEDIALIB_CALLBACK_FUN_TELL)_tell;
	mux_open_input.m_CBFunc.m_FunWrite	= (MEDIALIB_CALLBACK_FUN_WRITE)_write;
	mux_open_input.m_CBFunc.m_FunFileHandleExist = file_handle_exist;

	hMedia = MediaLib_Mux_Open(&mux_open_input, &mux_open_output);
	if (AK_NULL == hMedia)
	{
		FileClose(fid);
		FileClose(index_fid);
		return 0;
	}

	if (MediaLib_Mux_GetInfo(hMedia, &MuxInfo) == AK_FALSE)
	{
		MediaLib_Mux_Close(hMedia);
		FileClose(fid);
		FileClose(index_fid);
		return 0;
	}

	if (AK_FALSE == MediaLib_Mux_Start(hMedia))
	{
		MediaLib_Mux_Close(hMedia);
		FileClose(fid);
		FileClose(index_fid);
		return 0;
	}

mux_loop:
	while(1)
	{
		if (mux_open_input.m_bCaptureAudio)
		{
			//get audio data from audio encoder
			audio_tytes = get_audio_data(&mux_param);
			if (audio_tytes ! = 0)
			{
				if(MediaLib_Mux_AddAudioData(hMedia, &mux_param) == AK_FALSE)
				{
					printf("MediaLib_Mux_AddAudioData error\r\n");
					break;
				}
				mux_status = MediaLib_Mux_Handle(hMedia);
			}
		}
		tickcount = get_system_time_ms();//get current time in ms from starting recording
		video_tytes = get_video_data(&mux_param);
		if (video_tytes != 0)
		{
			if (MediaLib_Mux_AddVideoData(hMedia, &mux_param) == AK_FALSE)
			{
				printf("MediaLib_Mux_AddVideoData error\r\n");
				break;
			}

			mux_status = MediaLib_Mux_Handle(hMedia);
		}

		press_key = is_stop_button();//check whether stop
		if (press_key)
		{
			break;
		}
		mux_status = MediaLib_Mux_GetStatus(hMedia);
		if (MEDIALIB_MUX_DOING != mux_status)
		{
			break;
		}
	}

	MediaLib_Mux_Stop(hMedia);
	FileClose(fid);

	if (continue_rec)//record to another file
	{
		fid = FileOpen(filename_new);
		if(fid <= 0)
		{
			printf("open file failed\r\n");
		}
		else
		{
			if (MediaLib_Mux_Restart(hMedia, fid))
			{
				goto mux_loop;
			}
		}
	}

	MediaLib_Mux_Close(hMedia);
	FileClose(index_fid);
}

	@endcode

 ***************************************************/
#ifndef _MEDIA_MUXER_LIB_H_
#define _MEDIA_MUXER_LIB_H_

#include "medialib_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	MEDIALIB_MUX_OPEN,
	MEDIALIB_MUX_STOP,
	MEDIALIB_MUX_DOING,
	MEDIALIB_MUX_SYSERR,
	MEDIALIB_MUX_MEMFULL,
	MEDIALIB_MUX_SYNERR,
	MEDIALIB_MUX_WAITING
}T_eMEDIALIB_MUX_STATUS;

typedef struct
{
	T_eMEDIALIB_REC_TYPE	m_MediaRecType;
	T_S32					m_hMediaDest;
	T_MEDIALIB_CB			m_CBFunc;

	T_BOOL					m_bCaptureAudio;
	T_BOOL					m_bNeedSYN;
	T_BOOL					m_bLocalMode;	//whether the video and audio data are from local device
	T_BOOL					m_bIdxInMem;	//flag indicating Index is saved in memory

	//for index
	T_U32					m_ulIndexMemSize;	//index size set by system
	T_S32					m_hIndexFile;		//for index file

	//for syn
	T_U32					m_ulVFifoSize;	//video fifo size
	T_U32					m_ulAFifoSize;	//audio fifo size

	T_U32					m_ulTimeScale;	//time scale

	//video
	T_eMEDIALIB_VIDEO_CODE	m_eVideoType;
	T_U16	m_nWidth;
	T_U16	m_nHeight;
	T_U16	m_nFPS;
	T_U16	m_nKeyframeInterval;			//关键帧间隔，该参数在合成库中不再使用

	//audio
	T_eMEDIALIB_AUDIO_CODE	m_eAudioType;
	T_U32	m_ulAudioBitrate;	//音频比特率
	T_U32	m_ulSamplesPerPack;	//每个音频包的采样点数
	T_U32	m_nSampleRate;		//采样率(如: 8000)
	T_U16	m_nChannels;		//立体声(2)、单声道(1)
	T_U16	m_wBitsPerSample;	//ADPCM时为采样点压缩后的比特数(常用的为4)，其他为16 bit固定(16)
	//以下对于pcm和amr可以不赋值
	T_U16	m_wFormatTag;		//audio format tag
	T_U32	m_nAvgBytesPerSec;
	T_U16	m_nBlockAlign;
	T_U16	m_cbSize;
	T_U8	*m_pszData;
}T_MEDIALIB_MUX_OPEN_INPUT;

typedef struct
{	
	T_eMEDIALIB_STATE		m_State;
}T_MEDIALIB_MUX_OPEN_OUTPUT;

typedef struct
{
	T_pDATA	m_pStreamBuf;
	T_U32	m_ulStreamLen;
	T_U32	m_ulTimeStamp;
	T_BOOL	m_bIsKeyframe;	//for video
}T_MEDIALIB_MUX_PARAM;

typedef struct
{
	//dynamic
	T_eMEDIALIB_MUX_STATUS m_MuxStatus;
	T_U32	m_ulTotalFrames;		//total frames, include video frames and audio packets
	T_U32	m_ulVideoFrames;		//video frames
	T_U32	m_ulAudioPackets;		//audio packets
	T_U32	m_ulInfoBytes;			//expect free space, to save header or index
	T_U32	m_ulFileBytes;			//current file size
	T_U32	m_ulTotalTime_ms;		//record time in millisecond
}T_MEDIALIB_MUX_INFO;


/**
 * @brief Open Muxer
 *
 * @author Su_Dan
 * @param	mux_open_input		[in]	pointer of T_MEDIALIB_MUX_OPEN_INPUT struct
 * @param	mux_open_output		[out]	pointer of T_MEDIALIB_MUX_OPEN_OUTPUT struct
 * @return T_MEDIALIB_STRUCT
 * @retval	AK_NULL			open failed
 * @retval	other			open ok
 */
T_MEDIALIB_STRUCT MediaLib_Mux_Open(T_MEDIALIB_MUX_OPEN_INPUT *mux_open_input, T_MEDIALIB_MUX_OPEN_OUTPUT *mux_open_output);


/**
 * @brief Close Muxer
 *
 * @author Su_Dan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Close ok
 * @retval	AK_FALSE	Close fail
 */
T_BOOL MediaLib_Mux_Close(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Start muxing
 *
 * @author Xia_Jiaquan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Start ok
 * @retval	AK_FALSE	Start fail
 */
T_BOOL MediaLib_Mux_Start(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Restart muxing
 *
 * @author Xia_Jiaquan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @param	hFile		[in]	file handle
 * @return T_BOOL
 * @retval	AK_TRUE		Restart ok
 * @retval	AK_FALSE	Restart fail
 */
T_BOOL MediaLib_Mux_Restart(T_MEDIALIB_STRUCT hMedia, T_S32 hFile);


/**
 * @brief Stop muxing
 *
 * @author Xia_Jiaquan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Stop ok
 * @retval	AK_FALSE	Stop fail
 */
T_BOOL MediaLib_Mux_Stop(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get information
 *
 * @author Xia_Jiaquan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @param	pInfo		[out]	pointer of T_MEDIALIB_MUX_INFO struct
 * @return T_BOOL
 * @retval	AK_FALSE	get information fail
 * @retval	AK_TRUE		get information ok
 */
T_BOOL MediaLib_Mux_GetInfo(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_MUX_INFO *pInfo);


/**
 * @brief Get current muxing status
 *
 * @author Xia_Jiaquan
 * @param	hMedia	[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @return T_eMEDIALIB_MUX_STATUS 
 * @retval	see struct T_eMEDIALIB_MUX_STATUS
 */
T_eMEDIALIB_MUX_STATUS MediaLib_Mux_GetStatus(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief add audio data
 *
 * @author Xia_Jiaquan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @param	pMuxPar		[in]	pointer of T_MEDIALIB_MUX_PARAM struct
 * @return T_S32
 * @retval	0	Add fail
 * @retval	1	Add ok
 */
T_S32 MediaLib_Mux_AddAudioData(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_MUX_PARAM *pMuxPar);

/**
 * @brief add video data
 *
 * @author Xia_Jiaquan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @param	pMuxPar		[in]	pointer of T_MEDIALIB_MUX_PARAM struct
 * @return T_S32
 * @retval	0	Add fail
 * @retval	1	Add ok
 */
T_S32 MediaLib_Mux_AddVideoData(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_MUX_PARAM *pMuxPar);


/**
 * @brief process video and audio data
 *
 * @author Xia_Jiaquan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Mux_Open function
 * @return T_eMEDIALIB_MUX_STATUS
 */
T_eMEDIALIB_MUX_STATUS MediaLib_Mux_Handle(T_MEDIALIB_STRUCT hMedia);

#ifdef __cplusplus
}
#endif

#endif//_MEDIA_MUXER_LIB_H_
