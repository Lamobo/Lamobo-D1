#include <sys/file.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "muxer.h"
#include "asyncwrite.h"
#include "headers.h"
#include "Mutex.h"
#include "inisetting.h"
#include "cgi_anyka.h"
#include "Thread.h"
#include "video_process.h"
#include "Tool.h"
#include "AkRecordManager.h"
#include "led.h"
#include "SDcard.h"



int audio_ffff = 0;
int video_ffff = 0;

void *hMedia = AK_NULL;
AkAsyncFileWriter *pfile1 = AK_NULL;
AkAsyncFileWriter *pfile2 = AK_NULL;
long fid;
long index_fid;
long audio_bytes = 0;
long video_bytes = 0;
unsigned long video_tytes = 0;
T_eMEDIALIB_MUX_STATUS mux_status;
//T_MEDIALIB_MUX_PARAM mux_param;
T_CHR gpath[MAX_PATH];
pthread_mutex_t muxMutex;
nthread_t	 ThredMuxID;
static int g_mux_exit = 0;
int times = 0;
static int close_flag = 1;
extern int sd_remove;
extern int Recordflag;
int stop_record_flag = 1;

init_parse parse;
static int RecordIndex = 1;
int Recordflag = 0;

static T_pVOID thread_enc( T_pVOID user );

int record_rename_file(){
    return ChangFileName();
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
int mux_open(T_MUX_INPUT *mux_input)
{
	T_MEDIALIB_MUX_OPEN_INPUT mux_open_input;
	T_MEDIALIB_MUX_OPEN_OUTPUT mux_open_output;
	T_MEDIALIB_MUX_INFO MuxInfo;
	T_CHR	strFile[MAX_PATH];

	memset(strFile, 0x00, sizeof(strFile));
	fid = GetRecordFile(strFile); //Get the file handle and file name
	if(fid <= 0)
	{
		goto err;
	}

	index_fid = open("/mnt/index", O_RDWR | O_CREAT | O_TRUNC); //create avi file index 
	
	if(index_fid <= 0 )
	{
		goto err;
	}
	
	pfile1 = ak_rec_cb_load(fid, AK_FALSE, 6*1024*1024, 16 * 1024);
	pfile2 = ak_rec_cb_load(index_fid, AK_TRUE, 4*1024*1024, 16 * 1024);
	if(pfile1 == AK_NULL || pfile2 == AK_NULL)
	{
		goto err;
	}
	memset(&mux_open_input, 0, sizeof(T_MEDIALIB_MUX_OPEN_INPUT));
	mux_open_input.m_MediaRecType	= mux_input->m_MediaRecType;//MEDIALIB_REC_AVI_NORMAL;
	mux_open_input.m_hMediaDest		= (T_S32)pfile1;
	mux_open_input.m_bCaptureAudio	= mux_input->m_bCaptureAudio;
	mux_open_input.m_bNeedSYN		= AK_TRUE;
	mux_open_input.m_bLocalMode		= AK_TRUE;
	mux_open_input.m_bIdxInMem		= AK_FALSE;
	mux_open_input.m_ulIndexMemSize	= 0;
	mux_open_input.m_hIndexFile		= (T_S32)pfile2;

	//for syn
	mux_open_input.m_ulVFifoSize	= 200*1024; //video fifo size
	mux_open_input.m_ulAFifoSize	= 100*1024; //audio fifo size
	mux_open_input.m_ulTimeScale	= 1000;		//time scale

// set video open info
	mux_open_input.m_eVideoType		= mux_input->m_eVideoType;//MEDIALIB_VIDEO_H264;
	mux_open_input.m_nWidth			= mux_input->m_nWidth;//640;
	mux_open_input.m_nHeight		= mux_input->m_nHeight;//480;
	mux_open_input.m_nFPS			= parse.fps1;
	mux_open_input.m_nKeyframeInterval	= parse.fps1-1;
	
// set audio open info
	mux_open_input.m_eAudioType			= mux_input->m_eAudioType;//MEDIALIB_AUDIO_PCM;
	mux_open_input.m_nSampleRate		= mux_input->m_nSampleRate;//8000;
	mux_open_input.m_nChannels			= 1;
	mux_open_input.m_wBitsPerSample		= 16;

	switch (mux_input->m_eAudioType)
	{
		case MEDIALIB_AUDIO_PCM:
		{
			mux_open_input.m_ulSamplesPerPack = mux_input->m_nSampleRate*32/1000;
			mux_open_input.m_ulAudioBitrate	= mux_input->m_nSampleRate*mux_open_input.m_wBitsPerSample
				*mux_open_input.m_ulSamplesPerPack;
			break;
		}
		case MEDIALIB_AUDIO_AAC:
		{
			mux_open_input.m_ulSamplesPerPack = 1024*mux_open_input.m_nChannels;
			mux_open_input.m_cbSize = 2;
			switch(mux_open_input.m_nSampleRate)
			{
				case 8000 :
					mux_open_input.m_ulAudioBitrate = 8000;
					break;
				case 11025 :
					mux_open_input.m_ulAudioBitrate = 11025;
					break;
				case 12000 :
					mux_open_input.m_ulAudioBitrate = 12000;
					break;
			
				case 16000:
					mux_open_input.m_ulAudioBitrate = 16000;
					break;
				case 22050:
					mux_open_input.m_ulAudioBitrate = 22050;
					break;
				case 24000:
					mux_open_input.m_ulAudioBitrate = 24000;
					break;
				case 32000:
					mux_open_input.m_ulAudioBitrate = 32000;
					break;
				case 44100:
					mux_open_input.m_ulAudioBitrate = 44100;
					break;
				case 48000:
					mux_open_input.m_ulAudioBitrate = 48000;
					break;
				default:
					mux_open_input.m_ulAudioBitrate = 48000;
					break;
			}
		
			break;
		}
		case MEDIALIB_AUDIO_ADPCM:
		{
			mux_open_input.m_wFormatTag = 0x11;
			mux_open_input.m_wBitsPerSample	= 4;
			switch(mux_open_input.m_nSampleRate)
			{
				case 8000:
				case 11025:
				case 12000:
				case 16000:
					mux_open_input.m_nBlockAlign = 0x100;
					break;
				case 22050:
				case 24000:
				case 32000:
					mux_open_input.m_nBlockAlign = 0x200;
					break;
				case 44100:
				case 48000:
				case 64000:
					mux_open_input.m_nBlockAlign = 0x400;
					break;
				default:
					mux_open_input.m_nBlockAlign = 0x400;
					break;
			}
			
			mux_open_input.m_ulSamplesPerPack =
				(mux_open_input.m_nBlockAlign-4)*8/4+1;
			
			mux_open_input.m_ulAudioBitrate = 
			mux_open_input.m_nAvgBytesPerSec = mux_open_input.m_nSampleRate
						* mux_open_input.m_nBlockAlign
						/ mux_open_input.m_ulSamplesPerPack;

			mux_open_input.m_nBlockAlign *= mux_open_input.m_nChannels;
			mux_open_input.m_cbSize = 2;
			mux_open_input.m_pszData = (T_U8 *)&mux_open_input.m_ulSamplesPerPack;
			break;
		}
		default:
			goto err;
			
	}

	mux_open_input.m_CBFunc.m_FunPrintf= (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	mux_open_input.m_CBFunc.m_FunMalloc= (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	mux_open_input.m_CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
	mux_open_input.m_CBFunc.m_FunRead= (MEDIALIB_CALLBACK_FUN_READ)ak_rec_cb_fread;
	mux_open_input.m_CBFunc.m_FunSeek= (MEDIALIB_CALLBACK_FUN_SEEK)ak_rec_cb_fseek;
	mux_open_input.m_CBFunc.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)ak_rec_cb_ftell;
	mux_open_input.m_CBFunc.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)ak_rec_cb_fwrite;
	mux_open_input.m_CBFunc.m_FunFileHandleExist = ak_rec_cb_lnx_fhandle_exist;

	hMedia = MediaLib_Mux_Open(&mux_open_input, &mux_open_output);
	if (AK_NULL == hMedia)
	{
		goto err;
	}

	if (MediaLib_Mux_GetInfo(hMedia, &MuxInfo) == AK_FALSE)
	{
		goto err;
	}

	if (AK_FALSE == MediaLib_Mux_Start(hMedia))
	{
		goto err;
	}

	Mutex_Initialize(&muxMutex);
	stop_record_flag = 0;
	return 0;


err:
	if(fid > 0)
	{
		close(fid);
	}
	if(index_fid > 0)
	{
		close(index_fid);
	}
	remove(strFile);
	if(pfile1 != AK_NULL)
	{
		ak_rec_cb_unload(pfile1);
	}
	if(pfile2 != AK_NULL)
	{
		ak_rec_cb_unload(pfile2);
	}
	if(hMedia != AK_NULL)
	{
		MediaLib_Mux_Close(hMedia);
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
	if( hMedia == AK_NULL || Recordflag == 0 )
	{
		return 0;
	}
	T_MEDIALIB_MUX_PARAM mux_param;
	mux_param.m_pStreamBuf = pbuf;
	mux_param.m_ulStreamLen = size;
	mux_param.m_ulTimeStamp = timestamp;
	mux_param.m_bIsKeyframe = AK_TRUE;
//	printf("audio = %u \n", timestamp);
	Mutex_Lock(&muxMutex);
	//if (audio_tytes ! = 0)
	{
		if(!MediaLib_Mux_AddAudioData(hMedia, &mux_param))
		{
            video_ffff++;
            if(video_ffff > 765){
                printf("you should restart akiserver\n");
                exit(1);
            }
			printf("WARNING! Add Audio Failure\r\n");
			//Mutex_Unlock(&muxMutex);
			//return -1;
		}
		else
		{
			mux_status = MediaLib_Mux_Handle(hMedia);
		}
	}
	Mutex_Unlock(&muxMutex);
	
	mux_status = MediaLib_Mux_GetStatus(hMedia);
	if ( MEDIALIB_MUX_SYSERR == mux_status ||MEDIALIB_MUX_MEMFULL == mux_status )
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
int mux_addVideo(void *pbuf, unsigned long size, unsigned long timestamp, int nIsIFrame)
{
	if( hMedia == AK_NULL || Recordflag == 0 )
	{
		return 0;
	}
	T_MEDIALIB_MUX_PARAM mux_param;

	mux_param.m_pStreamBuf = pbuf;
	mux_param.m_ulStreamLen = size;
	mux_param.m_ulTimeStamp = timestamp;
	mux_param.m_bIsKeyframe = nIsIFrame;

	Mutex_Lock(&muxMutex);


	if (!MediaLib_Mux_AddVideoData(hMedia, &mux_param))
	{
        video_ffff++;
        if(video_ffff>765){
            printf("you should restart akipcserver\n");
            exit(1);
        }
		printf("WARNING! Add Video Failure! ts: %lu IF: %d\n", timestamp, nIsIFrame);
	}
	else
	{
		mux_status = MediaLib_Mux_Handle(hMedia);
	}
	
	Mutex_Unlock(&muxMutex);
	
	mux_status = MediaLib_Mux_GetStatus(hMedia);
	if (MEDIALIB_MUX_SYSERR == mux_status || MEDIALIB_MUX_MEMFULL == mux_status)
	{
		return -1;
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
int mux_close(void)
{
	T_CHR	strFile[MAX_PATH];
	printf("%s\n", __func__);
	bzero(strFile, sizeof(strFile));
	sprintf(strFile, "/mnt/index" );
	
	MediaLib_Mux_Stop(hMedia);
	ak_rec_cb_unload(pfile1);
	fsync(fid);
	close(fid);

	MediaLib_Mux_Close(hMedia);
	hMedia = AK_NULL;
	ak_rec_cb_unload(pfile2);
	fsync(index_fid);
	close(index_fid);
	remove(strFile);

	Mutex_Destroy(&muxMutex);
	return 0;
}


//init_parse parse;

int Get_Resolution(char *Res, int *width, int *height)
{
	if( !strcmp(Res, "720") )
	{
		*width = 1280;
		*height = 720;
	}
	else if( !strcmp(Res, "vga") )
	{
		*width = 640;
		*height = 480;
		parse.real_width2 = 640;
		parse.real_height2 = 480;
	}
	else if( !strcmp(Res, "qvga") )
	{
		*width = 320;
		*height = 240;
		parse.real_width2 = 480;
		parse.real_height2 = 270;
	}
	else if( !strcmp(Res, "d1"))
	{
		*width = 720;
		*height = 576;
		parse.real_width2 = 720;
		parse.real_height2 = 576;
	}
	else
	{
		printf(" no support Resolution \n ");
		return -1;
	}

	return 0;
}

void read_Parse(void *mSettings)
{	
	demo_setting * set = (demo_setting *) mSettings;
	IniSetting_init();
	struct video_info * video = IniSetting_GetVideoInfo();
	if( !strcmp(video->format1, "mjpeg") )
	{
		set->video_types = 1;
		parse.format1 = 1;
	}
	else
	{
		set->video_types = 0;
		parse.format1 = 0;
	}
	
	if( !strcmp(video->kbps_mode1, "static_kbps") )
	{
		parse.kbps_mode1 = 0;
		set->kbpsmode = 0;
	}
	else
	{
		parse.kbps_mode1 = 1;
		set->kbpsmode = 1;
	}
	
	parse.kbps1 = atoi( video->kbps1 );
	if(parse.kbps1 <= 6144 && parse.kbps1 >= 256)
		parse.kbps1 = parse.kbps1 * 1024;
	else
		parse.kbps1 = 256 * 1024;
	
	//parse.minqp1 = atoi( video->minqp1 );
	parse.group1 = atoi( video->group1);
	parse.fps1 = atoi( video->fps1 );
	Get_Resolution(video->dpi1, &parse.width, &parse.height);
	
	if(parse.fps1 <= 30 && parse.fps1 >= 1)
		set->framePerSecond = parse.fps1;
	if(parse.width %4 == 0 && parse.width > 0)
		set->width = parse.width;
	if(parse.height %2 == 0 && parse.height > 0)
		set->height = parse.height;
	if( !strcmp(video->format2, "mjpeg") )
	{
		parse.format2 = 1;
	}
	else
	{
		parse.format2 = 0;
	}
	
	if( !strcmp(video->kbps_mode2, "static_kbps") )
	{
		parse.kbps_mode2 = 0;
	}
	else
	{
		parse.kbps_mode2 = 1;
	}
	
	parse.kbps2 = atoi( video->kbps2 );
	if(parse.kbps2 <= 6144 && parse.kbps2 >= 64)
		parse.kbps2 = parse.kbps2 * 1000;
	else
		parse.kbps2 = 64 * 1000;
	
	parse.quality = atoi( video->quality );
	
	parse.group2 = atoi( video->group2 );
	
	parse.fps2 = atoi( video->fps2 );
	
	Get_Resolution(video->dpi2 , &parse.width2 , &parse.height2);
	
	printf("width2 = %d, height2 = %d", parse.width2, parse.height2);
		
	if(set->kbpsmode == 0)//static
	{
		if(parse.kbps1 <= 5000 && parse.kbps1 >= 64)
			set->bitPerSecond = parse.kbps1 * 1000;
		set->minQp = 20;
		set->maxQp = 45;
	}
	else
	{
		set->bitPerSecond = set->width * set->height * 3 / 2 * 8 * set->framePerSecond / 40;
		set->minQp = 30;
		set->maxQp = 30;
	}
	parse.video_kbps = atoi( video->video_kbps);
//	printf("width = %d, height = %d,bitPerSecond=%d", set->width, set->height, set->bitPerSecond);
	IniSetting_destroy();
	
}

typedef struct BitsRateTable_st
{
	T_U32 	nVideoWidth;
	T_U32	nVideoHeight;
	T_BOOL	nHasAudio;
	T_U32	nBitsRate;
}BitsRateTable;

static const BitsRateTable BRTable[] = {
	{ 720, 576, AK_TRUE,  (200) *1024 * 1024 * 8 },
	{ 640, 480, AK_TRUE,  (500 + 30) * 1024 * 8 },
	{ 320, 240, AK_TRUE,  (170 + 30) * 1024 * 8 }
	
};

static const BitsRateTable BRTable_h264[] = {
	{ 1280, 720, AK_TRUE,  300000 },
	{ 720, 576, AK_TRUE,   250000 },
	{ 640, 480, AK_TRUE,   200000 },
	{ 320, 240, AK_TRUE,   100000 }
};


#define MIN_LIMIT_FREE_SPACE_SIZE		268435456		// 256M
#define MIN_LIMIT_FREE_SPACE_SIZE_CALC	314572800		// 300M

int start_record( int cycrecord )
{
	signed long long DiskSize = 0;
	T_S64 DiskSizeTenth = 0;
	T_S32 bavail, bsize;
	int format;
	int wid;
	
#if 1		
	DiskFreeSize( "/mnt", &bavail, &bsize);
	DiskSize = (T_S64)(T_U32)(bavail) * (T_S64)(T_U32)(bsize);
	printf("avail= %ld, bsize = %ld, DiskSize = %lld\n", bavail, bsize, DiskSize);

    while(DiskSize < (T_S64)MIN_LIMIT_FREE_SPACE_SIZE) {
        delete_oldest_file();
        DiskFreeSize( "/mnt", &bavail, &bsize);
        DiskSize = (T_S64)(T_U32)(bavail) * (T_S64)(T_U32)(bsize);
        printf("avail= %ld, bsize = %ld, DiskSize = %lld\n", bavail, bsize, DiskSize);
    }
    /*
	if (DiskSize < (T_S64)MIN_LIMIT_FREE_SPACE_SIZE) 
	{
		printf( "get %s disk size full!\n", "/mnt" );
		return -1;
	}
    */
	
	DiskSizeTenth = DiskSize /10;
	if ( DiskSizeTenth > MIN_LIMIT_FREE_SPACE_SIZE_CALC ) {
		SetMinRecordLimit( MIN_LIMIT_FREE_SPACE_SIZE_CALC );
	}else if ( ( DiskSizeTenth < MIN_LIMIT_FREE_SPACE_SIZE ) && 
				 ( DiskSize > MIN_LIMIT_FREE_SPACE_SIZE ) ) {
		SetMinRecordLimit( MIN_LIMIT_FREE_SPACE_SIZE );
	}else if ( DiskSizeTenth > MIN_LIMIT_FREE_SPACE_SIZE ) {
		SetMinRecordLimit( DiskSizeTenth );
	}else {
		printf("the %s is disk size is %lldM is too less!", "/mnt/", DiskSize);
		
		return -1;
	}
#endif

	IniSetting_init();
	struct recoder_info * recoder = IniSetting_GetRecordInfo();
	T_S32 video_index = atoi(recoder->video_index);
	T_S32 leng = atoi(recoder->length);
	T_S32 time = atoi(recoder->time);
	
	if (leng <= 0)
		leng = 3;
	
	if (time <= 0)
		time = 1;
	
	times = (time*3600)/(leng*60);
	if (times <= 0)
		times = 1;

	IniSetting_destroy();

	T_U32 bitsrate;
	if (1 == video_index)
		bitsrate = (T_U32)BRTable_h264[0].nBitsRate;
	else if (2 == video_index)
	{
		switch (parse.format2)
		{
			case 0:
				format = 0;
				break;
			case 1:
				format = 1;
				break;
			default:
				format = 0;
		}
		
		switch (parse.width2)
		{
			case 720:
				wid = 0;
				break;
			case 640:
				wid = 1;
				break;
			case 320:
				wid = 2;
				break;
			default:
				wid = 1;
		}

		if (format == 0)
			bitsrate = BRTable_h264[wid].nBitsRate;
		else
			bitsrate = BRTable[wid].nBitsRate;
	}

	if (0 > recmgr_open("/mnt", bitsrate, leng*60*1000, cycrecord))
	{
		printf("open record manager error \n");
		return -1;
	}
	
	T_MUX_INPUT mux_input1;
	mux_input1.m_MediaRecType = MEDIALIB_REC_AVI_NORMAL;
	
	if (video_index == 1)
	{
		//mux video
		if(parse.format1 == 0)
			mux_input1.m_eVideoType = MEDIALIB_VIDEO_H264;
		else if(parse.format1 == 1)
			mux_input1.m_eVideoType = MEDIALIB_VIDEO_MJPEG;

		mux_input1.m_nWidth = parse.width;
		mux_input1.m_nHeight = parse.height;
	}
	else
	{
		//mux video
		if (parse.format2 == 0)
			mux_input1.m_eVideoType = MEDIALIB_VIDEO_H264;
		else if (parse.format2 == 1)
			mux_input1.m_eVideoType = MEDIALIB_VIDEO_MJPEG;

		mux_input1.m_nWidth = parse.width2;
		mux_input1.m_nHeight = parse.height2;
	}
	
	//mux audio
	mux_input1.m_bCaptureAudio = 1;
	mux_input1.m_eAudioType = MEDIALIB_AUDIO_AAC;
	mux_input1.m_nSampleRate = 8000;
//	mux_input1.abitsrate = 0;
//	mux_input1.pfile1 = AK_NULL;
//	mux_input1.pfile2 = AK_NULL;
	if (mux_open(&mux_input1) < 0)
	{
		printf("open mux err \n");
		return -1;
	}

	close_flag = 0;
	g_mux_exit = 0;
	pthread_attr_t SchedAttr;
	struct sched_param	SchedParam;
	memset(&SchedAttr, 0, sizeof(pthread_attr_t));
	memset(&SchedParam, 0, sizeof(SchedParam));

	pthread_attr_init( &SchedAttr );				
	SchedParam.sched_priority = 60;	
	pthread_attr_setschedparam( &SchedAttr, &SchedParam );
	pthread_attr_setschedpolicy( &SchedAttr, SCHED_RR );
	if (pthread_create(&ThredMuxID, &SchedAttr, thread_enc, NULL ) != 0 ) 
	{
		pthread_attr_destroy(&SchedAttr);
//		printf( "unable to create a thread for osd = %d!\n" );
		return -1;
	}
	
	pthread_attr_destroy(&SchedAttr);
	setled_record_start(video_index-1);
	
	return 0;
}

int stop_record()
{
	mux_close();
	CloseRecordManager();
	return 0;
}
extern T_MUX_INPUT mux_input;


int mux_write_data(int type, void *pbuf, unsigned long size, unsigned long timestamp, int nIsIFrame)
{
	int ret = 0;

	if (hMedia == AK_NULL || close_flag == 1 || stop_record_flag == 1)
	{
		
		return 0;
	}
#if 0	
	MediaLib_Mux_GetInfo(hMedia, &mux_info);
	
	if(ReachLimit(mux_info.m_ulFileBytes, mux_info.m_ulTotalTime_ms))
	{
		mux_close();
		times--;
		if( times == 0 )
		{
			printf("Record over \n");
			CloseRecordManager();
			Recordflag = 0;
			return -1;
		}
		else
		{		
			mux_open( &mux_input );
						
		}
		
	}
#endif	
	if (type == 0)
		ret = mux_addAudio(pbuf, size, timestamp);
	else
		ret = mux_addVideo(pbuf, size, timestamp, nIsIFrame);

	return ret;
}

static T_pVOID thread_enc( T_pVOID user )
{
	T_U8* pbuf = (T_U8*)malloc(100 * 1024);
	T_MEDIALIB_MUX_INFO mux_info;
	
	while (1)
	{
		if (g_mux_exit == 1)
		{
			printf("video mux thread exit \n");
			break;
		}

		usleep(10000);
		MediaLib_Mux_GetInfo(hMedia, &mux_info);

		// Close DV File
		if (ReachLimit(mux_info.m_ulFileBytes, mux_info.m_ulTotalTime_ms) /*|| sd_remove == 1*/ )
		{
			stop_record_flag = 1;

			MediaLib_Mux_Stop(hMedia);			
			ak_rec_cb_unload(pfile1);
			fsync(fid);
			close(fid);

			ChangFileName();

            times--;
            signed long long DiskSize = 0;
            T_S32 bavail, bsize;

            DiskFreeSize( "/mnt", &bavail, &bsize);
            DiskSize = (T_S64)(T_U32)(bavail) * (T_S64)(T_U32)(bsize);
            printf("avail= %ld, bsize = %ld, DiskSize = %lld\n", bavail, bsize, DiskSize);

            while(DiskSize < (T_S64)MIN_LIMIT_FREE_SPACE_SIZE) {
                delete_oldest_file();
                DiskFreeSize( "/mnt", &bavail, &bsize);
                DiskSize = (T_S64)(T_U32)(bavail) * (T_S64)(T_U32)(bsize);
                printf("avail= %ld, bsize = %ld, DiskSize = %lld\n", bavail, bsize, DiskSize);
            }

            printf("[##]times=%d\n",times);
            if (times <= 0)
            {
                printf("Record over \n");
				
				CloseRecordManager();
				Recordflag = 0;
				setled_record_stop(RecordIndex);
				CloseListenSD();
				break;
			}
			else
			{		
				T_CHR	strFile[MAX_PATH];

				memset(strFile, 0x00, sizeof(strFile));
				fid = GetRecordFile(strFile); //Get the file handle and file name
				if (fid <= 0)
					break;

				pfile1 = ak_rec_cb_load(fid, AK_FALSE, 6*1024*1024, 16 * 1024);
				//pfile2 = ak_rec_cb_load(index_fid, AK_TRUE, 4*1024*1024, 16 * 1024);
				if (pfile1 == AK_NULL )
				{
					printf("Open file err \n");
					break;
				}
				
				MediaLib_Mux_Restart(hMedia, (T_S32)pfile1);
			}
			
			stop_record_flag = 0;
		}
	}
	
	free(pbuf);
	return NULL;
}


int mux_exit( void )
{
	if (hMedia == AK_NULL)
		return 0;
	
	g_mux_exit = 1;
	printf("mux process close \n");
	//wait capture thread return
	pthread_join(ThredMuxID, NULL);
	ThredMuxID	= thread_zeroid();

	mux_close();
	CloseRecordManager();
	CloseListenSD();
	
	return 0;
}

