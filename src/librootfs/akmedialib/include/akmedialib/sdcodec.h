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
    Ϊ���ڸ�ƽ̨���¿��ʱ��ͷ�ļ������޸ģ�����ĳ�FOR_SPOTLIGHT 
    ��Ϊspotlightƽ̨��sbc������ƽֱ̨�ӵ�����Ƶ�⣬��sbc����ƽ̨����ý��⣻
    ��spotlightƽ̨��û��������������sbc����ʱ��Ҫ�������µ�����������
    �����ڷ�spotlightƽ̨��ƽ̨��������ֱ�ӵ�����Ƶ����룬ƽ̨��Ҳ�Ѿ���������������Ҫ��Ƶ��������������
    */
#ifdef FOR_SPOTLIGHT //BLUETOOTH_PLAY 
    /* �������������Ǹ�����ƽ̨ʹ�õ� */
    T_U32  decVolEna;   // decode volume enable::  1: ��Ƶ��������ʱ������������,   0:��Ƶ��������ʱ������������
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
	T_U16	m_nChannel;		//������(2)��������(1)
	T_U16	m_BitsPerSample;//16 bit�̶�(16)
	T_U32	m_nSampleRate;	//������(8000)
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
 * @brief	��ȡ������汾��Ϣ.
 * @author	Deng Zhou
 * @date	2008-04-21
 * @param	[in] T_VOID
 * @return	T_S8 *
 * @retval	���ؿ�汾��
 */
T_S8 *_SD_GetAudioCodecVersionInfo(void);

/**
 * @brief	���ý����������䴫�����������Ա��ڵ��ûص�ʹ��
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_input:
 * ������Ϣ������ṹ
 * @param	[in] T_VOID *pHandle:
 * ����ľ��
 * @return	T_VOID *
 * @retval	������Ƶ���ڲ�����ṹ��ָ�룬�ձ�ʾʧ��
 */
T_VOID _SD_SetHandle(T_VOID *audio_decode, T_VOID *pHandle);
/**
 * @brief	����Ƶ�����豸.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_input:
 * ������Ϣ������ṹ
 * @param	[in] audio_output:
 * Ҫ��pcm������ṹ
 * @return	T_VOID *
 * @retval	������Ƶ���ڲ�����ṹ��ָ�룬�ձ�ʾʧ��
 */
T_VOID *_SD_Decode_Open(T_AUDIO_DECODE_INPUT *audio_input, T_AUDIO_DECODE_OUT *audio_output);

/**
 * @brief	��Ƶ����.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @param	[in] audio_output:
 * Ҫ��pcm������ṹ
 * @return	T_S32
 * @retval	������Ƶ����������Ƶ���ݴ�С����byteΪ��λ
 */
T_S32 _SD_Decode(T_VOID *audio_decode, T_AUDIO_DECODE_OUT *audio_output);

/**
 * @brief	�ر���Ƶ�����豸.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @return	T_S32
 * @retval	AK_TRUE :  �رճɹ�
 * @retval	AK_FLASE :  �ر��쳣
 */
T_S32 _SD_Decode_Close(T_VOID *audio_decode);

/**
 * @brief	��Ƶ����seek.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @return	T_S32
 * @retval	AK_TRUE :  seek�ɹ�
 * @retval	AK_FLASE :  seek�쳣
 */
T_S32 _SD_Decode_Seek(T_VOID *audio_decode, T_AUDIO_SEEK_INFO *seek_info);

// #ifdef BLUETOOTH_PLAY
/**
 * @brief	set digital volume
 * @author	Tang Xuechai
 * @date    2012-02-29
 * @param	[in] audio_decode: ��Ƶ������ڲ����뱣��ṹ
 * @param   [in] volume: Ŀ������ֵ
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
 * @brief	���ý��뻺����С�ӳٳ���.
 * @author	Tang Xuechai
 * @date	      2012-4-20
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @param	[in] len:
 * Ŀ�껺���ӳ�����
 * @return	
 */
T_U32 _SD_SetInbufMinLen(T_VOID *audio_decode, T_U32 len);

/**
 * @brief	���ý��뻺�幤��ģʽ.
 * @author	Deng Zhou
 * @date	2009-8-7
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @param	[in] bufmode:
 * ���幤��ģʽ
 * @return	
 */
T_S32 _SD_SetBufferMode(T_VOID *audio_decode, T_AUDIO_BUFFER_MODE buf_mode);

/**
 * @brief	��ȡwma���������ͣ�LPC/Mid/High rate����
 * @author	Li Jun
 * @date	2010-1-14
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @return  ���ر��������ͣ�0/1/2�ֱ��ӦLPC/Mid/High rate	
 */
T_S32 _SD_GetWMABitrateType(T_VOID *audio_decode);

/**
 * @brief	�����Ƶ�����ڲ�������free�ռ��С.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @param	[in] buffer_control:
 * ��Ƶ�����ڲ�������״̬�ṹ
 * @return	T_AUDIO_BUF_STATE
 * @retval	������״̬
 */
T_AUDIO_BUF_STATE _SD_Buffer_Check(T_VOID *audio_decode, T_AUDIO_BUFFER_CONTROL *buffer_control);

/**
 * @brief	������Ƶ�����ڲ�������дָ��.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @param	[in] len:
 * ����Ƶ�����ڲ�������д�볤��
 * @return	T_S32
 * @retval	AK_TRUE : ���³ɹ�
 * @retval	AK_FLASE : ����ʧ��
 */
T_S32 _SD_Buffer_Update(T_VOID *audio_decode, T_U32 len);

/**
 * @brief	�����Ƶ�����ڲ�������.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @return	T_S32
 * @retval	AK_TRUE : ����ɹ�
 * @retval	AK_FLASE : ���ʧ��
 */
T_S32 _SD_Buffer_Clear(T_VOID *audio_decode);

/**
 * @brief	��¼���豸.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] enc_input:
 * ¼��������Ϣ�ṹ
 * @param	[in] enc_output:
 * ¼�������Ϣ�ṹ
 * @return	T_VOID *
 * @retval	¼����Ƶ�ڲ��ṹָ�룬Ϊ�ձ�ʾ��ʧ��
 */
T_VOID *_SD_Encode_Open(T_AUDIO_REC_INPUT *enc_input, T_AUDIO_ENC_OUT_INFO *enc_output);

/**
 * @brief	��¼�õ�pcm���ݽ��б���.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_record:
 * ¼����Ƶ���ڲ��ṹ
 * @param	[in] enc_buf_strc:
 * �������bufferָ�볤�Ƚṹ
 * @return	T_S32 
 * @retval	���볤��
 */
T_S32 _SD_Encode(T_VOID *audio_encode, T_AUDIO_ENC_BUF_STRC *enc_buf_strc);

/**
 * @brief	�ر�¼���豸.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_encode:
 * ¼����Ƶ���ڲ��ṹ
 * @return	T_S32 
 * @retval	AK_TRUE : �رճɹ�
 * @retval	AK_FALSE : �ر�ʧ��
 */
T_S32 _SD_Encode_Close(T_VOID *audio_encode);

/**
 * @brief	��ȡ�����ʱ��.
 * @author	Deng Zhou
 * @date	2007-10-15
 * @param	[in] audio_codec:
 * �������ڲ��ṹ
 * @param	[in] codec_flag:
 * ������־ 0������ 1������
 * @return	T_S32 
 * @retval	��ȡ��ʱ��
 */
T_S32 _SD_GetCodecTime(T_VOID *audio_codec, T_U8 codec_flag);


/**
 * @brief	��ȡ��ǰ���뻺���е�����������.
 * @author	Li Jun
 * @date	2007-10-15
 * @param	[in] audio_codec:
 * �������ڲ��ṹ
 * @param	[in] T_U8 *pBuf:
 * �洢�����Ļ���
 * @param	[in] T_U32 *len:
 * �洢�����Ļ����������ĳ���
 * @return	T_VOID 
 */
T_VOID _SD_LogBufferSave(T_U8 *pBuf, T_U32 *len,T_VOID *audio_codec);

/**
 * @brief	�Դ�������ʱ����ƵPCM�ź�, ������Ƶ�ײ�ԭַ����. 
 *          �ýӿڻ����wma�������е�fftģ��,������WMA����ģ�鿪��ʱ����ʹ��
 * @author	Li Jun
 * @date	2011-4-14
 * @param	[in] T_S32 data[]
 * ʱ����ƵPCM����    
 * @param	[in] T_U16 size
 * ʱ����ƵPCM���ݵĳ���
 * @param	[in] T_AUDIO_CB_FUNS *cbfun
 * �ص������ṹ��,��Ҫ��malloc,free��printf������
 * @return	T_S32 
 * AK_FALSE �����ڴ����ʧ�ܶ���FALSE
 * AK_TRUE  ����Ƶ�����ݳɹ�,Ƶ��������data��, ��Ч������size/2
 */
T_S32 _SD_GetAudioSpectrum(T_S32 *data,T_U16 size, T_AUDIO_CB_FUNS *cbfun);

// #if ((defined (NEWWAY_FILL_BUF)) || defined(ANDROID))
/**
 * @brief	��ȡ��Ƶ�����ڲ��������ĵ�ַָ��.
 * @author	Cheng RongFei
 * @date	2011-7-13
 * @param	[in] audio_decode:
 * �������ڲ��ṹ
 * @param	[in] len:
 * ��Ҫһ��д��buffer�����ݳ���
 * @return	T_VOID* 
 * @retval	��ȡbuffer�ĵ�ַָ��
 */
T_VOID* _SD_Buffer_GetAddr(T_VOID *audio_decode, T_U32 len);

/**
 * @brief	������Ƶ�����ڲ�������дָ��.
 * @author	Cheng RongFei
 * @date	2011-7-13
 * @param	[in] audio_decode:
 * ��Ƶ������ڲ����뱣��ṹ
 * @return	T_S32
 * @retval	AK_TRUE : ���³ɹ�
 * @retval	AK_FLASE : ����ʧ��
 */
T_S32 _SD_Buffer_UpdateAddr(T_VOID *audio_decode, T_U32 len);
// #endif

/** 
 * @brief   ��������
 * @author  Zhou Jiaqing
 * @date   2012-5-16
 * @param  [in] audio_codec:¼����Ƶ���ڲ��ṹ
 *		   [in] enc_buf_strc:�������buff
 * @return T_S32
 * @retval ����������ݳ���                                           
 */
T_S32 _SD_Encode_Last(T_VOID *audio_encode,T_AUDIO_ENC_BUF_STRC *enc_buf_strc);

/** 
 * @brief   ����ʱ�������Ƿ񷵻�aac����֡ͷ����
 * @author  Tang Xuechai
 * @date   2013-5-20
 * @param  [in] audio_codec:¼����Ƶ���ڲ��ṹ
 *		   [in] flag: T_AUDIO_ENC_FRMHEAD_STATE��ö��֮һ
 *                    _SD_ENC_SAVE_FRAME_HEAD������֡ͷ����
 *                    _SD_ENC_CUT_FRAME_HEAD��������֡ͷ���ݣ�ֻ���ر������������
 * @return T_S32
 * @retval AK_TRUE: ���óɹ�  
 *         AK_FALSE: ����ʧ��
 */
T_S32 _SD_Encode_SetFramHeadFlag(T_VOID *audio_encode, int flag);

/**
 * @brief  ��������ʱ���л�����ʱ����
 *		   ֻ����ogg vorbis��������ʱʹ��       
 * @date  2012-6-6
 * @param [in] audio_decode :��Ƶ������ڲ��ṹ
 * @return T_S32
 * @retval >0: ��ȡ�ɹ�  
 *         <0: ��ȡʧ��
 *         =0: �������ݲ�����ʧ��
 */
T_S32 _SD_Decode_ParseFHead(T_VOID *audio_decode);

/**
 * @brief ��������ʱ���򿪲����ļ�
 *	      ֻ����ogg vorbis��������ʹ��
 * @date 2012-6-6
 * @param [in] audio_input :��Ƶ��Ϣ����ṹ
 *        [in] audio_output:pcm��Ϣ����ṹ
 * @return T_VOID *
 * @retval �������óɹ�����T_VOIDָ�룬���򷵻�AK_NULL
 */
T_VOID *_SD_Decode_Open_Fast(T_AUDIO_DECODE_INPUT *audio_input, T_AUDIO_DECODE_OUT *audio_output);

/**
 * @brief ��ȡ��Ƶ������buf�����Ϣ
 * @date 2012-7-6
 * @param [in] audio_decode :��Ƶ����ṹ��
 *		  [in]  type: T_AUDIO_INBUF_STATE ָ����Ҫ��ȡ����Ϣ���ֱ����ȡ����ֵ��
 *	 			  _STREAM_BUF_LEN,         ������������buf��buf���ȣ�
 *				  _STREAM_BUF_REMAIN_DATA, ������������buf��ʣ��δ�������ݵĳ��ȣ�
 *				  _STREAM_BUF_MIN_LEN,     �������ؽ���������Сbuf����				
 * @return T_S32
 * @retval 0:  buffer�գ�û��ʣ������
 *         >0: buffer��ʣ��û�������ݵĳ���
 *         <0: ����ָ��Ƿ�
 */
T_S32  _SD_Get_Input_Buf_Info(T_VOID *audio_decode,T_AUDIO_INBUF_STATE type);


#ifdef __cplusplus
}
#endif

#endif

/* end of sdcodec.h */

/*@}*/
