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

#include "anyka_types.h"
#include "akuio.h"
#include "camera.h"
#include "encode.h"
#include "muxer.h"
#include "headers.h"
#include "audio.h"
#include "log.h"
#include "Tool.h"
#include "video_process.h"
#include "akIPCNetControl.h"
#include "led.h"
#include "DemuxForLive555.h"
#include "PTZControl.h"
#include "photograph.h"

#include <execinfo.h>
#include <AKRTSPServer.hh>

#include <BasicUsageEnvironment.hh>
#include <AKIPCH264OnDemandMediaSubsession.hh>
#include <AKIPCH264FramedSource.hh>
#include <AKIPCMJPEGOnDemandMediaSubsession.hh>
#include <AKIPCMJPEGFramedSource.hh>
#include <AKIPCAACAudioOnDemandMediaSubsession.hh>
UsageEnvironment* env;
Boolean reuseFirstSource = False;
Boolean iFramesOnly = False;
#define RTSPPORT 554
static int bHasAudio = 0;
#define DELETE_PTR( ptr )                       \
  do {                                          \
    if ( ptr != NULL ) {                        \
      free( ptr );                              \
      ptr = NULL;                               \
    }                                           \
  } while( 0 )



extern init_parse parse;
//init the demo setting struct
static void Settings_Initialize( demo_setting *main );
#if 0
//destroy the demo setting struct
//static void Settings_Destroy( demo_setting *mSettings );
static void startFTPSrv()
{
	printf("start ftp server\n");
	system("tcpsvd 0 21 ftpd /mnt &");
}
#endif
static void appExit()
{
	printf("##appExit\n");
	//if (bHasAudio)
	{
		audio_stop();
		audio_close();
		printf("audio_close\n");
	}
	close_encode();
	audio_dec_exit();
	video_process_stop();
	usleep(200 * 1000);
	camera_close();
	printf("camera_close\n");
	encode_close();
	printf("encode_close\n");
	mux_exit();
	printf("mux_close\n");

	encode_destroy();
	akuio_pmem_fini();
	setled_off();
	PTZControlDeinit();
	printf("akuio_pmem_fini\n");
	
    record_rename_file();

}
static void sigprocess(int sig)
{
	int ii = 0;
	void *tracePtrs[16];
	int count = backtrace(tracePtrs, 16);
	char **funcNames = backtrace_symbols(tracePtrs, count);
	for(ii = 0; ii < count; ii++)
		printf("%s\n", funcNames[ii]);
	free(funcNames);
	fflush(stderr);
	printf("##signal %d caught\n", sig);
	fflush(stdout);
	
	if(sig == SIGINT || sig == SIGSEGV || sig == SIGTERM)
	{
		appExit();	
	}

	exit(1);
}

static int sig_init(void)
{
	signal(SIGSEGV, sigprocess);
	signal(SIGINT, sigprocess);
	signal(SIGTERM, sigprocess);
	return 0;
}

static void getDeviceID(char* strDeviceID)
{
	FILE* f = fopen("/sys/kernel/serial_number/sn", "r");
	if(f == NULL)
	{
		printf("read device id error\n");
		return ;	
	}
	fseek(f, 10, SEEK_SET);
	fread(strDeviceID, 6, 1, f);
	fclose(f);
	f = NULL;
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

T_MUX_INPUT mux_input;
int main( int argc, char **argv )
{
	//int ret = 0;
	PTZControlInit();
	demo_setting * ext_gSettings = NULL;
	
	// Allocate the "global" settings
	ext_gSettings = (demo_setting*)malloc( sizeof( demo_setting ) );
	if ( NULL == ext_gSettings ) {
		printf( "main::out of memory!\n" );
		return -1;
	}
	
	sig_init();
    atexit(appExit);
	//init the setting struct
	Settings_Initialize( ext_gSettings );

	read_Parse(ext_gSettings);
	//printf("video type = %d \n", ext_gSettings->video_types);
	//...do your job

	//close the led
	setled_off();
	//init dma memory
	akuio_pmem_init();
	encode_init();
	printf("encode_init ok\n");
	//open camera
	camera_open(ext_gSettings->width, ext_gSettings->height);
	printf("camera_open ok\n");

	//encode_open
	T_ENC_INPUT encInput;
	encInput.width = ext_gSettings->width;			//实际编码图像的宽度，能被4整除
	encInput.height = ext_gSettings->height;			//实际编码图像的长度，能被2整除
	encInput.kbpsmode = ext_gSettings->kbpsmode; 
	encInput.qpHdr = ext_gSettings->qpHdr;			//初始的QP的值
	encInput.iqpHdr = ext_gSettings->iqpHdr;			//初始的QP的值
	encInput.bitPerSecond = ext_gSettings->bitPerSecond;	//目标bps
	encInput.minQp = ext_gSettings->minQp;
	encInput.maxQp = ext_gSettings->maxQp;
	encInput.framePerSecond = ext_gSettings->framePerSecond;
	encInput.video_tytes = ext_gSettings->video_types;
	encode_open(&encInput);
	printf("encode_open ok\n");

	//set mux
	mux_input.rec_path = ext_gSettings->rec_path;
	mux_input.m_MediaRecType = MEDIALIB_REC_AVI_NORMAL;

	if (ext_gSettings->bhasAudio)
	{
		bHasAudio = 1;
		//mux_input.m_bCaptureAudio = 1;
	}
	else
	{
		bHasAudio = 0;
		//mux_input.m_bCaptureAudio = 0;
	}
	mux_input.m_bCaptureAudio = 1;
	//mux video
	if(parse.format2 == 0)
	{
		mux_input.m_eVideoType = MEDIALIB_VIDEO_H264;
	}
	else if(parse.format2 == 1)
	{
		mux_input.m_eVideoType = MEDIALIB_VIDEO_MJPEG;
	}
	mux_input.m_nWidth = parse.width2;
	mux_input.m_nHeight = parse.height2;
	
	//mux audio
	mux_input.m_eAudioType = MEDIALIB_AUDIO_AAC;
	mux_input.m_nSampleRate = 8000;
	//mux_input.abitsrate = ext_gSettings->abitsrate;

	printf("mux_open ok\n");

	//if (ext_gSettings->bhasAudio)
	{
		T_AUDIO_INPUT audioInput;
		audioInput.enc_type = (AUDIO_ENCODE_TYPE_CC)ext_gSettings->audioType;
		audioInput.nBitsRate = ext_gSettings->abitsrate;
		audioInput.nBitsPerSample = 16;
		audioInput.nChannels = 1;
		audioInput.nSampleRate = ext_gSettings->aSamplerate;
		audio_open(&audioInput);
		printf("audio_open ok\n");
		audio_start();
	}

	//start ftp server
	//startFTPSrv();

	Init_photograph();
	//PTZControlInit();
	//start video process
	video_process_start();
	InitMotionDetect();
	DemuxForLiveSetCallBack();
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
	// To implement client access control to the RTSP server, do the following:
	authDB = new UserAuthenticationDatabase;
	authDB->addUserRecord("username1", "password1"); // replace these with real strings
	// Repeat the above with each <username>, <password> that you wish to allow
	// access to the server.
#endif
       
	// Create the RTSP server:
	RTSPServer* rtspServer = AKRTSPServer::createNew(*env, RTSPPORT, authDB);
	if (rtspServer == NULL) 
	{
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		appExit();
		exit(1);
	}

	char const* descriptionString = "Session streamed by \"testOnDemandRTSPServer\"";

	// Set up each of the possible streams that can be served by the
	// RTSP server.  Each such stream is implemented using a
	// "ServerMediaSession" object, plus one or more
	// "ServerMediaSubsession" objects for each audio/video substream.

	int vsIndex = 0;
	VIDEO_MODE vm[2] = {VIDEO_MODE_VGA,VIDEO_MODE_VGA};
	const char* streamName1 = "vs1";
	const char* streamName2 = "vs2";
	((AKRTSPServer*)rtspServer)->SetStreamName(streamName1, streamName2);	
	
	if(ext_gSettings->video_types == 1)
	{
		if(ext_gSettings->width == 640)
		{
			vm[0] = VIDEO_MODE_VGA;
		}
		else if(ext_gSettings->width == 320)
		{
			vm[0] = VIDEO_MODE_QVGA;
		}
		else if(ext_gSettings->width == 720)
		{
			vm[0] = VIDEO_MODE_D1;
		}
		
		AKIPCMJPEGFramedSource* ipcMJPEGSourcecam = NULL;
		ServerMediaSession* smsMJPEGcam = ServerMediaSession::createNew(*env, streamName1, 0, descriptionString);
		AKIPCMJPEGOnDemandMediaSubsession* subsMJPEGcam = AKIPCMJPEGOnDemandMediaSubsession::createNew(*env,ipcMJPEGSourcecam, ext_gSettings->width, ext_gSettings->height, vsIndex);
		smsMJPEGcam->addSubsession(subsMJPEGcam); 
		subsMJPEGcam->getframefunc = video_process_get_buf;
		subsMJPEGcam->setledstart = setled_view_start;
		subsMJPEGcam->setledexit = setled_view_stop;
		
		if(bHasAudio)
			smsMJPEGcam->addSubsession(AKIPCAACAudioOnDemandMediaSubsession::createNew(*env,True,getAACBuf, vsIndex));

		rtspServer->addServerMediaSession(smsMJPEGcam);
		char* url1 = rtspServer->rtspURL(smsMJPEGcam);
		*env << "using url \"" << url1 <<"\"\n";
		delete[] url1;
	}
	else if(ext_gSettings->video_types == 0)
	{
		if(ext_gSettings->width == 1280)
		{
			vm[0] = VIDEO_MODE_720P;
		}
		else if(ext_gSettings->width == 640)
		{
			vm[0] = VIDEO_MODE_VGA;
		}
		else if(ext_gSettings->width == 320)
		{
			vm[0] = VIDEO_MODE_QVGA;
		}
		else if(ext_gSettings->width == 720)
		{
			vm[0] = VIDEO_MODE_D1;
		}
		
		AKIPCH264FramedSource* ipcSourcecam = NULL;
		ServerMediaSession* smscam = ServerMediaSession::createNew(*env, streamName1, 0, descriptionString);
		AKIPCH264OnDemandMediaSubsession* subscam = AKIPCH264OnDemandMediaSubsession::createNew(*env,ipcSourcecam, 0, vsIndex);
		smscam->addSubsession(subscam);
		if(bHasAudio)
			smscam->addSubsession(AKIPCAACAudioOnDemandMediaSubsession::createNew(*env,True,getAACBuf, vsIndex));
	
		subscam->getframefunc = video_process_get_buf;
		subscam->setledstart = setled_view_start;
		subscam->setledexit = setled_view_stop;

		rtspServer->addServerMediaSession(smscam);
		char* url1 = rtspServer->rtspURL(smscam);
		*env << "using url \"" << url1 <<"\"\n";
		delete[] url1;
	}

	vsIndex = 1;
	
	if(parse.format2 == 0)//264
	{
		if(parse.width2 == 1280)
		{
			vm[1] = VIDEO_MODE_720P;
		}
		else if(parse.width2 == 640)
		{
			vm[1] = VIDEO_MODE_VGA;
		}
		else if(parse.width2 == 320)
		{
			vm[1] = VIDEO_MODE_QVGA;
		}
		else if(parse.width2 == 720)
		{
			vm[1] = VIDEO_MODE_D1;
		}
		
		AKIPCH264FramedSource* ipcSourcecam = NULL;
		ServerMediaSession* smscam = ServerMediaSession::createNew(*env, streamName2, 0, descriptionString);
		AKIPCH264OnDemandMediaSubsession* subscam = AKIPCH264OnDemandMediaSubsession::createNew(*env,ipcSourcecam, 0, vsIndex);
		smscam->addSubsession(subscam);
		if(bHasAudio)
			smscam->addSubsession(AKIPCAACAudioOnDemandMediaSubsession::createNew(*env,True,getAACBuf, vsIndex));
	
		subscam->getframefunc = video_process_get_buf;
		subscam->setledstart = setled_view_start;
		subscam->setledexit = setled_view_stop;

		rtspServer->addServerMediaSession(smscam);
		char* url2 = rtspServer->rtspURL(smscam);
		*env << "using url \"" << url2 <<"\"\n";
		delete[] url2;
	}
	else if(parse.format2 == 1)//mjpeg
	{
		if(parse.width2 == 640)
		{
			vm[1] = VIDEO_MODE_VGA;
		}
		else if(parse.width2 == 320)
		{
			vm[1] = VIDEO_MODE_QVGA;
		}
		else if(parse.width2 == 720)
		{
			vm[1] = VIDEO_MODE_D1;
		}
		
		AKIPCMJPEGFramedSource* ipcMJPEGSourcecam = NULL;
		ServerMediaSession* smsMJPEGcam = ServerMediaSession::createNew(*env, streamName2, 0, descriptionString);
		AKIPCMJPEGOnDemandMediaSubsession* subsMJPEGcam = AKIPCMJPEGOnDemandMediaSubsession::createNew(*env,ipcMJPEGSourcecam, parse.width2, parse.height2, vsIndex);
		smsMJPEGcam->addSubsession(subsMJPEGcam); 
		subsMJPEGcam->getframefunc = video_process_get_buf;
		subsMJPEGcam->setledstart = setled_view_start;
		subsMJPEGcam->setledexit = setled_view_stop;
		
		if(bHasAudio)
			smsMJPEGcam->addSubsession(AKIPCAACAudioOnDemandMediaSubsession::createNew(*env,True,getAACBuf, vsIndex));

		rtspServer->addServerMediaSession(smsMJPEGcam);
		char* url2 = rtspServer->rtspURL(smsMJPEGcam);
		*env << "using url \"" << url2 <<"\"\n";
		delete[] url2;
	}
#if 0
	if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) 
	{
		*env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
	}
	else 
	{
		*env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
	}
#endif

	//printf("streamName:%s,Port:%d\n", streamName1, RTSPPORT);
	
	
	NetCtlSrvPar ncsp;
	memset(&ncsp, 0, sizeof(ncsp));
	getDeviceID(ncsp.strDeviceID);
	printf("device id:**%s**\n", ncsp.strDeviceID);
	strcpy(ncsp.strStreamName1, streamName1);
	strcpy(ncsp.strStreamName2, streamName2);
	ncsp.vm1 = vm[0];
	ncsp.vm2 = vm[1];
	ncsp.nRtspPort = RTSPPORT;
	ncsp.nMainFps = parse.fps1;
	ncsp.nSubFps = parse.fps2;
	//start net command server
	startNetCtlServer(&ncsp);

    printf("[##]start record...\n");
    auto_record_file();
    printf("[##]auto_record_file() called..\n");

	//at last,start rtsp loop
	env->taskScheduler().doEventLoop(); // does not return

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
	memset( main, 0, sizeof(demo_setting) );
	main->width		= 1280;			//实际编码图像的宽度，能被4整除
	main->height		= 720;			//实际编码图像的长度，能被2整除 
	main->kbpsmode = 0;//默认静态， 动态设置为1
	main->qpHdr		= 30;			//初始的QP的值
	main->iqpHdr		= 30;			//初始的iQP的值
	main->minQp = 25;
	main->maxQp = 40;
	main->framePerSecond = 30;
	main->bitPerSecond	= 1000* 400;		//目标bps
	main->enc_time		= 10;
	main->audioType		= ENC_TYPE_AAC;
	main->aSamplerate	= 8000;
	main->video_types   	= 0;
	main->bhasAudio = 1;
}

#if 0
static void Settings_Destroy( demo_setting *mSettings )
{
	assert( mSettings );
	DELETE_PTR( mSettings->strHelpString );
	DELETE_PTR( mSettings );
}
#endif
