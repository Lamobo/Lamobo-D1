/* 
 * @file AkAudioRecorder.c
 * @brief
 *  this file is the main recorder process
 *
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author
 * @date 2012-7-18
 * @version 1.0
 */
 
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

#include "AkAudioRecorder.h"
#include "AkAlsaHardware.h"
#include "AkAudioEncoder.h"
#include "Thread.h"
#include "log.h"
#include "sdfilter.h"
#include "AkAudioDemo.h"
#include "cfifo.h"
#include "headers.h"


#define TEMP_FILE_NAME		"Audio_Temp"
#define FILE_SUFFIZ_WAV		".wav"	//file suffiz wav
#define FILE_SUFFIZ_PCM		".pcm"	//file suffiz pcm
#define FILE_SUFFIZ_AAC		".aac"	//file suffiz aac
#define FILE_SUFFIZ_AMR		".amr"	//file suffiz amr

#define MAX_TIME_STRING		25

#define WAVE_FORMAT_PCM 	1
#define WAVE_FORMAT_ADPCM 	17

#define AAC_SINGLE_ENCBUFFER_SIZE 	2048
#define AAC_STEREO_ENCBUFFER_SIZE	4096

#define AUDIO_FIFO_SIZE_ORDER 		(5)
#define AUDIO_FIFO_SIZE 			(1 << AUDIO_FIFO_SIZE_ORDER)

#if BYTE_ORDER == BIG_ENDIAN
#define cpu_to_le32(x) SWAP4((x))
#define cpu_to_le16(x) SWAP2((x))
#define le32_to_cpu(x) SWAP4((x))
#define le16_to_cpu(x) SWAP2((x))
#else
#define cpu_to_le32(x) (x)
#define cpu_to_le16(x) (x)
#define le32_to_cpu(x) (x)
#define le16_to_cpu(x) (x)
#endif

/* MMIO macros */
#define mmioFOURCC(ch0, ch1, ch2, ch3) \
	((T_U32)(T_U8)(ch0) | ((T_U32)(T_U8)(ch1) << 8) | \
	((T_U32)(T_U8)(ch2) << 16) | ((T_U32)(T_U8)(ch3) << 24))

#define FOURCC_RIFF		mmioFOURCC ('R', 'I', 'F', 'F')		//RIFF
#define FOURCC_LIST		mmioFOURCC ('L', 'I', 'S', 'T')		//LIST
#define FOURCC_WAVE		mmioFOURCC ('W', 'A', 'V', 'E')		//WAVE
#define FOURCC_FMT		mmioFOURCC ('f', 'm', 't', ' ')		//fmt
#define FOURCC_DATA		mmioFOURCC ('d', 'a', 't', 'a')		//data

static T_U8 fileHeadG711[60] = {0x52,0x49,0x46,0x46,0x00,0x00,0x00,0x00,0x57,
			0x41,0x56,0x45,0x66,0x6D,0x74,0x20,0x14,0x00,0x00,0x00,0x06,0x00,
			0x01,0x00,0x40,0x1F,0x00,0x00,0x40,0x1F,0x00,0x00,0x01,0x00,0x08,
			0x00,0x02,0x00,0x00,0x00,0x66,0x61,0x63,0x74,0x04,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x64,0x61,0x74,0x61,0x00,0x00,0x00,0x00}; 


typedef T_U8	BYTE;
typedef T_U16	WORD;
typedef T_U32	DWORD;
typedef T_U32	FOURCC;    /* a four character code */


typedef struct CHUNKHDR {	
	FOURCC ckid;        /* chunk ID */	
	DWORD dwSize;       /* chunk size */
} CHUNKHDR;

/* 
 * simplified Header for standard WAV files 
 */
typedef struct WAVEHDR {	
	CHUNKHDR chkRiff;
	FOURCC fccWave;
	CHUNKHDR chkFmt;
    
    /*
     * format type 
     */
	WORD wFormatTag; 
    
    /* 
     * number of channels (i.e. mono, stereo, etc.) 
     */    
	WORD nChannels; 
    
    /* 
     * sample rate 
     */    
	DWORD nSamplesPerSec;  
    
    /* 
     * for buffer estimation 
     */
	DWORD nAvgBytesPerSec; 
    
    /* 
     * block size of data 
     */
	WORD nBlockAlign;       
	WORD wBitsPerSample;
	CHUNKHDR chkData;
} WAVEHDR;

/*
* adpcm header structure
*/
typedef struct{
    T_S16 wFormatTag;// wave_format_dvi_adpcm / wave_format_ima_adpcm (0x0011)
    T_S16 nChannels;  //channel 
    T_S32 nSamplesPerSec;    // samplerate
    T_S32 nAvgBytesPerSec;   // ?????¨¤¨¦¨´??¡Á??¨², 4055 = 256*8000/505  
    T_S16 nBlockAlign;       // ¨ºy?Y?¨¦¦Ì?¦Ì¡Â??¨ºy, ¨¨???¨®¨²????2¨¦?¨´¦Ì?¦Ì???¨ºy - 256 
    T_S16 wBitsPerSample;    // ???¨´¡À?¨ºy?Y??¨ºy - 4  
    T_S16 cbSize;            // ¡À¡ê¨¢?2?¨ºy- 2 
    T_S16 wSamplesPerBlock;  // ????¨ºy?Y?¨¦¡ã¨¹o?2¨¦?¨´¦Ì?¨ºy, 505
}IMA_ADPCM_WAVEFORMAT;

typedef struct ADPCM_HEADER {
    T_CHR  riff[4];        // "RIFF"
    T_S32  file_size;       // in bytes
    T_CHR  wavefmt_[8];    // "WAVE"
    T_S32  imachunk_size;      // in bytes (0x14 for IMA ADPCM)
    IMA_ADPCM_WAVEFORMAT ima_format;
    T_CHR  fact[4];         //"fact"
    T_S32  factchunk_size;  //fact chunk size
    T_S32  factdata_size;  //fact data size
    T_CHR  data[4];         // "data"
    T_S32  data_length;     // in bytes
} ADPCM_HEADER;



typedef struct File_Suffize_t
{
	T_pCSTR			strSuffize;
	AUDIO_REC_TYPE	enRecType;
}File_Suffize_t;

static const File_Suffize_t FileSuffix[] = {
	{ FILE_SUFFIZ_WAV, AUDIO_REC_TYPE_WAV_PCM },
	{ FILE_SUFFIZ_WAV, AUDIO_REC_TYPE_ADPCM_IMA },
	{ FILE_SUFFIZ_AAC, AUDIO_REC_TYPE_AAC },
	{ FILE_SUFFIZ_AMR, AUDIO_REC_TYPE_AMR },
	{ FILE_SUFFIZ_WAV, AUDIO_REC_TYPE_G711_ULOW },
	{ FILE_SUFFIZ_WAV, AUDIO_REC_TYPE_G711_ALOW },
};

static const T_S32 FileSuffixLen = ( sizeof(FileSuffix) / sizeof(File_Suffize_t) );

static const unsigned int MAX_FILE_SIZE				= 4187593113;	// 3.9G
static const unsigned int MIN_DISK_SIZE_FOR_WRITE	= 52428800;		// 50M 

typedef struct audio_filter_handle
{
	void * pfilter;
	char * obuf;
}audio_filter_handle_t;



typedef struct audio_recorder_handle
{
	T_U32				nOutBitsRate;
	AUDIO_REC_TYPE		enType;
	AUDIO_IN_DEV		enInDev;
	T_pSTR				strPath;
	T_pSTR				strFileName;

	T_S32				nFID;
	
	struct cfifo 		*cf;
	nthread_t			TID;
	nthread_t			WID;
	Condition			condition;
	Condition			StopConditon;

	alsa_handle_t		stAlsaHandle;
	//AkAlsaHardware		stAlsaModule;

	T_U32				nRawBufferSize;
	T_U8			*	pEncBuffer;
	T_U32				nEncBufferSize;

	T_U32				nPeriodDuration;
	T_BOOL				bIsRunning;
	T_BOOL				bIsStart;
	T_BOOL				bIsStop;

	T_U32				nMaxFileSize;

	audio_filter_handle_t filter;


}audio_recorder_handle;

T_U32	nAvgBytesPerSec;
T_U16	nBlockAlign;
T_U16	wBitsPerSample;
T_U16	nSamplesPerPacket;


//capture thread run
static T_VOID Run( audio_recorder_handle * handle );

static T_pVOID write_audio_data(T_pVOID args);

//make the record file name
static T_pSTR MakeFileName( audio_recorder_handle * handle );

//judge the file or dir was already exists in path?
static T_S32 IsExists( T_pSTR strFilePath );

//create the path
static T_S32 CompleteCreateDirectory(  T_pSTR strRecPath );

//the thread create call back function
static T_pVOID thread_begin( T_pVOID user )
{
	audio_recorder_handle * handle = (audio_recorder_handle*)(user);
	if ( handle == NULL ) {
		loge( "thread_begin have a invail param!" );
		return NULL;
	}
	
	Run( handle );
	return NULL;
}

/**
* @brief   open the audio record lib, alsa lib
* @author hankejia
* @date 2012-07-05
* @param[in] ppHandle  			the pointer point to the AkAudioRecorder.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 OpenRecorder( T_pVOID * ppHandle )
{
	audio_recorder_handle * pHandle = NULL;
	T_S32					ret = 0;
	
	
	assert( ppHandle );

	pHandle = (audio_recorder_handle *)malloc( sizeof( audio_recorder_handle ) );
	if ( NULL == pHandle ) {
		loge( "can't alloc the recorder handle, out of memory!\n" );
		return -1;
	}

	bzero( pHandle, sizeof( audio_recorder_handle ) );
	Condition_Initialize( &(pHandle->condition) );
	Condition_Initialize( &(pHandle->StopConditon) );
	pHandle->nPeriodDuration = 40; //20ms
	pHandle->nFID = -1;
	pHandle->TID = thread_zeroid();
	pHandle->WID = thread_zeroid();
		
	//getAlsaModule( &(pHandle->stAlsaModule) );

	ret = OpenAD( &(pHandle->stAlsaHandle), NULL );
	if ( ret < 0 ) {
		loge( "OpenRecorder::can't open AD!\n" );
		return -1;
	}
	
	
	
	*ppHandle = (T_pVOID)pHandle;
	return 0;
}

/**
* @brief   set audio capture pcm params
* @author hankejia
* @date 2012-07-05
* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
* @param[in] nChannels  		audio channels.
* @param[in] nSampleRate  		sample rate.
* @param[in] nBitsPerSample  	bits per sample.
* @param[in] nFilterType  	 Filter Type.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 SetAudioParams(T_pVOID pHandle, T_U32 nChannels, T_U32 nSampleRate, T_U32 nBitsPerSample, T_U32 nFilterType)
{
	audio_recorder_handle * handle = NULL;
	T_S32					ret = 0;
	T_AUDIO_FILTER_INPUT 	s_ininfo;
	
	assert( pHandle );

	handle = (audio_recorder_handle*)pHandle;

	Condition_Lock( handle->condition );

	if ( handle->enInDev == IN_DEV_LINE ) {
		ret = SetLineIn(&(handle->stAlsaHandle));
	}else {
		ret = SetMicIn(&(handle->stAlsaHandle));
	}
	if ( ret < 0 ) {
		loge( "StartRec::can't set the audio in dev!\n" );
		Condition_Unlock( handle->condition );
		return -1;
	}

	ret = SetParams(&(handle->stAlsaHandle), nSampleRate, nBitsPerSample, nChannels);
	Condition_Unlock( handle->condition );

	if(nFilterType != _SD_FILTER_UNKNOWN) {
		s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
		s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
		s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
		s_ininfo.cb_fun.delay = AK_NULL;

		s_ininfo.m_info.m_BitsPerSample = nBitsPerSample;
		s_ininfo.m_info.m_Channels 		= nChannels;
		s_ininfo.m_info.m_SampleRate 	= nSampleRate;

		s_ininfo.m_info.m_Type 			= nFilterType;
		switch (s_ininfo.m_info.m_Type)
		{
			case _SD_FILTER_AGC:
				printf("audio support agc.\n");
				s_ininfo.m_info.m_Private.m_agc.AGClevel = 1024;
				break;

			case _SD_FILTER_DENOICE:
				printf("audio support denoice.\n");
				s_ininfo.m_info.m_Private.m_NR.ASLC_ena = 1;
				s_ininfo.m_info.m_Private.m_NR.NR_Level = 0;
				break;

			default:
				loge("Unsupport audio filter type\n");
				break;
		}

		handle->filter.pfilter = _SD_Filter_Open(&s_ininfo);

		if (!handle->filter.pfilter)
		{
			printf("open the audio filter lib err \n");
			return -1;
		}
	} else {
		printf("not support agc & nr func.\n");
		handle->filter.pfilter = NULL;
	}

	handle->filter.obuf = malloc(2048*4);
	if (AK_NULL == handle->filter.obuf)
	{
		printf("malloc mem failed \n");
		return -1;
	}
	
	if ( ret < 0 ) {
		loge( "set audio params failed!\n" );
		return -1;
	}
	
	return 0;
}

/**
* @brief   set audio encode params
* @author hankejia
* @date 2012-07-05
* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
* @param[in] enType  			audio encode type.
* @param[in] nBitsRate  		encode out bits rate per second.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 SetEncodeParams( T_pVOID pHandle, T_U32 nBitsRate, AUDIO_REC_TYPE enType )
{
	audio_recorder_handle * handle = NULL;

	assert( pHandle );

	if ( ( enType < 0 ) || ( enType > AUDIO_REC_TYPE_COUNT ) ) {
		loge( "SetEncodeParams::Invalid parameters !\n" );
		return -1;
	}

	handle = (audio_recorder_handle*)pHandle;

	Condition_Lock( handle->condition );
	handle->enType = enType;
	handle->nOutBitsRate = nBitsRate;
	Condition_Unlock( handle->condition );
	
	return 0;
}

/**
* @brief   set audio pcm data input device
* @author hankejia
* @date 2012-07-05
* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
* @param[in] enInDev  			input device type line or mic.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 SetAudioInDev( T_pVOID pHandle, AUDIO_IN_DEV enInDev )
{
	audio_recorder_handle * handle = NULL;
	assert( pHandle );

	if ( ( enInDev < 0 ) || ( enInDev > IN_DEV_COUNT ) ) {
		logw( "Unknown the audio in device, use mic in instead!\n" );
		enInDev = IN_DEV_MIC;
	}
	
	handle = (audio_recorder_handle*)pHandle;
	
	Condition_Lock( handle->condition );
	handle->enInDev = enInDev;
	Condition_Unlock( handle->condition );

	return 0;
}

/**
* @brief   set record path
* @author hankejia
* @date 2012-07-05
* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
* @param[in] strPath  			record path.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 SetRecordPath( T_pVOID pHandle, T_pSTR strPath )
{
	audio_recorder_handle * handle = NULL;
	T_S32 					ret = 0;
	
	assert( pHandle || strPath );

	handle = (audio_recorder_handle*)pHandle;
	
	if ( !IsExists( strPath ) ) {
		if ( CompleteCreateDirectory( strPath ) < 0 ) {
			loge( "SetRecordePath::can't create the recorde dir!\n" );
			return -1;
		}
	}

	Condition_Lock( handle->condition );

	if ( handle->strPath != NULL ) {
		free( handle->strPath );
		handle->strPath = NULL;
	}

	handle->strPath = (T_pSTR)malloc( ( strlen( strPath ) + 1 ) * sizeof( T_CHR ) );
	if ( NULL == handle->strPath ) {
		loge( "SetRecordePath::out of memory!\n" );
		ret = -1;
		goto done;		
	}

	bzero( handle->strPath, ( strlen( strPath ) + 1 ) * sizeof( T_CHR ) );

	strcpy( handle->strPath, strPath );

done:
	Condition_Unlock( handle->condition );
	return 0;
}

T_S32 SetAudioMicVolume(T_pVOID pHandle, T_U32 volume)
{
	audio_recorder_handle * handle = NULL;

	assert( pHandle );

	handle = (audio_recorder_handle*)pHandle;

	return SetMixerLevel( &(handle->stAlsaHandle), MIC_CAPTURE_VOLUME, volume);
}


T_S32 SetAudioLineInVolume(T_pVOID pHandle, T_U32 volume)
{
	audio_recorder_handle * handle = NULL;

	assert( pHandle );

	handle = (audio_recorder_handle*)pHandle;

	return SetMixerLevel( &(handle->stAlsaHandle), LINE_IN_CAPTURE_VOLUME, volume);
}


/**
* @brief   open the audio decode lib
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point to the AkAudioRecorder.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 OpenSDCodec( audio_recorder_handle * handle )
{
	AudioEncIn	stEncIn;
	AudioEncOut	stEncOut;
	T_S32		ret = 0;
	
	assert( handle );
	
	stEncIn.nChannels = handle->stAlsaHandle.nChannels;

	if ( handle->stAlsaHandle.nFormat == SND_PCM_FORMAT_S16_LE ) {
		stEncIn.nBitsPerSample = 16;
	} else {
		stEncIn.nBitsPerSample = 8;
	}

	stEncIn.nSampleRate = handle->stAlsaHandle.nSampleRate;

	switch ( handle->enType )
	{
	case AUDIO_REC_TYPE_ADPCM_IMA:
		stEncOut.enc_type = ENC_TYPE_ADPCM_IMA;
		break;
	case AUDIO_REC_TYPE_AAC:
		stEncOut.enc_type = ENC_TYPE_AAC;
		break;
	case AUDIO_REC_TYPE_AMR:
		stEncOut.enc_type = ENC_TYPE_AMR;
		break;
	case AUDIO_REC_TYPE_G711_ULOW:
		stEncOut.enc_type = ENC_TYPE_G711_ULOW;
		break;
	case AUDIO_REC_TYPE_G711_ALOW:
		stEncOut.enc_type = ENC_TYPE_G711_ALOW;
		break;
	default:
		loge( "can't open the SdCodec, because the recorder type invalid!\n" );
		ret = -1;
		goto done;
	}

	stEncOut.nBitsRate = handle->nOutBitsRate;

	ret = openSdEncodeLib( &stEncIn, &stEncOut );
	if ( ret < 0 ) {
		loge( "can't open the SdCodec!\n" );
	}
	
done:
	return ret;
}

/*
 * @brief		get disk free size
 * @param	[in] pthis			the pointer point to the AkMediaLib.
 * @param	[in] pstrRecPath	record path
 * @return	T_S32
 * @retval	-1 for error, otherwise return the free size of disk .
 */
static T_S64 GetDiskFreeSize( T_pSTR pstrRecPath )
{
	struct statfs disk_statfs = { 0 };

	assert( pstrRecPath );

	while ( statfs( pstrRecPath, &disk_statfs ) == -1 ) {
		if ( errno != EINTR ) {
			loge( "statfs: %s Last error == %s\n", pstrRecPath, strerror(errno) );
			return -1;
		}
	}

	return disk_statfs.f_bavail * disk_statfs.f_bsize;
}

/**
* @brief   get the max record file size
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point to the AkAudioRecorder.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 MakeFileSizeLimit( audio_recorder_handle * handle )
{
	T_S64 DiskFreeSize = 0;
	
	assert( handle );
	
	DiskFreeSize = GetDiskFreeSize( handle->strPath );
	if ( DiskFreeSize <= MIN_DISK_SIZE_FOR_WRITE ) {
		loge( "the disk free size less then 50M, can't record please free disk space!\n" );
		return -1;
	}

	if ( DiskFreeSize > MAX_FILE_SIZE ) {
		handle->nMaxFileSize = MAX_FILE_SIZE;
	}else {
		handle->nMaxFileSize = (T_U32)(DiskFreeSize - MIN_DISK_SIZE_FOR_WRITE);
	}

	return 0;
}

/**
* @brief   is AkAudioRecorder module running? capture data and process data
* @author hankejia
* @date 2012-07-05
* @param[in] pHandle  			the pointer to the AkAudioRecorder.
* @return T_BOOL
* @retval if return AK_TURE running,AK_FALSE not running
*/
 T_BOOL IsRunning( T_pVOID pHandle )
{
	audio_recorder_handle * handle = NULL;

	assert( pHandle );

	handle = (audio_recorder_handle*)pHandle;

	Condition_Lock( handle->condition );

	if ( handle->bIsStart || handle->bIsRunning ) {
		Condition_Unlock( handle->condition );
		return AK_TRUE;
	}

	Condition_Unlock( handle->condition );

	return AK_FALSE;
}

/**
* @brief   get the recorded file is path and name
* @author hankejia
* @date 2012-07-05
* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
* @param[in] strFileName  		file path and name.
* @param[out] pnStrlen  		strFileName string len.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 GetRecordFile( T_pVOID pHandle, T_pSTR strFileName, T_U32 * pnStrlen )
{
	T_U32 nStrFileNameLen = 0;
		
	audio_recorder_handle * handle = NULL;

	assert( pHandle || strFileName || pnStrlen );

	handle = (audio_recorder_handle*)pHandle;

	Condition_Lock( handle->condition );

	if ( NULL == handle->strFileName ) {
		Condition_Unlock( handle->condition );
		*pnStrlen = 0;
		return 0;
	}

	nStrFileNameLen = strlen( handle->strFileName ) + 1;
	if ( *pnStrlen < nStrFileNameLen ) {
		Condition_Unlock( handle->condition );
		//the strFileName no enought space
		*pnStrlen = nStrFileNameLen;
		return 1;
	}

	*pnStrlen = nStrFileNameLen;

	strncpy( strFileName, handle->strFileName, nStrFileNameLen );

	Condition_Unlock( handle->condition );
	
	return 0;
}

/**
* @brief   start recorder, create the capture thread and process data thread
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the audio_recorder_handle.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 StartRec( T_pVOID pHandle )
{
	audio_recorder_handle * handle = NULL;
	T_U32					nBytesPerSample = 0;

	mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
	
	assert( pHandle );

	handle = (audio_recorder_handle*)pHandle;

	Condition_Lock( handle->condition );
	
	if ( handle->bIsStart || handle->bIsRunning ) {
		Condition_Unlock( handle->condition );
		logi( "StartRec::already start!\n" );
		return 0;
	}

	if ( handle->enType != AUDIO_REC_TYPE_WAV_PCM ) {
		if ( OpenSDCodec( handle ) < 0 ) {
			Condition_Unlock( handle->condition );
			loge( "can't start the Recorder thread, because the Sdcodec open error!\n" );
			return -1;
		}
	}

	if ( handle->strFileName ) {
		free( handle->strFileName );
		handle->strFileName = NULL;
	}

	handle->strFileName = MakeFileName( handle );
	if ( NULL == handle->strFileName ) {
		Condition_Unlock( handle->condition );
		return -1;
	}

	if ( handle->nFID > 0 ) {
		close( handle->nFID );
		handle->nFID = -1;
	}
	
	handle->nFID = open( handle->strFileName, O_LARGEFILE | O_RDWR | O_CREAT | O_EXCL, mode );
	if ( handle->nFID < 0 ) {
		Condition_Unlock( handle->condition );
		loge( "StartRec::can't not open file %s! error = %s", handle->strFileName, strerror(errno) );
		return -1;
	}

	if(handle->cf) {
		cfifo_release(handle->cf);
		handle->cf = NULL;
	}

	if ( handle->pEncBuffer ) {
		free( handle->pEncBuffer );
		handle->pEncBuffer = NULL;
	}

	if ( handle->stAlsaHandle.nFormat == SND_PCM_FORMAT_S16_LE ) {
		nBytesPerSample = 2;
	}else {
		nBytesPerSample = 1;
	}
	
	handle->nRawBufferSize = ( handle->stAlsaHandle.nSampleRate * handle->stAlsaHandle.nChannels *
							   handle->nPeriodDuration * nBytesPerSample ) / 1000;

	if ( ( AUDIO_REC_TYPE_AAC == handle->enType ) && 
			  	( 2 == handle->stAlsaHandle.nChannels ) ) {
		handle->nRawBufferSize = AAC_STEREO_ENCBUFFER_SIZE;
	} else if ( ( AUDIO_REC_TYPE_AAC == handle->enType ) &&
				( 1 == handle->stAlsaHandle.nChannels ) ) {
		handle->nRawBufferSize = AAC_SINGLE_ENCBUFFER_SIZE;
	}

		/*size_order, element size*/
	handle->cf = cfifo_init(handle->nRawBufferSize, AUDIO_FIFO_SIZE_ORDER);
	if(handle->cf == NULL) {
		return -1;
	}

	handle->nEncBufferSize = handle->nRawBufferSize;

	handle->pEncBuffer = (T_U8*)malloc( handle->nEncBufferSize );
	if ( NULL == handle->pEncBuffer ) {
		Condition_Unlock( handle->condition );
		loge( "can't alloc enc buffer, out of memory!\n" );
		return -1;
	}
/*
	if ( handle->enInDev == IN_DEV_LINE ) {
		ret = SetLineIn( &(handle->stAlsaHandle) );
	}else {
		ret = SetMicIn( &(handle->stAlsaHandle) );
	}
	if ( ret < 0 ) {
		Condition_Unlock( handle->condition );
		loge( "StartRec::can't set the audio in dev!\n" );
		return -1;
	}
*/
	if ( MakeFileSizeLimit( handle ) < 0  ) {
		Condition_Unlock( handle->condition );
		return -1;
	}

	pthread_attr_t SchedAttr;
	struct sched_param SchedParam;
	memset(&SchedAttr, 0, sizeof(pthread_attr_t));
	memset(&SchedParam, 0, sizeof(SchedParam));

	pthread_attr_init( &SchedAttr );
	SchedParam.sched_priority = 90;
	pthread_attr_setschedparam( &SchedAttr, &SchedParam );
	pthread_attr_setschedpolicy( &SchedAttr, SCHED_RR );

	if ( pthread_create( &(handle->TID), &SchedAttr, thread_begin, handle ) != 0 ) {
		Condition_Unlock( handle->condition );
		loge( "unable to create a thread for Recorder!\n" );
		return -1;	
	}

	if ( pthread_create( &(handle->WID), NULL, write_audio_data, handle ) != 0 ) {
		Condition_Unlock( handle->condition );
		loge( "unable to create a thread for write Recorder!\n" );
		return -1;	
	}

	handle->bIsStart = AK_TRUE;
	Condition_Unlock( handle->condition );
	
	return 0;
}

/**
* @brief   stop recorder, stop the capture thread and process data thread
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point o the audio_recorder_handle.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 StopRec( T_pVOID pHandle )
{
	audio_recorder_handle * handle = NULL;
	
	assert( pHandle );

	handle = (audio_recorder_handle*)pHandle;

	Condition_Lock( handle->condition );
	if ( !(handle->bIsStart) && !(handle->bIsRunning) ) {
		Condition_Unlock( handle->condition );
		return 0;
	}
	handle->bIsStop = AK_TRUE;
	Condition_Unlock( handle->condition );

	pthread_join( handle->TID, NULL );
	pthread_join( handle->WID, NULL );

	Condition_Lock( handle->condition );
	handle->bIsStart = AK_FALSE;
	handle->TID	= thread_zeroid();
	handle->WID	= thread_zeroid();
	Condition_Unlock( handle->condition );

	if ( handle->enType != AUDIO_REC_TYPE_WAV_PCM ) {
		if ( closeSdEncodeLib() < 0 ) {
			loge( "close sdcodec error!\n" );
			return -1;
		}
	}

	return 0;
}

/**
* @brief   close the audio record lib, alsa lib
* @author hankejia
* @date 2012-07-05
* @param[in] pHandle  			the pointer point to the AkAudioRecorder.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
 T_S32 CloseRecorder( T_pVOID pHandle )
{
	audio_recorder_handle * handle = NULL;

	assert( pHandle );

	handle = (audio_recorder_handle*)pHandle;

	Condition_Lock( handle->condition );
	if ( handle->bIsRunning || handle->bIsStart ) {
		Condition_Unlock( handle->condition );
		logw( "can't not close recorde, when thread is running, \
			   please stop it first!\n" );
		return 1; 
	}
	Condition_Unlock( handle->condition );
	
	if ( handle->strFileName != NULL ) {
		free( handle->strFileName );
	}

	if ( handle->strPath != NULL ) {
		free( handle->strPath );
	}

	if ( handle->pEncBuffer != NULL ) {
		free( handle->pEncBuffer );
	}

	cfifo_release(handle->cf);
	Condition_Destroy( &(handle->condition) );
	Condition_Destroy( &(handle->StopConditon) );
	
	if ( Close( &(handle->stAlsaHandle) ) < 0 ) {
		loge( "can't close the AD!\n" );
		return -1;
	}
	
	free(handle->filter.obuf);
	handle->filter.obuf = AK_NULL;
	
	_SD_Filter_Close(handle->filter.pfilter);
	
	free( handle );
	
	return 0;
}

/*
 * @brief		judge the file or dir was already exists in path?
 * @param	[in] pthis			the pointer point to the AkMediaLib.
 * @param	[in] pstrFilePath	the file path
 * @return	T_S32
 * @retval	0 for not exists , 1 for exists.
 */
static T_S32 IsExists( T_pSTR strFilePath )
{
	if ( NULL == strFilePath )
		return 0;

	if ( access( strFilePath, F_OK ) == 0 )
		return 1;

	return 0;
}

/*
 * @brief		create the path
 * @param	[in] pthis			the pointer point to the AkMediaLib.
 * @param	[in] pstrRecPath	record path
 * @return	T_S32
 * @retval	0 for sucess , 1 for failed.
 */
static T_S32 CompleteCreateDirectory(  T_pSTR strRecPath )
{
	T_S32 iRet = 0;
	T_CHR *pstrTemp = NULL, *pszBackSlash = NULL;
	
	assert( strRecPath != NULL );
	
	T_pSTR pstrPath = (T_pSTR)malloc( ( strlen( strRecPath ) + 1 ) );
	if ( NULL == pstrPath ) {
		loge( "CompleteCreateDirectory::Out of memory!\n" );
		return -1;
	}

	strcpy( pstrPath, strRecPath );

	pstrTemp = strchr( pstrPath, '/' );

	while( 1 ) {
		pszBackSlash = strchr( pstrTemp + 1, '/' );
		if ( NULL == pszBackSlash )
			break;
		
		*pszBackSlash= '\0';

		if ( IsExists( pstrPath ) ) {
			*pszBackSlash = '/';
			pstrTemp = pszBackSlash;
			continue;
		}

		if ( mkdir( pstrPath, S_IRWXU | S_IRWXG | S_IRWXO ) != 0 ) {
			loge( "CompleteCreateDirectory::can't create dir %s, error = %s", pstrPath, strerror(errno) );
			iRet = -1;
			goto Exit;
		}

		*pszBackSlash = '/';
        pstrTemp = pszBackSlash;
	}

	if ( ( mkdir( pstrPath, S_IRWXU | S_IRWXG | S_IRWXO ) != 0 ) && ( errno != EEXIST ) ) {
		loge( "CompleteCreateDirectory::can't complete create dir %s! error = %s!\n", pstrPath, strerror(errno) );
		iRet = -1;
	}
	
Exit:
	free( pstrPath );
	return iRet;
}

/*
 * @brief		get current local time and convert it to string
 * @param	[in] pthis 	 the pointer point to the AkMediaLib.
 * @return	T_pSTR
 * @retval	NULL for error,otherwise the string of current time
 */
static T_pSTR GetCurTimeStr()
{
	time_t now = { 0 };
	now = time(0);
	char * pstrTime= NULL;

	struct tm *tnow = localtime(&now);
	if ( NULL == tnow ) {
		loge( "getCurTimeStr::get local time error!\n" );
		return (T_pSTR)NULL;
	}

	pstrTime = (char *)malloc( MAX_TIME_STRING );
	if ( NULL == pstrTime ) {
		loge( "getCurTimeStr::Out of memory!\n" );
		return (T_pSTR)NULL;
	}

	memset( pstrTime, 0, MAX_TIME_STRING );
	sprintf( pstrTime, "%4d,%02d,%02d-%02d,%02d,%02d", 1900 + tnow->tm_year, tnow->tm_mon + 1,
			 tnow->tm_mday, tnow->tm_hour, tnow->tm_min, tnow->tm_sec );

	return pstrTime;
}

/**
* @brief   make the record file name
* @author hankejia
* @date 2012-07-05
* @param[in] handle  			the pointer point to the audio_recorder_handle.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/

extern demo_name g_name;

static T_pSTR MakeFileName( audio_recorder_handle * handle )
{
	T_U32 	nSuffizeIndex = 0, nFileIndex = 0, nLastLen = 0;
	T_pSTR	strTime = NULL;
	T_pSTR	strFileName = NULL;
	char 	name[20];
	char 	num[5];
	assert( handle );

	for ( nSuffizeIndex = 0; nSuffizeIndex < FileSuffixLen; ++nSuffizeIndex ) {
		if ( FileSuffix[nSuffizeIndex].enRecType == handle->enType ) {
			break;
		}
	}

	if ( nSuffizeIndex >= FileSuffixLen ) {
		loge( "MakeFileName::Unknow recorde file type!\n" );
		return (T_pSTR)NULL;
	}

	strTime = GetCurTimeStr();
	memset(name, 0x00, 20);
	sprintf(name,"%s_%s_%s_%s", g_name.src, g_name.format, g_name.samplerate, g_name.volume);
	printf("name = %s_%s_%s_%s \n", g_name.src,g_name.format,g_name.samplerate, g_name.volume);
	printf("%s", name);
	if ( NULL == strTime ) {
		strFileName = (T_pSTR)malloc( ( strlen(handle->strPath) +
										strlen(FileSuffix[nSuffizeIndex].strSuffize) +
									    strlen(TEMP_FILE_NAME) ) + 5 );
		if ( NULL == strFileName ) {
			loge( "StartRec::out of memory!\n" );
			goto err;
		}

		strcpy( strFileName, handle->strPath );
		strcat( strFileName, TEMP_FILE_NAME );
		strcat( strFileName, FileSuffix[nSuffizeIndex].strSuffize );
	} else {
		strFileName = (T_pSTR)malloc( ( strlen(handle->strPath) +
										strlen(FileSuffix[nSuffizeIndex].strSuffize) +
							  		    strlen(name) ) + 5 );
		if ( NULL == strFileName ) {
			loge( "StartRec::out of memory!\n" );
			goto err;
		}

		strcpy( strFileName, handle->strPath );
		//strcat( strFileName, strTime );
		strcat(strFileName, name);
		strcat( strFileName, FileSuffix[nSuffizeIndex].strSuffize );

		free( strTime );
		strTime = NULL;
	}
	
	nLastLen = strlen(strFileName);
	
	while ( AK_TRUE ) {
		if ( IsExists( strFileName ) ) {
			++nFileIndex;
			if ( nFileIndex > 99 ) {
				loge( "MakeFileName::too many file have same name!\n" );
				goto err;
			}
			sprintf(num, "_%02d", nFileIndex);
			strcpy( strFileName, handle->strPath );
			strcat(strFileName, name);
			strcat(strFileName, num);
			strcat( strFileName, FileSuffix[nSuffizeIndex].strSuffize );
			//strNumer = strFileName + nLastLen;
			//sprintf( strNumer, "(%02d)", (int)(nFileIndex++) );
			continue;
		}

		break;
	}

	return strFileName;
err:
	if ( strTime ) {
		free( strTime );
	}

	if ( strFileName ) {
		free( strFileName );
		strFileName = NULL;
	}
	
	return strFileName;
}

/**
* @brief   complete write the buffer to file
* @author hankejia
* @date 2012-07-05
* @param[in] fd  		file fd
* @param[in] pData  	the data.
* @param[in] nDataSize  data length.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 WriteComplete( T_S32 fd, T_U8 * pData, T_U32 nDataSize )
{
	T_S32 n = 0, sent = 0;

	if ( fd <= 0 ) {
		loge( "WriteComplete::Invalid fd parameter\n" );
		return -1;
	}

	do {
		n = write( fd, pData + sent, nDataSize - sent );
		if ( n < 0 ) {
			loge( "WriteComplete::write file error! error = %s\n", strerror(errno) );
			return -1;
		} else {
			sent += n;
		}
		
	}while( sent < nDataSize );

	return 0;
}

/**
* @brief   write the wav file header to file
* @author hankejia
* @date 2012-07-05
* @param[in] fd  			file fd
* @param[in] nChannels  	channels.
* @param[in] nSampleRate	Sample rate.
* @param[in] nDataSize		current file len.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 WriteWavHeader( T_S32 fd, T_U32 nChannels, T_U32 nSampleRate, 
								 T_U32 nBitsPerSample, T_U32 nDataSize )
{
	WAVEHDR stFileheader;
	T_U32 nAvgBytesPerSec = 0, nBlockAlign = 0, nFileSize = 0;
	
	if ( fd <= 0 ) {
		loge( "can't write the wav header, because the fd is invalid!\n" );
		return -1;
	}

	nBlockAlign = nChannels * ( ( nBitsPerSample + 7 ) / 8 );
	nAvgBytesPerSec = nBlockAlign * nSampleRate;
	nFileSize = nDataSize + sizeof(WAVEHDR) - sizeof(CHUNKHDR);
	
	bzero( &stFileheader, sizeof( WAVEHDR ) );

	stFileheader.chkRiff.ckid    = cpu_to_le32(FOURCC_RIFF);	
	stFileheader.fccWave         = cpu_to_le32(FOURCC_WAVE);	
	stFileheader.chkFmt.ckid     = cpu_to_le32(FOURCC_FMT);	
	stFileheader.chkFmt.dwSize   = cpu_to_le32(16);	
	stFileheader.wFormatTag      = cpu_to_le16(WAVE_FORMAT_PCM);	
	stFileheader.nChannels       = cpu_to_le16(nChannels);	
	stFileheader.nSamplesPerSec  = cpu_to_le32(nSampleRate);	
	stFileheader.nAvgBytesPerSec = cpu_to_le32(nAvgBytesPerSec);	
	stFileheader.nBlockAlign     = cpu_to_le16(nBlockAlign);	
	stFileheader.wBitsPerSample  = cpu_to_le16(nBitsPerSample);	
	stFileheader.chkData.ckid    = cpu_to_le32(FOURCC_DATA);	
	stFileheader.chkRiff.dwSize  = cpu_to_le32(nFileSize);	
	stFileheader.chkData.dwSize  = cpu_to_le32(nDataSize /* data length */);

	lseek( fd, 0, SEEK_SET );

	return WriteComplete( fd, (T_U8*)(&stFileheader), sizeof(WAVEHDR) );
}


/**
* @brief   write the adpcm wav file header to file
* @author hankejia
* @date 2012-07-05
* @param[in] fd  			file fd
* @param[in] nChannels  	channels.
* @param[in] nSampleRate	Sample rate.
* @param[in] nDataSize		current file len.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 WriteADwavHeader( T_S32 fd, T_U32 nChannels, T_U32 nSampleRate, 
								 T_U32 nBitsPerSample, T_U32 nDataSize )
{
	ADPCM_HEADER adpcmh = 
	{
	    {"RIFF"},                     //"RIFF"
	        0,                          //file size
	    {"WAVEfmt "},               //"WAVEfmt "
	    0x14,                       //IMAADPCMWAVEFORMAT struct size(0x14 for IMA ADPCM)
	    {
		    0x11,                      //IMA adpcm format
		    1,                          //channel 1=mono, 2=stereo
		    8000,                       //default 8k sample rate
		    0,                          //bytes_per_sec
		    2,                          //block align
		    16,                         //bits per sample
		    2,                          //reserve bit
		    505
		},                       //samples per packet
	    {"fact"},                    //"fact"
	    4,                          //fact chuck size
	    0,                          //the number of samples
	    {"data"},                   //"data"
	    0                           //data len
	};
	//T_U32 nAvgBytesPerSec = 0, nBlockAlign = 0, nFileSize = 0;
	T_U32 nFileSize = 0;
	if ( fd <= 0 ) {
		loge( "can't write the wav header, because the fd is invalid!\n" );
		return -1;
	}

	//nBlockAlign = nChannels * ( ( nBitsPerSample + 7 ) / 8 );
	//nAvgBytesPerSec = nBlockAlign * nSampleRate;

	printf("nSamplesPerPacket = %d,nAvgBytesPerSec = %d, nBlockAlign = %d wBitsPerSample = %d ", 
	        nSamplesPerPacket, nAvgBytesPerSec, nBlockAlign, wBitsPerSample);
	nFileSize = nDataSize + sizeof(ADPCM_HEADER) - 8;

	//adpcmh.riff = cpu_to_le32(FOURCC_RIFF);
	//adpcmh.wavefmt_ = cpu_to_le32(FOURCC_WAVE);
	//adpcmh.data   = cpu_to_le32(FOURCC_DATA);

	
	adpcmh.ima_format.nChannels  = cpu_to_le16(nChannels);	
	adpcmh.ima_format.nSamplesPerSec  = cpu_to_le32(nSampleRate);	

	adpcmh.ima_format.wSamplesPerBlock = nSamplesPerPacket;
	adpcmh.ima_format.nAvgBytesPerSec = nAvgBytesPerSec;	
	adpcmh.ima_format.nBlockAlign     = nBlockAlign;	
	adpcmh.ima_format.wBitsPerSample  = 4;//(nBitsPerSample);	
		
	adpcmh.file_size = cpu_to_le32(nFileSize);
	adpcmh.factdata_size = 0;
	adpcmh.data_length  = cpu_to_le32(nDataSize /* data length */);

	lseek( fd, 0, SEEK_SET );

	return WriteComplete( fd, (T_U8*)(&adpcmh), sizeof(ADPCM_HEADER) );
}


/**
* @brief   write the amr file header to file
* @author hankejia
* @date 2012-07-05
* @param[in] fd  	file fd
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 WriteAmrHeader( T_S32 fd )
{
	T_U8 amrHdr[6];
	
	amrHdr[0] = 0x23;
	amrHdr[1] = 0x21;
	amrHdr[2] = 0x41;
	amrHdr[3] = 0x4d;
	amrHdr[4] = 0x52;
	amrHdr[5] = 0x0a;

	lseek( fd, 0, SEEK_CUR );
	
	return WriteComplete( fd, amrHdr, sizeof(amrHdr) );
}


static T_S32 WriteG711Header( T_S32 fd, T_U32 nChannels, T_U32 nSampleRate, 
								 T_U32 nBitsPerSample, T_U32 nDataSize)
{
	T_U32 nFileSize = 0;
	ADPCM_HEADER* fhG711 = (ADPCM_HEADER*)fileHeadG711;	

	fhG711->ima_format.nChannels = cpu_to_le16(nChannels);
	fhG711->ima_format.nSamplesPerSec = cpu_to_le32(nSampleRate);
	fhG711->ima_format.wBitsPerSample = cpu_to_le32(nBitsPerSample);
	nFileSize = nDataSize + sizeof(ADPCM_HEADER) - 8;

	fhG711->file_size = cpu_to_le32(nFileSize);
	fhG711->factdata_size = 0;
	fhG711->data_length  = cpu_to_le32(nDataSize); /* data length */

	lseek( fd, 0, SEEK_SET );

	return WriteComplete(fd, (T_U8*)fhG711, sizeof(ADPCM_HEADER));
}


/**
* @brief   write the file heard
* @author hankejia
* @date 2012-07-05
* @param[in] handle  	the pointer point to the audio_recorder_handle
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 WriteFileHeader( audio_recorder_handle * handle )
{
	T_S32 ret = 0;
	T_U32 nBitsPerSample = 0;
	
	assert( handle );

	if ( handle->stAlsaHandle.nFormat == SND_PCM_FORMAT_S16_LE ) {
		nBitsPerSample = 16;
	}else {
		nBitsPerSample = 8;
	}

	switch ( handle->enType )
	{
	case AUDIO_REC_TYPE_WAV_PCM:
		ret = WriteWavHeader( handle->nFID, handle->stAlsaHandle.nChannels, 
							  handle->stAlsaHandle.nSampleRate, nBitsPerSample, 0 );
		break;
	case AUDIO_REC_TYPE_ADPCM_IMA:
		ret = WriteADwavHeader(handle->nFID, handle->stAlsaHandle.nChannels,
							handle->stAlsaHandle.nSampleRate, nBitsPerSample, 0 );
		break;
	case AUDIO_REC_TYPE_AMR:
		ret = WriteAmrHeader( handle->nFID );
		break;

	case AUDIO_REC_TYPE_G711_ULOW:
		fileHeadG711[20]= 0x7; //wFormatTag 
		WriteG711Header(handle->nFID, handle->stAlsaHandle.nChannels,
							handle->stAlsaHandle.nSampleRate, nBitsPerSample, 0 );
		break;
	case AUDIO_REC_TYPE_G711_ALOW:
		fileHeadG711[20]= 0x6; //wFormatTag 
		WriteG711Header(handle->nFID, handle->stAlsaHandle.nChannels,
							handle->stAlsaHandle.nSampleRate, nBitsPerSample, 0 );
		break;
	default :
		break;
	}

	return ret;
}

/**
* @brief   judge record file type need write data to file when record finish
* @author hankejia
* @date 2012-07-05
* @param[in] handle  		the pointer point to the audio_recorder_handle
* @param[in] nTotalDataLen  current file length
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 FinishFileWriteProcess( audio_recorder_handle * handle, T_U32 nTotalDataLen )
{
	T_S32 ret = 0;
	T_U32 nBitsPerSample = 0;
	
	assert( handle );

	if ( AUDIO_REC_TYPE_WAV_PCM == handle->enType ) {
		
		if ( handle->stAlsaHandle.nFormat == SND_PCM_FORMAT_S16_LE ) {
			nBitsPerSample = 16;
		}else {
			nBitsPerSample = 8;
		}
		
		ret = WriteWavHeader( handle->nFID, handle->stAlsaHandle.nChannels, 
					  handle->stAlsaHandle.nSampleRate, nBitsPerSample, nTotalDataLen );
	}

	if( AUDIO_REC_TYPE_ADPCM_IMA ==  handle->enType )
	{
		nBitsPerSample = 8;
		ret = WriteADwavHeader( handle->nFID, handle->stAlsaHandle.nChannels, 
					  handle->stAlsaHandle.nSampleRate, nBitsPerSample, nTotalDataLen );
	}

	if (AUDIO_REC_TYPE_G711_ULOW == handle->enType
		|| AUDIO_REC_TYPE_G711_ALOW == handle->enType)
	{
		nBitsPerSample = 8;
		ret = WriteG711Header(handle->nFID, handle->stAlsaHandle.nChannels, 
					  handle->stAlsaHandle.nSampleRate, nBitsPerSample, nTotalDataLen);
	}
	
	close( handle->nFID );
	handle->nFID = -1;

	return ret;
}

/**
* @brief   run the record thread
* @author hankejia
* @date 2012-07-05
* @param[in] handle  		the pointer point to the audio_recorder_handle
* @return none
*/
static T_VOID Run( audio_recorder_handle * handle )
{
	T_S32 	ret = 0;
	T_U8 *	pBuffer = NULL;
	assert( handle );

	Condition_Lock( handle->condition );
	handle->bIsRunning = AK_TRUE;
	Condition_Unlock( handle->condition );

	while( AK_TRUE ) {
		Condition_Lock( handle->condition );
		if ( handle->bIsStop ) {
			cfifo_cancel_wait(handle->cf);
			Condition_Unlock( handle->condition );
			break;
		}
		Condition_Unlock( handle->condition );
		
		pBuffer = cfifo_get_in(handle->cf);

		//printf("---%s-%d:%d--.\n", __func__, handle->cf->in, handle->cf->out );
		ret = ReadAD(&handle->stAlsaHandle, pBuffer, handle->nRawBufferSize );

		//printf("ret=%d.\n",ret);
		if (ret < 0) {
			printf( "recorder thread read audio error! ret = %d\n", ret );
			continue;
		}else if (ret == 0) {
			printf( "AD running error! but already recover!\n" );
			continue;
		}else if (ret != handle->nRawBufferSize) {
			//overrun occur,need restart.
			printf("overrun occur .  ret = %d\n", ret );
			continue;
		}

		cfifo_in_signal(handle->cf);
	}
	printf("read audio thread stop.\n");

	Condition_Lock( handle->condition );
	handle->bIsRunning = AK_FALSE;
	handle->bIsStart = AK_FALSE;
	Condition_Unlock( handle->condition );

	logi( "rec thread done!\n" );
}

static T_pVOID write_audio_data(T_pVOID args)
{
	T_S32 	ret = 0;
	T_U32 	nDataLen = 0;
	T_U32 	nTotalDataLen = 0;
	T_U8 *	pBuffer = NULL;
	T_AUDIO_FILTER_BUF_STRC fbuf_strc;
	audio_recorder_handle * handle = (audio_recorder_handle*)args;

	if ( handle == NULL ) {
		loge( "write audio data have a invail param!" );
		return NULL;
	}

	if ( WriteFileHeader( handle ) < 0 ) {
		loge( "the thread can't run, because write file header error!\n" );
		goto End;
	}

	while (AK_TRUE) {
		cfifo_wait_empty(handle->cf);	

		Condition_Lock( handle->condition );
		if ( handle->bIsStop && cfifo_empty(handle->cf)) {
			Condition_Unlock( handle->condition );
			break;
		}
		Condition_Unlock( handle->condition );
		
		pBuffer = (T_U8*)cfifo_get_out(handle->cf);

		if(handle->filter.pfilter) {
			fbuf_strc.buf_in = pBuffer;		
			fbuf_strc.len_in = handle->nRawBufferSize;
			fbuf_strc.buf_out = handle->filter.obuf;
			fbuf_strc.len_out = 2048*4;

			ret = _SD_Filter_Control(handle->filter.pfilter, &fbuf_strc);
			if ( ret <= 0 ) {
				if ( ret < 0 )
					loge( "recorder thread read audio from AD error!" 
							" ret = %d, handle->nRawBufferSize = %d\n", ret, handle->nRawBufferSize );
				cfifo_out(handle->cf);
				continue;
			}
			pBuffer = (T_U8*)handle->filter.obuf;
		} else {
			ret = handle->nRawBufferSize;
		}

		if ( handle->enType != AUDIO_REC_TYPE_WAV_PCM )
		{
			//struct timeval tv;
			//gettimeofday(&tv, NULL);
			//unsigned int start = (tv.tv_sec * 1000 + tv.tv_usec / 1000 ); // microseconds to milliseconds
			ret = audio_pcm_encode( (T_U8*)pBuffer, ret, 
					handle->pEncBuffer, handle->nEncBufferSize );
			//gettimeofday(&tv, NULL);
			//unsigned int now = (tv.tv_sec * 1000 + tv.tv_usec / 1000 ); // microseconds to milliseconds
			//int dist = now - start;
			//logi("@@@@audio_pcm_encode,consumes time (%u)ms\n", now-start);
			if ( ret <= 0 ) {
				if ( ret < 0 )
					loge( "recorder thread encode pcm data error!\n " );

				cfifo_out(handle->cf);
				continue;
			}

			pBuffer = handle->pEncBuffer;
		}

		nDataLen = ret;
		ret = WriteComplete( handle->nFID, pBuffer, nDataLen );
		if ( ret < 0 ) {
			loge( "write audio data to file error!\n" );
			continue;
		}
		nTotalDataLen += nDataLen;

		cfifo_out(handle->cf);

		if ( nTotalDataLen >= handle->nMaxFileSize ) {
			logw( "record file size reach to the limit size, exit the record process!\n" );
			break;
		}
	}

	printf("audio recoder finish, file total len: %d \n", nTotalDataLen);
	if ( FinishFileWriteProcess( handle, nTotalDataLen ) < 0 ) {
		loge( "file finish process error, the record file may be can't use!\n" );
	}

End:
	Condition_Lock( handle->condition );
	handle->bIsStop = AK_TRUE;
	Condition_Unlock( handle->condition );
	return 0;
}


