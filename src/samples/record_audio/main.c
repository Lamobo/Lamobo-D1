#include "headers.h"
#include <fcntl.h>

#include "AkAudioDemo.h"
#include "log.h"
#include "gnu_getopt.h"

#define LOG_TAG "Linux_AudioDemo"

#define DELETE_PTR( ptr )                       \
  do {                                          \
    if ( ptr != NULL ) {                        \
      free( ptr );                              \
      ptr = NULL;                               \
    }                                           \
  } while( AK_FALSE )

typedef struct EncTypeSupportSR {
	DEMO_ENCODE_TYPE	enEncType;
	const T_U32 *		pcSR;
}EncTypeSupportSR;

//support audio sample rate list
static const T_U32 support_sample_rate[] = { 
						8000, 11025, 16000, 22050, 32000, 44100, 48000, 0 };

//wav support sample rata list
static const T_U32 wav_support_sr[] = {
						8000, 11025, 16000, 22050, 32000, 44100, 0 };

//amr support sample rate list
static const T_U32 amr_support_sr[] = {8000, 0};

//g711 support sample rate list
static const T_U32 g711_support_sr[] = {8000, 0};


//encode type support is sample rate list 
static const EncTypeSupportSR SupportSR[] = {
		{ D_ENC_TYPE_WAV_PCM, wav_support_sr },
		{ D_ENC_TYPE_AMR, amr_support_sr },
		{ D_ENC_TYPE_AAC, support_sample_rate },
		{ D_ENC_TYPE_WAV_ADPCM, wav_support_sr },
		{ D_ENC_TYPE_G711, g711_support_sr }
};

static const T_S32 SupportSRLen = ( sizeof(SupportSR) / sizeof(EncTypeSupportSR) );


typedef struct EncType2Str
{
	DEMO_ENCODE_TYPE	enEncType;
	T_pCSTR				pcStrs;
}EncType2Str;

//demo support audio encode type list
static const EncType2Str encType2Str[] = { 
	{ D_ENC_TYPE_WAV_PCM, "WAV" }, 
	{ D_ENC_TYPE_WAV_PCM, "wav" },
	{ D_ENC_TYPE_AMR, "AMR" }, 
	{ D_ENC_TYPE_AMR, "amr" },
	{ D_ENC_TYPE_AAC, "AAC" }, 
	{ D_ENC_TYPE_AAC, "aac" },
};

static const T_S32 encType2StrLen = ( sizeof(encType2Str) / sizeof(EncType2Str) );

#define LONG_OPTIONS()
const struct option long_options[] =
{
{"bitrate",  		  required_argument, NULL, 'b'},	//audio bits rate
{"encode_type",  	  required_argument, NULL, 'e'},	//audio encode type
{"help",  	  		  no_argument, NULL, 'h'},	        //printf the help document
{"line_in",	  		  no_argument, NULL, 'l'},	        //set audio input device is line
{"mic_in",	  		  no_argument, NULL, 'm'},	        //set audio input device is mic
{"record_path",		  required_argument, NULL, 'r'},	//set the recorde file save in which directory
{"sample_rate",		  required_argument, NULL, 's'},	//set audio sample rate
{"recode_time", 	  required_argument, NULL, 't'},	//set recode time
{"volume",			  required_argument, NULL, 'v'},	//set volume
{0, 0, 0, 0}
};


#define SHORT_OPTIONS()
const char short_options[] = "b:e:hlmr:s:t:v:";

int g_MachType = MACH_UNKNOW;

//init the demo setting struct
static T_VOID Settings_Initialize( demo_setting *main );

// parse settings from app's command line
static T_VOID Settings_ParseCommandLine( T_S32 argc, T_CHR **argv, 
												 demo_setting *mSettings );

// parse settings from app's command line
static T_VOID Settings_Interpret( T_CHR option, const T_CHR *optarg, 
								   demo_setting *mExtSettings );

//check input bit rate is legality ?
static T_VOID CheckBitsRate( demo_setting *mExtSettings );

//check sample rate is legality ?
static T_VOID CheckSampleRate( demo_setting *mExtSettings );

//check mach type, ak98 or ak37
//static T_VOID CheckMach();

//destory the setting struct
static T_VOID Settings_Destroy( demo_setting *mSettings );

//printf the help document
static T_VOID help();
#define 	EFAULTPATH 	"/root/testaudio/"

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

demo_name g_name;

T_S32 main( T_S32 argc, T_CHR **argv )
{
	T_S32 ret = 0;
	
	demo_setting * ext_gSettings = NULL;
	
	log_init();

	// Allocate the "global" settings
	ext_gSettings = (demo_setting*)malloc( sizeof( demo_setting ) );
	if ( NULL == ext_gSettings ) {
		loge( "main::out of memory!\n" );
		return -1;
	}
	
	Settings_Initialize( ext_gSettings );

	// read settings from command-line parameters
    Settings_ParseCommandLine( argc, argv, ext_gSettings );
	
	ret = OpenDemo( ext_gSettings );
	if ( ret < 0 ) {
		loge( "main::can't not open the record demo!\n" );
		goto End;
	}

	ret = RunDemo();
	if ( ret < 0 ) {
		loge( "main::RunDemo error!\n" );
	}
	
End:
	Settings_Destroy( ext_gSettings );

	logi( "exit the demo!\n" );
	return ret;
}

/**
* @brief  init the demo setting struct
* 
* @author hankejia
* @date 2012-07-05
* @param[out] main  setting struct pointer
* @return NONE
*/
static T_VOID Settings_Initialize( demo_setting *main ) 
{
	// Everything defaults to zero or NULL with
    // this memset. Only need to set non-zero values
    // below.
    memset( main, 0, sizeof(demo_setting) );
	main->nSampleRate 	= 8000;          		//default 8000 
	main->enEncType		= D_ENC_TYPE_AAC;		//default AAC
	main->nChannels		= 1;                    //only support mono channel
	main->strRecordPath = EFAULTPATH;
	main->enInDev 		= D_IN_DEV_MIC;         //default MIC
	main->time			= 10;                   //default 10s
	main->volume		= 100;                  //default max volume

	memset(&g_name, 0x00, sizeof(g_name));
	sprintf(g_name.src, "m");
	sprintf(g_name.format, "aac");
	sprintf(g_name.samplerate, "8000");
	sprintf(g_name.volume, "16");
	printf("default file name = %s_%s_%s_%s \n", g_name.src,g_name.format,g_name.samplerate, g_name.volume);
	
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
static T_VOID Settings_ParseCommandLine( T_S32 argc, T_CHR **argv, demo_setting *mSettings )
{
    T_S32 option = 0;
    while ( (option =
             gnu_getopt_long( argc, argv, short_options,
                              long_options, NULL )) != EOF ) {
        Settings_Interpret( option, gnu_optarg, mSettings );
    }

	for ( int i = gnu_optind; i < argc; i++ ) {
        logi( "%s: ignoring extra argument -- %s\n", argv[0], argv[i] );
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
static T_VOID Settings_Interpret( T_CHR option, const T_CHR *optarg, 
								   demo_setting *mExtSettings )
{
	T_U32 type;
	assert( mExtSettings );
	
	switch( option ) {
	case 'b':
		mExtSettings->nOutBitsRate = (T_U32)(atof( optarg ) * 1000.0);
		CheckBitsRate( mExtSettings );
		break;

	case 'e':
		/*
		for ( nIndex = 0; nIndex < encType2StrLen; ++nIndex ) {
			if ( strcmp( encType2Str[nIndex].pcStrs, optarg ) == 0 ) {
				mExtSettings->enEncType = encType2Str[nIndex].enEncType;
				break;
			}
		}
		*/
		type = (T_U32) (atoi( optarg));
		
		memset(g_name.format, 0x00, 6);

		switch (type)
		{
		default:
		case 0:
			sprintf(g_name.format, "pcm");
			mExtSettings->enEncType = D_ENC_TYPE_WAV_PCM;
			break;

		case 1:
			sprintf(g_name.format, "adpcm");
			mExtSettings->enEncType = D_ENC_TYPE_WAV_ADPCM;
			break;

		case 2:
			sprintf(g_name.format, "aac");
			mExtSettings->enEncType = D_ENC_TYPE_AAC;
			break;

		case 3:
			sprintf(g_name.format, "amr");
			mExtSettings->enEncType = D_ENC_TYPE_AMR;
			printf("D_ENC_TYPE_AMR\n");
			break;

		case 4:
			sprintf(g_name.format, "g711");
			mExtSettings->enEncType = D_ENC_TYPE_G711;
			break;

		}
		
		CheckBitsRate( mExtSettings );
		CheckSampleRate( mExtSettings );
		break;

	case 'h':
		help();
		Settings_Destroy( mExtSettings );
		exit(0);

	case 'l':
		mExtSettings->enInDev = D_IN_DEV_LINE;
		sprintf(g_name.src, "l");
		break;

	case 'm':
		mExtSettings->enInDev = D_IN_DEV_MIC;
		sprintf(g_name.src, "m");
		break;

	case 'r':
		mExtSettings->strRecordPath = (T_pSTR)malloc( ( strlen(optarg) + 2 ) * sizeof( char ) );
		bzero( mExtSettings->strRecordPath, ( strlen(optarg) + 2 ) * sizeof( char ) );
		strcpy( mExtSettings->strRecordPath, optarg );

		if ( mExtSettings->strRecordPath[strlen(optarg) - 1] != '/' ) {
			mExtSettings->strRecordPath[strlen(optarg)] = '/';
		}
		break;

	case 's':
		mExtSettings->nSampleRate = (T_U32)(atoi( optarg ));
		CheckSampleRate( mExtSettings );
		memset(g_name.samplerate, 0x00, 6);
		sprintf(g_name.samplerate, "%s", optarg);
		break;
	case 't':
		mExtSettings->time = (T_U32) (atoi( optarg));
		break;
	case 'v':
		memset(g_name.volume, 0x00, 3);
		sprintf(g_name.volume, "%s", optarg);
		mExtSettings->volume = (T_U32) (atoi( optarg));
		if(mExtSettings->volume < 0 )
			mExtSettings->volume = 0;
		if(mExtSettings->volume > 16 )
			mExtSettings->volume = 16;
		mExtSettings->volume = mExtSettings->volume * 6;
		break;
	default:
		break;
	}
}

/**
* @brief check input bit rate is legality ?
* 
* @author hankejia
* @date 2012-07-05
* @param[in] mExtSettings  	setting struct pointer.
* @return NONE
*/
static T_VOID CheckBitsRate( demo_setting *mExtSettings )
{
	assert( mExtSettings );

	if ( mExtSettings->nOutBitsRate == 0 ) {
		printf("bits rate is zero!!!\n\n");
		//return;
	}
	
	if ( D_ENC_TYPE_WAV_PCM == mExtSettings->enEncType ||
		D_ENC_TYPE_AAC == mExtSettings->enEncType ) {
		logw( "wav and aac no need to set bits rate!\n" );
		mExtSettings->nOutBitsRate = 0;
	}else if ( D_ENC_TYPE_AMR == mExtSettings->enEncType ) {
		switch ( mExtSettings->nOutBitsRate ) {
		case 12200: case 10200: case 7950: case 7400:
		case 6700 : case 5900 : case 5150: case 4750:
		//	break;
		default:
			logw( "amr can't support the %d bits rate! use 10.2kbps instead\n", 
				  mExtSettings->nOutBitsRate );
			mExtSettings->nOutBitsRate = 10200;
			break;
		}
	}else {
		logw( "need set encode type! -b\n" );
	}
}

/**
* @brief check sample rate is legality ?
* 
* @author hankejia
* @date 2012-07-05
* @param[in] mExtSettings  	setting struct pointer.
* @return NONE
*/
static T_VOID CheckSampleRate( demo_setting *mExtSettings )
{
	T_U32 nIndex = 0, nIndex1 = 0;
	
	assert( mExtSettings );

	for( nIndex = 0; nIndex < SupportSRLen; ++nIndex ) {
		if ( mExtSettings->enEncType == SupportSR[nIndex].enEncType ) {
			break;
		}
	}

	if ( nIndex == SupportSRLen ) {
		logw( "need set encode type! -b\n" );
		return;
	}
	
	if ( SupportSR[nIndex].pcSR == NULL ) {
		loge( "sample rate table error!\n" );
		return;
	}

	for ( nIndex1 = 0; SupportSR[nIndex].pcSR[nIndex1] != 0; ++nIndex1 ) {
		if ( SupportSR[nIndex].pcSR[nIndex1] == mExtSettings->nSampleRate ) {
			break;
		}
	}

	if ( SupportSR[nIndex].pcSR[nIndex1] == 0 ) {
		logw( "not support the %d sample rate, use 8KHz instead\n", mExtSettings->nSampleRate );
		mExtSettings->nSampleRate = 8000;
	}
}

#if 0
/**
* @brief check mach type, ak98 or ak37
* 
* @author hankejia
* @date 2012-07-05
* @param none
* @return NONE
*/
static T_VOID CheckMach()
{	
	T_CHR buf[256];
	T_S32 fd, len;
	bzero( buf, 256 );

	system( "uname -r > /tmp/machinfo" );
	fd = open( "/tmp/machinfo", O_RDONLY );
	
	read(fd, buf, 256);
	close(fd);
	
	system("rm -rf /tmp/machinfo");

	len = strlen(buf);
	
	if( strncmp( buf + len - strlen("ak98") - 1, "ak98", 4 ) == 0 ) {
		logi("mach ak98.....\n");
		g_MachType = MACH_AK98;
	} else if( strncmp( buf + len - strlen("ak37") - 1, "ak37", 4 ) == 0 ) {
		logi("mach ak37.....\n");
		g_MachType = MACH_AK37;
	} else {
		logi("unknow mach.....\n");
		g_MachType = MACH_UNKNOW;
	}
		
};
#endif

/**
* @brief destory the setting struct
* 
* @author hankejia
* @date 2012-07-05
* @param[in] mExtSettings  	setting struct pointer.
* @return NONE
*/
static T_VOID Settings_Destroy( demo_setting *mSettings )
{
	assert( mSettings );
//	DELETE_PTR( mSettings->strRecordPath );
	DELETE_PTR( mSettings );
}

/**
* @brief printf the help document
* 
* @author hankejia
* @date 2012-07-05
* @param[in] mExtSettings  	setting struct pointer.
* @return NONE
*/
static T_VOID help()
{
	printf( "Command: \n \
		-e --- set the encode type support set [pcm, adpcm, AAC, AMR, G711] 0=pcm 1= adpcm 2=aac 3=amr 4=g711\n \
		-s --- set the audio sample rate in [8000, 11025, 16000, 22050, 32000, 44100]\n \
		-m --- set audio in device is mic\n \
		-l --- set audio in device is line\n \
		-r --- set the record file path\n\
		-t --- set audio record time\n\
		-v --- set audio volume (0-16)\n\
		-h --- help!\n \
		eg:\n \
		audio_recorder -e 2 -s 8000 -m -r /mnt/ -t 10 -v 5\n" );
}
