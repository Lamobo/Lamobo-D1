#ifndef _AUDIO_H_
#define _AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_process.h"
#include "audio_enc.h"

typedef struct {
	T_U32		nSampleRate;
	T_U32		nBitsPerSample;
	T_U32		nChannels;
	T_U32		nBitsRate;
	AUDIO_ENCODE_TYPE_CC enc_type;
}T_AUDIO_INPUT;


/**
* @brief init to get pcm from ad
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] input
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int audio_open(T_AUDIO_INPUT *input);

/**
* @brief  close adc
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int audio_close(void);

/**
* @brief start to get pcm from ad
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int audio_start(void);

/**
* @brief stop to get pcm from ad
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int audio_stop(void);

T_S32 SetAudioMicVolume( T_U32 volume );

T_S32 GetAudioMicVolume( void );



#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
