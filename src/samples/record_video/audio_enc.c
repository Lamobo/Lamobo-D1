#include <stdarg.h>
#include <sdcodec.h>
#include <assert.h>

#include "audio_enc.h"
#include "Condition.h"
#include "log.h"
#include "asyncwrite.h"

#define BITS_RATE_128Kbps		128000	//for mp3 bits out 128kbps
#define BITS_RATE_12Dot2Kbps	12200	//for amr bits out 12.2kbps
#define BITS_RATE_10Dot2Kbps	10200	//for amr bits out 10.2kbps
#define BITS_RATE_7Dot95Kbps	7950	//for amr bits out 7.95kbps
#define BITS_RATE_7Dot4Kbps		7400	//for amr bits out 7.4kbps
#define BITS_RATE_6Dot7Kbps		6700	//for amr bits out 6.7kbps
#define BITS_RATE_5Dot9Kbps		5900	//for amr bits out 5.9kbps
#define BITS_RATE_5Dot15Kbps	5150	//for amr bits out 5.15kbps
#define BITS_RATE_4Dot75Kbps	4750	//for amr bits out 4.75kbps

static T_VOID * 			g_EncLib 	  = NULL;
static T_U8 *				g_EncBuffer   = NULL;
static T_U32				g_nEncBufSize = 0;
static T_U32				g_nBufUse 	  = 0;
static T_U32				g_nChannels   = 0;
static AUDIO_ENCODE_TYPE	g_nEncType 	  = ENC_TYPE_COUNT;

#define MP3_MAX_INPACKET_SIZE		8192	// mp3 encoder max input is 8k
#define AAC_SINGLE_INPACKET_SIZE 	2048	// aac one channel input is 2k
#define AAC_STEREO_INPACKET_SIZE	4096	// aac two channels input is 4k




/**
* @brief  open the AK audio encode lib
* @author hankejia
* @date 2012-07-05
* @param[in] pstEncIn		the audio pcm data is params.
* @param[in] pstEncOut		the audio encode data is params.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_enc_open( AudioEncIn * pstEncIn, AudioEncOut * pstEncOut )
{
	T_AUDIO_REC_INPUT 		inConfig;
	T_AUDIO_ENC_OUT_INFO	outConfig;
	int err;
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

	if ( NULL != g_EncBuffer ) {
		free( g_EncBuffer );
		g_EncBuffer = NULL;
	}

	//config the call back function
	inConfig.cb_fun.Malloc = ak_rec_cb_malloc;
	inConfig.cb_fun.Free = ak_rec_cb_free;
	inConfig.cb_fun.printf = ak_rec_cb_printf;
	inConfig.cb_fun.delay = ak_rec_cb_lnx_delay;

	//config the encode input pcm data is params
	inConfig.enc_in_info.m_nChannel = pstEncIn->nChannels;
	inConfig.enc_in_info.m_nSampleRate = pstEncIn->nSampleRate;
	inConfig.enc_in_info.m_BitsPerSample = pstEncIn->nBitsPerSample;

	g_nChannels = pstEncIn->nChannels;
	g_nEncType = pstEncOut->enc_type;
	
	switch ( pstEncOut->enc_type ) 
	{
		
	case ENC_TYPE_AAC:
		inConfig.enc_in_info.m_Type = _SD_MEDIA_TYPE_AAC;

		if ( 1 == pstEncIn->nChannels ) {
			g_EncBuffer = (T_U8 *)malloc( AAC_SINGLE_INPACKET_SIZE );
			g_nEncBufSize = AAC_SINGLE_INPACKET_SIZE;
		} else if ( 2 == pstEncIn->nChannels ) {
			g_EncBuffer = (T_U8 *)malloc( AAC_STEREO_INPACKET_SIZE );
			g_nEncBufSize = AAC_STEREO_INPACKET_SIZE;
		}else {
			loge( "openSdEncodeLib::no support %d channels!\n", pstEncIn->nChannels );
			return -1;
		}

		if ( NULL == g_EncBuffer ) {
			loge( "openSdEncodeLib::out of memory!\n"  );
			return -1;
		}

		bzero( g_EncBuffer, g_nEncBufSize );
		
		break;
		
	case ENC_TYPE_ADPCM_IMA:
		inConfig.enc_in_info.m_Type = _SD_MEDIA_TYPE_ADPCM_IMA;
		inConfig.enc_in_info.m_private.m_adpcm.enc_bits = 4; //̶Ϊ4
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

	
	err = _SD_Encode_SetFramHeadFlag(g_EncLib, 1);

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
T_S32 audio_enc_close()
{
	if ( NULL == g_EncLib ) {
		return 0;
	}

	if( _SD_Encode_Close( g_EncLib ) != AK_TRUE ){
		loge("unable close encode lib!\n");
		return -1;
	}

	g_EncLib = NULL;

	if ( g_EncBuffer ) {
		free( g_EncBuffer );
		g_EncBuffer = NULL;
	}

	//clear for next use 
	g_nEncBufSize = 0;
	g_nBufUse 	  = 0;
	g_nChannels   = 0;
	g_nEncType 	  = ENC_TYPE_COUNT;
	
	return 0;
}

#if 0
/**
* @brief  encode the pcm data to the mp3 data
* @author hankejia
* @date 2012-07-05
* @param[in] pRawData	the audio pcm raw data
* @param[in] nRawDataSize	raw data buffer size
* @param[out] pEncBuf		encode out buffer
* @param[in] nEncBufSize	encode out buffer size
* @return T_S32
* @retval if >=0 encode return size and success, otherwise failed 
*/
static T_S32 audio_pcm2mp3( T_U8 * pRawData, T_U32 nRawDataSize, T_U8 * pEncBuf, T_U32 nEncBufSize )
{
	T_AUDIO_ENC_BUF_STRC encBuf	= { NULL, NULL, 0, 0 };
	T_S32 ret = 0;
	
	encBuf.buf_in	= pRawData;
	encBuf.len_in 	= nRawDataSize;
	encBuf.buf_out	= pEncBuf;
	encBuf.len_out	= nEncBufSize;
	
	while ( AK_TRUE ) {

		if ( encBuf.len_out < 0 ) {
			loge( "audio_pcm2mp3::mp3 out buffer out of memory!\n" );
			return -1;
		}
		
		if ( nRawDataSize > MP3_MAX_INPACKET_SIZE ) {
			encBuf.len_in = MP3_MAX_INPACKET_SIZE;
		}
		
		ret = _SD_Encode( g_EncLib, &encBuf );
		if ( ret < 0 ) {
			return ret;
		}

		nRawDataSize -= encBuf.len_in;
		if ( nRawDataSize == 0 ) {
			break;
		}

		encBuf.buf_in = encBuf.buf_in + encBuf.len_in;
		encBuf.buf_out += ret;
		encBuf.len_out -= ret;
	}

	return (T_U8*)encBuf.buf_out - (T_U8*)pEncBuf;
}
#endif

/**
* @brief  encode the pcm data to the aac data
* @author hankejia
* @date 2012-07-05
* @param[in] pRawData	the audio pcm raw data
* @param[in] nRawDataSize	raw data buffer size
* @param[out] pEncBuf		encode out buffer
* @param[in] nEncBufSize	encode out buffer size
* @return T_S32
* @retval if >=0 encode return size and success, otherwise failed 
*/
static T_S32 audio_pcm2aac( T_U8 * pRawData, T_U32 nRawDataSize, T_U8 * pEncBuf, T_U32 nEncBufSize )
{
	T_AUDIO_ENC_BUF_STRC encBuf	= { NULL, NULL, 0, 0 };
	T_S32 ret = 0;
	T_S32 nEncSize = 0;
	encBuf.buf_out	= pEncBuf;
	encBuf.len_out	= nEncBufSize;
	
	while( nRawDataSize > 0 ) 
	{
		
		if ( nRawDataSize < ( g_nEncBufSize - g_nBufUse ) ) 
		{
			memcpy( g_EncBuffer + g_nBufUse, pRawData, nRawDataSize );
			g_nBufUse += nRawDataSize;
			
			return nEncSize;
		} else if ( nRawDataSize >= ( g_nEncBufSize - g_nBufUse ) ) 
		{
			memcpy( g_EncBuffer + g_nBufUse, pRawData, g_nEncBufSize - g_nBufUse );
			

			pRawData += (g_nEncBufSize - g_nBufUse);
			
			nRawDataSize -= (g_nEncBufSize - g_nBufUse);
			g_nBufUse = g_nEncBufSize;
			
		}else 
		{
			printf( "audio_pcm2aac::how could this happend?\n" );
			ret = -1;
			break;
		}
		
		encBuf.buf_in	= g_EncBuffer;
		encBuf.len_in 	= g_nEncBufSize;
		encBuf.buf_out	= pEncBuf + nEncSize;
		encBuf.len_out	= nEncBufSize - nEncSize;
		
		if ( encBuf.len_out <= 0 ) {
			printf( "audio_pcm2aac::the encode out buffer too small!\n" );
			ret = -1;
			break;
		}
		
		ret = _SD_Encode( g_EncLib, &encBuf );
		if ( ret < 0 ) {
			break;
		}
		
		nEncSize += ret;

		g_nBufUse = 0;
	}

	if ( ret < 0 )
		g_nBufUse = 0;
	
	return ret;
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
T_S32 audio_encode( T_U8 * pRawData, T_U32 nRawDataSize, T_U8 * pEncBuf, T_U32 nEncBufSize )
{
	assert( pRawData && pEncBuf );

	if ( NULL == g_EncLib ) {
		loge( "audio_pcm_encode::can't encode pcm data before open SdCodec! please call the openSdEncodeLib\n" );
		return -1;
	}

	//open encode lib with aac encode type
	
	 if ( ENC_TYPE_AAC == g_nEncType ) {
		return audio_pcm2aac( pRawData, nRawDataSize, pEncBuf, nEncBufSize );
	}else {
	
		T_AUDIO_ENC_BUF_STRC encBuf	= { NULL, NULL, 0, 0 };
		
		encBuf.buf_in	= pRawData;
		encBuf.len_in 	= nRawDataSize;
		encBuf.buf_out	= pEncBuf;
		encBuf.len_out	= nEncBufSize;

		return _SD_Encode( g_EncLib, &encBuf );
	}

	loge( "audio_pcm_encode::how could this happen?\n" );
	return -1;
}

