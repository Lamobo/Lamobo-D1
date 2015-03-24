
#include <assert.h>
#include <errno.h>
#include "AkAlsaHardware.h"
#include "log.h"
#include <sdfilter.h>

#define DEFAULT_ALSA_HARDWARE	"default"
#define CAPTURE_ROUTE_NAME				"ADC Capture Route"
#define CAPTURE_LINE_ROUTE_NAME		"LineIn Capture Route"
#define DEFAULT_SAMPLE_RATE_DA	44100
#define DEFAULT_SAMPLE_RATE_AD	8000
#define DEFAULT_RESAMPLE_UNIT	8	//can use 2,4,8

#define SOURCE_DAC 			(0b001)
#define SOURCE_LINEIN 		(0b010)
#define SOURCE_MIC 			(0b100)
#define SIGNAL_SRC_MUTE		(0b000)


#define DECLARE_SAFE_SIZE	20
#define BUFF_LENG 	4092
typedef struct  audio_buff
{
	int audio_leng;
	int audio_index;
	int audio_last;
	T_U8 buff[BUFF_LENG];
}audio_buff;

static T_U8 temp_buff[BUFF_LENG];

audio_buff g_audio_buff = 
{
	.audio_leng		= 0,
	.audio_index	= 0,
	.audio_last		= 0,
};

//default capture pcm data is params
static alsa_handle_t _defaultsIn = {
	.handle 		= NULL,
	.ctrlHandle		= NULL,
	.nSampleRate	= DEFAULT_SAMPLE_RATE_AD,
	.nChannels		= 2,
	.nFormat 		= SND_PCM_FORMAT_S16_LE,
	.nBufferSize	= 10240,
	.nLatency		= 250000,
	.bIsAD			= AK_TRUE,
};

//default playback pcm data is params
static alsa_handle_t _defaultsOut = {
	.handle 		= NULL,
	.ctrlHandle		= NULL,
	.nSampleRate	= DEFAULT_SAMPLE_RATE_DA,
	.nChannels		= 2,
	.nFormat 		= SND_PCM_FORMAT_S16_LE,
	.nBufferSize	= DEFAULT_SAMPLE_RATE_DA / 5,
	.nLatency		= 200000,
	.bIsAD			= AK_FALSE,
};

static T_pVOID					pSdFilter = NULL;
static T_S32 Close( alsa_handle_t * handle );


static T_S32 OpenSDFilter( int nChannels, int nActuallySR, int nSampleRate )
{
	T_AUDIO_FILTER_INPUT 	s_ininfo;

	// open
	s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
	s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;

	
	s_ininfo.m_info.m_BitsPerSample = 16;

	
	s_ininfo.m_info.m_Channels = nChannels;
	s_ininfo.m_info.m_SampleRate = nActuallySR; //我们ADC的实际采样率

	s_ininfo.m_info.m_Type = _SD_FILTER_RESAMPLE;
	s_ininfo.m_info.m_Private.m_resample.maxinputlen = 0;
	s_ininfo.m_info.m_Private.m_resample.outSrindex = 0; 
	s_ininfo.m_info.m_Private.m_resample.outSrFree = nSampleRate; // 需要转换成的采样率
	s_ininfo.m_info.m_Private.m_resample.reSampleArithmetic = RESAMPLE_ARITHMETIC_1; //1是新算法，比较省内存；0是老算法，比较耗内存。

	pSdFilter = _SD_Filter_Open(&s_ininfo);
	if ( AK_NULL == pSdFilter ) {
		loge( "can't open the sd filter!\n" );
		return -1;
	}

	return 0;
}



//find out the device is direction
static snd_pcm_stream_t direction( alsa_handle_t *handle )
{
    return ( handle->bIsAD ) ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK;
}

//return stream name
static T_pCSTR streamName( alsa_handle_t *handle )
{
    return snd_pcm_stream_name( direction(handle) );
}

/**
* @brief   open alsa lib, pcm device
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @param[in] strDev  			use pcm device is name.
* @param[in] bIsAD  			tAD or DA.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 OpenAlsa( alsa_handle_t * handle, T_pSTR strDev, T_BOOL bIsAD )
{
	T_S32 ret = 0;
	T_pCSTR strDevTemp = strDev;
	T_BOOL bNeedTryDef = AK_TRUE;
	
	assert( handle );

	Close( handle );
	
	if ( NULL == strDevTemp ) {
		strDevTemp = DEFAULT_ALSA_HARDWARE;
		bNeedTryDef = AK_FALSE;
	}

	//config the default struct
	if ( bIsAD ) {
		memcpy( handle, &_defaultsIn, sizeof( alsa_handle_t ) );
	} else {
		memcpy( handle, &_defaultsOut, sizeof( alsa_handle_t ) );
	}

	while ( AK_TRUE )
	{
		if ( bIsAD ) {
			ret = snd_pcm_open( &(handle->handle), strDevTemp, SND_PCM_STREAM_CAPTURE, 0 );
		}else {
			ret = snd_pcm_open( &(handle->handle), strDevTemp, SND_PCM_STREAM_PLAYBACK, 0 );
		}

		if ( ( ret < 0 ) && bNeedTryDef ) {
			strDevTemp = DEFAULT_ALSA_HARDWARE;
			bNeedTryDef = AK_FALSE;
			continue;
		}
		
		break;
	}
	
	if ( ret < 0 ) {
        loge( "Failed to Initialize any ALSA %s device: %s\n", streamName(handle), strerror(errno));
		return ret;
	}

	//open the control
	ret = snd_ctl_open( &(handle->ctrlHandle), strDevTemp, 0 );
	if ( ret < 0 ) {
		loge( "can't not open %s device is control handle:%s\n", strDevTemp );
	}
	
	return ret;
}


/**
* @brief   config the alsa hardware params
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 setHardwareParams( alsa_handle_t *handle )
{
	snd_pcm_hw_params_t *hardwareParams = NULL;
	T_S32 err = 0;

	snd_pcm_uframes_t bufferSize = handle->nBufferSize;
	T_U32 requestedRate = handle->nSampleRate;
	T_U32 latency = handle->nLatency;

	
    T_BOOL validFormat = ( (int)(handle->nFormat) > SND_PCM_FORMAT_UNKNOWN ) && 
						 ( (int)(handle->nFormat) <= SND_PCM_FORMAT_LAST );
    T_pCSTR formatDesc = validFormat ? snd_pcm_format_description(
            handle->nFormat) : "Invalid Format";
    T_pCSTR formatName = validFormat ? snd_pcm_format_name(handle->nFormat)
            : "UNKNOWN";

	//alloc the hardware params struct
	if ( snd_pcm_hw_params_malloc( &hardwareParams ) < 0 ) {
		loge( "Failed to allocate ALSA hardware parameters!\n" );
		return -1;
	}

	//set to any, init the params
	err = snd_pcm_hw_params_any( handle->handle, hardwareParams );
	if (err < 0) {
        loge( "Unable to configure hardware: %s\n", snd_strerror(err) );
        goto done;
    }

	// Set the interleaved read and write format.
    err = snd_pcm_hw_params_set_access( handle->handle, hardwareParams, 
    									SND_PCM_ACCESS_RW_INTERLEAVED );
	if (err < 0) {
        loge( "Unable to configure PCM read/write format: %s\n", snd_strerror(err) );
        goto done;
    }

	//set the sample is format, 16bit or 8bit or ....
	err = snd_pcm_hw_params_set_format( handle->handle, hardwareParams, handle->nFormat );
    if (err < 0) {
        loge( "Unable to configure PCM format %s (%s): %s\n", formatName, formatDesc, snd_strerror(err) );
        goto done;
    }

	logi( "Set %s PCM format to %s (%s)\n", streamName( handle ), formatName, formatDesc );

	err = snd_pcm_hw_params_set_channels( handle->handle, hardwareParams, handle->nChannels );
    if (err < 0) {
        loge( "Unable to set channel count to %i: %s\n", handle->nChannels, snd_strerror(err) );
        goto done;
    }

    logi( "Using %i %s for %s.\n", handle->nChannels,
        handle->nChannels == 1 ? "channel" : "channels", streamName( handle ) );

	err = snd_pcm_hw_params_set_rate_near( handle->handle, hardwareParams, (unsigned int *)&requestedRate, 0 );
	if (err < 0)
        loge( "Unable to set %s sample rate to %u: %s\n",
                streamName(handle), handle->nSampleRate, snd_strerror(err) );
    else if ( requestedRate != handle->nSampleRate )
        // Some devices have a fixed sample rate, and can not be changed.
        // This may cause resampling problems; i.e. PCM playback will be too
        // slow or fast.
        logw( "Requested rate (%u HZ) does not match actual rate (%u HZ)\n",
                handle->nSampleRate, requestedRate );
    else
        logi("Set sample rate to %u HZ\n", requestedRate);

#ifdef DISABLE_HARWARE_RESAMPLING
    // Disable hardware re-sampling.
    err = snd_pcm_hw_params_set_rate_resample( handle->handle, hardwareParams, (T_S32)(resample) );
    if ( err < 0 ) {
        loge( "Unable to %s hardware resampling: %s\n", resample ? "enable" : "disable",
              snd_strerror(err) );
        goto done;
    }
#endif

	// Make sure we have at least the size we originally wanted
    err = snd_pcm_hw_params_set_buffer_size( handle->handle, hardwareParams, bufferSize );
    if (err < 0) {
		err = snd_pcm_hw_params_set_buffer_size_near( handle->handle, hardwareParams, &bufferSize );
		if ( err < 0 ) {
        	loge( "Unable to set buffer size to %d:  %s\n", (T_S32)bufferSize, snd_strerror(err) );
        	goto done;
		}
    }

	// Setup buffers for latency
    err = snd_pcm_hw_params_set_buffer_time_near( handle->handle, hardwareParams, (unsigned int *)&latency, NULL );
	if (err < 0) {
		/* That didn't work, set the period instead */
        T_U32 periodTime = latency / 4;
		snd_pcm_uframes_t periodSize;
		
        err = snd_pcm_hw_params_set_period_time_near( handle->handle, hardwareParams, (unsigned int *)&periodTime, NULL );
		if ( err < 0 ) {
            loge( "Unable to set the period time for latency: %s\n", snd_strerror(err) );
            goto done;
        }

        err = snd_pcm_hw_params_get_period_size( hardwareParams, &periodSize, NULL );
		if ( err < 0 ) {
            loge( "Unable to get the period size for latency: %s\n", snd_strerror(err) );
            goto done;
        }

		bufferSize = periodSize * 4;
        if ( bufferSize < handle->nBufferSize ) 
			bufferSize = handle->nBufferSize;

		err = snd_pcm_hw_params_set_buffer_size_near( handle->handle, hardwareParams, &bufferSize );
		if ( err < 0 ) {
            loge( "Unable to set the buffer size for latency: %s\n", snd_strerror(err) );
            goto done;
        }
	} else { 
		// OK, we got buffer time near what we expect. See what that did for bufferSize.
		T_U32 periodTime = 0;
		
        err = snd_pcm_hw_params_get_buffer_size( hardwareParams, &bufferSize );
		if ( err < 0 ) {
            loge( "Unable to get the buffer size for latency: %s\n", snd_strerror(err) );
            goto done;
        }

		// Does set_buffer_time_near change the passed value? It should.
        err = snd_pcm_hw_params_get_buffer_time(hardwareParams, (unsigned int *)&latency, NULL);
        if ( err < 0 ) {
            loge( "Unable to get the buffer time for latency: %s\n", snd_strerror(err) );
            goto done;
        }

		periodTime = latency / 4;
		err = snd_pcm_hw_params_set_period_time_near( handle->handle, hardwareParams, (unsigned int *)&periodTime, NULL );
		if (err < 0) {
            loge( "Unable to set the period time for latency: %s\n", snd_strerror(err) );
            goto done;
        }
	}

	logi( "Buffer size: %d\n", (T_S32)bufferSize );
    logi( "Latency: %d\n", (T_S32)latency );

	handle->nBufferSize = bufferSize;
    handle->nLatency = latency;

	// Commit the hardware parameters back to the device.
    err = snd_pcm_hw_params( handle->handle, hardwareParams );
    if (err < 0) loge( "Unable to set hardware parameters: %s\n", snd_strerror(err) );
	
done:
    snd_pcm_hw_params_free( hardwareParams );
	return err;
}


/**
* @brief   config the alsa software params
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 setSoftwareParams( alsa_handle_t *handle )
{
	snd_pcm_sw_params_t * softwareParams;
    T_S32 err = 0;

    snd_pcm_uframes_t bufferSize = 0;
    snd_pcm_uframes_t periodSize = 0;
    snd_pcm_uframes_t startThreshold = 0, stopThreshold = 0;

	//alloc the software params struct
	if ( snd_pcm_sw_params_malloc(&softwareParams) < 0 ) {
        loge( "Failed to allocate ALSA software parameters!\n" );
        return -1;
    }

	// Get the current software parameters
    err = snd_pcm_sw_params_current( handle->handle, softwareParams );
    if (err < 0) {
        loge( "Unable to get software parameters: %s\n", snd_strerror(err) );
        goto done;
    }

	// Configure ALSA to start the transfer when the buffer is almost full.
    snd_pcm_get_params( handle->handle, &bufferSize, &periodSize );

	if ( handle->bIsAD ) {
        // For recording, configure ALSA to start the transfer on the
        // first frame.
        startThreshold = 1;
        stopThreshold = bufferSize;
    } else {
		// For playback, configure ALSA to start the transfer when the
        // buffer is full.
        startThreshold = bufferSize - 1;
        stopThreshold = bufferSize;
    }

	//set the start condition
	err = snd_pcm_sw_params_set_start_threshold( handle->handle, softwareParams, startThreshold );
    if (err < 0) {
        loge( "Unable to set start threshold to %lu frames: %s\n", startThreshold, snd_strerror(err) );
        goto done;
    }

	err = snd_pcm_sw_params_set_stop_threshold( handle->handle, softwareParams, stopThreshold );
    if (err < 0) {
        loge( "Unable to set stop threshold to %lu frames: %s\n", stopThreshold, snd_strerror(err));
        goto done;
    }

    // Allow the transfer to start when at least periodSize samples can be
    // processed.
    err = snd_pcm_sw_params_set_avail_min( handle->handle, softwareParams, periodSize );
    if (err < 0) {
        loge( "Unable to configure available minimum to %lu: %s\n", periodSize, snd_strerror(err) );
        goto done;
    }

	// Commit the software parameters back to the device.
    err = snd_pcm_sw_params( handle->handle, softwareParams );
    if (err < 0) loge( "Unable to configure software parameters: %s\n", snd_strerror(err) );

done:
	snd_pcm_sw_params_free(softwareParams);
	return err;
}


/**
* @brief   set control value
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @param[in] name  			control name.
* @param[in] value  			control value.
* @param[in] index  			control index.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 SetCtrlVal( alsa_handle_t *handle, T_pCSTR name, T_U32 value, T_S32 index )
{
	snd_ctl_elem_id_t *id = NULL;
    snd_ctl_elem_info_t *info = NULL;
	snd_ctl_elem_value_t *control = NULL;
	snd_ctl_elem_type_t type;
	T_S32 ret = 0, count = 0;
	
	assert( handle );
	
	if ( !(handle->ctrlHandle) ) {
        loge( "Control not initialized\n" );
        return -1;
    }

    snd_ctl_elem_id_alloca( &id );
    snd_ctl_elem_info_alloca( &info );

    snd_ctl_elem_id_set_interface( id, SND_CTL_ELEM_IFACE_MIXER );
    snd_ctl_elem_id_set_name( id, name );
    snd_ctl_elem_info_set_id( info, id );

    ret = snd_ctl_elem_info( handle->ctrlHandle, info );
    if (ret < 0) {
        loge("Control '%s' cannot get element info: %d\n", name, ret);
        return -1;
    }

    count = snd_ctl_elem_info_get_count( info );
    if ( index >= count ) {
        loge( "Control '%s' index is out of range (%d >= %d)\n", name, index, count );
        return -1;
    }

    if (index == -1)
        index = 0; // Range over all of them
    else
        count = index + 1; // Just do the one specified

    type = snd_ctl_elem_info_get_type(info);

    snd_ctl_elem_value_alloca( &control );

    snd_ctl_elem_info_get_id( info, id );
    snd_ctl_elem_value_set_id( control, id );

    for ( int i = index; i < count; i++ ) {
        switch ( type ) {
            case SND_CTL_ELEM_TYPE_BOOLEAN:
                snd_ctl_elem_value_set_boolean(control, i, value);
                break;
            case SND_CTL_ELEM_TYPE_INTEGER:
                snd_ctl_elem_value_set_integer(control, i, value);
                break;
            case SND_CTL_ELEM_TYPE_INTEGER64:
                snd_ctl_elem_value_set_integer64(control, i, value);
                break;
            case SND_CTL_ELEM_TYPE_ENUMERATED:
                snd_ctl_elem_value_set_enumerated(control, i, value);
                break;
            case SND_CTL_ELEM_TYPE_BYTES:
                snd_ctl_elem_value_set_byte(control, i, value);
                break;
            default:
                break;
        }
    }

    ret = snd_ctl_elem_write( handle->ctrlHandle, control );
    return ret;
}


/**
* @brief   open the AD device
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @param[in] strDev  			device name
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 OpenAD( alsa_handle_t * handle, T_pSTR strDev )
{
	int ret = 0;
	
	if ( NULL == handle ) {
		loge( "AkAlsaHardware::OpenAD Invalid parameter!\n" );
		return -1;
	}

	ret = OpenAlsa( handle, strDev, AK_TRUE );
	if ( ret < 0 ) {
		loge( "AkAlsaHardware::OpenAD error!\n" );
		return -1;
	}
	return 0;
}


/**
* @brief   open the DA device
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @param[in] strDev  			device name
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 OpenDA( alsa_handle_t * handle, T_pSTR strDev )
{
	int ret = 0;
	
	if ( NULL == handle ) {
		loge( "AkAlsaHardware::OpenDA Invalid parameter!\n" );
		return -1;
	}

	ret = OpenAlsa( handle, strDev, AK_FALSE );
	if ( ret < 0 ) {
		loge( "AkAlsaHardware::OpenDA error!\n" );
		return -1;
	}
	return 0;
}


/**
* @brief   set audio DA/AD params
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @param[in] nSampleRate  		sample rate
* @param[in] nBits  			bits per sample
* @param[in] nChannels  		channels
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 SetParams( alsa_handle_t * handle, T_U32 nSampleRate, T_U32 nBits, T_U32 nChannels )
{
	T_S32 err = 0;
	int nActuallySR = 0;
	assert( handle );
	
	if ( nBits == 16 ) {
		handle->nFormat = SND_PCM_FORMAT_S16_LE;
	} else if ( nBits == 8 ) {
		handle->nFormat = SND_PCM_FORMAT_S8;
	} else {
		logw( "can't set %d bits per sample, use 16 bits per sample instead!\n" );
		handle->nFormat = SND_PCM_FORMAT_S16_LE;
	}

	handle->nSampleRate = nSampleRate;
	handle->nChannels = nChannels;

	err = setHardwareParams(handle);
    if ( err == 0 )
		err = setSoftwareParams(handle);
	
	switch (nSampleRate)
	{
		case 8000:
			nActuallySR = 10986;
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
			nActuallySR = nSampleRate;
	}

	OpenSDFilter(nChannels, nActuallySR, nSampleRate);
	if( NULL == pSdFilter)
	{
		printf("open sd Filter err \n");
		return -1;
	}
	return err;
}


/**
* @brief   set pcm input device is mic
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 SetMicIn( alsa_handle_t * handle )
{
	assert( handle );

	if ( !(handle->bIsAD) ) {
		logw( "playback can't set mic in!\n" );
		return 1;
	}
	
	return SetCtrlVal( handle, CAPTURE_ROUTE_NAME, SOURCE_MIC, 0 );
}


/**
* @brief   set pcm input device is line
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 SetLineIn( alsa_handle_t * handle )
{
	assert( handle );

	if ( !(handle->bIsAD) ) {
		logw( "playback can't set line in!\n" );
		return 1;
	}
		
	return SetCtrlVal( handle, CAPTURE_ROUTE_NAME, SOURCE_LINEIN, 0 );
}


/**
* @brief   read AD is capture data
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @param[in] pPcmData  		the pcm data load buffer. 
* @param[in] nBytes  			how many bytes you want to read from AD
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 ReadAD( alsa_handle_t * handle, T_U8 * pPcmData, T_U32 nBytes )
{

	snd_pcm_sframes_t n; 
	snd_pcm_sframes_t frames = snd_pcm_bytes_to_frames( handle->handle, nBytes );
	T_AUDIO_FILTER_BUF_STRC fbuf_strc;
	int nReSRDataLen;
	assert( handle );

	if ( !(handle->bIsAD) ) {
			logw( "can't read playback!\n" );
			return 0;
	}
		
	while(1)
	{
		if( g_audio_buff.audio_leng >= nBytes )
		{
				
			if( g_audio_buff.audio_last + nBytes > BUFF_LENG)
			{
				int len = BUFF_LENG-g_audio_buff.audio_last;
				memcpy(pPcmData, g_audio_buff.buff + g_audio_buff.audio_last, len);
				memcpy(pPcmData + len, g_audio_buff.buff, nBytes-len);
				g_audio_buff.audio_last = (g_audio_buff.audio_last+nBytes)%BUFF_LENG;
					//g_audio_buff.audio_leng -= nBytes;
			}
			else
			{
				memcpy( pPcmData, g_audio_buff.buff + g_audio_buff.audio_last, nBytes);
				g_audio_buff.audio_last = (g_audio_buff.audio_last+nBytes);
					
			}
			g_audio_buff.audio_leng -= nBytes;
			return nBytes;
		}
		else
		{
			do {
				n = snd_pcm_readi( handle->handle, temp_buff, frames );
				if ( n < frames ) 
				{
					if ( handle->handle ) 
					{
						if ( n == -EPIPE )
						{
							printf( "overrun occur!\n" );
							n = snd_pcm_prepare( handle->handle );
							n = handle->nBufferSize;
							return n;
						} else if ( n < 0 ) {
							printf( "snd_pcm_recover!\n" );
							n = snd_pcm_recover( handle->handle, n, 0 );
						} else {
							printf( "snd_pcm_prepare!\n" );
							n = snd_pcm_prepare( handle->handle );
						}
					}
					// printf("****** n= %d\n", n);
					return (T_S32)(snd_pcm_frames_to_bytes( handle->handle, n ));
				}
			} while ( n == -EAGAIN );
	
			fbuf_strc.buf_in = temp_buff;
			fbuf_strc.buf_out = temp_buff; //输入输出允许同buffer，但是要保证buffer够大，因为输出数据会比输入数据长
			fbuf_strc.len_in = nBytes; // 这里指输入ibuf中的有效数据长度
			fbuf_strc.len_out = BUFF_LENG;	 // 这里指输出buffer 大小，库里面做判断用，防止写buffer越界。
			nReSRDataLen = _SD_Filter_Control( pSdFilter, &fbuf_strc );
			//audio_size += nReSRDataLen;
			//printf("nResrdatalen = %d\n", nReSRDataLen);
			if(g_audio_buff.audio_index + nReSRDataLen > BUFF_LENG)
			{
				int len = BUFF_LENG-g_audio_buff.audio_index;
				memcpy(g_audio_buff.buff + g_audio_buff.audio_index, temp_buff , len);
				memcpy(g_audio_buff.buff, temp_buff+len, nReSRDataLen-len);
				g_audio_buff.audio_index = (g_audio_buff.audio_index + nReSRDataLen)%BUFF_LENG;
			}
			else
			{
				memcpy( g_audio_buff.buff + g_audio_buff.audio_index, temp_buff, nReSRDataLen);
				g_audio_buff.audio_index = (g_audio_buff.audio_index + nReSRDataLen);
			}
	
			g_audio_buff.audio_leng += nReSRDataLen;
				//printf("change data %d->%d\n", nBytes, nReSRDataLen);
				//return (T_S32)(snd_pcm_frames_to_bytes( handle->handle, n ));
		}
	}
		//return nReSRDataLen;
}	

/**
* @brief   write the pcm data to DA
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @param[in] pPcmData  		the pcm data to playback. 
* @param[in] nBytes  			how many bytes you want to playback
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 WriteDA( alsa_handle_t * handle, T_U8 * pPcmData, T_U32 nBytes )
{
	snd_pcm_sframes_t 	n;
    T_S32				sent = 0;
	
	assert( handle );

	if ( handle->bIsAD ) {
		logw( "can't write capture!\n" );
		return 0;
	}

    do {
	    n = snd_pcm_writei( handle->handle,
	                        pPcmData + sent,
	                        snd_pcm_bytes_to_frames( handle->handle, nBytes - sent ) );
	    if ( n == -EBADFD ) {
	        // Somehow the stream is in a bad state. The driver probably
	        // has a bug and snd_pcm_recover() doesn't seem to handle this.
	        loge( "writei EBADFD error!" );
			return -1;
	    }
	    else if ( n < 0 ) {
	        if ( handle->handle ) {
	            // snd_pcm_recover() will return 0 if successful in recovering from
	            // an error, or -errno if the error was unrecoverable.
	            n = snd_pcm_recover( handle->handle, n, 1 );
	            if (n) return (T_S32)(n);
	        }
	    }
	    else
	    {
	        sent += (T_S32)(snd_pcm_frames_to_bytes( handle->handle, n ) );
	    }

	} while ( handle->handle && sent < nBytes );

	return sent;
}


/**
* @brief   close the alsa lib, and pcm device
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the alsa_handle_t.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 Close( alsa_handle_t * handle )
{
	T_S32 err = 0;
	
	assert( handle );

	if ( handle->ctrlHandle ) {
		snd_ctl_close( handle->ctrlHandle );
		handle->ctrlHandle = NULL;
	}

	if ( handle->handle ) {
		snd_pcm_drain( handle->handle );
        err = snd_pcm_close( handle->handle );
		handle->handle = NULL;
	}

	if ( pSdFilter ) {
		_SD_Filter_Close( pSdFilter );
		pSdFilter = NULL;
	}
	return err;
}


/**
* @brief   get module functions is pointer
* @author hankejia
* @date 2012-07-05
* @param[in] pstAlsaModule  	the pointer point o the AkAlsaHardware.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 getAlsaModule( AkAlsaHardware * pstAlsaModule )
{
	if ( NULL == pstAlsaModule ) {
		loge( "getAlsaModule::Invalid parameter!\n" );
		return -1;
	}

	pstAlsaModule->OpenAD 		= OpenAD;
	pstAlsaModule->OpenDA 		= OpenDA;
	pstAlsaModule->SetParams 	= SetParams;
	pstAlsaModule->SetMicIn		= SetMicIn;
	pstAlsaModule->SetLineIn	= SetLineIn;
	pstAlsaModule->ReadAD		= ReadAD;
	pstAlsaModule->WriteDA		= WriteDA;
	pstAlsaModule->Close		= Close;
	
	return 0;
}
