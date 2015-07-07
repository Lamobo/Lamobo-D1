#include <stdarg.h>
#include <sdcodec.h>
#include <assert.h>

#include "Condition.h"
#include "log.h"
#include "Thread.h"
#include "audio_enc.h"
#include "audio_process.h"
#include "muxer.h"
#include <sdfilter.h>

#define DV_DENOISE
#undef DV_DENOISE

#define PCM_BUFFER_NUM (100)
#define DECLARE_SAFE_SIZE	20
#define ENCODE_SIZE 4096
typedef struct {
	//buffer manage
	T_pDATA pData;
	T_U32 size;
}T_PCM_DATA;


typedef struct {
	nthread_t	TID;
	T_BOOL		bIsStop;
	AUDIO_ENCODE_TYPE encType;
	//buffer manage
	T_PCM_DATA pcmData[PCM_BUFFER_NUM];
	T_U32 wpos;
	T_U32 rpos;
	T_U32 oneBufsize;
	
	T_pDATA pEncBuffer;
	T_U32	nEncBufferSize;

}T_AUDIO_PROC;

static T_AUDIO_PROC gaudioproc;

static T_pVOID thread_enc( T_pVOID user );

#ifdef DV_DENOISE
T_pVOID			g_noicefilter = NULL;

static T_S32 OpenNoiceFilter( int nChannels, int nBitsPerSample, int nSampleRate )
{
	T_AUDIO_FILTER_INPUT s_info;

	s_info.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	s_info.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
	s_info.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	s_info.cb_fun.delay = AK_NULL;

	s_info.m_info.m_BitsPerSample = nBitsPerSample;
	s_info.m_info.m_Channels 		= nChannels;
	s_info.m_info.m_SampleRate 	= nSampleRate;
	
	s_info.m_info.m_Type = _SD_FILTER_DENOICE;

	s_info.m_info.m_Private.m_NR.ASLC_ena = 1;
	s_info.m_info.m_Private.m_NR.NR_Level = 0;

	g_noicefilter = _SD_Filter_Open(&s_info);

	if (AK_NULL == g_noicefilter) 
	{
		loge( "can't open the noice filter!\n" );
		return -1;
	}

	return 0;
}
#endif

/**
* @brief  open the AK audio process
* @author dengzhou 
* @date 2013-04-25
* @param[in] NONE
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_process_open( AudioEncIn * pstEncIn, AudioEncOut * pstEncOut, T_U32 oneBufsize )
{
	int i;
	int j;
	T_S32 ret;
	
	//init global param
	memset(&gaudioproc, 0, sizeof(gaudioproc));
	for (i=0; i<PCM_BUFFER_NUM; i++)
	{		
		gaudioproc.pcmData[i].pData = AK_NULL;
		gaudioproc.pcmData[i].pData = malloc(oneBufsize+DECLARE_SAFE_SIZE);
		if (AK_NULL == gaudioproc.pcmData[i].pData)
		{
			for (j=0; j<PCM_BUFFER_NUM; j++)
			{
				if (gaudioproc.pcmData[j].pData != AK_NULL)
				{
					free(gaudioproc.pcmData[j].pData);
				}
			}
			return -1;
		}
	}
	gaudioproc.wpos = 0;
	gaudioproc.rpos = 0;

	gaudioproc.encType = pstEncOut->enc_type;
	//init audio codec
	if (ENC_TYPE_PCM != pstEncOut->enc_type)
	{
		AudioEncIn	stEncIn;
		AudioEncOut	stEncOut;
		
		stEncIn.nChannels = 1;
		stEncIn.nBitsPerSample = 16;
		stEncIn.nSampleRate = pstEncIn->nSampleRate;


		stEncOut.nBitsRate = pstEncOut->nBitsRate;
		stEncOut.enc_type = pstEncOut->enc_type;
			
		ret = audio_enc_open( &stEncIn, &stEncOut );
		if ( ret < 0 ) {
			loge( "can't open the SdCodec!\n" );
		}

		gaudioproc.nEncBufferSize = ENCODE_SIZE;//oneBufsize - AUDIO_INFORM_LENGTH;
		gaudioproc.pEncBuffer = malloc(ENCODE_SIZE);//malloc(gaudioproc.nEncBufferSize);
		if (AK_NULL == gaudioproc.pEncBuffer)
		{
			return -1;
		}
	}

#ifdef DV_DENOISE
	//open noicefilter 
	ret = OpenNoiceFilter(1, 16, pstEncIn->nSampleRate);
	if( ret < 0 )
	{
		printf( " don't open filter \n" );
	}
#endif

	//create the encode data thread
	pthread_attr_t SchedAttr;
	struct sched_param	SchedParam;
	memset(&SchedAttr, 0, sizeof(pthread_attr_t));
	memset(&SchedParam, 0, sizeof(SchedParam));
				
	pthread_attr_init( &SchedAttr );				
	SchedParam.sched_priority = 60;	
	pthread_attr_setschedparam( &SchedAttr, &SchedParam );
	pthread_attr_setschedpolicy( &SchedAttr, SCHED_RR );
	if ( ( ret = pthread_create( &(gaudioproc.TID), &SchedAttr, thread_enc, &gaudioproc ) ) != 0 ) {
		pthread_attr_destroy(&SchedAttr);
		loge( "unable to create a thread for read data ret = %d!\n", ret );
		return -1;	
	}

	pthread_attr_destroy(&SchedAttr);
	
	return 0;
}


/**
* @brief  close the AK audio process
* @author dengzhou 
* @date 2013-04-25
* @param[in] NONE
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_process_close()
{	
	int i;
	gaudioproc.bIsStop = AK_TRUE;
	printf("audio process close \n");
	//wait capture thread return
	pthread_join(gaudioproc.TID, NULL);
	gaudioproc.TID	= thread_zeroid();

#ifdef DV_DENOISE
	if (g_noicefilter)
	{
		_SD_Filter_Close(g_noicefilter);
		g_noicefilter = NULL;
	}
#endif

	for (i=0; i<PCM_BUFFER_NUM; i++)
	{
		if (gaudioproc.pcmData[i].pData != AK_NULL)
		{
			printf("free pData %p \n", gaudioproc.pcmData[i].pData);
			free(gaudioproc.pcmData[i].pData);
		}
	}
	printf("free gaudioproc pdata \n");
	if ( gaudioproc.encType != ENC_TYPE_PCM) 
	{
		if (AK_NULL != gaudioproc.pEncBuffer)
		{
			free(gaudioproc.pEncBuffer);
		}
		if ( audio_enc_close() < 0 ) {
			loge( "close sdcodec error!\n" );
			return -1;
		}
	}
	printf("free encbuff \n");
	return 0;
}

/**
* @brief  write audio date to process
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_process_writedata(T_pVOID pdata, T_U32 size)
{	
	if (gaudioproc.bIsStop)
	{
		return -1;
	}
	
	if (((gaudioproc.wpos+1)%PCM_BUFFER_NUM) == gaudioproc.rpos)
	{
		printf("Audio Write DATA overflow\n");
		return -1;
	}

	memcpy(gaudioproc.pcmData[gaudioproc.wpos].pData, pdata, size);
	gaudioproc.pcmData[gaudioproc.wpos].size = size;
	gaudioproc.wpos++;
	if (gaudioproc.wpos >= PCM_BUFFER_NUM)
	{
		gaudioproc.wpos = 0;
	}
	return 0;
}

/**
* @brief  get pcm data from buffer
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_process_getpdata(T_pDATA *pdata, T_U32 *size)
{	
	if (gaudioproc.wpos == gaudioproc.rpos)
	{
		return -1;
	}

	*pdata = gaudioproc.pcmData[gaudioproc.rpos].pData;
	*size = gaudioproc.pcmData[gaudioproc.rpos].size;
	return 0;
}

/**
* @brief call this when the buffer is unused
* @author dengzhou 
* @date 2013-04-25
* @param[in] NONE
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_process_useok()
{	
	gaudioproc.rpos++;
	if (gaudioproc.rpos >= PCM_BUFFER_NUM)
	{
		gaudioproc.rpos = 0;
	}
	return 0;
}

static T_U32 nTimeStamp = 0;

//the thread create call back function
static T_pVOID thread_enc( T_pVOID user )
{
	T_AUDIO_PROC * handle = (T_AUDIO_PROC*)(user);
	T_S32 	ret = 0;
	T_U8 *pRawBuffer = NULL;
	T_U32 size = 0;

	T_U32	nDataLen = 0;//, nTimeStamp = 0;
	T_U32 nRecTimeStamp = 0;
	T_BOOL bNeedUpdateTime = AK_TRUE;	
	
	if (handle == NULL) {
		loge( "thread_begin have a invail param!" );
		return NULL;
	}

	while(1)
	{
		if (handle->bIsStop)
		{
			break;
		}

		ret = audio_process_getpdata(&pRawBuffer, &size);
		if (ret != 0) 
		{
			loge( "audio_process_getpdata error! ret = %d\n", ret );
			usleep(1000);
			continue;
		}

#ifdef DV_DENOISE
		if (g_noicefilter)
		{
			int nReSRDataLen;
			T_AUDIO_FILTER_BUF_STRC fbuf_strc;
			
			fbuf_strc.buf_in = pRawBuffer+AUDIO_INFORM_LENGTH;
			fbuf_strc.buf_out = pRawBuffer+AUDIO_INFORM_LENGTH; //输入输出允许同buffer，但是要保证buffer够大，因为输出数据会比输入数据长
			fbuf_strc.len_in = size-AUDIO_INFORM_LENGTH; // 这里指输入ibuf中的有效数据长度
			fbuf_strc.len_out = gaudioproc.nEncBufferSize;	 // 这里指输出buffer 大小，库里面做判断用，防止写buffer越界。
			nReSRDataLen = _SD_Filter_Control(g_noicefilter, &fbuf_strc );
		}
#endif
		nTimeStamp = *((T_U32 *)pRawBuffer);
		//encoder
		if (ENC_TYPE_PCM == handle->encType)
		{
			mux_addAudio(pRawBuffer+AUDIO_INFORM_LENGTH, size-AUDIO_INFORM_LENGTH, nTimeStamp);
		}
		else
		{
			//encode the pcm data
			ret = audio_encode(pRawBuffer+AUDIO_INFORM_LENGTH, size-AUDIO_INFORM_LENGTH,
									handle->pEncBuffer, ENCODE_SIZE/*handle->nEncBufferSize*/ );
			if ( ret <= 0 )
			{
				if ( ret < 0 )
					printf( "recorder thread encode pcm data error!\n" );
				else
				{
					if ( bNeedUpdateTime )
						nRecTimeStamp = nTimeStamp;
					bNeedUpdateTime = AK_FALSE;
				}
				
				audio_process_useok();
				continue;
			}
			
			if ( !bNeedUpdateTime ) {
				nTimeStamp = nRecTimeStamp;
			}
			
			bNeedUpdateTime = AK_TRUE;
			nDataLen = ret;
			if (mux_addAudio(handle->pEncBuffer, nDataLen, nTimeStamp) < 0)
			{
				loge("mux Add Audio err, exit\n");
				//g_exit = 1;
			}
		}
		audio_process_useok();
	}

	return NULL;
}


