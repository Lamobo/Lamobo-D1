#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <execinfo.h>

#include "anyka_types.h"
#include "gnu_getopt.h"
#include "akuio.h"
#include "camera.h"
#include "encode.h"
#include "muxer.h"
#include "headers.h"
#include "audio.h"
#include "log.h"
#include "SDcard.h"
#include "Tool.h"

#define FILE_NAME_LEN 12
#define MAX_WIDTH	1280
#define MAX_HEIGHT	720
int g_exit = 0;
int g_width = 0;
int g_height = 0;

const char* avi_fname_1 = "test.avi";
const char* avi_fname_2 = "test_2.avi";

const char* mp4_fname_1 = "test.mp4";
const char* mp4_fname_2 = "test_2.mp4";

#define DELETE_PTR( ptr )                       \
  do {                                          \
    if ( ptr != NULL ) {                        \
      free( ptr );                              \
      ptr = NULL;                               \
    }                                           \
  } while( 0 )

#define DV_FPS	30
/*
 * 当宏定义DUAL_CHAN_PAR 为真时, 为双通道并行编码,
 * 否则, 为双通道串行编码.
 */
#define DUAL_CHAN_PAR
//#undef DUAL_CHAN_PAR

//长操作命令的定义，-Help/-help
#define LONG_OPTIONS()
const struct option long_options[] =
{
//-Help 带参数（即-H *）,对应的短操作参数 -h
{"Help",			required_argument, NULL, 'H'},	//printf the help document
//-help 没有参数（即-H）, 对应的短操作参数 -H
{"help",					  no_argument, NULL, 'H'},	//printf the help document
{0, 0, 0, 0}
};

//短操作命令的定义，-h/-H,带参数的操作命令后面跟':'符号
const char short_options[] = "h:w:b:q:i:t:p:a:s:r:v:m:x:y:e:f:z:c:HM:V:l:r:P:";
//即-H后面可以跟输入，-h后面不跟，带不带参数根据长操作命令定义中的required_argument/no_argument决定

//init the demo setting struct
static void Settings_Initialize( demo_setting *main );

// parse settings from app's command line
static void Settings_ParseCommandLine( int argc, char **argv, 
												 demo_setting *mSettings );

// parse settings from app's command line
static void Settings_Interpret( char option, const char *optarg, 
								   demo_setting *mExtSettings );

static void Settings_Destroy( demo_setting *mSettings );
	
//printf the help document
static void help( char * strHelpArg );

static void sigprocess( int inSigno )
{
	logi( "need to exit the Demo!\n" );
	
	g_exit = 1;
}

/**
* @brief  init muxer input parameter
* 
* @author xiewenzhong
* @date 2013-09-12
* @param[out] muxIn,  return muxer parameter
* @param[in] envSet,  envionment variable
* @param[in] dualChannel,  "0" for first channel, "1" for second channel
* @return T_VOID
*/
T_VOID mux_initPara(T_MUX_INPUT* muxIn, demo_setting* envSet, T_U8 dualChannel)
{
	switch (envSet->audioType)
	{
	case ENC_TYPE_PCM:
		muxIn->m_eAudioType = MEDIALIB_AUDIO_PCM;
		break;
	
	case ENC_TYPE_AAC:
		muxIn->m_eAudioType = MEDIALIB_AUDIO_AAC;
		break;
		
	case ENC_TYPE_ADPCM_IMA:
		muxIn->m_eAudioType = MEDIALIB_AUDIO_ADPCM;
		break;
	}

	if (envSet->filetype == 0)
		muxIn->m_MediaRecType = MEDIALIB_REC_AVI_NORMAL;
	else
		muxIn->m_MediaRecType = MEDIALIB_REC_3GP;


	muxIn->rec_path = envSet->rec_path;

	muxIn->m_nSampleRate = envSet->aSamplerate;

	if (dualChannel)
	{
		muxIn->m_nWidth 	= envSet->width2;
		muxIn->m_nHeight 	= envSet->height2;
		muxIn->m_bCaptureAudio = 0;	
		
		//video
		if (envSet->video_types2 == 0)
		{
			muxIn->m_eVideoType = MEDIALIB_VIDEO_H264;
		}
		else if(envSet->video_types2 == 1)
		{
			muxIn->m_eVideoType = MEDIALIB_VIDEO_MJPEG;
		}
	}
	else
	{
		muxIn->m_nWidth 	= envSet->width;
		muxIn->m_nHeight	= envSet->height;

		if (envSet->bhasAudio)
		{
			muxIn->m_bCaptureAudio = 1;
		}
		
		//video
		if (envSet->video_types == 0)
		{
			muxIn->m_eVideoType = MEDIALIB_VIDEO_H264;
		}
		else if(envSet->video_types == 1)
		{
			muxIn->m_eVideoType = MEDIALIB_VIDEO_MJPEG;
		}
	}
}


static void video_proc(demo_setting* ext_gSettings)
{
	T_U8 time_flag = 0;	
	T_U32 ts = 0;
	unsigned long size;
	unsigned long times;
	struct timeval tv;
	unsigned long timestamp = 0;	
	unsigned long t0;
	unsigned long t1 = 0;
	unsigned long num = 0;
	unsigned long insert_num = 0;
	
	gettimeofday(&tv, NULL);
	t0 = tv.tv_sec*1000 + tv.tv_usec/1000;
	times = t0 + ext_gSettings->enc_time*1000;
	
	do{
		void *pbuf;
		void *pencbuf;
		void *pencbuf2;

		gettimeofday(&tv, NULL);
		if (1 != camera_getframe(&pbuf, &size, &ts))
		{
			if( g_exit == 1 )
				break;
			printf("Error!!! CAN NOT Get frame\n");
			continue;
		}

		// discard init FRAME
		if (0 == time_flag)
		{
			if (ts > 1000)
			{
				printf("get ts is %ld\n",ts);
				camera_usebufok();

				gettimeofday(&tv, NULL);
				t0 = tv.tv_sec*1000 + tv.tv_usec/1000;
				times = t0 + ext_gSettings->enc_time*1000;
				
				continue;
			}
			
			time_flag = 1;
		}

		t1 = (tv.tv_sec*1000 + tv.tv_usec/1000);
		unsigned long ts1 = t1-t0;
		
		if (ts1 > timestamp + 1000/DV_FPS)
		{	
			int diff_num;
			if ((diff_num = (ts1 - timestamp)*DV_FPS/1000) > 2)
				printf("t: %lu/%lu, %lu  +%d\n", timestamp, ts1, ts, diff_num);
			num += diff_num;
			insert_num += diff_num;
			timestamp += diff_num*1000/DV_FPS;
		}
		else if (ts1 + 300 < timestamp)
		{
			// Discard Frame, When Video stamp faster than system stamp, 300ms.
			printf("T: %lu/%lu, %lu\n", timestamp, ts1, ts);
			camera_usebufok();
			continue;			
		}		
		
		long frameLenA;
		int iframe;		
		// Dual Channel Mode
		if (ext_gSettings->mode == 2) {
			long frameLenB;
			int iframe2;
			long offset = (ext_gSettings->height*ext_gSettings->width*3/2);
#ifdef DUAL_CHAN_PAR
			// 1st & 2nd Channel Encode
			encode_frame(&frameLenA, pbuf, &pencbuf, &iframe, &frameLenB, pbuf+offset, &pencbuf2, &iframe2, ext_gSettings->vbr);			
#else
			// 1st Channel Encode
			encode_frame(&frameLenA, pbuf, &pencbuf, &iframe, NULL, NULL, NULL, NULL, ext_gSettings->vbr);
			// 2nd Channel Encode
			encode_frame(NULL, NULL, NULL, NULL, &frameLenB, pbuf+offset, &pencbuf2, &iframe2, ext_gSettings->vbr);
#endif		
			// Write 2nd Channel Data
			mux_addVideo(eCHAN_DUAL, pencbuf2, frameLenB, timestamp, iframe2);
		}
		// Single Channle Mode
		else {
			encode_frame(&frameLenA, pbuf, &pencbuf, &iframe, NULL, NULL, NULL, NULL, ext_gSettings->vbr);
		}
			
		camera_usebufok();	
		// Write 1nd Channel Data
		if (mux_addVideo(eCHAN_UNI, pencbuf, frameLenA, timestamp, iframe) < 0)	{
			printf("mux_addVideo err \n");
			break;
		}
		
		if (!ext_gSettings->bhasAudio)
		{
			ts = timestamp;
		}
#if 1
		timestamp = ++num * 1000 / DV_FPS;
#else
		timestamp = mux_getToltalTime(eCHAN_UNI) + 33;
#endif
		
	}while(t1 <= times && !g_exit);
	g_exit = 1;
	printf("video thread eixt \n");
	printf("insert frame: %lu/%lu\n", insert_num, num);
}

void dump(int signo)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	printf("Obtained %zd stack frames \n", size);

	for(i = 0; i<size; i++)
		printf("%s\n", strings[i]);

	free(strings);

	exit(0);
}

/**
* @brief  main
* 
* @author hankejia
* @date 2012-07-05
* @param[in] argc  arg count
* @param[in] argv  arg array.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int main( int argc, char **argv )
{
	demo_setting * ext_gSettings = NULL;	
	signal(SIGINT, sigprocess);
	signal(SIGSEGV, dump);

	// Allocate the "global" settings
	ext_gSettings = (demo_setting*)malloc( sizeof( demo_setting ) );
	if ( NULL == ext_gSettings ) {
		printf( "main::out of memory!\n" );
		return -1;
	}

	//init the setting struct
	Settings_Initialize(ext_gSettings);
	// read settings from command-line parameters
	Settings_ParseCommandLine(argc, argv, ext_gSettings);
	if (ext_gSettings->width > MAX_WIDTH
		|| ext_gSettings->height > MAX_HEIGHT){
		//Unsupport Size
		printf("Unsupport Size, W: %lu, H: %lu\n", ext_gSettings->width, ext_gSettings->height);
		free(ext_gSettings);
		return -1;
	}
#if 0	
	// MJPEG
	if (ext_gSettings->video_types == 1	
		&& (ext_gSettings->width > 720 || ext_gSettings->height > 480)){
		printf("1st channel Unsuppose Format, MJPEG W: %lu, H: %lu\n", ext_gSettings->width, ext_gSettings->height);
		free(ext_gSettings);
		return -1;
	}
	
	// MJPEG 	
	if (ext_gSettings->video_types2 == 1	
		&& (ext_gSettings->width2 > 640 || ext_gSettings->height2 > 480)){
		printf("2nd channel Unsuppose Format, MJPEG W: %ld, H: %ld\n", ext_gSettings->width2, ext_gSettings->height2);
		free(ext_gSettings);
		return -1;
	}
#endif	
	if (access("/dev/mmcblk0", R_OK) < 0)
	{
		//no sd card
		printf("no SDsard exit \n");
		free(ext_gSettings);
		return -1;
	}
	else
	{
		printf("install sdcard \n");
		mount_sd();
		InitListenSD();		
	}

	signed long long Disksize = GetDiskFreeSize("/mnt/");

	if (Disksize < (T_S64)30)
	{
		printf("disk full \n");
		free(ext_gSettings);
		return -1;
	}
			
	// init dma memory
	akuio_pmem_init();
	
	g_width 	= ext_gSettings->width;
	g_height 	= ext_gSettings->height;
	// open camera device
	camera_open(ext_gSettings);
	printf("camera_open ok\n");		

	// init encode lib
	encode_init();
	T_ENC_INPUT* pEncIn1 = NULL;
	T_ENC_INPUT* pEncIn2 = NULL;

	pEncIn1 = (T_ENC_INPUT*)malloc(sizeof(T_ENC_INPUT));

	//the first encode channel
	pEncIn1->width 		= ext_gSettings->width;			//实际编码图像的宽度，能被4整除		
	pEncIn1->height 	= ext_gSettings->height;			//实际编码图像的长度，能被2整除 
	pEncIn1->qpHdr 		= ext_gSettings->qpHdr;			//初始的QP的值
	pEncIn1->iqpHdr 	= ext_gSettings->iqpHdr;			//初始的QP的值
	pEncIn1->qpMax 		= ext_gSettings->qpMax;
	pEncIn1->qpMin 		= ext_gSettings->qpMin;
	pEncIn1->bitPerSecond = ext_gSettings->bitPerSecond;	//目标bps
	pEncIn1->video_tytes = ext_gSettings->video_types;
	pEncIn1->profile	= ext_gSettings->profile;		// Profile Level

	//the second encode channel
	if (ext_gSettings->mode == 2)
	{
		pEncIn2 = (T_ENC_INPUT*)malloc(sizeof(T_ENC_INPUT));
		
		pEncIn2->width 		= ext_gSettings->width2;			//实际编码图像的宽度，能被4整除
		pEncIn2->height 	= ext_gSettings->height2;			//实际编码图像的长度，能被2整除 
		pEncIn2->qpHdr 		= ext_gSettings->qpHdr;			//初始的QP的值
		pEncIn2->iqpHdr 	= ext_gSettings->iqpHdr;			//初始的QP的值
		pEncIn2->bitPerSecond = ext_gSettings->bitPerSecond;	//目标bps
		pEncIn2->video_tytes = ext_gSettings->video_types2;
		pEncIn2->qpMax		= ext_gSettings->qpMax;
		pEncIn2->qpMin 		= ext_gSettings->qpMin;
		pEncIn2->profile	= ext_gSettings->profile;	// Profile Level
	}
	
	// open encode lib
	int ret = encode_open(pEncIn1, pEncIn2);

	if (pEncIn1)
		free(pEncIn1);
	if (pEncIn2)
		free(pEncIn2);

	if (0 != ret) {
		camera_close();
		akuio_pmem_fini();
		encode_destroy();	
		CloseListenSD();

		return -1;
	}
		
	//mux_open
	T_MUX_INPUT mux_input1;
	T_MUX_INPUT mux_input2;
	char filename[FILE_NAME_LEN];
	memset(&mux_input1, 0, sizeof(T_MUX_INPUT));
	memset(&mux_input2, 0, sizeof(T_MUX_INPUT));	
	memset(filename, 0, FILE_NAME_LEN);
	
	mux_initPara(&mux_input1, ext_gSettings, 0);
	
	if( ext_gSettings->filetype == 0)
		memcpy(filename, avi_fname_1, strlen(avi_fname_1));
	else
		memcpy(filename, mp4_fname_1, strlen(mp4_fname_1));
	
	if(-1 == mux_open(eCHAN_UNI, &mux_input1, filename))
	{
		printf("mux_open eCHAN_UNI err\n");
		return 0;
	}
	
	if (ext_gSettings->mode == 2)
	{
		mux_initPara(&mux_input2, ext_gSettings, 1);
		
		if( ext_gSettings->filetype == 0)
			memcpy(filename, avi_fname_2, strlen(avi_fname_2));
		else
			memcpy(filename, mp4_fname_2, strlen(mp4_fname_2));
		
		if(-1 == mux_open(eCHAN_DUAL, &mux_input2, filename))
		{
			printf("mux_open eCHAN_DUAL err\n");
			return 0;
		}
	}
	
	if (ext_gSettings->rec_path != NULL)
	{
		free(ext_gSettings->rec_path);
		ext_gSettings->rec_path = NULL;
	}
	printf("mux_open ok\n");

	//audio
	if (ext_gSettings->bhasAudio)
	{
		T_AUDIO_INPUT audioInput;
		audioInput.enc_type = ext_gSettings->audioType;
		audioInput.nBitsRate = 0;
		audioInput.nBitsPerSample = 16;
		audioInput.nChannels = 1;
		audioInput.nSampleRate = ext_gSettings->aSamplerate;

		audio_open(&audioInput);
		audio_start();
		printf("audio is ready\n");
	}
	
	camera_start();	
	
	//handle video encode and mux
	video_proc(ext_gSettings);
	
	if (ext_gSettings->bhasAudio)
	{
		printf("audio_close\n");
		audio_stop();
		audio_close();
	}
	usleep(100*1000);
	camera_close();
	encode_close(eCHAN_UNI);
	mux_close(eCHAN_UNI);
	//close second channel
	if (ext_gSettings->mode == 2)
	{
		encode_close(eCHAN_DUAL);
		mux_close(eCHAN_DUAL);
	}

	encode_destroy();
	akuio_pmem_fini();
	CloseListenSD();

	printf("Recorder Process Exit\n");

	return 0;	
}

/**
* @brief  init the demo setting struct
* 
* @author hankejia
* @date 2012-07-05
* @param[out] main  setting struct pointer
* @return NONE
*/
static void Settings_Initialize( demo_setting *main ) 
{
	// Everything defaults to zero or NULL with
    // this memset. Only need to set non-zero values
    // below.
    memset(main, 0, sizeof(demo_setting));
	main->width			= 1280;			//实际编码图像的宽度，能被4整除
	main->height		= 720;			//实际编码图像的长度，能被2整除 
	main->qpHdr			= -1;			//初始的QP的值
	main->iqpHdr		= 30;			//初始的iQP的值
	main->qpMax			= 50;
	main->qpMin			= 10;
	main->bitPerSecond	= 1024*4000;	//目标bps
	main->vbr			= 1;			// Default Support VBR
	main->profile		= 0;			// profile level
	main->enc_time		= 300;          //默认300秒
	
	main->bhasAudio 	= 1;
	main->audioType		= ENC_TYPE_AAC; //默认AAC编码
	main->aSamplerate	= 8000;
	
	main->mode			= 0;            //默认单通道编码
	main->video_types   = 0;            //默认H264编码
	main->filetype 		= 0;

    //通道2设置
	main->video_types2	= 0;
	main->width2  		= 320;
	main->height2		= 240;
	
	main->times			= 0;
}

/**
* @brief  parse settings from app's command line
* 
* @author hankejia
* @date 2012-07-05
* @param[in] argc  		arg count
* @param[in] argv  		arg array.
* @param[out] mSettings  	setting struct pointer.
* @return NONE
*/
static void Settings_ParseCommandLine( int argc, char **argv, demo_setting *mSettings )
{
    int option = 0;
	//find out the option
    while (EOF != (option = gnu_getopt_long(argc, argv, short_options, long_options, NULL ))) {
        Settings_Interpret(option, gnu_optarg, mSettings);
    }

	for ( int i = gnu_optind; i < argc; i++ ) {
        printf( "%s: ignoring extra argument -- %s\n", argv[0], argv[i] );
    }
}


/**
* @brief  parse settings from app's command line
* 
* @author hankejia
* @date 2012-07-05
* @param[in] option  			option [-s/-c/-a...]
* @param[in] optarg  			arg
* @param[out] mExtSettings  	setting struct pointer.
* @return NONE
*/
static void Settings_Interpret(char option, const char *optarg, 
								   demo_setting *mExtSettings )
{
	assert( mExtSettings );
	
	switch (option) {
		
	case 'H':	//printf the help document
		help( NULL );
		Settings_Destroy( mExtSettings );
		exit(0);
		break;

	case 'h':	
		mExtSettings->height = (T_U32)(atoi( optarg ));
		break;

	case 'w':	
		mExtSettings->width = (T_U32)(atoi( optarg ));
		break;

	case 'b':	
		mExtSettings->bitPerSecond = (T_U32)(atoi( optarg ));
		break;

	case 'q':	
		mExtSettings->qpHdr = (T_U32)(atoi( optarg ));
		break;

	case 'i':	
		mExtSettings->iqpHdr = (T_U32)(atoi( optarg ));
		break;
	case 'r':
		mExtSettings->vbr = (T_U8)(atoi(optarg));
		break;
	case 'P':
		mExtSettings->profile = (T_U8)(atoi(optarg));
		break;
	case 't':	
		mExtSettings->enc_time = (T_U32)(atoi( optarg ));
		break;

	case 'p':	
		mExtSettings->rec_path = (T_pSTR)malloc((strlen(optarg)+2)*sizeof(char));
		bzero(mExtSettings->rec_path, (strlen(optarg)+2)*sizeof(char));
		strcpy( mExtSettings->rec_path, optarg );

		if ( mExtSettings->rec_path[strlen(optarg) - 1] != '/' ) {
			mExtSettings->rec_path[strlen(optarg)] = '/';
		}

		if ( !IsExists( mExtSettings->rec_path ) ) {
			if ( CompleteCreateDirectory( mExtSettings->rec_path ) < 0 ) {
				printf("can't not create %s!\n", mExtSettings->rec_path);
				free( mExtSettings->rec_path );
				mExtSettings->rec_path = NULL;
			}
		}
				
		break;

	case 'a':	
		mExtSettings->bhasAudio = 1;
		mExtSettings->audioType = (T_U32)(atoi( optarg ));
		break;

	case 's':	
		mExtSettings->aSamplerate= (T_U32)(atoi( optarg ));
		break;

	case 'v':
		mExtSettings->video_types = (T_U32) (atoi(optarg));
		break;
	case 'c':
		mExtSettings->video_types2 = (T_U32) (atoi(optarg));
		break;
	case 'm':
		mExtSettings->mode = (T_U32) (atoi(optarg));
		break;
	case 'e':
		mExtSettings->width2 = (T_U32) (atoi(optarg));
		break;
	case 'f':
		mExtSettings->height2 = (T_U32) (atoi(optarg));
		break;
	case 'z':
		mExtSettings->times = (T_U32) (atoi(optarg));
		break;
	case 'l':
		mExtSettings->filetype = (T_U32) (atoi(optarg));
		break;
	case 'M':
		mExtSettings->qpMax = (T_U32) (atoi(optarg));
		break;
	case 'V':
		mExtSettings->qpMin = (T_U32) (atoi(optarg));
		break;
	default:
		break;
	}
}

static void Settings_Destroy( demo_setting *mSettings )
{
	assert( mSettings );
	DELETE_PTR( mSettings->strHelpString );
	DELETE_PTR( mSettings );
}

static void help( char * strHelpArg )
{
	if ( strHelpArg ) {
		printf( "--------------you ask: %s---------\n", strHelpArg );
		return;
	}

	printf( "\
		-p --- set record file path\n" );
	printf( "\
		-h --- set height of camera \n" );
	printf( "\
		-w --- set width of camera \n" );
	printf( "\
		-b --- set video bitrate (bps)\n" );
	printf( "\
		-M --- set video qp Max \n");
	printf( "\
		-V --- set video qp Min \n");
	printf( "\
		-m --- set mode 0=NULL, 1=zoom, 2=2channel, 3=occ \n" );

	printf( "\
		-e --- set width of channel2 \n" );
	printf( "\
		-f --- set height of channel2 \n" );
		
	printf( "\
		-z --- set zoom times in [1,3] \n" );
	printf( "\
		-q --- set quality of encode, the value must in [25,40]\n" );
	printf( "\
		-i --- set I frame quality of encode, the value must in [25,40]\n" );
	printf( "\
		-t --- set recode time (second),default is 300s\n" );
	printf( "\
		-v --- set channle1 video type: 0 is h264;1 is mjpeg,default is h264\n" );
	printf( "\
		-c --- set channel2 video type: 0 is h264;1 is mjpeg,default is h264\n" );
	printf( "\
		-a --- open audio and set audio type: 0 is pcm;1 adpcm_ima; 2 is aac\n" );
			
	printf( "\
		-s --- set audio samplerate,default is 8000 Hz\n" );
	printf( "\
		-l --set file type 0 is avi, 1 is mp4\n");
	printf( "\
		-r --set VBR/CBR, 0 is CBR, 1 is VBR\n");
	printf( "\
		-P --set H264 Profile, 0 is baseline, 1 is main, 2 is high profile\n");

}
