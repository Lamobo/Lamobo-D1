#ifndef _MUXER_H
#define _MUXER_H 

#include "media_muxer_lib.h"
#include "asyncwrite.h"
#include "headers.h"

typedef struct {
	T_pSTR rec_path;
	T_eMEDIALIB_REC_TYPE	m_MediaRecType;
	T_BOOL					m_bCaptureAudio;

	//video
	T_eMEDIALIB_VIDEO_CODE	m_eVideoType;
	T_U16	m_nWidth;
	T_U16	m_nHeight;

	//audio
	T_eMEDIALIB_AUDIO_CODE	m_eAudioType;
	T_U32	m_nSampleRate;		//²ÉÑùÂÊ(8000)
}T_MUX_INPUT;

/**
* @brief  open muxer to write file
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 1 success, otherwise failed 
*/
int mux_open(T_REC_CHAN chan, T_MUX_INPUT *mux_input, char *filename);

/**
* @brief  mux audio data to file
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 1 success, otherwise failed 
*/
int mux_addAudio(void *pbuf, unsigned long size, unsigned long timestamp);

/**
* @brief  mux video data to file
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 1 success, otherwise failed 
*/
int mux_addVideo(T_REC_CHAN chan, void *pbuf, unsigned long size, unsigned long timestamp, int iframe);

/**
* @brief  close muxer
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 1 success, otherwise failed 
*/
int mux_close(T_REC_CHAN chan);
#endif /* _ENCODE_H */
