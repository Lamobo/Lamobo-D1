#ifndef AUDIO_RECORDER_AUDIO_DEMO
#define AUDIO_RECORDER_AUDIO_DEMO

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"

typedef enum
{
	D_ENC_TYPE_WAV_PCM,		//encode type wav
	D_ENC_TYPE_WAV_ADPCM,
	D_ENC_TYPE_AMR,		//encode type amr
	D_ENC_TYPE_AAC,		//encode type aac
	D_ENC_TYPE_G711,	//encode type g711
	D_ENC_TYPE_COUNT
}DEMO_ENCODE_TYPE;

typedef enum
{
	D_IN_DEV_MIC,		//pcm in device mic
	D_IN_DEV_LINE,		//pcm in device line
	D_IN_DEV_COUNT
}DEMO_AUDIO_IN_DEV;

typedef struct demo_setting {
	T_pSTR				strRecordPath; 
	T_U32 				nSampleRate;
	T_U32				nOutBitsRate;
	T_U32				nChannels;
	DEMO_ENCODE_TYPE	enEncType;
	DEMO_AUDIO_IN_DEV	enInDev;
	T_U32				time;
	T_U32				volume;
}demo_setting;

typedef struct demo_name
{
	char src[2];
	char format[6];
	char samplerate[6];
	char volume[3];
}demo_name;

/**
* @brief   open the audio record demo
* @author hankejia
* @date 2012-07-05
* @param[in] Setting  			the pointer point to the alsa_handle_t.
* @param[in] strDev  			device name
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 OpenDemo( demo_setting * Setting );

/**
* @brief   run the record demo
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 RunDemo();


#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
