#ifndef _MUXER_H
#define _MUXER_H 

#include "media_muxer_lib.h"
#ifdef __cplusplus
extern "C"
{
#endif

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
	T_U32	abitsrate;
	
}T_MUX_INPUT;


typedef struct init_parse
{
	int format1;
	int width;
	int height;
	int kbps1;
	int kbps_mode1;
	int group1;
	int fps1;
	int format2;
	int width2;
	int height2;
	int kbps2;
	int kbps_mode2;
	int quality;
	int group2;
	int fps2;
	int video_kbps;
	int real_width2;
	int real_height2;
}init_parse;

/**
* @brief  open muxer to write file
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 1 success, otherwise failed 
*/
int mux_open(T_MUX_INPUT *mux_input);

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
int mux_addVideo(void *pbuf, unsigned long size, unsigned long timestamp, int nIsIFrame);

/**
* @brief  close muxer
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 1 success, otherwise failed 
*/
int mux_close(void);

int mux_exit(void);

void read_Parse(void *mSettings);

int start_record( int cycrecord );

int mux_write_data(int type, void *pbuf, unsigned long size, unsigned long timestamp, int nIsIFrame);

int record_rename_file();

#ifdef __cplusplus
}
#endif
#endif /* _ENCODE_H */
