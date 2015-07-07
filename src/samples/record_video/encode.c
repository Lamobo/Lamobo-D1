#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "video_stream_lib.h"
#include "akuio.h"
#include "encode.h"
#include "headers.h"

#define ENCMEM (256*1024)

typedef struct encode{
	T_pVOID hVS;
	T_pVOID pBuff;
	T_pVOID pPhysBuff;
}T_VIDEO_ENC;

T_VIDEO_ENC videoEnc[eCHAN_NUM] = {{0}, {0}};

static T_pVOID enc_mutex_create(T_VOID)
{
	pthread_mutex_t *pMutex;
	pMutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(pMutex, NULL); 
	return pMutex;
}

static T_S32 enc_mutex_lock(T_pVOID pMutex, T_S32 nTimeOut)
{
	pthread_mutex_lock(pMutex);
	return 1;
}

static T_S32 enc_mutex_unlock(T_pVOID pMutex)
{
	pthread_mutex_unlock(pMutex);
	return 1;
}

static T_VOID enc_mutex_release(T_pVOID pMutex)
{
	int rc = pthread_mutex_destroy( pMutex ); 
    if ( rc == EBUSY ) {                      
        pthread_mutex_unlock( pMutex );             
        pthread_mutex_destroy( pMutex );    
    } 
	free(pMutex);
}
/*
 * @brief		media lib will call this while idle.
 * @param	ticks[in]
 * @return	T_BOOL
 * @retval	AK_TRUE for success,other for error.
 */
T_BOOL ak_enc_delay(T_U32 ticks)
{
	//printf("delay 0x%lx ticks\n",ticks);
	
	//usleep (ticks*1000);
	akuio_wait_irq();
	return AK_TRUE;
}

/**
* @brief  init video encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_init(void)
{
	T_VIDEOLIB_CB	init_cb_fun;
	int ret;

	memset(&init_cb_fun, 0, sizeof(T_VIDEOLIB_CB));
	/* set callback */
	init_cb_fun.m_FunPrintf			= (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	init_cb_fun.m_FunMalloc  		= (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	init_cb_fun.m_FunFree    		= free;
	init_cb_fun.m_FunRtcDelay       = ak_enc_delay;
				
	init_cb_fun.m_FunDMAMalloc		= (MEDIALIB_CALLBACK_FUN_DMA_MALLOC)akuio_alloc_pmem;
  	init_cb_fun.m_FunDMAFree		= (MEDIALIB_CALLBACK_FUN_DMA_FREE)akuio_free_pmem;
  	init_cb_fun.m_FunVaddrToPaddr	= (MEDIALIB_CALLBACK_FUN_VADDR_TO_PADDR)akuio_vaddr2paddr;
  	init_cb_fun.m_FunMapAddr		= (MEDIALIB_CALLBACK_FUN_MAP_ADDR)akuio_map_regs;
  	init_cb_fun.m_FunUnmapAddr		= (MEDIALIB_CALLBACK_FUN_UNMAP_ADDR)akuio_unmap_regs;
  	init_cb_fun.m_FunRegBitsWrite	= (MEDIALIB_CALLBACK_FUN_REG_BITS_WRITE)akuio_sysreg_write;
	init_cb_fun.m_FunVideoHWLock	= (MEDIALIB_CALLBACK_FUN_VIDEO_HW_LOCK)akuio_lock_timewait;
	init_cb_fun.m_FunVideoHWUnlock	= (MEDIALIB_CALLBACK_FUN_VIDEO_HW_UNLOCK)akuio_unlock;

	//add for using api about fifo in multithread
	init_cb_fun.m_FunMutexCreate	= enc_mutex_create;
	init_cb_fun.m_FunMutexLock		= enc_mutex_lock;
	init_cb_fun.m_FunMutexUnlock	= enc_mutex_unlock;
	init_cb_fun.m_FunMutexRelease	= enc_mutex_release;

	ret = VideoStream_Enc_Init(&init_cb_fun);
	printf("enc init %d\n",ret);
	return 0;
}

/**
* @brief  destroy vedio encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_destroy(void)
{
	VideoStream_Enc_Destroy();
	return 0;
}

static T_pVOID encode_openLib(T_ENC_INPUT *pEncIn)
{
	T_VIDEOLIB_ENC_OPEN_INPUT open_input;
	memset(&open_input, 0, sizeof(T_VIDEOLIB_ENC_OPEN_INPUT));
	
	if (pEncIn->video_tytes == 0)
	{
		open_input.encFlag = VIDEO_DRV_H264;
		open_input.encH264Par.width = pEncIn->width;	//实际编码图像的宽度，能被4整除
		open_input.encH264Par.height = pEncIn->height;	//实际编码图像的长度，能被2整除 
		open_input.encH264Par.rotation = 0; 			//编码前yuv图像的旋转
		open_input.encH264Par.frameRateDenom = 1;		//帧率的分母
		open_input.encH264Par.frameRateNum = 30;		//帧率的分子
		open_input.encH264Par.qpHdr = pEncIn->qpHdr;			//初始的QP的值
		open_input.encH264Par.qpMin = pEncIn->qpMin;	//25;			 
		open_input.encH264Par.qpMax = pEncIn->qpMax; 
		open_input.encH264Par.gopLen = 30; 	// 150; 	  
		open_input.encH264Par.fixedIntraQp = 0;//pencInput1->iqpHdr;	//为所有的intra帧设置QP
		open_input.encH264Par.bitPerSecond = pEncIn->bitPerSecond; //目标bps
		open_input.encH264Par.streamType = 0;			//有startcode和没startcode两种
		//open_input.encH264Par.maxVideoSize = outbuflen;
		//open_input.encH264Par.outStreamBuf = pencbuf;		
		open_input.encH264Par.lumWidthSrc 	= pEncIn->width;
		open_input.encH264Par.lumHeightSrc 	= pEncIn->height;
		open_input.encH264Par.horOffsetSrc 	= 0;
		open_input.encH264Par.verOffsetSrc 	= 0;
		switch (pEncIn->profile)
		{
		case 0:
			// Baseline Profile
			open_input.encH264Par.enableCabac = 0;
			open_input.encH264Par.transform8x8Mode = 0;
			break;
		case 1: 
			// Main Profile
			open_input.encH264Par.enableCabac = 1;
			open_input.encH264Par.transform8x8Mode = 0;
			break;
		case 2:
			// High Profile
			open_input.encH264Par.enableCabac = 1;
			open_input.encH264Par.transform8x8Mode = 2;
			break;
		default:
			break;
		}
	}
	else if (pEncIn->video_tytes == 1)
	{
		open_input.encFlag = VIDEO_DRV_MJPEG;
		open_input.encMJPEGPar.frameType = 0;	//JPEGENC_YUV420_PLANAR;
		open_input.encMJPEGPar.format = 0;
		open_input.encMJPEGPar.thumbWidth = 0;
		open_input.encMJPEGPar.thumbHeight = 0;
		open_input.encMJPEGPar.thumbData = AK_NULL;
		open_input.encMJPEGPar.thumbDataLen = 0;
		open_input.encMJPEGPar.qLevel = 7;
		open_input.encMJPEGPar.width = pEncIn->width;
		open_input.encMJPEGPar.height = pEncIn->height;
		
		open_input.encMJPEGPar.lumWidthSrc = pEncIn->width;
		open_input.encMJPEGPar.lumHeightSrc = pEncIn->height;
		open_input.encMJPEGPar.horOffsetSrc = 0;
		open_input.encMJPEGPar.verOffsetSrc = 0;
	}
	
	return VideoStream_Enc_Open(&open_input);
}

/**
* @brief  open vedio encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_open(T_ENC_INPUT *pEncIn1, T_ENC_INPUT *pEncIn2)
{
	T_U32 temp;
	
	if (pEncIn1)
	{
		videoEnc[eCHAN_UNI].pBuff = akuio_alloc_pmem(ENCMEM);
		if (AK_NULL == videoEnc[eCHAN_UNI].pBuff)
		{
			return -1;
		}

		temp = akuio_vaddr2paddr(videoEnc[eCHAN_UNI].pBuff) & 7;
		//编码buffer 起始地址必须8字节对齐
		videoEnc[eCHAN_UNI].pPhysBuff = ((T_U8 *)videoEnc[eCHAN_UNI].pBuff) + ((8-temp)&7);

		if (!(videoEnc[eCHAN_UNI].hVS = encode_openLib(pEncIn1))) {
			encode_close(eCHAN_UNI);
			printf("Open EncodeLib Failure\n");
			return -1;
		}
	}

	if (pEncIn2)
	{
		videoEnc[eCHAN_DUAL].pBuff = akuio_alloc_pmem(ENCMEM);
		if (AK_NULL == videoEnc[eCHAN_DUAL].pBuff)
		{
			return -1;
		}

		temp = akuio_vaddr2paddr(videoEnc[eCHAN_DUAL].pBuff) & 7;
		//编码buffer 起始地址必须8字节对齐
		videoEnc[eCHAN_DUAL].pPhysBuff = ((T_U8 *)videoEnc[eCHAN_DUAL].pBuff) + ((8-temp)&7);

		if (!(videoEnc[eCHAN_DUAL].hVS = encode_openLib(pEncIn2))) {			
			encode_close(eCHAN_DUAL);
			encode_close(eCHAN_UNI);
			return -1;
		}
	}
		
	return 0;
}

/**
* @brief  vedio encode one frame
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_frame(long *frameLenA, void *pinbuf, void **poutbuf, int *nIsIFrame,
					long *frameLenB, void *pinbuf2, void **poutbuf2, int *nIsIFrame2, unsigned char vbr)
{
	T_VIDEOLIB_ENC_IO_PAR video_enc_param1;
	T_VIDEOLIB_ENC_IO_PAR video_enc_param2;
	static int IFCtrl_1 = 0;
	static int IFCtrl_2 = 0;
	
	// 1st Channel
	if (pinbuf) {
		//编码当前帧的QP
		if (vbr) 		
			video_enc_param1.QP = 30;
		else 
			video_enc_param1.QP = 0;
		
		//编码类型I/P帧, 0:I，1:P
		if (IFCtrl_1 == 0) {			
			video_enc_param1.mode = 0;
			*nIsIFrame = 1;
		}
		else {
			video_enc_param1.mode = 1;
			*nIsIFrame = 0;
		}

		//I frame interval is 30
		if (++IFCtrl_1 >= 30)
			IFCtrl_1 = 0;
		//yuv输入地址
		video_enc_param1.p_curr_data 	= pinbuf;	
		//码流输出地址
		video_enc_param1.p_vlc_data		= videoEnc[eCHAN_UNI].pPhysBuff;
		//输出码流的大小
		video_enc_param1.out_stream_size = ENCMEM;
	}

	// 2nd Channel
	if (pinbuf2) {
		//编码当前帧的QP
		if (vbr) 		
			video_enc_param2.QP = 30;
		else 
			video_enc_param2.QP = 0;
		
		//编码类型I/P帧, 0:I，1:P
		if (IFCtrl_2 == 0) {			
			video_enc_param2.mode = 0;
			*nIsIFrame2 = 1;
		}
		else {
			video_enc_param2.mode = 1;
			*nIsIFrame2 = 0;
		}
		//I frame interval is 30
		if (++IFCtrl_2 >= 30)
			IFCtrl_2 = 0;
		
		//yuv输入地址
		video_enc_param2.p_curr_data 	= pinbuf2;	
		//码流输出地址
		video_enc_param2.p_vlc_data		= videoEnc[eCHAN_DUAL].pPhysBuff;
		//输出码流的大小
		video_enc_param2.out_stream_size = ENCMEM;
	}

	//dual encode , 1st & 2nd Channel
	if (pinbuf != AK_NULL && pinbuf2 != AK_NULL) {		
		VideoStream_Enc_Encode(videoEnc[eCHAN_UNI].hVS, videoEnc[eCHAN_DUAL].hVS, &video_enc_param1, &video_enc_param2);
		*poutbuf 	= video_enc_param1.p_vlc_data;
		*frameLenA 	= video_enc_param1.out_stream_size;
		*poutbuf2 	= video_enc_param2.p_vlc_data;
		*frameLenB 	= video_enc_param2.out_stream_size;
	}
	//single encode, 1st Channel
	else if (pinbuf) {
		VideoStream_Enc_Encode(videoEnc[eCHAN_UNI].hVS, AK_NULL, &video_enc_param1, AK_NULL);
		*poutbuf 	= video_enc_param1.p_vlc_data;
		*frameLenA 	= video_enc_param1.out_stream_size;
	}
	//single encode, 2nd Channel
	else if (pinbuf2) {
		VideoStream_Enc_Encode(videoEnc[eCHAN_DUAL].hVS, AK_NULL, &video_enc_param2, AK_NULL);
		*poutbuf2 	= video_enc_param2.p_vlc_data;
		*frameLenB 	= video_enc_param2.out_stream_size;
	}
	
	return 0;
}

/**
* @brief  close vedio encoder
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int encode_close(T_REC_CHAN chan)
{
	if (videoEnc[chan].hVS){
		VideoStream_Enc_Close(videoEnc[chan].hVS);
		videoEnc[chan].hVS = NULL;
	}

	if (videoEnc[chan].pBuff){
		akuio_free_pmem(videoEnc[chan].pBuff);
		videoEnc[chan].pBuff = NULL;
		videoEnc[chan].pPhysBuff = NULL;
	}
	
	return 0;
}
