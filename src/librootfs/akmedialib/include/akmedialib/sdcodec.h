/**
* @file	sdcodec.h
* @brief	Anyka Sound Device Module interfaces header file.
*
* This file declare Anyka Sound Device Module interfaces.\n
* Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
* @author	Deng Zhou
* @date	2013-08-01
* @version V0.0.1
* @ref
*/

#ifndef __SOUND_DEVICE_CODEC_H__
#define __SOUND_DEVICE_CODEC_H__

#include "medialib_global.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup AUDIOLIB Audio library
 * @ingroup ENG
 */
/*@{*/


/* @{@name Define audio version*/
/**	Use this to define version string */	
#define AUDIOCODEC_VERSION_STRING		(T_U8 *)"AudioCodec Version V1.9.19"
/** @} */

#ifdef _WIN32
#ifndef UNDER_CE
#define _SD_MODULE_MIDI_SUPPORT
#define _SD_MODULE_MP3_SUPPORT
#define _SD_MODULE_ENC_MP3_SUPPORT
#define _SD_MODULE_WMA_SUPPORT
#define _SD_MODULE_APE_SUPPORT
#define _SD_MODULE_FLAC_SUPPORT
#define _SD_MODULE_PCM_SUPPORT
#define _SD_MODULE_ADPCM_SUPPORT
#define _SD_MODULE_ENC_ADPCM_SUPPORT
#define _SD_MODULE_AAC_SUPPORT
#define _SD_MODULE_OGG_VORBIS_SUPPORT
#define _SD_MODULE_AMR_SUPPORT
#define _SD_MODULE_AMR_ENC_SUPPORT
#define _SD_MODULE_ENC_AAC_SUPPORT
#define _SD_MODULE_RA8LBR_SUPPORT
#define _SD_MODULE_DRA_SUPPORT
#define _SD_MODULE_AC3_SUPPORT
#define _SD_MODULE_G711_SUPPORT
#define _SD_MODULE_G711_ENC_SUPPORT
#define _SD_MODULE_SBC_SUPPORT
#define _SD_MODULE_SBC_ENC_SUPPORT
#define _SD_MODULE_SPEEX_SUPPORT
#define _SD_MODULE_SPEEX_ENC_SUPPORT

#else 
// for CE
#define _SD_MODULE_MIDI_SUPPORT
#define _SD_MODULE_MP3_SUPPORT
#define _SD_MODULE_ENC_MP3_SUPPORT
#define _SD_MODULE_WMA_SUPPORT
#define _SD_MODULE_APE_SUPPORT
#define _SD_MODULE_FLAC_SUPPORT
#define _SD_MODULE_PCM_SUPPORT
#define _SD_MODULE_ADPCM_SUPPORT
#define _SD_MODULE_ENC_ADPCM_SUPPORT
#define _SD_MODULE_AAC_SUPPORT
#define _SD_MODULE_OGG_VORBIS_SUPPORT
#define _SD_MODULE_AMR_SUPPORT
#define _SD_MODULE_AMR_ENC_SUPPORT
#define _SD_MODULE_ENC_AAC_SUPPORT
#define _SD_MODULE_RA8LBR_SUPPORT
#define _SD_MODULE_DRA_SUPPORT
#define _SD_MODULE_AC3_SUPPORT
#define _SD_MODULE_G711_SUPPORT
#define _SD_MODULE_G711_ENC_SUPPORT
#endif /* #ifndef UNDER_CE */

#endif /* #ifdef _WIN32 */

#if defined ANDROID
#if defined AIMER39_PLAT
#define _SD_MODULE_PCM_SUPPORT
#define _SD_MODULE_ADPCM_SUPPORT
#define _SD_MODULE_ENC_ADPCM_SUPPORT
#define _SD_MODULE_AAC_SUPPORT
#define _SD_MODULE_ENC_AAC_SUPPORT
#define _SD_MODULE_G711_SUPPORT
#define _SD_MODULE_G711_ENC_SUPPORT
#define _SD_MODULE_AMR_SUPPORT
#define _SD_MODULE_AMR_ENC_SUPPORT

#else /* #if defined AIMER39_PLAT */
#define _SD_MODULE_MIDI_SUPPORT
#define _SD_MODULE_MP3_SUPPORT
#define _SD_MODULE_ENC_MP3_SUPPORT  
#define _SD_MODULE_WMA_SUPPORT
#define _SD_MODULE_APE_SUPPORT
#define _SD_MODULE_FLAC_SUPPORT
#define _SD_MODULE_PCM_SUPPORT
#define _SD_MODULE_ADPCM_SUPPORT
#define _SD_MODULE_ENC_ADPCM_SUPPORT
#define _SD_MODULE_AAC_SUPPORT
#define _SD_MODULE_OGG_VORBIS_SUPPORT
#define _SD_MODULE_AMR_SUPPORT
#define _SD_MODULE_AMR_ENC_SUPPORT
#define _SD_MODULE_ENC_AAC_SUPPORT
#define _SD_MODULE_RA8LBR_SUPPORT
//#define _SD_MODULE_DRA_SUPPORT
#define _SD_MODULE_AC3_SUPPORT
#define _SD_MODULE_G711_SUPPORT
#define _SD_MODULE_G711_ENC_SUPPORT
// #define _SD_MODULE_SPEEX_SUPPORT
// #define _SD_MODULE_SPEEX_ENC_SUPPORT
#endif /* #if defined AIMER39_PLAT */
#endif /* #if defined ANDROID */

#ifdef FOR_SPOTLIGHT
#define THRIFT_STACK_SPACE
#endif

typedef enum
{
	_SD_MEDIA_TYPE_UNKNOWN ,
	_SD_MEDIA_TYPE_MIDI ,
	_SD_MEDIA_TYPE_MP3 ,
	_SD_MEDIA_TYPE_AMR ,
	_SD_MEDIA_TYPE_AAC ,
	_SD_MEDIA_TYPE_WMA ,
	_SD_MEDIA_TYPE_PCM ,
	_SD_MEDIA_TYPE_ADPCM_IMA ,
	_SD_MEDIA_TYPE_ADPCM_MS ,
	_SD_MEDIA_TYPE_ADPCM_FLASH ,
	_SD_MEDIA_TYPE_APE ,
	_SD_MEDIA_TYPE_FLAC ,
	_SD_MEDIA_TYPE_OGG_FLAC ,
	_SD_MEDIA_TYPE_RA8LBR ,
	_SD_MEDIA_TYPE_DRA,
	_SD_MEDIA_TYPE_OGG_VORBIS,
	_SD_MEDIA_TYPE_AC3,
	_SD_MEDIA_TYPE_PCM_ALAW,
	_SD_MEDIA_TYPE_PCM_ULAW,
	_SD_MEDIA_TYPE_SBC,
	_SD_MEDIA_TYPE_SPEEX
}T_AUDIO_TYPE;

typedef enum
{
	_SD_BUFFER_FULL ,
	_SD_BUFFER_WRITABLE ,
	_SD_BUFFER_WRITABLE_TWICE ,
	_SD_BUFFER_ERROR
}T_AUDIO_BUF_STATE;

typedef enum
{
	_STREAM_BUF_LEN = 0,
	_STREAM_BUF_REMAIN_DATA,
	_STREAM_BUF_MIN_LEN
}T_AUDIO_INBUF_STATE;

typedef enum
{
    _SD_ENC_SAVE_FRAME_HEAD = 0,
    _SD_ENC_CUT_FRAME_HEAD  = 1
}T_AUDIO_ENC_FRMHEAD_STATE;

typedef struct
{
	MEDIALIB_CALLBACK_FUN_MALLOC			Malloc;
	MEDIALIB_CALLBACK_FUN_FREE				Free;
	MEDIALIB_CALLBACK_FUN_PRINTF			printf;
	MEDIALIB_CALLBACK_FUN_RTC_DELAY			delay;
	MEDIALIB_CALLBACK_FUN_CMMBSYNCTIME		cmmbsynctime;
	MEDIALIB_CALLBACK_FUN_CMMBAUDIORECDATA  cmmbaudiorecdata;
#ifdef FOR_SPOTLIGHT
	MEDIALIB_CALLBACK_FUN_READ				read;
	MEDIALIB_CALLBACK_FUN_SEEK				seek; 
	MEDIALIB_CALLBACK_FUN_TELL				tell;
#endif
}T_AUDIO_CB_FUNS;

typedef struct
{
#ifdef FOR_SPOTLIGHT
	T_S32	haudio;
#endif
	T_U32	m_Type;				//media type
	T_U32	m_SampleRate;		//sample rate, sample per second
	T_U16	m_Channels;			//channel number
	T_U16	m_BitsPerSample;	//bits per sample


	
	T_U32   m_InbufLen;         //input buffer length
	T_U8    *m_szData; 
	T_U32   m_szDataLen;
	union {
		struct
		{
			T_U32	cmmb_adts_flag;
		}m_aac;
		struct  
		{
			T_U32	nFileSize;
		} m_midi;
	}m_Private;
    /*
    为了在给平台更新库的时候，头文件不用修改，这里改成FOR_SPOTLIGHT 
    因为spotlight平台上sbc解码是平台直接调用音频库，非sbc解码平台调用媒体库；
    而spotlight平台上没有音量处理，所以sbc解码时需要增加如下的两个变量；
    而对于非spotlight平台，平台本来就是直接调用音频库解码，平台上也已经有音量处理，不需要音频库再做音量处理。
    */
#ifdef FOR_SPOTLIGHT //BLUETOOTH_PLAY 
    /* 下面两个变量是给蓝牙平台使用的 */
    T_U32  decVolEna;   // decode volume enable::  1: 音频库里解码的时候做音量控制,   0:音频库里解码的时候不做音量控制
	T_U32  decVolume;   // decode volume value::   this volume is effective, when decVolCtl==1
#endif
}T_AUDIO_IN_INFO;

typedef struct
{
	T_AUDIO_CB_FUNS		cb_fun;
	T_AUDIO_IN_INFO		m_info;
}T_AUDIO_DECODE_INPUT;

typedef struct
{
#if defined ANDROID 
	volatile T_U8	*pwrite;	//pointer of write pos
	T_U32	free_len;	//buffer free length
	volatile T_U8	*pstart;	//buffer start address
	T_U32	start_len;	//start free length
#else
  T_U8	*pwrite;	//pointer of write pos
	T_U32	free_len;	//buffer free length
  T_U8	*pstart;	//buffer start address
	T_U32	start_len;	//start free length
#endif
}T_AUDIO_BUFFER_CONTROL;


typedef enum{ AMR_ENC_MR475 = 0,
			AMR_ENC_MR515,
			AMR_ENC_MR59,
			AMR_ENC_MR67,
			AMR_ENC_MR74,
			AMR_ENC_MR795,
			AMR_ENC_MR102,
			AMR_ENC_MR122,

			AMR_ENC_MRDTX,

			AMR_ENC_N_MODES	/* number of (SPC) modes */

			} T_AUDIO_AMR_ENCODE_MODE ;


typedef struct
{
	T_U32	m_Type;			//media type
	T_U16	m_nChannel;		//立体声(2)、单声道(1)
	T_U16	m_BitsPerSample;//16 bit固定(16)
	T_U32	m_nSampleRate;	//采样率(8000)
	union{
		struct{
			T_AUDIO_AMR_ENCODE_MODE mode;
		}m_amr_enc;
		struct{
			T_U32 enc_bits;
		}m_adpcm;
		struct{
			T_U32 bitrate;
			T_BOOL mono_from_stereo;
		}m_mp3;
		struct{
			T_U32 bitrate;   
			T_BOOL cbr;
			T_BOOL dtx_disable;
			char *comments[64];
		}m_speex;
	}m_private;
	T_U32 encEndFlag;
}T_AUDIO_ENC_IN_INFO;

typedef struct
{
	T_U16	wFormatTag;
	T_U16	nChannels;
	T_U32	nSamplesPerSec;

	union {
		struct {
			T_U32	nAvgBytesPerSec;
			T_U16	nBlockAlign;
			T_U16	wBitsPerSample;
			T_U16	nSamplesPerPacket;
		} m_adpcm;
	}m_Private;
	
}T_AUDIO_ENC_OUT_INFO;

typedef struct
{
	T_VOID *buf_in;
	T_VOID *buf_out;
	T_U32 len_in;
	T_U32 len_out;
}T_AUDIO_ENC_BUF_STRC;

typedef struct
{
	T_AUDIO_CB_FUNS		cb_fun;
	T_AUDIO_ENC_IN_INFO		enc_in_info;
}T_AUDIO_REC_INPUT;

typedef enum 
{
	_SD_BM_NORMAL = 0,
	_SD_BM_ENDING = 1,
    _SD_BM_LIVE = 1
} T_AUDIO_BUFFER_MODE;


/**
 * @brief	获取编解码库版本信息.
 * @author	Deng Zhou
 * @date	2008-04-21
 * @param	[in] T_VOID
 * @return	T_S8 *
 * @retval	返回库版本号
 */
T_S8 *_SD_GetAudioCodecVersionInfo(void);

/**
 * @brief	设置解码句柄，将其传给解码器，以便在调用回调使用
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_input:
 * 播放信息的输入结构
 * @param	[in] T_VOID *pHandle:
 * 传入的句柄
 * @return	T_VOID *
 * @retval	返回音频库内部解码结构的指针，空表示失败
 */
T_VOID _SD_SetHandle(T_VOID *audio_decode, T_VOID *pHandle);
/**
 * @brief	打开音频播放设备.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_input:
 * 播放信息的输入结构
 * @param	[in] audio_output:
 * 要求pcm的输出结构
 * @return	T_VOID *
 * @retval	返回音频库内部解码结构的指针，空表示失败
 */
T_VOID *_SD_Decode_Open(T_AUDIO_DECODE_INPUT *audio_input, T_AUDIO_DECODE_OUT *audio_output);

/**
 * @brief	音频解码.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @param	[in] audio_output:
 * 要求pcm的输出结构
 * @return	T_S32
 * @retval	返回音频库解码出的音频数据大小，以byte为单位
 */
T_S32 _SD_Decode(T_VOID *audio_decode, T_AUDIO_DECODE_OUT *audio_output);

/**
 * @brief	关闭音频解码设备.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @return	T_S32
 * @retval	AK_TRUE :  关闭成功
 * @retval	AK_FLASE :  关闭异常
 */
T_S32 _SD_Decode_Close(T_VOID *audio_decode);

/**
 * @brief	音频解码seek.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @return	T_S32
 * @retval	AK_TRUE :  seek成功
 * @retval	AK_FLASE :  seek异常
 */
T_S32 _SD_Decode_Seek(T_VOID *audio_decode, T_AUDIO_SEEK_INFO *seek_info);

// #ifdef BLUETOOTH_PLAY
/**
 * @brief	set digital volume
 * @author	Tang Xuechai
 * @date    2012-02-29
 * @param	[in] audio_decode: 音频解码库内部解码保存结构
 * @param   [in] volume: 目标音量值
 * @return	T_S32
 * @retval	AK_TRUE :  set volume sucess
 * @retval	AK_FLASE :	set volume fail
 */
T_S32 _SD_Decode_SetDigVolume(T_VOID *audio_decode, T_U32 volume);

/**
 * @brief	decode one packet data
 * @author	Tang Xuechai
 * @date    2012-02-30
 * @param	[in] audio_decode: decode struct, get from _SD_Decode_Open
 * @param   [in] in: in data stream
 * @param   [in] isize: in data stream length
 * @param   [in/out] audio_output: output information and pcm
 * @return	T_S32
 * @retval	<=0 : decode error
 * @retval	>0 :  output pcm size (byte)
 */
//T_S32 _SD_Decode_OnePacket(T_VOID *audio_decode, T_U8 *in, T_U32 isize, T_AUDIO_DECODE_OUT *audio_output);
//#endif

/**
 * @brief	设置解码缓冲最小延迟长度.
 * @author	Tang Xuechai
 * @date	      2012-4-20
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @param	[in] len:
 * 目标缓冲延长长度
 * @return	
 */
T_U32 _SD_SetInbufMinLen(T_VOID *audio_decode, T_U32 len);

/**
 * @brief	设置解码缓冲工作模式.
 * @author	Deng Zhou
 * @date	2009-8-7
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @param	[in] bufmode:
 * 缓冲工作模式
 * @return	
 */
T_S32 _SD_SetBufferMode(T_VOID *audio_decode, T_AUDIO_BUFFER_MODE buf_mode);

/**
 * @brief	获取wma比特率类型，LPC/Mid/High rate三种
 * @author	Li Jun
 * @date	2010-1-14
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @return  返回比特率类型，0/1/2分别对应LPC/Mid/High rate	
 */
T_S32 _SD_GetWMABitrateType(T_VOID *audio_decode);

/**
 * @brief	检测音频播放内部缓冲区free空间大小.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @param	[in] buffer_control:
 * 音频播放内部缓冲区状态结构
 * @return	T_AUDIO_BUF_STATE
 * @retval	缓冲区状态
 */
T_AUDIO_BUF_STATE _SD_Buffer_Check(T_VOID *audio_decode, T_AUDIO_BUFFER_CONTROL *buffer_control);

/**
 * @brief	更新音频播放内部缓冲区写指针.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @param	[in] len:
 * 向音频播放内部缓冲区写入长度
 * @return	T_S32
 * @retval	AK_TRUE : 更新成功
 * @retval	AK_FLASE : 更新失败
 */
T_S32 _SD_Buffer_Update(T_VOID *audio_decode, T_U32 len);

/**
 * @brief	清空音频播放内部缓冲区.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @return	T_S32
 * @retval	AK_TRUE : 清除成功
 * @retval	AK_FLASE : 清除失败
 */
T_S32 _SD_Buffer_Clear(T_VOID *audio_decode);

/**
 * @brief	打开录音设备.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] enc_input:
 * 录音输入信息结构
 * @param	[in] enc_output:
 * 录音输出信息结构
 * @return	T_VOID *
 * @retval	录音音频内部结构指针，为空表示打开失败
 */
T_VOID *_SD_Encode_Open(T_AUDIO_REC_INPUT *enc_input, T_AUDIO_ENC_OUT_INFO *enc_output);

/**
 * @brief	对录好的pcm数据进行编码.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_record:
 * 录音音频库内部结构
 * @param	[in] enc_buf_strc:
 * 输入输出buffer指针长度结构
 * @return	T_S32 
 * @retval	编码长度
 */
T_S32 _SD_Encode(T_VOID *audio_encode, T_AUDIO_ENC_BUF_STRC *enc_buf_strc);

/**
 * @brief	关闭录音设备.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_encode:
 * 录音音频库内部结构
 * @return	T_S32 
 * @retval	AK_TRUE : 关闭成功
 * @retval	AK_FALSE : 关闭失败
 */
T_S32 _SD_Encode_Close(T_VOID *audio_encode);

/**
 * @brief	获取编解码时间.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_codec:
 * 编解码库内部结构
 * @param	[in] codec_flag:
 * 编解码标志 0：解码 1：编码
 * @return	T_S32 
 * @retval	获取的时间
 */
T_S32 _SD_GetCodecTime(T_VOID *audio_codec, T_U8 codec_flag);


/**
 * @brief	获取当前解码缓冲中的码流供分析.
 * @author	Li Jun
 * @date	2007-10-15
 * @param	[in] audio_codec:
 * 编解码库内部结构
 * @param	[in] T_U8 *pBuf:
 * 存储码流的缓冲
 * @param	[in] T_U32 *len:
 * 存储码流的缓冲中码流的长度
 * @return	T_VOID 
 */
T_VOID _SD_LogBufferSave(T_U8 *pBuf, T_U32 *len,T_VOID *audio_codec);

/**
 * @brief	对传进来的时域音频PCM信号, 计算其频谱并原址返回. 
 *          该接口会调用wma解码器中的fft模块,所以在WMA解码模块开启时才能使用
 * @author	Li Jun
 * @date	2011-4-14
 * @param	[in] T_S32 data[]
 * 时域音频PCM数据    
 * @param	[in] T_U16 size
 * 时域音频PCM数据的长度
 * @param	[in] T_AUDIO_CB_FUNS *cbfun
 * 回调函数结构体,需要将malloc,free和printf传进来
 * @return	T_S32 
 * AK_FALSE 由于内存分配失败而回FALSE
 * AK_TRUE  计算频域数据成功,频域数据在data中, 有效长度是size/2
 */
T_S32 _SD_GetAudioSpectrum(T_S32 *data,T_U16 size, T_AUDIO_CB_FUNS *cbfun);

// #if ((defined (NEWWAY_FILL_BUF)) || defined(ANDROID))
/**
 * @brief	获取音频播放内部缓冲区的地址指针.
 * @author	Cheng RongFei
 * @date	2011-7-13
 * @param	[in] audio_decode:
 * 编解码库内部结构
 * @param	[in] len:
 * 需要一次写入buffer的数据长度
 * @return	T_VOID* 
 * @retval	获取buffer的地址指针
 */
T_VOID* _SD_Buffer_GetAddr(T_VOID *audio_decode, T_U32 len);

/**
 * @brief	更新音频播放内部缓冲区写指针.
 * @author	Cheng RongFei
 * @date	2011-7-13
 * @param	[in] audio_decode:
 * 音频解码库内部解码保存结构
 * @return	T_S32
 * @retval	AK_TRUE : 更新成功
 * @retval	AK_FLASE : 更新失败
 */
T_S32 _SD_Buffer_UpdateAddr(T_VOID *audio_decode, T_U32 len);
// #endif

/** 
 * @brief   结束编码
 * @author  Zhou Jiaqing
 * @date   2012-5-16
 * @param  [in] audio_codec:录音音频库内部结构
 *		   [in] enc_buf_strc:输入输出buff
 * @return T_S32
 * @retval 最后编码的数据长度                                           
 */
T_S32 _SD_Encode_Last(T_VOID *audio_encode,T_AUDIO_ENC_BUF_STRC *enc_buf_strc);

/** 
 * @brief   编码时，设置是否返回aac编码帧头数据
 * @author  Tang Xuechai
 * @date   2013-5-20
 * @param  [in] audio_codec:录音音频库内部结构
 *		   [in] flag: T_AUDIO_ENC_FRMHEAD_STATE的枚举之一
 *                    _SD_ENC_SAVE_FRAME_HEAD：返回帧头数据
 *                    _SD_ENC_CUT_FRAME_HEAD：不返回帧头数据，只返回编码的码流数据
 * @return T_S32
 * @retval AK_TRUE: 设置成功  
 *         AK_FALSE: 设置失败
 */
T_S32 _SD_Encode_SetFramHeadFlag(T_VOID *audio_encode, int flag);

/**
 * @brief  持续播放时，切换歌曲时调用
 *		   只限于ogg vorbis连续播放时使用       
 * @date  2012-6-6
 * @param [in] audio_decode :音频库解码内部结构
 * @return T_S32
 * @retval >0: 获取成功  
 *         <0: 获取失败
 *         =0: 输入数据不够，失败
 */
T_S32 _SD_Decode_ParseFHead(T_VOID *audio_decode);

/**
 * @brief 持续播放时，打开播放文件
 *	      只限于ogg vorbis连续播放使用
 * @date 2012-6-6
 * @param [in] audio_input :音频信息输入结构
 *        [in] audio_output:pcm信息输出结构
 * @return T_VOID *
 * @retval 函数调用成功返回T_VOID指针，否则返回AK_NULL
 */
T_VOID *_SD_Decode_Open_Fast(T_AUDIO_DECODE_INPUT *audio_input, T_AUDIO_DECODE_OUT *audio_output);

/**
 * @brief 获取音频库输入buf相关信息
 * @date 2012-7-6
 * @param [in] audio_decode :音频解码结构体
 *		  [in]  type: T_AUDIO_INBUF_STATE 指定需要获取的信息，分别可以取以下值：
 *	 			  _STREAM_BUF_LEN,         函数返回输入buf的buf长度，
 *				  _STREAM_BUF_REMAIN_DATA, 函数返回输入buf中剩余未解码数据的长度，
 *				  _STREAM_BUF_MIN_LEN,     函数返回解码所需最小buf长度				
 * @return T_S32
 * @retval 0:  buffer空，没有剩余数据
 *         >0: buffer中剩余没解码数据的长度
 *         <0: 输入指针非法
 */
T_S32  _SD_Get_Input_Buf_Info(T_VOID *audio_decode,T_AUDIO_INBUF_STATE type);


#ifdef __cplusplus
}
#endif

#endif

/* end of sdcodec.h */

/*@}*/
