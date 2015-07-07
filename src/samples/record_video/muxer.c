#include <sys/file.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "muxer.h"
#include "headers.h"
#include "Mutex.h"
#include "Tool.h"

typedef struct{
	void *hMedia;
	long fd;
	long index_fd;	
	AkAsyncFileWriter *pfile1;
	AkAsyncFileWriter *pfile2;
}T_VIDEO_MUXER;

T_VIDEO_MUXER muxMgr[eCHAN_NUM] = {{0}, {0}};

pthread_mutex_t muxMutex;

static void mux_setCB(T_MEDIALIB_CB* cbFunc)
{	
	cbFunc->m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	cbFunc->m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	cbFunc->m_FunFree	= (MEDIALIB_CALLBACK_FUN_FREE)free;
	cbFunc->m_FunRead	= (MEDIALIB_CALLBACK_FUN_READ)ak_rec_cb_fread;
	cbFunc->m_FunSeek	= (MEDIALIB_CALLBACK_FUN_SEEK)ak_rec_cb_fseek;
	cbFunc->m_FunTell	= (MEDIALIB_CALLBACK_FUN_TELL)ak_rec_cb_ftell;
	cbFunc->m_FunWrite	= (MEDIALIB_CALLBACK_FUN_WRITE)ak_rec_cb_fwrite;
	cbFunc->m_FunFileHandleExist = ak_rec_cb_lnx_fhandle_exist;
}

static void* mux_openLib(T_MUX_INPUT *mux_input, T_REC_CHAN chan)
{
	T_MEDIALIB_MUX_OPEN_INPUT open_input;
	T_MEDIALIB_MUX_OPEN_OUTPUT open_output;
	
	memset(&open_input, 0, sizeof(T_MEDIALIB_MUX_OPEN_INPUT));
	open_input.m_MediaRecType	= mux_input->m_MediaRecType;//MEDIALIB_REC_AVI_NORMAL;
	open_input.m_hMediaDest		= (T_S32)muxMgr[chan].pfile1;
	open_input.m_bCaptureAudio	= mux_input->m_bCaptureAudio;
	open_input.m_bNeedSYN		= AK_TRUE;
	open_input.m_bLocalMode		= AK_TRUE;
	open_input.m_bIdxInMem		= AK_FALSE;
	open_input.m_ulIndexMemSize	= 0;
	open_input.m_hIndexFile		= (T_S32)muxMgr[chan].pfile2;

	//for syn
	open_input.m_ulVFifoSize	= 200*1024; //video fifo size
	open_input.m_ulAFifoSize	= 100*1024; //audio fifo size
	open_input.m_ulTimeScale	= 1000;		//time scale

	// set video open info
	open_input.m_eVideoType		= mux_input->m_eVideoType;//MEDIALIB_VIDEO_H264;
	open_input.m_nWidth			= mux_input->m_nWidth;//640;
	open_input.m_nHeight		= mux_input->m_nHeight;//480;
	open_input.m_nFPS			= 30;
	open_input.m_nKeyframeInterval	= 24;
	
	// set audio open info
	open_input.m_eAudioType		= mux_input->m_eAudioType;//MEDIALIB_AUDIO_PCM;
	open_input.m_nSampleRate	= mux_input->m_nSampleRate;//8000;
	open_input.m_nChannels		= 1;
	open_input.m_wBitsPerSample	= 16;

	switch (mux_input->m_eAudioType)
	{
	case MEDIALIB_AUDIO_PCM:
		open_input.m_ulSamplesPerPack = mux_input->m_nSampleRate*50/1000;
		open_input.m_ulAudioBitrate	
			= mux_input->m_nSampleRate*open_input.m_wBitsPerSample*open_input.m_ulSamplesPerPack;
		break;

	case MEDIALIB_AUDIO_AAC:
		open_input.m_ulSamplesPerPack = 1024*open_input.m_nChannels;
		open_input.m_cbSize = 2;
		switch(open_input.m_nSampleRate)
		{
		case 8000 :
			open_input.m_ulAudioBitrate = 8000;
			break;
		case 11025 :
			open_input.m_ulAudioBitrate = 11025;
			break;
		case 12000 :
			open_input.m_ulAudioBitrate = 12000;
			break;
		
		case 16000:
			open_input.m_ulAudioBitrate = 16000;
			break;
		case 22050:
			open_input.m_ulAudioBitrate = 22050;
			break;
		case 24000:
			open_input.m_ulAudioBitrate = 24000;
			break;
		case 32000:
			open_input.m_ulAudioBitrate = 32000;
			break;
		case 44100:
			open_input.m_ulAudioBitrate = 44100;
			break;
		case 48000:
			open_input.m_ulAudioBitrate = 48000;
			break;
		default:
			open_input.m_ulAudioBitrate = 48000;
			break;
		}
	
		break;

	case MEDIALIB_AUDIO_ADPCM:
		open_input.m_wFormatTag = 0x11;
		
		switch(open_input.m_nSampleRate)
		{
			case 8000:
			case 11025:
			case 12000:
			case 16000:
				open_input.m_nBlockAlign = 0x100;
				break;
			case 22050:
			case 24000:
			case 32000:
				open_input.m_nBlockAlign = 0x200;
				break;
			case 44100:
			case 48000:
			case 64000:
				open_input.m_nBlockAlign = 0x400;
				break;
			default:
				open_input.m_nBlockAlign = 0x400;
				break;
		}
		
		open_input.m_ulSamplesPerPack =	(open_input.m_nBlockAlign-4)*8/4+1;
		
		open_input.m_ulAudioBitrate 
			= open_input.m_nAvgBytesPerSec 
			= open_input.m_nSampleRate * open_input.m_nBlockAlign / open_input.m_ulSamplesPerPack;

		open_input.m_nBlockAlign *= open_input.m_nChannels;
		open_input.m_cbSize = 2;
		open_input.m_pszData = (T_U8 *)&open_input.m_ulSamplesPerPack;
		break;
	
	default:
		return NULL;
		
	}
	
	mux_setCB(&open_input.m_CBFunc);
	
	return MediaLib_Mux_Open(&open_input, &open_output);

}



/**
* @brief  open muxer to write file
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int mux_open(T_REC_CHAN chan, T_MUX_INPUT *mux_input, char *filename)
{
	T_MEDIALIB_MUX_INFO MuxInfo;
	T_CHR	strFile[MAX_PATH];
	int wrBuf;

	if (filename == AK_NULL)
	{
		printf("filename is null \n");
		return -1;
	}
	
	bzero(strFile, sizeof(strFile));
	sprintf(strFile, "%s%s", mux_input->rec_path, filename);

	if (AK_NULL != mux_input->rec_path)
	{
		muxMgr[chan].fd = open(strFile, O_LARGEFILE | O_RDWR | O_CREAT | O_TRUNC);
	}
	else
	{
		muxMgr[chan].fd = open(filename, O_LARGEFILE | O_RDWR | O_CREAT | O_TRUNC);
	}
	if (muxMgr[chan].fd <= 0) {
		printf("Open File Failure\n");
		goto err;
	}
	

	if (AK_NULL != mux_input->rec_path)
	{
		bzero(strFile, sizeof(strFile));
		sprintf(strFile, "%s%sindex", mux_input->rec_path, filename);
		muxMgr[chan].index_fd = open(strFile, O_RDWR | O_CREAT | O_TRUNC); //create avi file index 
	}
	else
	{
		bzero(strFile, sizeof(strFile));
		sprintf(strFile, "%sindex", filename);
		muxMgr[chan].index_fd = open(strFile, O_RDWR | O_CREAT | O_TRUNC); //create avi file index 
	}
	if (muxMgr[chan].index_fd <= 0) {
		printf("Open Index File Failure\n");
		goto err;
	}

	if (mux_input->m_eVideoType == MEDIALIB_VIDEO_MJPEG)
		wrBuf = 6<<20;
	else
		wrBuf = 2<<20;
	muxMgr[chan].pfile1 = ak_rec_cb_load(muxMgr[chan].fd, AK_FALSE, wrBuf, 16 * 1024);
	muxMgr[chan].pfile2 = ak_rec_cb_load(muxMgr[chan].index_fd, AK_FALSE, 1*1024*1024, 16 * 1024);
	if (muxMgr[chan].pfile1 == AK_NULL || muxMgr[chan].pfile1 == AK_NULL) {
		printf("Open AsynWrite Failure\n");
		goto err;
	}

	if (!(muxMgr[chan].hMedia = mux_openLib(mux_input, chan))
		|| !MediaLib_Mux_GetInfo(muxMgr[chan].hMedia, &MuxInfo)
		|| !MediaLib_Mux_Start(muxMgr[chan].hMedia)) {
		printf("Open MuxLib Failure\n");
		goto err;
	}

	Mutex_Initialize(&muxMutex);
	return 0;


err:
	if (muxMgr[chan].fd > 0)
	{
		close(muxMgr[chan].fd);
		muxMgr[chan].fd = -1;
	}
	if (muxMgr[chan].index_fd > 0)
	{
		close(muxMgr[chan].index_fd);
		muxMgr[chan].index_fd = -1;
	}
	remove(strFile);
	if (muxMgr[chan].pfile1 != AK_NULL)
	{
		ak_rec_cb_unload(muxMgr[chan].pfile1);
		muxMgr[chan].pfile1 = NULL;
	}
	if (muxMgr[chan].pfile2 != AK_NULL)
	{
		ak_rec_cb_unload(muxMgr[chan].pfile2);
		muxMgr[chan].pfile2 = NULL;
	}
	
	if (muxMgr[chan].hMedia != AK_NULL)
	{
		MediaLib_Mux_Close(muxMgr[chan].hMedia);
		muxMgr[chan].hMedia = NULL;
	}
	
	return -1;	
}

/**
* @brief  mux audio data to file
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int mux_addAudio(void *pbuf, unsigned long size, unsigned long timestamp)
{
	T_eMEDIALIB_MUX_STATUS mux_status;
	T_MEDIALIB_MUX_PARAM mux_param;

	mux_param.m_pStreamBuf = pbuf;
	mux_param.m_ulStreamLen = size;
	mux_param.m_ulTimeStamp = timestamp;
	mux_param.m_bIsKeyframe = AK_TRUE;

	Mutex_Lock(&muxMutex);
	
	if (!MediaLib_Mux_AddAudioData(muxMgr[eCHAN_UNI].hMedia, &mux_param))
	{
		printf("MediaLib_Mux_AddAudioData error\r\n");
	}
	else
	{
		//MediaLib_Mux_Handle(hMedia);
	}
	
	Mutex_Unlock(&muxMutex);
	
	mux_status = MediaLib_Mux_GetStatus(muxMgr[eCHAN_UNI].hMedia);
	if (MEDIALIB_MUX_SYSERR == mux_status
		|| MEDIALIB_MUX_MEMFULL == mux_status)
	{
		return -1;
	}
	
	return 0;
}

/**
* @brief  mux video data to file
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int mux_addVideo(T_REC_CHAN chan, void *pbuf, unsigned long size, unsigned long timestamp, int iframe)
{
	T_eMEDIALIB_MUX_STATUS mux_status;
	T_MEDIALIB_MUX_PARAM mux_param;
	
	mux_param.m_pStreamBuf 	= pbuf;
	mux_param.m_ulStreamLen = size;
	mux_param.m_ulTimeStamp = timestamp;
	mux_param.m_bIsKeyframe = iframe;

	//check disk size
	signed long long Disksize = GetDiskFreeSize( "/mnt/" );
	if(Disksize < (T_S64)30)
	{
		printf("disk full \n");
		return -1;
	}
	
	Mutex_Lock(&muxMutex);
	
	if (!MediaLib_Mux_AddVideoData(muxMgr[chan].hMedia, &mux_param))
	{
		printf("MediaLib_Mux_AddVideoData error\r\n");
	}
	else
	{
		MediaLib_Mux_Handle(muxMgr[chan].hMedia);
	}
	
	Mutex_Unlock(&muxMutex);
	
	mux_status = MediaLib_Mux_GetStatus(muxMgr[chan].hMedia);
	if (MEDIALIB_MUX_SYSERR == mux_status 
		|| MEDIALIB_MUX_MEMFULL == mux_status)
	{
		printf("stats = %d \n", mux_status);
		return -1;
	}
	
	return 0;
}

T_U32 mux_getToltalTime(T_REC_CHAN chan)
{
	T_MEDIALIB_MUX_INFO pInfo;
	
	if (MediaLib_Mux_GetInfo(muxMgr[chan].hMedia, &pInfo))
	{
		return pInfo.m_ulTotalTime_ms;
	}
		

	return 0;
}

/**
* @brief  close muxer
* 
* @author dengzhou
* @date 2013-04-07
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int mux_close(T_REC_CHAN chan)
{
	MediaLib_Mux_Stop(muxMgr[chan].hMedia);
	MediaLib_Mux_Close(muxMgr[chan].hMedia);
	muxMgr[chan].hMedia = NULL;
	
	ak_rec_cb_unload(muxMgr[chan].pfile1);
	fsync(muxMgr[chan].fd);
	close(muxMgr[chan].fd);
	muxMgr[chan].pfile1 = NULL;
	muxMgr[chan].fd = -1;
	
	ak_rec_cb_unload(muxMgr[chan].pfile2);
	fsync(muxMgr[chan].index_fd);
	close(muxMgr[chan].index_fd);
	//remove(strFile);
	muxMgr[chan].pfile2 = NULL;
	muxMgr[chan].index_fd = -1;

	Mutex_Destroy(&muxMutex);
	return 0;
}
