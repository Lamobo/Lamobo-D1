#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "akaudioDecode.h"
#include "sdcodec.h"
#include "Mutex.h"

#define 	BUFLENG 2048


T_VOID		*pAudioCodec = NULL;
static 		T_AUDIO_DECODE_OUT audio_output;
char		*obuf = NULL;
T_AUDIO_BUFFER_CONTROL 			bufctrl;
T_AUDIO_BUF_STATE 				bufstate;
pthread_mutex_t 				muxDecode;

T_BOOL lnx_delay(T_U32 ticks)
{
    return (usleep (ticks*5000) == 0);
}


int decode_open(media_info *media)
{
	T_AUDIO_DECODE_INPUT audio_input;

        memset(&audio_input, 0, sizeof(T_AUDIO_DECODE_INPUT));
        audio_input.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE) free;
        audio_input.cb_fun.Malloc =  (MEDIALIB_CALLBACK_FUN_MALLOC) malloc;
        audio_input.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF) printf;
        audio_input.cb_fun.delay = lnx_delay;

        audio_input.m_info.m_Type = media->m_AudioType;
		 if (_SD_MEDIA_TYPE_AAC == audio_input.m_info.m_Type)
        {
            //no use setting now
            //may be use in the future.
            printf("aac \n");
            audio_input.m_info.m_Private.m_aac.cmmb_adts_flag = 0;
        }
		audio_input.m_info.m_InbufLen = 7*1024;
        audio_input.m_info.m_SampleRate = media->m_nSamplesPerSec;
        audio_input.m_info.m_Channels = media->m_nChannels;
        audio_input.m_info.m_BitsPerSample = media->m_wBitsPerSample;

	//	  audio_input.m_info.m_SampleRate = 8000;
    //    audio_input.m_info.m_Channels = 1;
    //    audio_input.m_info.m_BitsPerSample = 16;
		
		printf("Samplerate = %d Channels = %d Bits = %d \n", 
				media->m_nSamplesPerSec, media->m_nChannels, media->m_wBitsPerSample);
        //must do this
        //audio_input.m_info.m_szData = media_info->m_szData;
        //audio_input.m_info.m_szDataLen = media_info->m_cbSize;


        memset(&audio_output, 0, sizeof(T_AUDIO_DECODE_OUT));

        pAudioCodec = _SD_Decode_Open(&audio_input, &audio_output);

		_SD_SetBufferMode(pAudioCodec, _SD_BM_ENDING);

        if (pAudioCodec == AK_NULL)
        {
            printf("audio decode init failed\n");
            return -1;
        }
		obuf =(char *) malloc(BUFLENG);
		if(NULL == obuf)
		{
			printf("malloc mem err \n");
			return -1;
		}

		Mutex_Initialize(&muxDecode);
		
        return 0;
}

int decode_close( void )
{
	Mutex_Lock(&muxDecode);
	if(NULL != pAudioCodec)
	{
		_SD_Buffer_Clear(pAudioCodec);
		_SD_Decode_Close(pAudioCodec);
		pAudioCodec = NULL;
	}
	Mutex_Unlock(&muxDecode);
	if(NULL != obuf)
	{
		free(obuf);
	}
	Mutex_Destroy(&muxDecode);
	return 0;
}

int  audioDecode(char *buf, int size)
{
    // printf("Audio Decoder thread run.\n");
     //decode

     T_S32 nSize = 0;
 //    int count = 0;
	 Mutex_Lock(&muxDecode); 
     if(pAudioCodec == NULL)
     {
     	return 0;
     }
     // compute the buffer address and length
     audio_output.m_pBuffer = (T_U8 *)obuf;
     audio_output.m_ulSize = BUFLENG;
             
     //decode
	
     nSize  = _SD_Decode(pAudioCodec, &audio_output);
	Mutex_Unlock(&muxDecode); 
	if(nSize <= 0)
		return nSize;
	
	if(nSize > 0)
	{
		memcpy(buf, obuf, nSize);
		return nSize;
	}
	
	return 0;
}

int GetFillSize()
{
	bufstate  = _SD_Buffer_Check(pAudioCodec ,&bufctrl);
	if(_SD_BUFFER_FULL	==	bufstate )
	{
		return 0;
	}
	else if(_SD_BUFFER_WRITABLE == bufstate)
	{	
		//printf("Fill size =%d \n", bufctrl.free_len);	
		return bufctrl.free_len;
	}
	else
	{	
		//printf("!!!!4\n");
		return bufctrl.free_len + bufctrl.start_len;
	}	
	
}

int FillAudioData( char * buf, int size)
{
	if(_SD_BUFFER_WRITABLE == bufstate)
	{
		
		memcpy(bufctrl.pwrite, buf,  size);
		
		_SD_Buffer_Update(pAudioCodec, size);
	}
	else
	{
		if(size <= bufctrl.free_len)
		{
			memcpy(bufctrl.pwrite, buf, size);
			_SD_Buffer_Update(pAudioCodec, size);
		}
		else if(size > bufctrl.free_len)
		{
			int leng = size - bufctrl.free_len;
			memcpy(bufctrl.pwrite, buf, bufctrl.free_len);
			_SD_Buffer_Update(pAudioCodec, bufctrl.free_len);
			memcpy(bufctrl.pstart, buf+bufctrl.free_len, leng);
			_SD_Buffer_Update(pAudioCodec, leng);
		}
	}
	
	return 0;
}

