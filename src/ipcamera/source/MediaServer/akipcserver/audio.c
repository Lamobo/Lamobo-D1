
#include <assert.h>
#include <errno.h>
#ifdef	CHIP_ID_AK37XX
#include <akmedialib/sdfilter.h>
#endif
#include "AkAlsaHardware.h"
#include "log.h"
#include "Thread.h"
#include "audio.h"


#define PER_AUDIO_DURATION		32		// 30ms

#define DEFAULT_ALSA_HARDWARE	"default"
#define CAPTURE_ROUTE_NAME				"ADC Capture Route"
#define CAPTURE_LINE_ROUTE_NAME		"LineIn Capture Route"
#define DEFAULT_SAMPLE_RATE_DA	ALSA_DEFAULT_SAMPLE_RATE
#define DEFAULT_SAMPLE_RATE_AD	8000
#define DEFAULT_RESAMPLE_UNIT	8	//can use 2,4,8
#define DECLARE_SAFE_SIZE	20

//#define AUDIO_TEST_FILE 

typedef struct {
	nthread_t	TID;

	T_BOOL		bIsStop;
	T_U32		nGetPacketNum;
	T_U32		nPeriodDuration;
	T_U32		nOnceBufSize;

	T_U32		nSampleRate;
	T_U32		nBitsPerSample;
	T_U32		nChannels;
	T_U32		nBitsRate;
	AUDIO_ENCODE_TYPE_CC enc_type;
}T_AUDIO_READ;

AkAlsaHardware	stAlsaModule;
alsa_handle_t	stAlsaHandle;

static T_AUDIO_READ gaudio;
extern T_S32 OpenAecFilter( alsa_handle_t * handle );
extern T_S32 closeAecFilter( alsa_handle_t * handle );
T_S32 Openaec( void );
T_S32 Closeaec( void );


//the thread create call back function
static T_pVOID thread_readAD( T_pVOID user )
{
	T_AUDIO_READ * handle = (T_AUDIO_READ*)(user);
	T_S32 	ret = 0;
	T_U8 *pRawBuffer = NULL;
	T_U8 *pdata = NULL;
	T_U64 bytenum = 0;
//	T_U16 bytems = gaudio.nSampleRate * 2 /1000;
//	T_U8  voerrunflag = 0;
//	struct timeval  tv1, tv2;
	if ( handle == NULL ) {
		loge( "thread_begin have a invail param!" );
		return NULL;
	}
	T_U32 rate = gaudio.nSampleRate * 2;
	//alloc the buffer to load the pcm data
	pRawBuffer = (T_U8*)malloc(gaudio.nOnceBufSize+AUDIO_INFORM_LENGTH);
	if ( NULL == pRawBuffer ) {
		loge( "the audio record thread can't run, out of memory!\n" );
		return NULL;
	}
	pdata = pRawBuffer + AUDIO_INFORM_LENGTH;
	bzero(pRawBuffer, gaudio.nOnceBufSize);
	
#ifdef AUDIO_TEST_FILE
	long fid = open("test.pcm", O_RDWR | O_CREAT | O_TRUNC);
	if(fid <= 0)
	{
		printf("open file failed\r\n");
		return 0;
	}
#endif

	while(1)
	{
		if (AK_TRUE == gaudio.bIsStop)
		{
			break;
		}

		ret = stAlsaModule.ReadAD(&stAlsaHandle, pdata, gaudio.nOnceBufSize);
		if ( ret < 0 ) {
//			printf( "recorder thread read audio from AD error! ret = %d\n", ret );
			continue;
		}else if ( ret == 0 ) {
			printf( "AD running error! but already recover this error!\n" );
			continue;
		}
		
		else if ( ret == -1 ) 
		{
			//overrun occur,need restart.		
	//		printf( "overrun occur need restart the timestamp for sync video time stamp!ret = %d\n", ret );
			//gettimeofday(&tv1, NULL);
			//voerrunflag = 1;
			continue;
		}
		#if 0
		if( 1 == voerrunflag )
		{
			voerrunflag = 0;
			T_U32 time;
			
			gettimeofday(&tv2, NULL);
			time = ((tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec)) /1000;
			bytenum += time*bytems;
			printf("add overrun time %d num %d\n", time, time*bytems);
		}
		#endif
#ifdef AUDIO_TEST_FILE
		write( fid, pdata, gaudio.nOnceBufSize );
#endif
		bytenum += ret;
		*((T_U32 *)pRawBuffer) = bytenum * 1000 / rate;
		//gaudio.nGetPacketNum++;
		//*((T_U32 *)pRawBuffer) = gaudio.nGetPacketNum*gaudio.nPeriodDuration;
		audio_process_writedata(pRawBuffer,ret+AUDIO_INFORM_LENGTH);
	}

#ifdef AUDIO_TEST_FILE
	close(fid);
#endif
	free(pRawBuffer);
	return NULL;
}

/**
* @brief  init to get pcm from ad
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] input
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int audio_open(T_AUDIO_INPUT *input)
{
	getAlsaModule(&stAlsaModule);

	gaudio.nSampleRate = input->nSampleRate;
	gaudio.nBitsPerSample = input->nBitsPerSample;
	gaudio.nChannels = input->nChannels;
	gaudio.enc_type = input->enc_type;
	gaudio.nBitsRate = input->nBitsRate;
	return 0;
}

/**
* @brief  close adc
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int audio_close(void)
{
	if (stAlsaModule.Close(&stAlsaHandle, AK_TRUE) < 0 ) {
		loge( "can't close the AD!\n" );
		return -1;
	}
	return 0;
}

/**
* @brief start to get pcm from ad
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int audio_start(void)
{
	T_S32	ret = 0;
	T_U32 nBytesPerSample;
	T_U32 nActuallySR;
	ret = stAlsaModule.OpenAD(&stAlsaHandle, NULL);
	if ( ret < 0 ) {
		loge( "StartRec::can't open AD!\n" );
		return -1;
	}

	
	ret = stAlsaModule.SetMicIn(&stAlsaHandle);
	if ( ret < 0 ) {
		loge( "StartRec::can't set the audio in dev!\n" );
		return -1;
	}

	ret = stAlsaModule.SetParams(&stAlsaHandle, gaudio.nSampleRate, 
										  gaudio.nBitsPerSample, gaudio.nChannels, 1);
	if ( ret < 0 ) {
		loge( "StartRec::set audio params failed!\n" );
		return -1;
	}
	
	SetAudioMicVolume( 60 );
	gaudio.nGetPacketNum = 0;
	gaudio.nPeriodDuration = PER_AUDIO_DURATION;
	if (stAlsaHandle.nFormat == SND_PCM_FORMAT_S16_LE) {
		nBytesPerSample = 2;
	}else {
		nBytesPerSample = 1;
	}
	switch (gaudio.nSampleRate)
	{
		case 8000:
			nActuallySR = 7990;
		break;

		case 11025:
			nActuallySR = 10986;
		break;

		case 16000:
			nActuallySR = 15980;
			break;
		case 22050:
			nActuallySR = 21972;
			break;
		case 32000:
			nActuallySR = 31960;
			break;
		default:
			nActuallySR = gaudio.nSampleRate;
	}
	gaudio.nOnceBufSize = (gaudio.nSampleRate * gaudio.nChannels *
							   gaudio.nPeriodDuration * nBytesPerSample ) / 1000;

	if(gaudio.nOnceBufSize %2 != 0)
	{
		gaudio.nOnceBufSize++;
	}
//	if(gaudio.enc_type == ENC_TYPE_AAC )
//	{
//		gaudio.nOnceBufSize = 2048;
//	}
//	printf("nOnceBufSize = %d \n", gaudio.nOnceBufSize);
	//set pcm data input device

	//create the capture data thread

	pthread_attr_t SchedAttr;
	struct sched_param	SchedParam;
	memset(&SchedAttr, 0, sizeof(pthread_attr_t));
	memset(&SchedParam, 0, sizeof(SchedParam));
				
	pthread_attr_init( &SchedAttr );				
	SchedParam.sched_priority = 60;	
	pthread_attr_setschedparam( &SchedAttr, &SchedParam );
	pthread_attr_setschedpolicy( &SchedAttr, SCHED_RR );
	if ( ( ret = pthread_create( &(gaudio.TID), &SchedAttr, thread_readAD, &gaudio ) ) != 0 ) {
		pthread_attr_destroy(&SchedAttr);
		loge( "unable to create a thread for read data ret = %d!\n", ret );
		return -1;	
	}

	pthread_attr_destroy(&SchedAttr);

	AudioEncIn	stEncIn;
	AudioEncOut	stEncOut;

	stEncIn.nBitsPerSample = nBytesPerSample * 8;
	stEncIn.nChannels = gaudio.nChannels;
	stEncIn.nSampleRate = gaudio.nSampleRate;

	//ÓÃ»§ÉèÖÃ
	stEncOut.nBitsRate = gaudio.nBitsRate;
	stEncOut.enc_type = gaudio.enc_type;
	
	audio_process_open(&stEncIn, &stEncOut, gaudio.nOnceBufSize+AUDIO_INFORM_LENGTH);
	return 0;
}

/**
* @brief stop to get pcm from ad
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int audio_stop(void)
{
	gaudio.bIsStop = AK_TRUE;
	//wait capture thread return
	pthread_join(gaudio.TID, NULL);
	gaudio.TID	= thread_zeroid();

	audio_process_close();
	return 0;
}
T_S32 SetAudioMicVolume( T_U32 volume ) 
{
	return stAlsaModule.SetMixerLevel( &stAlsaHandle, MIC_CAPTURE_VOLUME, volume);
}

T_S32 GetAudioMicVolume( void )
{
	T_S32	 volume;
	stAlsaModule.GetMixerLevel( &stAlsaHandle, MIC_CAPTURE_VOLUME, &volume );
	return volume;
}

T_S32 Openaec( void )
{
	OpenAecFilter( &stAlsaHandle );
	return 0;
}

T_S32 Closeaec( void )
{
	closeAecFilter( &stAlsaHandle );
	return 0;
}

