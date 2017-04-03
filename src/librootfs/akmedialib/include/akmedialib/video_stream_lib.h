/**
 * @file video_stream_lib.h
 * @brief This file provides H264/MJPEG encoding functions
 *
 * Copyright (C) 2013 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Hu_Jing
 * @date 2013-4-9
 * @version 0.1.00
 * @version ever green group version: x.x
 * @note




	@endcode

	************************************************************

 * @note
	The following is an example to use H264/MJPEG encoding APIs
   @code

	编码部分：
	T_pVOID g_hVS;							//句柄1
	T_VIDEOLIB_ENC_OPEN_INPUT openInput;
	T_VIDEOLIB_ENC_IO_PAR video_enc_io_param;

	T_VIDEOLIB_CB callback_fun;//回调函数	
	
	u8 *DataIn = NULL;					
	u8 *DataOut = NULL;						//输出码流地址
	u8 *picvirtualAddress = NULL;
	u32 lStreamLen = 0;						//输出码流长度
	u32 picture_size;						//图片大小
    //u32 outbuf_size;
	i32 next, last;							//编码帧数的控制
	u8 mode;								//I/P帧
	u32 outBufLen;
	u8 ret = 0;


	memset(&callback_fun, 0, sizeof(T_VIDEO_CODEC_CALLBACK_FUN));
	memset(&openInput,0,sizeof(T_VIDEOLIB_ENC_OPEN_INPUT));
	memset(&video_enc_io_param,0,sizeof(T_VIDEOLIB_ENC_IO_PAR));

	openInput.encFlag = VIDEO_DRV_H264;//VIDEO_DRV_MJPEG;//VIDEO_DRV_H264;
	openInput.encH264Par.bitPerSecond = 2048*1000;//8294400;//2048*1000;//228096;
	openInput.encH264Par.fixedIntraQp = 40;	
	openInput.encH264Par.frameRateDenom = 1;
	openInput.encH264Par.frameRateNum = 30;
	openInput.encH264Par.height = 720;//144;
	openInput.encH264Par.width = 1280;//176;	
	openInput.encH264Par.qpHdr = 40;
	openInput.encH264Par.qpMax = 51;
	openInput.encH264Par.qpMin = 0;
	openInput.encH264Par.rotation = 0;
	openInput.encH264Par.streamType = 0;
	openInput.encH264Par.gopLen = 150;

	
	callback_fun.m_FunMalloc	= (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc;
	callback_fun.m_FunFree	= (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free;
	callback_fun.m_FunPrintf	= (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	callback_fun.m_FunRtcDelay = AK_NULL;

	callback_fun.m_FunMMUInvalidateDCache = (MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE)AK_NULL;
	callback_fun.m_FunCleanInvalidateDcache =(MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE)MMU_InvalidateDCache;
	callback_fun.m_FunCheckDecBuf		= (MEDIALIB_CALLBACK_FUN_CHECK_DEC_BUF)AK_NULL;
	callback_fun.m_FunMapAddr			= VD_EXfunc_MapAddr;
	callback_fun.m_FunUnmapAddr		= VD_EXfunc_UnmapAddr;
	callback_fun.m_FunDMAMalloc		= (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc;;
	callback_fun.m_FunDMAFree			= (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free;
	callback_fun.m_FunVaddrToPaddr	= VD_EXfunc_VaddrToPaddr;
	callback_fun.m_FunRegBitsWrite	= VD_EXfunc_RegBitsWrite;
	callback_fun.FunVideoHWLock		= open_input->m_CBFunc.m_FunVideoHWLock;
	callback_fun.FunVideoHWUnlock	= open_input->m_CBFunc.m_FunVideoHWUnlock;


	outBufLen = 100*1024;
	picture_size = openInput.encH264Par.height*openInput.encH264Par.width*3/2;	
	picvirtualAddress = 0x80b00000;
    DataOut = (u8 *)Fwl_Malloc(sizeof(u8)*outBufLen);//分配输出码流存放空间
	
 	VideoStream_Enc_Init(&init_cb_fun);	
 	
	g_hVS = VideoStream_Enc_Open(&open_input);
	if (AK_NULL == g_hVS)
	{
		return;
	}

	video_enc_io_param.p_curr_data = curr_buf;	//输入要编码的图像地址
	video_enc_io_param.p_vlc_data = vlc_buf;	//编码后的码流地址
	video_enc_io_param.QP = 10;					//编码的QP值大小
	video_enc_io_param.mode = 0;				//编码模式，I帧为0，P帧为1
	video_enc_io_param.out_stream_size = 200*1024;//输入为输入buf的size，输出为编码码流的size

	while ((next <= last) )//&& (lStreamLen != 0))
	{

		VideoStream_Enc_Encode(g_hVS, AK_NULL,&video_enc_io_param,AK_NULL);

		picvirtualAddress += picture_size;
		DataOut = (u8 *)Fwl_Malloc(sizeof(u8)*outBufLen);//分配输出码流存放空间
		video_enc_io_param.p_curr_data = picvirtualAddress;
		video_enc_io_param.p_vlc_data = DataOut;
		video_enc_io_param.out_stream_size = outBufLen;
		video_enc_io_param.QP = 10;
		video_enc_io_param.mode = 1;
	}


	VideoStream_Enc_Close(g_hVS);
	
	VideoStream_Enc_Destroy();
	@endcode

***************************************************/

#ifndef _VIDEO_STREAM_CODEC_H_
#define _VIDEO_STREAM_CODEC_H_

#include "medialib_global.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VIDEO_LIB_VERSION		"AK39 Encoder Lib V1.2.00"

//typedef enum
//{
//	VIDEO_DRV_H264,
//	VIDEO_DRV_MJPEG
//}T_eVIDEO_DRV_TYPE;

/*为了同demuxer库保持一致，特使用同demuxer库一致的定义，
	但本库只支持VIDEO_DRV_H264同VIDEO_DRV_MJPEG*/																			
typedef enum
{
	VIDEO_DRV_UNKNOWN = 0,
	VIDEO_DRV_H263,
	VIDEO_DRV_MPEG,
	VIDEO_DRV_FLV263,
	VIDEO_DRV_H264,
	VIDEO_DRV_RV,
	VIDEO_DRV_AVC1,
	VIDEO_DRV_MJPEG,
	VIDEO_DRV_MPEG2,
	VIDEO_DRV_H264DMX
}T_eVIDEO_DRV_TYPE;

//for mutex
typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_MUTEX_CREATE)(void);
/*
	nTimeOut [input]	-1		: the function's time-out interval never elapses
						0		: the function returns immediately
						other	: specifies the time-out interval, in milliseconds
	return: 1-ok; 0-failed or timeout
*/
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_MUTEX_LOCK)(T_pVOID pMutex, T_S32 nTimeOut);
/*
	return: 1-ok; 0-failed
*/
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_MUTEX_UNLOCK)(T_pVOID pMutex);
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_MUTEX_RELEASE)(T_pVOID pMutex);

typedef struct
{
	MEDIALIB_CALLBACK_FUN_PRINTF				m_FunPrintf;

	MEDIALIB_CALLBACK_FUN_MALLOC				m_FunMalloc;
	MEDIALIB_CALLBACK_FUN_FREE					m_FunFree;

	MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE	m_FunMMUInvalidateDCache;
	MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE	m_FunCleanInvalidateDcache;
	MEDIALIB_CALLBACK_FUN_CHECK_DEC_BUF			m_FunCheckDecBuf;
	MEDIALIB_CALLBACK_FUN_RTC_DELAY				m_FunRtcDelay;

	//add for Linux and CE
	MEDIALIB_CALLBACK_FUN_DMA_MALLOC			m_FunDMAMalloc;
	MEDIALIB_CALLBACK_FUN_DMA_FREE				m_FunDMAFree;
	MEDIALIB_CALLBACK_FUN_VADDR_TO_PADDR		m_FunVaddrToPaddr;
	MEDIALIB_CALLBACK_FUN_MAP_ADDR				m_FunMapAddr;
	MEDIALIB_CALLBACK_FUN_UNMAP_ADDR			m_FunUnmapAddr;
	MEDIALIB_CALLBACK_FUN_REG_BITS_WRITE		m_FunRegBitsWrite;

	//add for hardware mutex
	MEDIALIB_CALLBACK_FUN_VIDEO_HW_LOCK			m_FunVideoHWLock;
	MEDIALIB_CALLBACK_FUN_VIDEO_HW_UNLOCK		m_FunVideoHWUnlock;

	//add for using api about fifo in multithread
	MEDIALIB_CALLBACK_FUN_MUTEX_CREATE			m_FunMutexCreate;
	MEDIALIB_CALLBACK_FUN_MUTEX_LOCK			m_FunMutexLock;
	MEDIALIB_CALLBACK_FUN_MUTEX_UNLOCK			m_FunMutexUnlock;
	MEDIALIB_CALLBACK_FUN_MUTEX_RELEASE			m_FunMutexRelease;
}T_VIDEOLIB_CB;


/* Picture rotation for initialization */
typedef enum
{
    ENC_ROTATE_0 = 0,
    ENC_ROTATE_90R = 1, /* Rotate 90 degrees clockwise */
    ENC_ROTATE_90L = 2  /* Rotate 90 degrees counter-clockwise */
} EncPictureRotation;

/* Stream type for initialization */
typedef enum
{
    ENC_BYTE_STREAM = 0,    /* H.264 annex B: NAL unit starts with
                                 * hex bytes '00 00 00 01' */
    ENC_NAL_UNIT_STREAM = 1 /* Plain NAL units without startcode */
} EncStreamType;
/* */
typedef struct _VIDEO_ENC_H264_PAR
{
	T_U32				width;			//实际编码图像的宽度，能被4整除
	T_U32				height;			//实际编码图像的长度，能被2整除 
	EncPictureRotation	rotation;		//编码前yuv图像的旋转
	
	T_S32				frameRateDenom;	//帧率的分母
	T_S32				frameRateNum;	//帧率的分子
	T_S32				qpHdr;			//初始的QP的值
    T_U32 				qpMin;           
	T_U32 				qpMax;    
	T_U32 				gopLen;       
	T_S32				fixedIntraQp;	//为所有的intra帧设置QP
	T_S32				bitPerSecond;	//目标bps
	EncStreamType 	streamType;		//有startcode和没startcode两种
	//T_U8*				outStreamBuf;
	//T_U32				maxVideoSize;
	T_S32	lumWidthSrc;	//cropping功能时使用，表示原始图像的宽度
	T_S32	lumHeightSrc;	//cropping功能时使用，表示原始图像的高度
	T_S32	horOffsetSrc;	//Cropping功能使用,表示横轴上cropping图像的起始位置,相对于原始图像的左上角的位置
	T_S32	verOffsetSrc;	//Cropping功能使用,表示纵轴上cropping图像的起始位置,相对于原始图像的左上角的位置

	T_U32	enableCabac;		//置1为使用，0为不使用。是否使用cabac特性，也是baseline，main，high profile的区分点。不使用cabac特性为baseline。
	T_U32	transform8x8Mode;	//置2为使用，0为不使用。是否使用transform8x8特性，也是baseline，main，high profile的区分点。使用transform8x8特性为high profile。
}T_VIDEOLIB_ENC_H264_PAR;


/* Picture YUV type for initialization */
typedef enum
{
    ENC_YUV420_PLANAR = 0,  /* YYYY... UUUU... VVVV */
    ENC_YUV420_SEMIPLANAR = 1,  /* YYYY... UVUVUV...    */
    ENC_YUV422_INTERLEAVED_YUYV = 2,    /* YUYVYUYV...          */
    ENC_YUV422_INTERLEAVED_UYVY = 3,    /* UYVYUYVY...          */
    ENC_RGB565 = 4, /* 16-bit RGB           */
    ENC_BGR565 = 5, /* 16-bit RGB           */
    ENC_RGB555 = 6, /* 15-bit RGB           */
    ENC_BGR555 = 7, /* 15-bit RGB           */
    ENC_RGB444 = 8, /* 12-bit RGB           */
    ENC_BGR444 = 9, /* 12-bit RGB           */
    ENC_RGB888 = 10,    /* 24-bit RGB           */
    ENC_BGR888 = 11,    /* 24-bit RGB           */
    ENC_RGB101010 = 12, /* 30-bit RGB           */
    ENC_BGR101010 = 13  /* 30-bit RGB           */
} EncFrameType;

typedef enum
{
    ENC_THUMB_JPEG = 0x10,  /* Thumbnail coded using JPEG  */
    ENC_THUMB_PALETTE_RGB8 = 0x11,  /* Thumbnail stored using 1 byte/pixel */
    ENC_THUMB_RGB24 = 0x13  /* Thumbnail stored using 3 bytes/pixel */
} EncThumbFormat;

    
typedef struct _VIDEO_ENC_MJPEG_PAR
{
	EncFrameType frameType; 	//YUV类型
	EncThumbFormat format;		//format of the thumbnail
	T_U8	thumbWidth;			//Width in pixels of thumbnail
	T_U8	thumbHeight;		//height in pixels of thumbnail
	T_U8	*thumbData;			//thumbnail data
	T_U16	thumbDataLen;		//data amount in bytes
	T_U32	qLevel;				//quantization level
	T_U32				width;			//实际编码图像的宽度，能被4整除
	T_U32				height;			//实际编码图像的长度，能被2整除 	
	T_S32	lumWidthSrc;	//cropping功能时使用，表示原始图像的宽度
	T_S32	lumHeightSrc;	//cropping功能时使用，表示原始图像的高度
	T_S32	horOffsetSrc;	//Cropping功能使用,表示横轴上cropping图像的起始位置,相对于原始图像的左上角的位置
	T_S32	verOffsetSrc;	//Cropping功能使用,表示纵轴上cropping图像的起始位置,相对于原始图像的左上角的位置

}T_VIDEOLIB_ENC_MJPEG_PAR;

typedef struct _VIDEO_ENC_OPEN_PAR
{

	T_eVIDEO_DRV_TYPE			encFlag;
	T_VIDEOLIB_ENC_H264_PAR 	encH264Par;
	T_VIDEOLIB_ENC_MJPEG_PAR 	encMJPEGPar;

}T_VIDEOLIB_ENC_OPEN_INPUT;

typedef struct _VIDEO_ENC_IO_PAR
{
	T_S32		QP;					//编码当前帧的QP
	T_U8		mode;				//编码类型I/P帧
	T_pDATA		p_curr_data;		//yuv输入地址
	T_pDATA		p_vlc_data;			//码流输出地址
	T_U32		out_stream_size;	//输出码流的大小
}T_VIDEOLIB_ENC_IO_PAR;


 typedef struct _VIDEO_ENC_ENC_RC
{
    T_S32 		qpHdr;        
    T_U32 		qpMin;  
    T_U32 		qpMax;         
    T_U32 		bitPerSecond;    
    T_U32 		gopLen;    
    T_S32		fixedIntraQp;	//为所有的intra帧设置QP      
} T_VIDEOLIB_ENC_RC;


/*
 * @brief Initial Video codec library and allocate global resource
 *
 * @author Hu_Jing
 * @param	init_cb_fun	[in]	pointer of T_MEDIALIB_INIT_CB struct for callback func
 * @return T_BOOL
 * @retval	AK_TRUE		Initial ok
 * @retval	AK_FALSE	Initial fail
 */
T_BOOL VideoStream_Enc_Init(const T_VIDEOLIB_CB *init_cb_fun);


/**
 * @brief Destroy Video codec library and free global resource
 *
 * @author Hu_Jing
 * @return T_VOID
 */
T_VOID VideoStream_Enc_Destroy(void);


/*
 * @brief Reset Video codec 
 *
 * @author Hu_Jing
 * @param	T_VOID
 * @return  T_VOID
 */
T_VOID VideoStream_Enc_Reset(void);

/*
 * @brief Initial encoder to encoder
 *
 * @author	Hu_Jing
 * @param	open_input	[in]	pointer of T_VIDEOLIB_ENC_OPEN_INPUT
 * @return 	T_pVOID
 * @retval	AK_NULL		open failed
 * @retval	other		handle of videostream
 */
T_pVOID VideoStream_Enc_Open(const T_VIDEOLIB_ENC_OPEN_INPUT *open_input);

/*
 * @brief close video encoder
 *
 * @author 	Hu_Jing
 * @param	hVideo		[in]	pointer returned by VideoStream_Enc_Open
 * @return 	T_VOID
 */
T_VOID VideoStream_Enc_Close(T_pVOID hVS);

/*
 * @brief Encode one frame
 *
 * @author	Hu_Jing
 * @param	hVS1				[in]		pointer returned by VideoStream_Enc_Open
 * @param	hVS2				[in]		pointer returned by VideoStream_Enc_Open
 * @param	video_enc_io_param1	[in/out]	pointer of T_VIDEOLIB_ENC_IO_PAR
 * @param	video_enc_io_param2	[in/out]	pointer of T_VIDEOLIB_ENC_IO_PAR
 * @return 	T_S32
 * @retval	T_VOID
 */
T_VOID VideoStream_Enc_Encode(T_pVOID hVS1,T_pVOID hVS2, T_VIDEOLIB_ENC_IO_PAR *video_enc_io_param1,T_VIDEOLIB_ENC_IO_PAR *video_enc_io_param2);

/*
 * @brief Encode one frame
 *
 * @author	Hu_Jing
 * @param	hVS				[in]		pointer returned by VideoStream_Enc_Open
 * @param	video_enc_rc	[in]	pointer of T_VIDEOLIB_ENC_RC
 * @return 	T_BOOL
 * @retval	AK_FALSE		fail
 * @retval	AK_TRUE   	  	OK
 */
T_BOOL VideoStream_Enc_setRC(T_pVOID hVS,T_VIDEOLIB_ENC_RC *video_enc_rc);


#ifdef __cplusplus
}
#endif

#endif//_VIDEO_STREAM_CODEC_H_
