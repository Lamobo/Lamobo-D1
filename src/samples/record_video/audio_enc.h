#ifndef LINUX_MEDIA_RECORDER_AkAudioEncode
#define LINUX_MEDIA_RECORDER_AkAudioEncode

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"

typedef struct AudioEncIn_st
{
	T_U32	nChannels;		//channels now only support set 1 or 2
	T_U32	nSampleRate;	//sample rate
	T_U32	nBitsPerSample;	//bits per sample
}AudioEncIn;

//support encode type
typedef enum 
{
	ENC_TYPE_PCM,
	ENC_TYPE_ADPCM_IMA,
	ENC_TYPE_AAC,
	ENC_TYPE_COUNT
}AUDIO_ENCODE_TYPE;

typedef struct AudioEncOut_st
{
	T_U32				nBitsRate;	//out bits rate per second
	AUDIO_ENCODE_TYPE	enc_type;
}AudioEncOut;


/**
* @brief  open the AK audio encode lib
* @author hankejia
* @date 2012-07-05
* @param[in] pstEncIn		the audio pcm data is params.
* @param[in] pstEncOut		the audio encode data is params.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_enc_open( AudioEncIn * pstEncIn, AudioEncOut * pstEncOut );


/**
* @brief  close the AK audio encode lib
* @author hankejia
* @date 2012-07-05
* @param[in] NONE
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_enc_close();


/**
* @brief  encode the pcm data
* @author hankejia
* @date 2012-07-05
* @param[in] pRawData	the audio pcm raw data
* @param[in] nRawDataSize	raw data buffer size
* @param[out] pEncBuf		encode out buffer
* @param[in] nEncBufSize	encode out buffer size
* @return T_S32
* @retval if >=0 encode return size and success, otherwise failed 
*/
T_S32 audio_encode( T_U8 * pRawData, T_U32 nRawDataSize, 
							  T_U8 * pEncBuf, T_U32 nEncBufSize );


#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif

