#ifndef AUDIO_RECORDER_AUDIO_RECORDER
#define AUDIO_RECORDER_AUDIO_RECORDER

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"
//recorder support is audio type
typedef enum {
	AUDIO_REC_TYPE_WAV_PCM,
	AUDIO_REC_TYPE_ADPCM_IMA,
	AUDIO_REC_TYPE_AAC,
	AUDIO_REC_TYPE_AMR,
	AUDIO_REC_TYPE_G711_ULOW,
	AUDIO_REC_TYPE_G711_ALOW,
	AUDIO_REC_TYPE_COUNT
}AUDIO_REC_TYPE;

typedef enum {
	IN_DEV_MIC,		//pcm in device mic
	IN_DEV_LINE,	//pcm in device line
	IN_DEV_COUNT
}AUDIO_IN_DEV;


//typedef struct AkAudioRecorder
//{
	/**
	* @brief   open the audio record lib, alsa lib
	* @author hankejia
	* @date 2012-07-05
	* @param[in] ppHandle  			the pointer point to the AkAudioRecorder.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	OpenRecorder( T_pVOID * ppHandle );
	
	/**
	* @brief   set audio capture pcm params
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
	* @param[in] nChannels  		audio channels.
	* @param[in] nSampleRate  		sample rate.
	* @param[in] nBitsPerSample  	bits per sample.
	* @param[in] nFilterType  	 Filter Type.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	SetAudioParams(T_pVOID pHandle, T_U32 nChannels, 
								T_U32 nSampleRate, T_U32 nBitsPerSample, T_U32 nFilterType);
	
	/**
	* @brief   set audio encode params
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
	* @param[in] enType  			audio encode type.
	* @param[in] nBitsRate  		encode out bits rate per second.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	SetEncodeParams( T_pVOID pHandle, AUDIO_REC_TYPE enType,
							  	T_U32 nBitRates );
	
	/**
	* @brief   set audio pcm data input device
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
	* @param[in] enInDev  			input device type line or mic.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	SetAudioInDev( T_pVOID pHandle, AUDIO_IN_DEV enInDev );
	
	/**
	* @brief   set record path
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
	* @param[in] strPath  			record path.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	SetRecordPath( T_pVOID pHandle, T_pSTR strPath );
	
	/**
	* @brief   is AkAudioRecorder module running? capture data and process data
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pHandle  			the pointer to the AkAudioRecorder.
	* @return T_BOOL
	* @retval if return AK_TURE running,AK_FALSE not running
	*/

	T_S32 SetAudioMicVolume(T_pVOID pHandle, T_U32 volume);

	T_S32 SetAudioLineInVolume(T_pVOID pHandle, T_U32 volume);
	
	T_BOOL	IsRunning( T_pVOID pHandle );
	
	/**
	* @brief   get the recorded file is path and name
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
	* @param[in] strFileName  		file path and name.
	* @param[out] pnStrlen  		strFileName string len.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	GetRecordFile( T_pVOID pHandle, T_pSTR strFileName, T_U32 * pnStrlen );
	
	/**
	* @brief   start recorder, create the capture thread and process data thread
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point o the audio_recorder_handle.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	StartRec( T_pVOID pHandle );
	
	/**
	* @brief   stop recorder, stop the capture thread and process data thread
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point o the audio_recorder_handle.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	StopRec( T_pVOID pHandle );
	
	/**
	* @brief   close the audio record lib, alsa lib
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pHandle  			the pointer point o the AkAudioRecorder.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32	CloseRecorder( T_pVOID pHandle );

	
	
//}AkAudioRecorder;

//T_S32 getAudioRecorderModule( AkAudioRecorder * pRecModule );

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
