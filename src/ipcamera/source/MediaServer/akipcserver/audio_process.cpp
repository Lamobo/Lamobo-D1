#include <stdarg.h>
#include <sdcodec.h>
#include <assert.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/statfs.h>

#include "Condition.h"
#include "log.h"
#include "Thread.h"
#include "audio_enc.h"
#include "audio_process.h"
#include <queue>
#include "muxer.h"
#include <sdfilter.h>

using namespace std;

#define PCM_BUFFER_NUM (20)
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
	AUDIO_ENCODE_TYPE_CC encType;
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

typedef enum
{
	BFREE = 0,
	BWRITING,
	BREADING,
	BWRITE,
	BREAD
}AACBUFFLAG;
#define AACBUFLEN 1024
typedef struct
{
	int nflag;//0 freeing 1 writing 2 reading 3 can write 4 can read
	unsigned int nlen;
	unsigned char buf[AACBUFLEN];
	struct timeval tv;
}T_AACBUF;

#define MAX_AQUEUE_SIZE 5

static queue<T_AACBUF*> queueBuf[2];
static queue<T_AACBUF*> queueBufFree[2];

static Condition con[2];

extern int stop_record_flag;
static void initAACBuf()
{
	Condition_Initialize(&con[0]);
	Condition_Initialize(&con[1]);
	T_AACBUF* pbuf;
	for(int i = 0; i < MAX_AQUEUE_SIZE; i ++)
	{
		pbuf = new T_AACBUF;
		queueBufFree[0].push(pbuf);
		
		pbuf = new T_AACBUF;
		queueBufFree[1].push(pbuf);
	}
		
};

int getAACBuf(void* buf, unsigned int* nlen, struct timeval* ptv, int index)
{
	struct timeval t;
	T_AACBUF* pbuf;

	Condition_Lock(con[index]);
	gettimeofday(&t, NULL);
	if(queueBuf[index].empty())
	{
		Condition_Unlock(con[index]);
		*nlen = 0;
		return 0;
	}
	
	pbuf = queueBuf[index].front();
	memcpy(buf, pbuf->buf, pbuf->nlen);
	*ptv = pbuf->tv;
	*nlen = pbuf->nlen;
	queueBufFree[index].push(pbuf);
	queueBuf[index].pop();

	Condition_Unlock(con[index]);
	return 0;
	
}

static void putAACBuf(unsigned char* buf, unsigned int nlen, struct timeval* ptv, int index)
{
	T_AACBUF* paacbuf = NULL;
	Condition_Lock(con[index]);
	paacbuf = queueBufFree[index].front();
	queueBufFree[index].pop();
	memcpy(paacbuf->buf, buf, nlen);
	paacbuf->tv = *ptv;
	paacbuf->nlen = nlen;
	queueBuf[index].push(paacbuf);
		
	if(queueBuf[index].size() > MAX_AQUEUE_SIZE - 2)
	{	
		T_AACBUF* pbuftmp = queueBuf[index].front();
		queueBuf[index].pop();
		queueBufFree[index].push(pbuftmp);
	}
	Condition_Unlock(con[index]);
}
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
	
	initAACBuf();
	//init global param
	memset(&gaudioproc, 0, sizeof(gaudioproc));
	for (i=0; i<PCM_BUFFER_NUM; i++)
	{		
		gaudioproc.pcmData[i].pData = (T_U8*)malloc(oneBufsize+DECLARE_SAFE_SIZE);
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

		gaudioproc.nEncBufferSize = (oneBufsize - AUDIO_INFORM_LENGTH);
		gaudioproc.pEncBuffer = (T_U8*)malloc(gaudioproc.nEncBufferSize * 2);
		if (AK_NULL == gaudioproc.pEncBuffer)
		{
			return -1;
		}
	}
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

	//wait capture thread return
	pthread_join(gaudioproc.TID, NULL);
	gaudioproc.TID	= thread_zeroid();

	for (i=0; i<PCM_BUFFER_NUM; i++)
	{
		if (gaudioproc.pcmData[i].pData != AK_NULL)
		{
			free(gaudioproc.pcmData[i].pData);
		}
	}

	if ( gaudioproc.encType != ENC_TYPE_PCM) {
		if (AK_NULL != gaudioproc.pEncBuffer)
		{
			free(gaudioproc.pEncBuffer);
		}
		if ( audio_enc_close() < 0 ) {
			loge( "close sdcodec error!\n" );
			return -1;
		}
	}

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
	if (AK_TRUE == gaudioproc.bIsStop)
	{
		printf("audio_process_writedata isStop\n");
		return -1;
	}
	
	if (((gaudioproc.wpos+1)%PCM_BUFFER_NUM) == gaudioproc.rpos)
	{
		printf("audio_process_writedata full\n");
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
T_S32 audio_process_getpdata(T_pVOID *pdata, T_U32 *size)
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
#ifdef NOICE
extern 	T_pVOID		g_noicefilter;
#endif
//the thread create call back function
static T_pVOID thread_enc( T_pVOID user )
{
	T_AUDIO_PROC * handle = (T_AUDIO_PROC*)(user);
	T_S32 	ret = 0;
	T_U8 *pRawBuffer = NULL;
	T_S32 size = 0;

	T_U32	nDataLen = 0, nTimeStamp = 0;
	T_U32 nRecTimeStamp = 0;
	T_BOOL bNeedUpdateTime = AK_TRUE;
	//T_AUDIO_FILTER_BUF_STRC fbuf_strc;
	//int nReSRDataLen;
	if ( handle == NULL ) {
		loge( "thread_begin have a invail param!" );
		return NULL;
	}

	struct timeval tv,tvt;
	T_BOOL bNeedTimeStamp = AK_TRUE;
	while(1)
	{
		if (AK_TRUE == handle->bIsStop)
		{
			break;
		}
		gettimeofday(&tvt, NULL);
		ret = audio_process_getpdata((T_VOID**)&pRawBuffer, (T_U32*)&size);
		if ( ret != 0 ) 
		{
			usleep(2000);
			continue;
		}
		else
		{
			#ifdef NOICE
			if( g_noicefilter != 0 )
			{
				fbuf_strc.buf_in = pRawBuffer+AUDIO_INFORM_LENGTH;
				fbuf_strc.buf_out = pRawBuffer+AUDIO_INFORM_LENGTH; //输入输出允许同buffer，但是要保证buffer够大，因为输出数据会比输入数据长
				fbuf_strc.len_in = size-AUDIO_INFORM_LENGTH; // 这里指输入ibuf中的有效数据长度
				fbuf_strc.len_out = gaudioproc.nEncBufferSize;	 // 这里指输出buffer 大小，库里面做判断用，防止写buffer越界。
				nReSRDataLen = _SD_Filter_Control( g_noicefilter, &fbuf_strc );
			}
			#endif
			
			nTimeStamp = *((T_U32 *)pRawBuffer);
			if(bNeedTimeStamp)
			{
				bNeedTimeStamp = AK_FALSE;
				tv = tvt;
			}
			
			//encode the pcm data
			ret = audio_encode( pRawBuffer+AUDIO_INFORM_LENGTH, size-AUDIO_INFORM_LENGTH, 
										(T_U8*)handle->pEncBuffer, handle->nEncBufferSize*2 );
				
			if ( ret <= 0 ) {
				if ( ret < 0 )
					loge( "recorder thread encode pcm data error!\n" );
				else {
					if ( bNeedUpdateTime )
						nRecTimeStamp = nTimeStamp;
						bNeedUpdateTime = AK_FALSE;
				}
				audio_process_useok();
				continue;
			}

			bNeedTimeStamp = AK_TRUE;
			if ( !bNeedUpdateTime ) {
				nTimeStamp = nRecTimeStamp;
			}

			bNeedUpdateTime = AK_TRUE;
			nDataLen = ret;
			if( stop_record_flag != 1 )
				mux_write_data(0, handle->pEncBuffer+7, nDataLen-7, nTimeStamp, 0);
				
			putAACBuf(handle->pEncBuffer, nDataLen, &tv, 0);
			putAACBuf(handle->pEncBuffer, nDataLen, &tv, 1);
			audio_process_useok();
		}
	}
//	close(fid);
	return NULL;
}


