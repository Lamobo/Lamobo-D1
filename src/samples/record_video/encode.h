#ifndef _ENCODE_H
#define _ENCODE_H 

#include "anyka_types.h"
#include "headers.h"

typedef struct _ENC_INPUT_PAR
{
	T_U32	width;			//实际编码图像的宽度，能被4整除
	T_U32	height;			//实际编码图像的长度，能被2整除 
	T_S32	qpHdr;			//初始的QP的值
	T_S32	qpMax;
	T_S32	qpMin;
	T_S32	iqpHdr;			//初始的i帧的QP值
	T_S32	bitPerSecond;	//目标bps
	T_U32 	video_tytes;
	T_U8	profile;	// H264 Profile Level, 0: baseline, 1: main, 2: high Profile
}T_ENC_INPUT;

/**
* @brief  init vedio encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_init(void);

/**
* @brief  open vedio encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_open(T_ENC_INPUT *pencInput1, T_ENC_INPUT *pencInput2);


/**
* @brief  encode one frame
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_frame(long *frameLenA, void *pinbuf, void **poutbuf,int *nIsIFrame,long *frameLenB, void *pinbuf2, void **poutbuf2, int *nIsIFrame2, unsigned char vbr);

/**
* @brief  close vedio encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_close(T_REC_CHAN chan);

/**
* @brief  destroy vedio encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_destroy(void);
#endif /* _ENCODE_H */
