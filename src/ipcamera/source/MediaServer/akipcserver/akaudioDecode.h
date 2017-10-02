#ifndef __AKAUDIODECODE_h__
#define __AKAUDIODECODE_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct media_info
{
	int m_AudioType;
	int m_nSamplesPerSec;
	int m_nChannels;
	int m_wBitsPerSample;
}media_info;

int decode_open(media_info *media);
int GetFillSize();
int FillAudioData( char * buf, int size);
int audioDecode(char *buf, int size);
int decode_close( void );



#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif