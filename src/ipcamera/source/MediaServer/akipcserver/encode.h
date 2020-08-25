#ifndef _ENCODE_H
#define _ENCODE_H 

#include "anyka_types.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct _ENC_INPUT_PAR
{
	T_U32	width;			//ʵ�ʱ���ͼ��Ŀ�ȣ��ܱ�4����
	T_U32	height;			//ʵ�ʱ���ͼ��ĳ��ȣ��ܱ�2���� 
	T_U8 kbpsmode;
	T_S32	qpHdr;			//��ʼ��QP��ֵ
	T_S32	iqpHdr;			//��ʼ��i֡��QPֵ
	T_S32 minQp;		//��̬���ʲ���[20,25]
	T_S32 maxQp;		//��̬���ʲ���[45,50]
	T_S32 framePerSecond; //֡��
	T_S32	bitPerSecond;	//Ŀ��bps
	T_U32 	video_tytes;
	T_U32	size;
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
int encode_open(T_ENC_INPUT *pencInput);

/**
* @brief  encode one frame
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_frame(T_ENC_INPUT *pencInput1, void *pinbuf, void **poutbuf,int* nIsIFrame1, T_ENC_INPUT *pencInput2, void *pinbuf2, void **poutbuf2, int* nIsIFrame2);

/**
* @brief  close vedio encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_close(void);

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
#ifdef __cplusplus
}
#endif
#endif /* _ENCODE_H */
