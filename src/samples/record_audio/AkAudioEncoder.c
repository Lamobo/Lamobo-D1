#include <stdarg.h>
#include <sdcodec.h>
#include <assert.h>

#include "AkAudioEncoder.h"
#include "Condition.h"
#include "log.h"

#define BITS_RATE_128Kbps		128000	//for mp3 bits out 128kbps
#define BITS_RATE_12Dot2Kbps	12200	//for amr bits out 12.2kbps
#define BITS_RATE_10Dot2Kbps	10200	//for amr bits out 10.2kbps
#define BITS_RATE_7Dot95Kbps	7950	//for amr bits out 7.95kbps
#define BITS_RATE_7Dot4Kbps		7400	//for amr bits out 7.4kbps
#define BITS_RATE_6Dot7Kbps		6700	//for amr bits out 6.7kbps
#define BITS_RATE_5Dot9Kbps		5900	//for amr bits out 5.9kbps
#define BITS_RATE_5Dot15Kbps	5150	//for amr bits out 5.15kbps
#define BITS_RATE_4Dot75Kbps	4750	//for amr bits out 4.75kbps

#define LOG_BUF_SIZE			1024

static T_VOID * 	g_EncLib 	= NULL;

/*
 * @brief		used for media lib to print 
 * @param	format [in] ,... [in]
 * @return	T_VOID
 * @retval	NONE
 */
static T_VOID ak_rec_cb_printf( T_pCSTR format, ... )
{
	//REC_TAG, used to identify print informations of media lib
#if 1
	va_list ap;
	char buf[LOG_BUF_SIZE];    

	va_start( ap, format );
	vsnprintf( buf, LOG_BUF_SIZE, format, ap );
	va_end( ap );

	fprintf( stderr, LOG_TAG );
	fprintf( stderr, "::" );
	fprintf( stderr, buf );
	fprintf( stderr, "\n" );
#endif
}

/*
 * @brief		used to alloc memory
 * @param	size [in] 
 * @return	T_pVOID
 * @retval	NULL for error,otherwise the handle of memory allocated.
 */
static T_pVOID ak_rec_cb_malloc( T_U32 size )
{
	//LOGV("malloc size 0x%lx",size);
	return malloc( size );
}

/*
 * @brief		free memory
 * @param	mem [in] 
 * @return	T_VOID
 * @retval	NONE
 */
static T_VOID ak_rec_cb_free( T_pVOID mem )
{
	//LOGV("free buffer %p",mem);
	return free( mem );
}

/*
 * @brief		media lib will call this while idle.
 * @param	ticks[in]
 * @return	T_BOOL
 * @retval	AK_TRUE for success,other for error.
 */
T_BOOL ak_rec_cb_lnx_delay(T_U32 ticks)
{
	//LOGV("delay 0x%lx ticks",ticks);
	
#ifdef ANDROID
	//usleep (ticks*1000);
	akuio_wait_irq();
	return true;
#else
	return (usleep(ticks*1000) == 0);
#endif
}

/**
* @brief  open the AK audio encode lib
* @author hankejia
* @date 2012-07-05
* @param[in] pstEncIn		the audio pcm data is params.
* @param[in] pstEncOut		the audio encode data is params.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/

extern T_U32	nAvgBytesPerSec;
extern T_U16	nBlockAlign;
extern T_U16	wBitsPerSample;
extern T_U16	nSamplesPerPacket;

T_S32 openSdEncodeLib( AudioEncIn * pstEncIn, AudioEncOut * pstEncOut )
{
	T_AUDIO_REC_INPUT 		inConfig;
	T_AUDIO_ENC_OUT_INFO	outConfig;
	bzero( &inConfig, sizeof( T_AUDIO_REC_INPUT ) );
	bzero( &outConfig, sizeof( T_AUDIO_ENC_OUT_INFO ) );
	
	if ( g_EncLib ) {
		loge( "already open encode lib, can't open again! please close first!\n" );
		return -1;
	}

	if ( ( NULL == pstEncIn ) || ( NULL == pstEncOut ) ) {
		loge( "openSdEncodeLib::Invalid parameter!\n" );
		return -1;
	}

	inConfig.cb_fun.Malloc = ak_rec_cb_malloc;
	inConfig.cb_fun.Free = ak_rec_cb_free;
	inConfig.cb_fun.printf = ak_rec_cb_printf;
	inConfig.cb_fun.delay = ak_rec_cb_lnx_delay;

	inConfig.enc_in_info.m_nChannel =1; //pstEncIn->nChannels;
	inConfig.enc_in_info.m_nSampleRate = pstEncIn->nSampleRate;
	inConfig.enc_in_info.m_BitsPerSample = pstEncIn->nBitsPerSample;
	
	switch ( pstEncOut->enc_type ) 
	{
	case ENC_TYPE_AMR:
		inConfig.enc_in_info.m_Type = _SD_MEDIA_TYPE_AMR;
		switch ( pstEncOut->nBitsRate ) 
		{
		case BITS_RATE_12Dot2Kbps:
			inConfig.enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR122;
			break;
		case BITS_RATE_10Dot2Kbps:
			inConfig.enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR102;
			break;
		case BITS_RATE_7Dot95Kbps:
			inConfig.enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR795;
			break;
		case BITS_RATE_7Dot4Kbps:
			inConfig.enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR74;
			break;
		case BITS_RATE_6Dot7Kbps:
			inConfig.enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR67;
			break;
		case BITS_RATE_5Dot9Kbps:
			inConfig.enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR59;
			break;
		case BITS_RATE_5Dot15Kbps:
			inConfig.enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR515;
			break;
		case BITS_RATE_4Dot75Kbps:
			inConfig.enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR475;
			break;
		default:
			loge( "SdCodec encode amr not support %f kbps!\n", 
				  (float)(pstEncOut->nBitsRate / 1000) );
			return -1;
		}
		break;
		
	case ENC_TYPE_AAC:
		inConfig.enc_in_info.m_Type = _SD_MEDIA_TYPE_AAC;
		break;
		
	case ENC_TYPE_ADPCM_IMA:
		inConfig.enc_in_info.m_Type = _SD_MEDIA_TYPE_ADPCM_IMA;
		inConfig.enc_in_info.m_private.m_adpcm.enc_bits = 4; //̶Ϊ4
		printf("enc adpcm \n");
		break;

	case ENC_TYPE_G711_ULOW:
		inConfig.enc_in_info.m_Type = _SD_MEDIA_TYPE_PCM_ULAW;
		break;

	case ENC_TYPE_G711_ALOW:
		inConfig.enc_in_info.m_Type = _SD_MEDIA_TYPE_PCM_ALAW;
		break;

	default:
		loge( "openSdEncodeLib::Unknow encode type!\n" );
		return -1;
	}

	//open the ak audio encode lib
	g_EncLib = _SD_Encode_Open(&inConfig, &outConfig);
	if ( NULL == g_EncLib ) {
		loge( "openSdEncodeLib::can't open SdCodec!\n" );
		return -1;
	}

	_SD_Encode_SetFramHeadFlag(g_EncLib, 0);
	nAvgBytesPerSec = outConfig.m_Private.m_adpcm.nAvgBytesPerSec;
	nBlockAlign = outConfig.m_Private.m_adpcm.nBlockAlign;
	wBitsPerSample = outConfig.m_Private.m_adpcm.wBitsPerSample;
	nSamplesPerPacket = outConfig.m_Private.m_adpcm.nSamplesPerPacket;
	printf("nSamplesPerPacket = %u,nAvgBytesPerSec = %lu, nBlockAlign = %u wBitsPerSample = %u ", 
	        nSamplesPerPacket, nAvgBytesPerSec, nBlockAlign, wBitsPerSample);

	return 0;
}

/**
* @brief  close the AK audio encode lib
* @author hankejia
* @date 2012-07-05
* @param[in] NONE
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 closeSdEncodeLib()
{
	if ( NULL == g_EncLib ) {
		return 0;
	}

	if( _SD_Encode_Close( g_EncLib ) != AK_TRUE ){
		loge("unable close encode lib!\n");
		return -1;
	}

	g_EncLib = NULL;
	
	return 0;
}

/**
* @brief  encode the pcm data
* @author hankejia
* @date 2012-07-05
* @param[in] pRawData	the audio pcm raw data
* @param[in] nRawDataSize	raw data buffer size
* @param[out] pEncBuf		encode out buffer
* @param[in] nEncBufSize	encode out buffer size
* @return T_S32
* @retval if >=0 encode return size and success, otherwise failed 
*/
T_S32 audio_pcm_encode( T_U8 * pRawData, T_U32 nRawDataSize, T_U8 * pEncBuf, T_U32 nEncBufSize )
{
	T_AUDIO_ENC_BUF_STRC encBuf	= { NULL, NULL, 0, 0 };
	
	assert( pRawData && pEncBuf );

	if ( NULL == g_EncLib ) {
		loge( "audio_pcm_encode::can't encode pcm data before open SdCodec! please call the openSdEncodeLib\n" );
		return -1;
	}

	encBuf.buf_in	= pRawData;
	encBuf.len_in 	= nRawDataSize;
	encBuf.buf_out	= pEncBuf;
	encBuf.len_out	= nEncBufSize;
	
	return _SD_Encode( g_EncLib, &encBuf );
}
