#ifndef LINUX_MEDIA_RECORDER_AkAlsaHardware
#define LINUX_MEDIA_RECORDER_AkAlsaHardware

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"
#include <alsa/asoundlib.h>

typedef struct alsa_handle_t
{
	snd_pcm_t *				handle;
	snd_ctl_t *             ctrlHandle;
	T_U32					nSampleRate;
	T_U32					nChannels;
	snd_pcm_format_t		nFormat;
	T_U32					nBufferSize;
	T_U32					nLatency;
	T_BOOL					bIsAD;
	snd_mixer_t *			mixerHandle;
	T_pSTR					strDevice;
#ifdef	CHIP_ID_AK37XX
	T_U32					nActuallySR;
	T_pVOID					pSdFilter;
	T_pVOID					pReSRBuffer;
	T_pVOID					pReadPoint;
	T_U32					nReSRBufLen;
	T_U32					nReSRDataLen;
#endif
}alsa_handle_t;
typedef enum {
	MASTER_VOLUME,
	PCM_VOLUME,
	CAPTURE_VOLUME,
	LINE_IN_CAPTURE_VOLUME,
	MIC_CAPTURE_VOLUME,
	MIC_PLAYBACK_VOLUME
} volume_t;

typedef struct AkAlsaHardware
{
	/**
	* @brief   open the AD device
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @param[in] strDev  			device name
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*OpenAD)( alsa_handle_t * handle, T_pSTR strDev  );

	/**
	* @brief   open the DA device
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @param[in] strDev  			device name
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*OpenDA)( alsa_handle_t * handle, T_pSTR strDev  );

	/**
	* @brief   set audio DA/AD params
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @param[in] nSampleRate  		sample rate
	* @param[in] nBits  			bits per sample
	* @param[in] nChannels  		channels
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*SetParams)( alsa_handle_t * handle, T_U32 nSampleRate, T_U32 nBits, T_U32 nChannels, int isAD );

	/**
	* @brief   set pcm input device is mic
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*SetMicIn)( alsa_handle_t * handle );

	/**
	* @brief   set pcm input device is line
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*SetLineIn)( alsa_handle_t * handle );

	/**
	* @brief   read AD is capture data
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @param[in] pPcmData  		the pcm data load buffer. 
	* @param[in] nBytes  			how many bytes you want to read from AD
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*ReadAD)( alsa_handle_t * handle, T_U8 * pPcmData, T_U32 nBytes );

	/**
	* @brief   write the pcm data to DA
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @param[in] pPcmData  		the pcm data to playback. 
	* @param[in] nBytes  			how many bytes you want to playback
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*WriteDA)( alsa_handle_t * handle, T_U8 * pPcmData, T_U32 nBytes );

	/**
	* @brief   close the alsa lib, and pcm device
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*Close)( alsa_handle_t * handle, int isAD );
		/**
	* @brief   close the alsa lib, and pcm device
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*GetMixerLevel)( alsa_handle_t * handle, volume_t type, T_S32 * level ) ;
	
	/**
	* @brief   close the alsa lib, and pcm device
	* @author hankejia
	* @date 2012-07-05
	* @param[in] handle  			the pointer point to the alsa_handle_t.
	* @return T_S32
	* @retval if return 0 success, otherwise failed 
	*/
	T_S32 (*SetMixerLevel)( alsa_handle_t * handle, volume_t type, T_S32 level );
}AkAlsaHardware;

/**
* @brief   get module functions is pointer
* @author hankejia
* @date 2012-07-05
* @param[in] pstAlsaModule  	the pointer point to the AkAlsaHardware.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 getAlsaModule( AkAlsaHardware * pstAlsaModule );

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
