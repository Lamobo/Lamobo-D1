/**
 * @file medialib_struct.h
 * @brief Define the global public types for media lib
 *
 * Copyright (C) 2012 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Su_Dan
 * @date 2007-11-22
 * @update date 2012-1-17
 * @version 1.0
 */

#ifndef _MEDIA_LIB_STRUCT_H_
#define _MEDIA_LIB_STRUCT_H_

#include "medialib_global.h"

#define MEDIA_LIB_VERSION	"Media Lib V1.16.04"

//media handle
typedef T_pVOID	T_MEDIALIB_STRUCT;


typedef enum
{
	MEDIALIB_OPEN_PLAY,
	MEDIALIB_OPEN_PREVIEW,			//only preview, not play
	MEDIALIB_OPEN_PLAY_WITHERR,		//play without checking decoder error, not recommended
	MEDIALIB_OPEN_PREVIEW_WITHERR,	//preview without checking decoder error, not recommended
	MEDIALIB_OPEN_GET_INFO
}T_eMEDIALIB_OPEN_TYPE;

typedef enum
{
	MEDIALIB_MEDIA_UNKNOWN,
	MEDIALIB_MEDIA_AKV,
	MEDIALIB_MEDIA_AVI,
	MEDIALIB_MEDIA_WAV,
	MEDIALIB_MEDIA_MP4,
	MEDIALIB_MEDIA_3GP,
	MEDIALIB_MEDIA_M4A,
	MEDIALIB_MEDIA_MOV,
	MEDIALIB_MEDIA_ASF,
	MEDIALIB_MEDIA_REAL,
	MEDIALIB_MEDIA_MP3,
	MEDIALIB_MEDIA_AKV2,	//add below
	MEDIALIB_MEDIA_AAC,
	MEDIALIB_MEDIA_AMR,
	MEDIALIB_MEDIA_MIDI,
	MEDIALIB_MEDIA_FLV,
	MEDIALIB_MEDIA_APE,
	MEDIALIB_MEDIA_FLAC,
	MEDIALIB_MEDIA_OGG_FLAC,
	MEDIALIB_MEDIA_RIFF_AV,
	MEDIALIB_MEDIA_OGG_VORBIS,
	MEDIALIB_MEDIA_MKV,
	MEDIALIB_MEDIA_F4V,
	MEDIALIB_MEDIA_AC3,
	MEDIALIB_MEDIA_SPEEX
}T_eMEDIALIB_MEDIA_TYPE;

typedef struct
{
	MEDIALIB_CALLBACK_FUN_PRINTF				m_FunPrintf;

	MEDIALIB_CALLBACK_FUN_READ					m_FunRead;
	MEDIALIB_CALLBACK_FUN_WRITE					m_FunWrite;
	MEDIALIB_CALLBACK_FUN_SEEK					m_FunSeek;
	MEDIALIB_CALLBACK_FUN_TELL					m_FunTell;
	MEDIALIB_CALLBACK_FUN_MALLOC				m_FunMalloc;
	MEDIALIB_CALLBACK_FUN_FREE					m_FunFree;

	//audio resource, only used in audio lib with dsp
	MEDIALIB_CALLBACK_FUN_LOADRESOURCE			m_FunLoadResource;
	MEDIALIB_CALLBACK_FUN_RELEASERESOURCE		m_FunReleaseResource;

	//system functions
	MEDIALIB_CALLBACK_FUN_RTC_DELAY				m_FunRtcDelay;
	MEDIALIB_CALLBACK_FUN_DMA_MEMCPY			m_FunDMAMemcpy;
	MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE	m_FunMMUInvalidateDCache;
	MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE	m_FunCleanInvalidateDcache;
	MEDIALIB_CALLBACK_FUN_CHECK_DEC_BUF			m_FunCheckDecBuf;
	MEDIALIB_CALLBACK_FUN_FILESYS_ISBUSY		m_FunFileSysIsBusy;

	//add for file handle
	MEDIALIB_CALLBACK_FUN_FILE_HANDLE_EXIST		m_FunFileHandleExist;
	MEDIALIB_CALLBACK_FUN_FILE_GET_LENGTH		m_FunFileGetLength;

	//add for Linux and CE
	MEDIALIB_CALLBACK_FUN_DMA_MALLOC			m_FunDMAMalloc;
	MEDIALIB_CALLBACK_FUN_DMA_FREE				m_FunDMAFree;
	MEDIALIB_CALLBACK_FUN_VADDR_TO_PADDR		m_FunVaddrToPaddr;
	MEDIALIB_CALLBACK_FUN_MAP_ADDR				m_FunMapAddr;
	MEDIALIB_CALLBACK_FUN_UNMAP_ADDR			m_FunUnmapAddr;
	MEDIALIB_CALLBACK_FUN_REG_BITS_WRITE		m_FunRegBitsWrite;

	//add for hardware mutex
	MEDIALIB_CALLBACK_FUN_VIDEO_HW_LOCK			m_FunVideoHWLock;
	MEDIALIB_CALLBACK_FUN_VIDEO_HW_UNLOCK		m_FunVideoHWUnlock;
}T_MEDIALIB_CB;

typedef struct
{
	T_eMEDIALIB_OPEN_TYPE	m_OpenType;
	T_eMEDIALIB_MEDIA_TYPE	m_MediaType;		//media type, input MEDIALIB_MEDIA_UNKNOWN is ok
	T_S32					m_hMediaSource;		//handle of resource, such as file handle
	T_AUDIO_OUT_INFO		m_AudioOutInfo;		//no use, only used in audio lib with dsp
	T_VIDEO_OUT_INFO		m_VideoOutInfo;		//no use
	T_eMEDIALIB_ROTATE		m_Rotate;
	T_U8					m_AudioAttribute;	//no use, only used in audio lib with dsp
	T_BOOL					m_bCapabilityTest;
	T_BOOL					m_bAVDiffTask;		//no use
	T_BOOL					m_bEncryptFile;		//whether file is encrypted by hc
	T_MEDIALIB_CB 			m_CBFunc;
}T_MEDIALIB_OPEN_INPUT;

typedef enum
{
	MEDIALIB_OK,
	MEDIALIB_FORMAT_ERR,
	MEDIALIB_PARAM_ERR,
	MEDIALIB_SYSTEM_ERR,
	MEDIALIB_SUPPORT_ERR
}T_eMEDIALIB_STATE;

typedef struct
{	
	T_eMEDIALIB_STATE		m_State;
	T_U32					m_ulAudioDecBufSize;	//音频译码缓冲区的期望大小
	T_U32					m_ulVideoDecBufSize;	//视频译码缓冲区的期望大小
	T_eMEDIALIB_ROTATE		m_Rotate;
	T_U8					m_AudioAttribute;		//no use, only used in audio lib with dsp
}T_MEDIALIB_OPEN_OUTPUT;

typedef struct
{
	T_BOOL	m_bHasVideo;
	T_BOOL	m_bHasAudio;
}T_MEDIALIB_CHECK_OUTPUT;

typedef struct
{
	T_U32	m_SampleRate;
	T_U16	m_Channels;
	T_U16	m_BitsPerSample;
	T_U32	m_Type;
	T_U32	m_BitRate;
}T_MEDIALIB_AUDIO_INFO;

typedef enum
{
	MEDIALIB_VIDEO_UNKNOWN,
	MEDIALIB_VIDEO_MPEG4,
	MEDIALIB_VIDEO_H263,
	MEDIALIB_VIDEO_WMV,
	MEDIALIB_VIDEO_FLV263,
	MEDIALIB_VIDEO_H264,
	MEDIALIB_VIDEO_RV,
	MEDIALIB_VIDEO_MJPEG,
	MEDIALIB_VIDEO_MPEG2
}T_eMEDIALIB_VIDEO_CODE;

typedef enum
{
	MEDIALIB_AUDIO_UNKNOWN,
	MEDIALIB_AUDIO_AMR,
	MEDIALIB_AUDIO_MP3,
	MEDIALIB_AUDIO_AAC,
	MEDIALIB_AUDIO_PCM,
	MEDIALIB_AUDIO_WMA,
	MEDIALIB_AUDIO_MIDI,
	MEDIALIB_AUDIO_ADPCM,
	MEDIALIB_AUDIO_APE,
	MEDIALIB_AUDIO_FLAC,
	MEDIALIB_AUDIO_RA,
	MEDIALIB_AUDIO_VORBIS,
	MEDIALIB_AUDIO_AC3,
	MEDIALIB_AUDIO_G711,
	MEDIALIB_AUDIO_SPEEX
}T_eMEDIALIB_AUDIO_CODE;

typedef struct _T_MEDIALIB_META_TYPE_INFO
{
	T_U8	VersionType;	//0: unicode; 1: non-unicode
	T_U8	TitleType;
	T_U8	ArtistType;
	T_U8	AlbumType;
	T_U8	YearType;
	T_U8	CommentType;
	T_U8	GenreType;
	T_U8	TrackType;
	T_U8	ComposerType;
}T_MEDIALIB_META_TYPE_INFO;

typedef struct _T_MEDIALIB_META_SIZE_INFO
{
	T_U16	uVersionLen;
	T_U16	uTitleLen;
	T_U16	uArtistLen;
	T_U16	uAlbumLen;
	T_U16	uYearLen;
	T_U16	uCommentLen;
	T_U16	uGenreLen;
	T_U16	uTrackLen;
	T_U16	uComposerLen;
}T_MEDIALIB_META_SIZE_INFO;

typedef struct _MEDIALIB_META_CONTENT
{
	T_VOID	*pVersion;		//metainfo version
	T_VOID	*pTitle;		//Title
	T_VOID	*pArtist;		//Artist
	T_VOID	*pAlbum;		//Album
	T_VOID	*pYear;			//Year
	T_VOID	*pComment;		//Comment
	T_VOID	*pGenre;		//Genre
	T_VOID	*pTrack;		//Track
	T_VOID	*pComposer;		//Composer
}T_MEDIALIB_META_CONTENT;

typedef struct _MEDIALIB_META_INFO
{
	T_MEDIALIB_META_TYPE_INFO	m_MetaTypeInfo;
	T_MEDIALIB_META_SIZE_INFO	m_MetaSizeInfo;
	T_MEDIALIB_META_CONTENT		m_MetaContent;
}T_MEDIALIB_META_INFO;

typedef struct
{
	T_U16	m_uOriWidth;	//not 16 align, in media resource
	T_U16	m_uOriHeight;	//not 16 align, in media resource
	T_U16	m_uWidth;
	T_U16	m_uHeight;
	T_U16	m_uFPS;
	T_U32	m_ulBitrate;
}T_MEDIALIB_VIDEO_INFO;

typedef struct
{
	T_eMEDIALIB_MEDIA_TYPE	m_MediaType;
	T_BOOL	m_bHasVideo;
	T_BOOL	m_bHasAudio;
	T_BOOL	m_bAllowSeek;
	T_U32	m_ulTotalTime_ms;

	T_eMEDIALIB_VIDEO_CODE	m_VideoCode;
	T_eMEDIALIB_AUDIO_CODE	m_AudioCode;
	T_MEDIALIB_VIDEO_INFO	m_VideoInfo;
	T_MEDIALIB_AUDIO_INFO	m_AudioInfo;

	T_MEDIALIB_META_INFO	*m_pMetaInfo;
}T_MEDIALIB_MEDIA_INFO;

//status of player
typedef enum
{
	MEDIALIB_END,
	MEDIALIB_PLAYING,
	MEDIALIB_FF,
	MEDIALIB_FR,
	MEDIALIB_PAUSE,
	MEDIALIB_STOP,
	MEDIALIB_ERR,
	MEDIALIB_SEEK
}T_eMEDIALIB_STATUS;

//for record
typedef enum
{
	MEDIALIB_REC_AVI_NORMAL,
	MEDIALIB_REC_AVI_CYC,
	MEDIALIB_REC_3GP,
	MEDIALIB_REC_AVI_SEGMENT
}T_eMEDIALIB_REC_TYPE;
#define MEDIALIB_REC_MP4	MEDIALIB_REC_3GP

typedef enum
{
	MEDIALIB_V_ENC_H263,
	MEDIALIB_V_ENC_MJPG,
	MEDIALIB_V_ENC_MPEG
}T_eMEDIALIB_V_ENC_TYPE;

//for segment record
typedef enum
{
	MEDIALIB_REC_EV_NORMAL,
	MEDIALIB_REC_EV_EMERGENCY,
	MEDIALIB_REC_EV_NOTHING
}T_eMEDIALIB_REC_EVENT;

typedef struct
{
	T_U32	m_Type;			//media type
	T_U16	m_nChannel;		//立体声(2)、单声道(1)
	T_U16	m_BitsPerSample;//16 bit固定(16)
	T_U32	m_nSampleRate;	//采样率(8000)
	T_U16	m_ulDuration;	//每个音频包持续时间
}T_AUDIO_RECORD_INFO;

typedef struct
{
	T_U16	m_nWidth;
	T_U16	m_nHeight;
	T_U16	m_nFPS;
	T_U16	m_nKeyframeInterval;
	T_U32	m_nvbps;
	T_eMEDIALIB_V_ENC_TYPE	m_eVideoType;
}T_VIDEO_RECORD_INFO;

/**
 * @brief Encode yuv to jpeg or other format
 *
 * @param	srcYUV		[in]		source YUV buffer
 * @param	dstStream	[out]		buffer to fill output stream
 * @param	pSize		[in/out]	in: buffer size; out: size of output stream after encode
 * @param	pic_width	[in]		picture width
 * @param	pic_height	[in]		picture height
 * @param	quality		[in]		quality for compressing
 * @return T_BOOL
 * @retval	AK_TRUE		Encode ok
 * @retval	AK_FALSE	Encode fail
 */
typedef T_BOOL (*MEDIALIB_EXFUNC_YUVENC)(T_U8 *srcYUV, T_U8 *dstStream, T_U32 *pSize,
								T_U32 pic_width, T_U32 pic_height, T_U32 quality);
typedef T_VOID (*MEDIALIB_EXFUNC_ENCTASK)(void);
typedef T_VOID (*MEDIALIB_EXFUNC_SETENCTASK)(const MEDIALIB_EXFUNC_ENCTASK cbFunc);

typedef struct
{
	T_eMEDIALIB_REC_TYPE	m_MediaRecType;
	T_S32					m_hMediaDest;
	T_S32					m_hIndexFile;
	T_AUDIO_RECORD_INFO		m_AudioRecInfo;
//	T_AUDIO_REC_PCM_INFO	m_AudioRecPCMInfo;
	T_VIDEO_RECORD_INFO		m_VideoRecInfo;
	T_MEDIALIB_CB			m_CBFunc;

	T_BOOL	m_bCaptureAudio;
	T_BOOL	m_bIdxInMem;		//flag indicating Index is saved in memory
	T_BOOL	m_bHighQuality;		//for 3gp recording

	T_U32	m_IndexMemSize;		//index size set by system

	T_U16	m_SectorSize;		//add the variable for a sector size of file system, no greater than 8192; eg: 4096

	T_U16	m_RecordSecond;		//add for time limit record

	MEDIALIB_EXFUNC_YUVENC	m_ExFunEnc;	//encode function, such as yuv2jpeg

	MEDIALIB_EXFUNC_SETENCTASK	m_ExFunSetEncTask;	//register the function to encode, implement writing file while encoding

	T_S32	m_hTmpFile;			//add for time limit record
	T_S32	m_Reserved;			//reserved, must be zero
}T_MEDIALIB_REC_OPEN_INPUT;

typedef struct
{	
	T_eMEDIALIB_STATE		m_State;
	T_U32					m_ulAudioEncBufSize;	//音频编码缓冲区的期望大小
	T_U32					m_ulVideoEncBufSize;	//视频编码缓冲区的期望大小
}T_MEDIALIB_REC_OPEN_OUTPUT;

typedef enum
{
	MEDIALIB_REC_OPEN,
	MEDIALIB_REC_STOP,
	MEDIALIB_REC_DOING,
	MEDIALIB_REC_SYSERR,
	MEDIALIB_REC_MEMFULL,
	MEDIALIB_REC_SYNERR
}T_eMEDIALIB_REC_STATUS;

typedef struct
{
	//fix
	T_U16	ori_width;
	T_U16	ori_height;
	T_U16	fps;
	T_U16	keyframeInterval;
	T_BOOL	bCaptureAudio;
	T_U16	record_second;			//add for time limit record

	//dynamic
	T_eMEDIALIB_REC_STATUS record_status;
	T_U32	total_frames;		//total frames, include video frames and audio packets
	T_U32	total_video_frames;	//total video frames
	T_U32	info_bytes;			//expect free space, to save header or index
	T_U32	file_bytes;			//current file size
	T_U32	total_time_ms;		//record time in millisecond
}T_MEDIALIB_REC_INFO;

typedef struct
{
	T_pDATA	pVideoData;
	T_U32	ulVideoLen;

	T_BOOL	bKeyframe;
	T_U32	ulTimestamp;
}T_MEDIALIB_REC_VIDEO_OUT;

//for demux
typedef enum
{
	T_eMEDIALIB_BLKTYPE_UNKNOWN,
	T_eMEDIALIB_BLKTYPE_VIDEO,
	T_eMEDIALIB_BLKTYPE_AUDIO
}T_MEDIALIB_BLKTYPE;

typedef struct
{
	T_MEDIALIB_BLKTYPE	m_eBlkType;
	T_U32				m_ulBlkLen;
}T_MEDIALIB_DMX_BLKINFO;

//解密回调函数和解密参数结构体
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_DECRYPT_INIT)(T_U8 *pData); //128 bytes
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_DECRYPT_DATA)(T_U8 *pData, T_U32 ulDataLen);
typedef struct
{
	MEDIALIB_CALLBACK_FUN_DECRYPT_INIT	m_FunDecryptInit;
	MEDIALIB_CALLBACK_FUN_DECRYPT_DATA	m_FunDecryptData;
}T_MEDIALIB_DECRYPT_PARAM;

#endif//_MEDIA_LIB_STRUCT_H_
