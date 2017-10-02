#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "video_stream_lib.h"
#include "akuio.h"
#include "encode.h"
#include "muxer.h"

#define ENCMEM (256*1024)


static T_pVOID hVS1;
static T_pVOID hVS2;
static T_pVOID poutbuf;
static T_pVOID pencbuf;
static T_pVOID poutbuf2;
static T_pVOID pencbuf2;

static T_eVIDEO_DRV_TYPE s_encType[2];

//static T_U32 outbuflen = ENCMEM;
static int framerate = 30;
extern init_parse parse;

extern int encode_init(void);


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


/**
* @brief  open vedio encoder
*
* @author dengzhou
* @date 2013-04-07
* @param[in]
* @return T_S32
* @retval if return 0 success, otherwise failed
*/


int encode_open(T_ENC_INPUT *pencInput)
{
	T_VIDEOLIB_ENC_OPEN_INPUT open_input;
	T_VIDEOLIB_ENC_OPEN_INPUT open_input2;
	T_U32 temp;

	poutbuf = akuio_alloc_pmem(ENCMEM);
	if (AK_NULL == poutbuf)
	{
		return -1;
	}

	temp = akuio_vaddr2paddr(poutbuf) & 7;
	//编码buffer 起始地址必须8字节对齐
	pencbuf = ((T_U8 *)poutbuf) + ((8-temp)&7);

	framerate = pencInput->framePerSecond;
	if (pencInput->video_tytes == 0)
	{
		open_input.encFlag = VIDEO_DRV_H264;
		s_encType[0] = VIDEO_DRV_H264;
		open_input.encH264Par.width = pencInput->width;			//实际编码图像的宽度，能被4整除
		open_input.encH264Par.height = pencInput->height;			//实际编码图像的长度，能被2整除
		open_input.encH264Par.rotation = 0;		//编码前yuv图像的旋转
		open_input.encH264Par.frameRateDenom = 1;	//帧率的分母
		open_input.encH264Par.frameRateNum = pencInput->framePerSecond;	//帧率的分子
		open_input.encH264Par.qpHdr = -1;			//初始的QP的值
	  	//open_input.encH264Par.qpMin = 1;//pencInput->minQp;
		//open_input.encH264Par.qpMax = 50;//pencInput->maxQp;
		open_input.encH264Par.gopLen = 150;
		open_input.encH264Par.fixedIntraQp = 30;	//为所有的intra帧设置QP
		open_input.encH264Par.bitPerSecond = parse.kbps1;	//目标bps
		open_input.encH264Par.streamType = 0;		//有startcode和没startcode两种
		open_input.encH264Par.lumWidthSrc = 1280;
		open_input.encH264Par.lumHeightSrc = 720;
		open_input.encH264Par.horOffsetSrc = 0;
		open_input.encH264Par.verOffsetSrc = 0;

		//High Profile
		open_input.encH264Par.enableCabac = 1;
		open_input.encH264Par.transform8x8Mode = 2;

		if (parse.kbps_mode1 == 0)
		{
			open_input.encH264Par.qpMin = 1;
			open_input.encH264Par.qpMax = 50;
			open_input.encH264Par.qpHdr = -1;
			open_input.encH264Par.fixedIntraQp = 0;
		}
		else
		{
			open_input.encH264Par.qpMin = 35;
			open_input.encH264Par.qpMax = 36;
			open_input.encH264Par.fixedIntraQp = 30;
			open_input.encH264Par.bitPerSecond = 8 * 1000 * 1000;
		}
		//open_input.encH264Par.maxVideoSize = outbuflen;
		//open_input.encH264Par.outStreamBuf = pencbuf;
	}
	else if (pencInput->video_tytes == 1)
	{
		open_input.encFlag = VIDEO_DRV_MJPEG;
		s_encType[0] = VIDEO_DRV_MJPEG;
		open_input.encMJPEGPar.frameType = 0;//JPEGENC_YUV420_PLANAR;
		open_input.encMJPEGPar.format = 0;
		open_input.encMJPEGPar.thumbWidth = 0;
		open_input.encMJPEGPar.thumbHeight = 0;
		open_input.encMJPEGPar.thumbData = AK_NULL;
		open_input.encMJPEGPar.thumbDataLen = 0;
		open_input.encMJPEGPar.qLevel = parse.quality;
		open_input.encMJPEGPar.width = pencInput->width;
		open_input.encMJPEGPar.height = pencInput->height;
	}

	hVS1 = VideoStream_Enc_Open(&open_input);

	// 2nd channel
	poutbuf2 = akuio_alloc_pmem(ENCMEM);
	if (AK_NULL == poutbuf2)
	{
		printf("pmem err \n");
		return -1;
	}

	temp = akuio_vaddr2paddr(poutbuf2) & 7;
	//编码buffer 起始地址必须8字节对齐
	pencbuf2 = ((T_U8 *)poutbuf2) + ((8-temp)&7);
	//printf("!!!! pencbuf2 = %p types = %d \n", pencbuf2, pencInput2->video_tytes);
	if (parse.format2 == 0)
	{
		open_input2.encFlag = VIDEO_DRV_H264;
		s_encType[1] = VIDEO_DRV_H264;
		open_input2.encH264Par.width = parse.width2;			//实际编码图像的宽度，能被4整除
		open_input2.encH264Par.height = parse.height2;			//实际编码图像的长度，能被2整除
		open_input2.encH264Par.rotation = 0;		//编码前yuv图像的旋转
		open_input2.encH264Par.frameRateDenom = 1;	//帧率的分母
		open_input2.encH264Par.frameRateNum = parse.fps2;	//帧率的分子
		open_input2.encH264Par.qpHdr = -1;			//初始的QP的值
	    //open_input2.encH264Par.qpMin = 1;//pencInput->minQp;
		//open_input2.encH264Par.qpMax = 50;//pencInput->maxQp;
		open_input2.encH264Par.gopLen = 150;
		open_input2.encH264Par.fixedIntraQp = 30;	//为所有的intra帧设置QP
		open_input2.encH264Par.bitPerSecond = parse.kbps2;	//目标bps
		open_input2.encH264Par.streamType = 0;		//有startcode和没startcode两种
		open_input2.encH264Par.lumWidthSrc = parse.real_width2;
		open_input2.encH264Par.lumHeightSrc = parse.real_height2;
		open_input2.encH264Par.horOffsetSrc = (parse.real_width2-parse.width2)/2;
		open_input2.encH264Par.verOffsetSrc = (parse.real_height2-parse.height2)/2;

		//High Profile
		open_input2.encH264Par.enableCabac = 1;
		open_input2.encH264Par.transform8x8Mode = 2;

		if(parse.kbps_mode2 == 0)
		{
			open_input2.encH264Par.qpMin = 1;
			open_input2.encH264Par.qpMax = 50;
			open_input2.encH264Par.qpHdr = -1;
			open_input2.encH264Par.fixedIntraQp = 0;
		}
		else
		{
			open_input2.encH264Par.qpMin = 35;
			open_input2.encH264Par.qpMax = 36;
			open_input2.encH264Par.fixedIntraQp = 30;
			open_input2.encH264Par.bitPerSecond = 8 * 1000 * 1000;
		}
		//open_input.encH264Par.maxVideoSize = outbuflen;
		//open_input.encH264Par.outStreamBuf = pencbuf;
	}
	else if (parse.format2 == 1)
	{
		open_input2.encFlag = VIDEO_DRV_MJPEG;
		s_encType[1] = VIDEO_DRV_MJPEG;
		open_input2.encMJPEGPar.frameType = 0;//JPEGENC_YUV420_PLANAR;
		open_input2.encMJPEGPar.format = 0;
		open_input2.encMJPEGPar.thumbWidth = 0;
		open_input2.encMJPEGPar.thumbHeight = 0;
		open_input2.encMJPEGPar.thumbData = AK_NULL;
		open_input2.encMJPEGPar.thumbDataLen = 0;
		open_input2.encMJPEGPar.qLevel = parse.quality;
		open_input2.encMJPEGPar.width = parse.width2;
		open_input2.encMJPEGPar.height = parse.height2;
		open_input2.encMJPEGPar.lumWidthSrc = parse.real_width2;
		open_input2.encMJPEGPar.lumHeightSrc = parse.real_height2;
		open_input2.encMJPEGPar.horOffsetSrc = (parse.real_height2-parse.height2)/2;
		open_input2.encMJPEGPar.verOffsetSrc = (parse.real_height2-parse.height2)/2;
		printf("width = %u , height = %u \n",(unsigned int)open_input2.encMJPEGPar.width,  (unsigned int)open_input2.encMJPEGPar.height );
	}

	hVS2 = VideoStream_Enc_Open(&open_input2);

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
//int encode_frame(void *pinbuf, void **poutbuf, unsigned long *size, int* nIsIFrame)
int encode_frame(T_ENC_INPUT *pencInput1, void *pinbuf, void **poutbuf,int *nIsIFrame1, T_ENC_INPUT *pencInput2, void *pinbuf2, void **poutbuf2, int* nIsIFrame2)
{
	T_VIDEOLIB_ENC_IO_PAR video_enc_io_param1;
	T_VIDEOLIB_ENC_IO_PAR video_enc_io_param2;
	static int IPctrl1 = 0;
	static int IPctrl2 = 0;

	video_enc_io_param1.QP = 0;					//编码当前帧的QP
	if (s_encType[0] == VIDEO_DRV_H264 && IPctrl1 == 0)
	{
		*nIsIFrame1 = 1;
		video_enc_io_param1.mode = 0;				//编码类型I/P帧,0，i，1，p
		//video_enc_io_param2.mode = 0;
	}
	else
	{
		*nIsIFrame1 = 0;
		video_enc_io_param1.mode = 1;
		//video_enc_io_param2.mode = 1;
	}
	video_enc_io_param2.QP = 0;
	if ((s_encType[1] == VIDEO_DRV_H264 && IPctrl2 == 0) || s_encType[1] == VIDEO_DRV_MJPEG)
	{
		*nIsIFrame2 = 1;

											//编码类型I/P帧,0，i，1，p
		video_enc_io_param2.mode = 0;
	}
	else
	{
		*nIsIFrame2 = 0;

		video_enc_io_param2.mode = 1;
	}

	video_enc_io_param1.p_curr_data = pinbuf;		//yuv输入地址
	video_enc_io_param1.p_vlc_data = pencbuf;			//码流输出地址
	video_enc_io_param1.out_stream_size = ENCMEM;	//输出码流的大小
	video_enc_io_param2.p_curr_data = pinbuf2;		//yuv输入地址
	video_enc_io_param2.p_vlc_data = pencbuf2;			//码流输出地址
	video_enc_io_param2.out_stream_size = ENCMEM;	//输出码流的大小

	VideoStream_Enc_Encode(hVS1, hVS2, &video_enc_io_param1, &video_enc_io_param2);
	*poutbuf2 = video_enc_io_param2.p_vlc_data;
	pencInput2->size = video_enc_io_param2.out_stream_size;
	*poutbuf = video_enc_io_param1.p_vlc_data;
	pencInput1->size = video_enc_io_param1.out_stream_size;

	IPctrl1++;
	if (IPctrl1 >= parse.group1)
		IPctrl1 = 0;

	IPctrl2++;
	if (IPctrl2 >= parse.group2)
		IPctrl2 = 0;

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
int encode_close(void)
{
	VideoStream_Enc_Close(hVS1);
	akuio_free_pmem(poutbuf);

	VideoStream_Enc_Close(hVS2);
	akuio_free_pmem(poutbuf2);

	return 0;
}
