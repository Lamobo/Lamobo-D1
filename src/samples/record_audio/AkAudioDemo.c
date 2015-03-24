#include <execinfo.h>
#include <stdlib.h>
#include "AkAudioDemo.h"
#include "AkAudioRecorder.h"
#include "sdfilter.h"
#include "Condition.h"
#include "log.h"
#include "headers.h"

#define DEFAULT_AUDIO_CHANNLES 			2		//default audio channles
#define DEFAULT_BITS_PER_SAMPLE			16		//default bits per sample
#define FILE_NAME_LEN					1024	//file name len
#define FILE_NAME_LEN_DOUBLE			2048	//file name len * 2
#define PLAYER_MEDIALIB_COMM			"player_codeclib -n "	//the player demo
#define AK98KBD_DEV_NAME 				"/dev/input/event1"		//keyboard
#define AK37KBD_DEV_NAME 				"/dev/input/event2"		//keyboard

extern int g_MachType;

//signal call back function type
typedef void Sigfunc(int);
typedef Sigfunc *SigfuncPtr;

static  T_U32	g_time;

//signal init process function
static SigfuncPtr my_signal( T_S32 inSigno, SigfuncPtr inFunc ) ;

//signal call back function
static void Sig_Interupt( int inSigno );

//static AkAudioRecorder * 	g_pstRecorder = NULL;
static T_pVOID				g_pRecordHandle = NULL;

//static Event				g_enEvent = EVENT_COUNT;
static Condition			g_EventCondition;

/**
* @brief   close the demo
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
//static T_S32 CloseDemo();

/**
* @brief  call back function, receive the keyborad listen module is inform msg
* @author hankejia
* @date 2012-07-05
* @param[in] eventMsg  	the msg come from keyboard listen module or net send ctrl module
* @param[in] param  		the other param, set by register
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
//static T_S32 EventCallBack( Event event, T_S32 param );


void dump(int signo)
{	
	void *array[10];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	printf("Obtained %zd stack frames\n", size);

	for(i = 0; i<size; i++)
		printf("%s\n", strings[i]);

	free(strings);
	exit(0);
}


/**
* @brief   open the recorder demo
* @author hankejia
* @date 2012-07-05
* @param[in] Setting		the point to the demo_setting struct, 
*						decide the program execute which function.
* @return T_S32
* @retval if return 0, open success, otherwise set failed 
*/
T_S32 OpenDemo( demo_setting * Setting )
{
	T_U32 filterType;
	AUDIO_REC_TYPE enRecType = AUDIO_REC_TYPE_COUNT;
	
	assert( Setting );

	//demo listen signal
	my_signal( SIGTERM, Sig_Interupt );
	my_signal( SIGINT, 	Sig_Interupt );
	my_signal( SIGALRM, Sig_Interupt );	
	my_signal(SIGPIPE, dump);
	my_signal(SIGSEGV, dump);
	Condition_Initialize( &g_EventCondition );


	if ( OpenRecorder( &g_pRecordHandle ) < 0 ) {
		loge( "OpenDemo::can't open the recorder!\n" );
		return -1;
	}

	if(Setting->enInDev == D_IN_DEV_LINE)
	{
		SetAudioLineInVolume(g_pRecordHandle, Setting->volume);
	}
	else
	{
		if( SetAudioMicVolume(g_pRecordHandle, Setting->volume))
		{
			loge( "OpenDemo::Can't set mic volume \n");
		}
	}

	switch ( Setting->enEncType )
	{
	case D_ENC_TYPE_WAV_PCM:
		enRecType = AUDIO_REC_TYPE_WAV_PCM;
		break;
	case D_ENC_TYPE_WAV_ADPCM:
		enRecType = AUDIO_REC_TYPE_ADPCM_IMA;
		break;
	case D_ENC_TYPE_AMR:
		enRecType = AUDIO_REC_TYPE_AMR;
		break;
	case D_ENC_TYPE_AAC:
		enRecType = AUDIO_REC_TYPE_AAC;
		break;
	case D_ENC_TYPE_G711:
		enRecType = AUDIO_REC_TYPE_G711_ULOW;
		break;
	default:
		enRecType = AUDIO_REC_TYPE_WAV_PCM;
		break;
	}

	if ( SetAudioInDev( g_pRecordHandle, 
				Setting->enInDev == D_IN_DEV_LINE ? IN_DEV_LINE: IN_DEV_MIC ) < 0 ) {
		loge( "OpenDemo::can't set audio in device!\n" );
		return -1;
	}

#if defined(AUDIO_NR)
	filterType = _SD_FILTER_DENOICE;
#elif defined(AUDIO_AGC)
	filterType = _SD_FILTER_AGC;
#else
	filterType = _SD_FILTER_UNKNOWN;
#endif

	if (0 > SetAudioParams(g_pRecordHandle, Setting->nChannels, 
			Setting->nSampleRate, DEFAULT_BITS_PER_SAMPLE, filterType)) {
		loge( "OpenDemo::can't set audio params!\n" );
		return -1;
	}
	
	if ( SetEncodeParams( g_pRecordHandle, Setting->nOutBitsRate, 
										 enRecType ) < 0 ) {
		loge( "OpenDemo::can't set encode params!\n" );
		return -1;
	}

	if ( SetRecordPath( g_pRecordHandle, 
										Setting->strRecordPath ) < 0 ) {
		loge( "OpenDemo::can't set record path!\n" );
		return -1;
	}
	g_time = Setting->time;
	return 0;
}

/**
* @brief   listen the message send from sub module,and process the message.
*		demo program main thread loop in this function. 
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 RunDemo()
{
	T_S32 ret = 0;


	logi( "start record-----!\n" );
	ret = StartRec( g_pRecordHandle );
	if ( ret < 0 ) 
	{
		loge( "can't start the record!\n" );
				
	}
	sleep(g_time);
	ret = StopRec( g_pRecordHandle );
	if ( ret < 0 ) 
	{
		loge( "can't stop the record!\n" );
	
	}
#if 0	
	while ( AK_FALSE )
	{
		Condition_Wait( &g_EventCondition );

		if ( g_enEvent == EVENT_START_STOP_RECORD ) {
			Condition_Unlock( g_EventCondition );
			if ( IsRunning( g_pRecordHandle ) ) {
				logi( "stop record--------!\n" );
				ret = StopRec( g_pRecordHandle );
				if ( ret < 0 ) {
					loge( "can't stop the record!\n" );
					continue;
				}
			}
			else{
				logi( "start record-----!\n" );
				ret = StartRec( g_pRecordHandle );
				if ( ret < 0 ) {
					loge( "can't start the record!\n" );
					continue;
				}
			}	
		} else if ( g_enEvent == EVENT_EXIT ) {
			Condition_Unlock( g_EventCondition );
			ret = CloseDemo();
			if ( ret < 0 ) {
				loge( "RunDemo::CloseDemo error!\n" );
			}

			return ret;
		} else if ( g_enEvent == EVENT_START_STOP_PLAYBACK ) {
			T_pSTR pstrFileName = NULL;
			T_pSTR pstrOld 		= NULL;
			T_U32 nFileNameLen = FILE_NAME_LEN;

			Condition_Unlock( g_EventCondition );

			if ( IsRunning( g_pRecordHandle ) ) {
				logi( "current record process is running, can't playback!\n" );
				continue;
			}

			do {
				pstrOld = pstrFileName;
				pstrFileName = (T_pSTR )realloc( pstrFileName, nFileNameLen );
				if ( NULL == pstrFileName ) {
					loge( "RunDemo::Out of memory!\n" );
					free( pstrOld );
					pstrOld = NULL;
					ret = -1;
					break;
				}

				ret = GetRecordFile( g_pRecordHandle, pstrFileName, &nFileNameLen );
				if ( 1 == ret ) {
					nFileNameLen += nFileNameLen;
				}
			} while ( ret == 1 );

			if ( ret == 0 ) {
				T_CHR aCommand[FILE_NAME_LEN_DOUBLE];
				bzero( aCommand, sizeof( aCommand ) );
				
				logi( "start playback-----!\n" );
				sprintf( aCommand, "%s %s", PLAYER_MEDIALIB_COMM, pstrFileName );
				system( aCommand );
			}

			if ( pstrFileName )
				free( pstrFileName );
		
		}else {
			Condition_Unlock( g_EventCondition );
			continue;
		}
	}
#endif
	return 0;
}

/**
* @brief  call back function, receive the keyborad listen module is inform msg
* @author hankejia
* @date 2012-07-05
* @param[in] eventMsg  	the msg come from keyboard listen module or net send ctrl module
* @param[in] param  		the other param, set by register
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
#if 0
static T_S32 EventCallBack( Event event, T_S32 param )
{
	Condition_Lock( g_EventCondition );
	g_enEvent = event;
	Condition_Unlock( g_EventCondition );
	Condition_Signal( &g_EventCondition );
	return 0;
}
#endif
#if 0
/**
* @brief   close the demo
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
static T_S32 CloseDemo()
{
	T_U32 ret = 0;

//	if ( g_pstRecorder )
{
		if ( g_pRecordHandle ) {
			ret = StopRec( g_pRecordHandle );
			if ( ret < 0 ) {
				loge( "CloseDemo::can't stop the record thread!\n" );
				return -1;
			}
			
			ret = CloseRecorder( g_pRecordHandle );
			if ( ret < 0 ) {
				loge( "CloseDemo::Close recorder error!\n" );
				return -1;
			}
			g_pRecordHandle = NULL;
		}

//		free( g_pstRecorder );
		//g_pstRecorder = NULL;
	}
#if 0
	ret = UnRegisterSink( g_nSinkFunRegNum );
	if ( ret < 0 ) {
		loge( "CloseDemo::UnRegister sink error!\n" );
		return -1;
	}

	ret = StopListenKeyBoard();
	if ( ret < 0 ) {
		loge( "CloseDemo::stop key board listen thread error!\n" );
		return -1;
	}
	
	ret = CloseKeyBoardDev();
	if ( ret < 0 ) {
		loge( "CloseDemo::close key board dev error!\n" );
		return -1;
	}
	
	ret = UninitKeyEventHandlerModule();
	if ( ret < 0 ) {
		loge( "CloseDemo::Uninit key event handler module error!\n" );
		return -1;
	}
#endif

	Condition_Destroy( &g_EventCondition );

	return 0;
}

#endif
/* -------------------------------------------------------------------
 * my_signal
 *
 * installs a signal handler, and returns the old handler.
 * This emulates the semi-standard signal() function in a
 * standard way using the Posix sigaction function.
 *
 * from Stevens, 1998, section 5.8
 * input param:
 * inSigno: signal number
 * inFunc: callback func
 * return value:
 * sinfnc ptr
 */
static SigfuncPtr my_signal( T_S32 inSigno, SigfuncPtr inFunc ) 
{
    struct sigaction theNewAction, theOldAction;

    assert( inFunc != NULL );

    theNewAction.sa_handler = inFunc;
    sigemptyset( &theNewAction.sa_mask );
    theNewAction.sa_flags = 0;

    if ( inSigno == SIGALRM ) {
#ifdef SA_INTERRUPT
        theNewAction.sa_flags |= SA_INTERRUPT;  /* SunOS 4.x */
#endif
    } else {
#ifdef SA_RESTART
        theNewAction.sa_flags |= SA_RESTART;    /* SVR4, 4.4BSD */
#endif
    }

    if ( sigaction( inSigno, &theNewAction, &theOldAction ) < 0 ) {
        return SIG_ERR;
    } else {
        return theOldAction.sa_handler;
    }
} /* end my_signal */


/* 
 * Signal handler sets the sInterupted flag, so the object can
 * respond appropriately.. [static]
 * input param:
 * inSigno: signal number
 * return value:
 * None 
 */
static void Sig_Interupt( int inSigno )
{
	logi( "need to exit the Demo!\n" );
	Condition_Lock( g_EventCondition );
	//g_enEvent = EVENT_EXIT;
	Condition_Unlock( g_EventCondition );
	Condition_Signal( &g_EventCondition );
}
