#include "StdAfx.h"
#include "FfmpegEnvoy.h"

#define MAX_START_CODE_LEN		1024

enum Event_en {
	EVENT_EXECUTE,
	EVENT_STOP,
	EVENT_CNT
};

CFfmpegEnvoy::CFfmpegEnvoy(void)
: m_bAsync( true ), m_icurCodecID( AV_CODEC_ID_NONE ), m_enFunType( FUN_DECODER ), m_numTruncatedBytes( -1 )
, m_EncodeCodecID(AV_CODEC_ID_NONE), m_AEncodeSamples(NULL), m_nSamplesSize(0), m_nEncodeSampOffset(0)
, m_codec( NULL ), m_context( NULL ), m_pc( NULL ), m_bIsFirst( true ), m_audioConv(NULL), m_avframe(NULL)
{
	memset( &m_stCurStreamParameter, 0, sizeof( STREAMPARAMETER ) );
}

CFfmpegEnvoy::~CFfmpegEnvoy(void)
{
	::SetEvent( m_hEvent[EVENT_STOP] );
	Join();
	
	if (m_bAsync) {
		for ( int i = 0; i < EVENT_CNT; ++i )
			::CloseHandle( m_hEvent[i] );
	}

	if ( m_enFunType == FUN_DECODER )
		Decode_free();
	else if ( m_enFunType == FUN_ENCODER )
		Encode_free();

	ClearDataManage();
}

CFfmpegEnvoy * CFfmpegEnvoy::createNew()
{
	return new CFfmpegEnvoy();
}

int CFfmpegEnvoy::OpenFfmpeg(bool bAsync, FUCTIONTYPE enFType)
{
	m_bAsync = bAsync;
	m_enFunType = enFType;
	
	m_numTruncatedBytes = -1;
	if ( m_bAsync ) {
		
		for ( int i = 0; i < EVENT_CNT; ++i )
			m_hEvent[i] = ::CreateEvent( NULL, FALSE, FALSE, NULL );

		//Start();
	}

	return 0;
}

int CFfmpegEnvoy::SetEncodeCodecID(enum AVCodecID enCodecID)
{
	m_EncodeCodecID = enCodecID;
	return 0;
}

int CFfmpegEnvoy::Start()
{
	if (!m_bAsync) return 0;

	return CThreadBase::Start();
}

int CFfmpegEnvoy::getStreamParameter( STREAMPARAMETER & stStreamParam )
{
	CAutoLock lock( &m_cs );

	stStreamParam = m_stCurStreamParameter;
	
	return 0;
}

int CFfmpegEnvoy::setStreamParameter( const STREAMPARAMETER & stStreamParam )
{
	CAutoLock lock( &m_cs );

	m_stCurStreamParameter = stStreamParam;
	
	return 0;
}

int CFfmpegEnvoy::RegisterSink( void * pISink, int sinktype )
{
	if (sinktype == SINK_VIDEO) m_vecVSink.push_back((IVideoSink*)pISink);
	else if (sinktype == SINK_AUDIO) m_vecASink.push_back((IAudioSink*)pISink);
	
	return 0;
}

int CFfmpegEnvoy::UnregisterSink( void * pISink, int sinktype )
{
	if (sinktype == SINK_VIDEO) {
		vector<IVideoSink *>::iterator iter = m_vecVSink.begin();
		for (; iter != m_vecVSink.end(); ++iter) {
			if (*iter == pISink) {
				iter = m_vecVSink.erase(iter);
				break;
			}
		}
	}
	
	if (sinktype == SINK_AUDIO) {
		vector<IAudioSink *>::iterator iter = m_vecASink.begin();
		for (; iter != m_vecASink.end(); ++iter) {
			if (*iter == pISink) {
				iter = m_vecASink.erase(iter);
				break;
			}
		}
	}

	return 0;
}

int CFfmpegEnvoy::SendData( PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime,
							const char * pstrMediumName, const char * pstrCodec, const char * pstrConfig )
{
	return ProcessData( pData, nDataLen, sPresentationTime, pstrMediumName, pstrCodec, pstrConfig );
}

int CFfmpegEnvoy::FindStartCode( const char * pstrCodec, const char * pstrConfig, int frameLen, uint8_t * startCode, int & iCodeLen )
{
	if ( strcmp( pstrCodec, "H264" ) == 0 || strcmp( pstrCodec, "h264" ) == 0 ) {//H264帧头部缺少01 00 00 00的头
		iCodeLen = 4;
		*((int *)startCode) = 0x1000000;
	}else if ( strcmp( pstrCodec, "MPEG4-GENERIC" ) == 0 || strcmp( pstrCodec, "mpeg4-generic" ) == 0 ){//aac缺少的头部数据根据pstrConfig重新组织。
		if ( NULL == pstrConfig ) return 0;
		iCodeLen = 7;
		frameLen += 7;

		char audioSpecificConfig[2];
		int iConfig = 0;
		sscanf(pstrConfig, "%x", &iConfig);
		audioSpecificConfig[0] = (iConfig >> 8) & 0x00FF;
		audioSpecificConfig[1] = iConfig & 0x00FF;

		memset( startCode, 0xFF, 2 );
		startCode[1] = startCode[1] & 0xF0 | 0x01; //protection_absent 1bits, allway be 1
		startCode[2] = (((audioSpecificConfig[0] >> 3) & 0x07) - 1) << 6; //profile 2bits
		startCode[2] |= ((((audioSpecificConfig[0] & 0x07) << 1) | ((audioSpecificConfig[1] >> 7) & 0x01)) << 2); //sampling_frequency_index 4bits
		startCode[2] |= (((audioSpecificConfig[1] >> 3) & 0x07) >> 2); //channel_configuration 3bits, 1bit in startCode[2]
		startCode[3] = (((audioSpecificConfig[1] >> 3) & 0x03) << 6) & 0xC0; //channel_configuration 3bits, 2bits in startCode[3]
		startCode[3] |= ((frameLen >> 11) & 0x03); //framelen 13bits, 2bits in startCode[3]
		startCode[4] = (frameLen >> 3) &0x00FF; //framelen 13bits, 8bits in startCode[4]
		startCode[5] = (((frameLen & 0x0007) << 5) & 0xE0);	//framelen 13bits, 3bits in startCode[5]
		startCode[5] |= 0x1F; //5bits 1
		startCode[6] = 0xFC; //FC
	}else {
		iCodeLen = 0;
	}

	return 0;
}

int CFfmpegEnvoy::ProcessData( PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime,
							   const char * pstrMediumName, const char * pstrCodec, const char * pstrConfig )
{
	uint8_t startCode[MAX_START_CODE_LEN];
	int iCodeLen = 0;
	
	{
		CAutoLock lock( &m_cs );
		if ( m_deDataManage.size() > 200) {
			fprintf(stderr, "WARN::decode/encode too slow! size = %d\n", m_deDataManage.size());
			return -1;
		}
	}

	RAWDATA * data = new RAWDATA;
	if ( NULL == data ) {
		fprintf(stderr, "can't alloc memory for decode/encode\n");
		return -1;
	}

	ZeroMemory(data, sizeof(RAWDATA));
	
	if (FUN_DECODER == m_enFunType) {//live555处理后得到的一帧编码帧，有可能会缺少部分头部固定数据，但可以通过live555中给出的信息，重新组织这些头部数据。
		FindStartCode( pstrCodec, pstrConfig, nDataLen, startCode, iCodeLen );
		nDataLen += iCodeLen;
	}

	data->pData = new BYTE[nDataLen];
	if ( NULL == data->pData ) {
		fprintf(stderr, "can't alloc memory for sava raw data\n");
		delete data;
		return -1;
	}
	
	if (FUN_DECODER == m_enFunType) {
		memcpy( data->pData, startCode, iCodeLen );
	}

	memcpy( data->pData + iCodeLen, pData, nDataLen - iCodeLen );
	data->nDataLen = nDataLen;
	
	if (FUN_DECODER == m_enFunType) {
		data->sPresentationTime = sPresentationTime;
		data->pstrCodec = new char[strlen(pstrCodec) + 1];
		if ( NULL == data->pstrCodec ) {
			fprintf(stderr, "can't alloc memory for save codec name\n");
			delete[] data->pData;
			delete data;
			return -1;
		}

		memcpy( data->pstrCodec, pstrCodec, strlen(pstrCodec) + 1 );
	}

	data->pstrMediumName = new char[strlen(pstrMediumName) + 1];
	if ( NULL == data->pstrMediumName ) {
		fprintf(stderr, "can't alloc memory for save medium name\n");
		delete[] data->pData;
		delete[] data->pstrCodec;
		delete data;
		return -1;
	}

	memcpy( data->pstrMediumName, pstrMediumName, strlen(pstrMediumName) + 1 );

	if ( NULL == data ) return -1;

	if ( m_bAsync ) {
		CAutoLock lock( &m_cs );
		m_deDataManage.push_back( data );

		::SetEvent( m_hEvent[EVENT_EXECUTE] );
		return 0;
	}
	
	int ret = 0;
	if (FUN_DECODER == m_enFunType) {
		ret = Decode( data );
	}else {
		ret = Encode( data );
	}
	
	if (data) {
		ClearData( data );
		delete data;
	}

	return ret;
}

void CFfmpegEnvoy::Run()
{
	int iIndex = 0;

	while( true ) {
		
		iIndex = WaitForMultipleObjects( EVENT_CNT, m_hEvent, FALSE, INFINITE );
		if ( iIndex - WAIT_OBJECT_0 == EVENT_EXECUTE ) {
			while ( !m_deDataManage.empty() ) {
				if ( m_enFunType == FUN_DECODER )
					Decode();
				else
					Encode();
			}
		}else if ( iIndex - WAIT_OBJECT_0 == EVENT_STOP ) {
			if ( m_enFunType == FUN_DECODER )
				Decode_free();
			else
				Encode_free();

			break;
		}else {
			//why? this is a windows error?
			fprintf(stderr, "don't know how! i hope this can auto fix by operating system, so continue again!\n");
		}
	}
}

int CFfmpegEnvoy::FindCodecIdFromCodecName( char const * strCodec )
{
	if ( strcmp( strCodec, "H264" ) == 0 || strcmp( strCodec, "h264" ) == 0 ) {
		return AV_CODEC_ID_H264;
	}else if ( strcmp( strCodec, "MPEG4-GENERIC" ) == 0 || strcmp( strCodec, "mpeg4-generic" ) == 0 ) {
		return AV_CODEC_ID_AAC;
	}else if ( strcmp( strCodec, "MPA" ) == 0 || strcmp( strCodec, "mpa" ) == 0 ){
		return AV_CODEC_ID_MP3;
	}else if ( strcmp( strCodec, "JPEG" ) == 0 || strcmp( strCodec, "jpeg" ) == 0 ){
		return AV_CODEC_ID_MJPEG;
	}else {
		return AV_CODEC_ID_NONE;
	}

	return AV_CODEC_ID_NONE;
}

#define LOCAL_ALOCK_BEGIN( cs ) { \
		CAutoLock lock( cs );
#define LOCAL_ALOCK_END	}

#define MAX_YUV_BUFFER	(1280 * 720 * 3)

int CFfmpegEnvoy::Decode( RAWDATA * data )
{
	BOOL bNeedFree = FALSE;
	int iRet = 0;
	if ( NULL == data ) {
		CAutoLock lock( &m_cs );
		if ( m_deDataManage.empty() )
			return 0;
		
		data = m_deDataManage.front();
		m_deDataManage.pop_front();
		bNeedFree = TRUE;
	}
	
	do {
		if ( AV_CODEC_ID_NONE == m_icurCodecID ) {
			m_icurCodecID = FindCodecIdFromCodecName( data->pstrCodec );

			if ( AV_CODEC_ID_NONE == m_icurCodecID ) {
				fprintf( stderr, "no support decode this stream! Codec = %s\n", data->pstrCodec );
				iRet = -1;
				break;
			}
			
			/* find the video decoder */
			m_codec = avcodec_find_decoder( (enum AVCodecID)m_icurCodecID );
			if ( !m_codec ) {
				fprintf(stderr, "Codec not found\n");
				iRet = -1;
				break;
			}

			m_context = avcodec_alloc_context3( m_codec );
			if ( !m_context ) {
				fprintf(stderr, "Could not allocate codec context\n");
				iRet = -1;
				break;
			}

			m_pc = av_parser_init( (enum AVCodecID)m_icurCodecID );
			if ( !m_pc ) {
				fprintf(stderr, "Could not allocate stream parser\n");
				iRet = -1;
				break;
			}
			
			if( m_codec->capabilities&CODEC_CAP_TRUNCATED )
				m_context->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

			/*  For some codecs, such as msmpeg4 and mpeg4, width and height
				MUST be initialized there because this information is not
				available in the bitstream. */
			if ((strcmp(data->pstrMediumName, "VIDEO") == 0) || (strcmp(data->pstrMediumName, "video") == 0)) {
				m_context->width = m_stCurStreamParameter.nVideoWidth;
				m_context->height = m_stCurStreamParameter.nVideoHeight;
			}

			/* open it */
			if (avcodec_open2( m_context, m_codec, NULL ) < 0) {
				fprintf(stderr, "Could not open codec\n");
				iRet = -1;
				break;
			}
		}

		if ((strcmp(data->pstrMediumName, "VIDEO") == 0) || (strcmp(data->pstrMediumName, "video") == 0)) {
			iRet = Decode_video(data);
		}else if ((strcmp(data->pstrMediumName, "AUDIO") == 0) || (strcmp(data->pstrMediumName, "audio") == 0)) {
			iRet = Decode_audio(data);
		}else {
			fprintf(stderr, "no support decode this medium type! %s\n", data->pstrMediumName);
		}

	}while( 0 );
	
	if (bNeedFree) {
		ClearData( data );
		delete data;
	}

	return iRet;
}

int CFfmpegEnvoy::Decode_video(RAWDATA * data)
{
	//int64_t pts = AV_NOPTS_VALUE, dts = AV_NOPTS_VALUE;
	int pos = 0, len = 0, iRet = 0;
	//uint8_t * pout = NULL;
	int got_frame = 0;

	AVPacket avpkt;
	av_init_packet(&avpkt);

	if (!m_avframe) {
		if (!(m_avframe = avcodec_alloc_frame())) {
				fprintf(stderr, "Could not allocate audio frame\n");
				return -1;
		}
	} else {
		avcodec_get_frame_defaults(m_avframe);
	}

	if (AV_CODEC_ID_MJPEG == m_icurCodecID) {
		if (m_numTruncatedBytes == -1) {
			unsigned int i = 0, j = 0;
			for(; i < data->nDataLen; ++i) {
				if ((data->pData[i] == 0xFF) && (data->pData[i + 1] == 0xD8) && 
					(data->pData[i + 2] == 0xFF) && (data->pData[i + 3] == 0xE0)) {
					++j;
				}

				if (j > 1) break;
			}
			
			if ((j == 1) && (i == data->nDataLen)) i = 0;

			m_numTruncatedBytes = i;
		}

		avpkt.data = data->pData + m_numTruncatedBytes;
		avpkt.size = data->nDataLen - m_numTruncatedBytes;

		len = avcodec_decode_video2(m_context, m_avframe, &got_frame, &avpkt);
		if (len < 0) {
			fprintf(stderr, "Error while decoding frame\n");
			return -1;
		}

		if ( got_frame ) {
			LOCAL_ALOCK_BEGIN( &m_cs )
			m_stCurStreamParameter.nVideoHeight = m_avframe->height;
			m_stCurStreamParameter.nVideoWidth = m_avframe->width;
			m_stCurStreamParameter.nAidioFmt = m_context->sample_fmt;
			m_stCurStreamParameter.nAudioChannels = m_context->channels;
			m_stCurStreamParameter.nAudioSampleRate = m_context->sample_rate;
			if (m_context->time_base.den)
				m_stCurStreamParameter.nFPS = m_context->time_base.num / m_context->time_base.den;
			m_stCurStreamParameter.nBitsRate = m_context->bit_rate;
			LOCAL_ALOCK_END

			InformFrame(m_avframe, NULL, 0, 0, 0, data->pstrMediumName, data->sPresentationTime);
		}
		
		return 0;
	}

	if (data->nDataLen == 9 && (data->pData[4] & 0x1F) == 5 ) { //无效I帧
		return 0;
	}

	//do {
		//len = av_parser_parse2(m_pc, m_context, &avpkt.data, &avpkt.size, (uint8_t *)data->pData, 
							//data->nDataLen - pos, pts, dts, AV_NOPTS_VALUE);
		//pos += len;

		avpkt.size = data->nDataLen;
		avpkt.data = data->pData;

		while ( avpkt.size > 0 ) {
			len = avcodec_decode_video2(m_context, m_avframe, &got_frame, &avpkt);
			if (len < 0) {
				if ((data->pData[4] & 0x1F) != 7 && (data->pData[4] & 0x1F) != 8) {//fps/pps
					fprintf(stderr, "Error while decoding frame\n");
					return 0;
				}
				//iRet = -1;
				//break;
				return -1;
			}

			if ( got_frame ) {
				LOCAL_ALOCK_BEGIN( &m_cs )
				m_stCurStreamParameter.nVideoHeight = m_avframe->height;
				m_stCurStreamParameter.nVideoWidth = m_avframe->width;
				m_stCurStreamParameter.nAidioFmt = m_context->sample_fmt;
				m_stCurStreamParameter.nAudioChannels = m_context->channels;
				m_stCurStreamParameter.nAudioSampleRate = m_context->sample_rate;
				m_stCurStreamParameter.nFPS = m_context->time_base.den / m_context->time_base.num;
				m_stCurStreamParameter.nBitsRate = m_context->bit_rate;
				LOCAL_ALOCK_END

				InformFrame(m_avframe, NULL, 0, 0, 0, data->pstrMediumName, data->sPresentationTime);
			}

			//if (iRet < 0) break;

			avpkt.size -= len;
			avpkt.data += len;
		}

	//}while( pos < data->nDataLen );

	return iRet;
}

int CFfmpegEnvoy::Decode_audio(RAWDATA * data)
{
	AVPacket avpkt;
	uint8_t * pcmbuf = NULL;
    av_init_packet(&avpkt);
	
	avpkt.data = data->pData;
	avpkt.size = data->nDataLen;
	
	int got_frame = 0, iRet = 0, len = 0;
	unsigned int pos = 0;
	int64_t pts = AV_NOPTS_VALUE, dts = AV_NOPTS_VALUE;

    while (pos < data->nDataLen) {
		len = av_parser_parse2(m_pc, m_context, &avpkt.data, &avpkt.size, (uint8_t *)data->pData, 
						data->nDataLen - pos, pts, dts, AV_NOPTS_VALUE);

		pos += len;

        if (!m_avframe) {
            if (!(m_avframe = avcodec_alloc_frame())) {
                fprintf(stderr, "Could not allocate audio frame\n");
                iRet = -1;
				break;
            }
        } else
            avcodec_get_frame_defaults(m_avframe);
		
		while ( avpkt.size > 0 ) {
			len = avcodec_decode_audio4(m_context, m_avframe, &got_frame, &avpkt);
			if (len < 0) {
				fprintf(stderr, "Error while decoding audio\n");
				iRet = -1;
				break;
			}

			if (got_frame) {
				/* if a frame has been decoded, output it */
				int data_size = av_samples_get_buffer_size(NULL, m_context->channels,
														   m_avframe->nb_samples,
														   m_context->sample_fmt, 1);

				LOCAL_ALOCK_BEGIN( &m_cs )
				m_stCurStreamParameter.nVideoHeight = m_avframe->height;
				m_stCurStreamParameter.nVideoWidth = m_avframe->width;
				m_stCurStreamParameter.nAidioFmt = m_context->sample_fmt;
				m_stCurStreamParameter.nAudioChannels = m_context->channels;
				m_stCurStreamParameter.nAudioSampleRate = m_context->sample_rate;
				m_stCurStreamParameter.nFPS = m_context->time_base.den / m_context->time_base.num;
				m_stCurStreamParameter.nBitsRate = m_context->bit_rate;
				LOCAL_ALOCK_END
				
				if (!m_audioConv) {
					int layout = 0;

					if (m_context->channels < 2) 
						layout = AV_CH_LAYOUT_MONO;
					else
						layout = AV_CH_LAYOUT_STEREO;

					m_audioConv = swr_alloc_set_opts(NULL, layout, AV_SAMPLE_FMT_S16, m_context->sample_rate, m_context->channel_layout, 
												m_context->sample_fmt, m_context->sample_rate, 0, NULL);

					swr_init(m_audioConv);
				}
				
				int bufsize = m_context->sample_rate * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
				pcmbuf = new uint8_t[bufsize];
				uint8_t * out[] = { pcmbuf };
				int len = swr_convert(m_audioConv, out, bufsize / av_get_bytes_per_sample(AV_SAMPLE_FMT_S16),
					(const uint8_t **)m_avframe->data, m_avframe->nb_samples);
				
				if (m_context->channels < 2) 
					len = len * 1 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
				else
					len = len * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
				
				/*
				int bufsize = m_context->sample_rate * m_context->channels * av_get_bytes_per_sample(m_context->sample_fmt);
				pcmbuf = new uint8_t[bufsize];
				
				int step = av_get_bytes_per_sample(m_context->sample_fmt);
				int j = 0, k = 0;
				for (int i = 0; i < m_avframe->linesize[0]; i += step) {
					for (k = 0; k < step; ++k) pcmbuf[j++] = *(m_avframe->data[0] + i + k);
					for (k = 0; k < step; ++k) pcmbuf[j++] = *(m_avframe->data[1] + i + k);
				}
				*/

				InformFrame(m_avframe, pcmbuf, 0, 0, len, data->pstrMediumName, data->sPresentationTime);
				delete[] pcmbuf;
			}
			
			if (iRet < 0) break;

			avpkt.size -= len;
			avpkt.data += len;
			avpkt.dts = avpkt.pts = AV_NOPTS_VALUE;
		}
    }

	return iRet;
}

int CFfmpegEnvoy::InformFrame(AVFrame *frame, PBYTE pExtendData, int nWidth, int nHeight, int nSize, char const * strMediumName, struct timeval sPresentationTime)
{
	if ((strcmp(strMediumName, "VIDEO") == 0) || (strcmp(strMediumName, "video") == 0)) {
		vector<IVideoSink *>::iterator iter = m_vecVSink.begin();
		for ( ; iter != m_vecVSink.end(); ++iter ) {
			(*iter)->SendVideo(frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1],
				frame->data[2], frame->linesize[2], sPresentationTime, frame->height, frame->width);
		}
	}

	if ((strcmp(strMediumName, "AUDIO") == 0) || (strcmp(strMediumName, "audio") == 0)) {
		unsigned int nChannels = 0, nBitsPerSample = 0;
		//we just support mono or stereo
		if (m_context->channels < 2)	nChannels = 1;
		else							nChannels = 2;
		
		//only 16 bits
		nBitsPerSample = 16;

		vector<IAudioSink *>::iterator iter = m_vecASink.begin();
		for ( ; iter != m_vecASink.end(); ++iter ) {
			((IAudioSink *)(*iter))->SendAudio(pExtendData, nSize, sPresentationTime, nChannels, m_context->sample_rate, nBitsPerSample);
		}
	}

	return 0;
}

int CFfmpegEnvoy::Decode_free()
{
	if ( NULL == m_context )
		return 0;

	AVPacket avpkt;
	int got_frame = 0;

	av_init_packet(&avpkt);

	avcodec_get_frame_defaults(m_avframe);

	/* some codecs, such as MPEG, transmit the I and P frame with a
       latency of one frame. You must do the following to have a
       chance to get the last frame of the video */
    avpkt.data = NULL;
    avpkt.size = 0;
    avcodec_decode_video2( m_context, m_avframe, &got_frame, &avpkt );

	av_parser_close(m_pc);

	avcodec_close( m_context );
	av_free( m_context );
	m_context = NULL;
	
	swr_free(&m_audioConv);
	avcodec_free_frame( &m_avframe );

	return 0;
}

/* check that a given sample format is supported by the encoder */
int CFfmpegEnvoy::check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

int CFfmpegEnvoy::Encode(  RAWDATA * data )
{
	BOOL bNeedFree = FALSE;
	int iRet = 0;

	if ( NULL == data ) {
		CAutoLock lock( &m_cs );
		if ( m_deDataManage.empty() )
			return 0;

		data = m_deDataManage.front();
		m_deDataManage.pop_front();
		bNeedFree = TRUE;
	}
	
	do {
		if ( AV_CODEC_ID_NONE == m_icurCodecID ) {
			if ((m_EncodeCodecID > CODEC_ID_FFMETADATA) || (m_EncodeCodecID <= AV_CODEC_ID_NONE)) {
				iRet = -1;
				break;
			}

			m_codec = avcodec_find_encoder(m_EncodeCodecID);
			if (!m_codec) {
				fprintf(stderr, "can't find the encode codec, codec id = %d!\n", m_EncodeCodecID);
				iRet = -1;
				break;
			}

			m_context = avcodec_alloc_context3( m_codec );
			if ( !m_context ) {
				fprintf(stderr, "Could not allocate codec context\n");
				iRet = -1;
				break;
			}

			m_context->bit_rate = m_stCurStreamParameter.nBitsRate;
			//audio
			if (m_stCurStreamParameter.nAidioFmt == 8)
				m_context->sample_fmt = AV_SAMPLE_FMT_U8;
			else if (m_stCurStreamParameter.nAidioFmt == 16)
				m_context->sample_fmt = AV_SAMPLE_FMT_S16;
			else if (m_stCurStreamParameter.nAidioFmt == 32)
				m_context->sample_fmt = AV_SAMPLE_FMT_S32;
			else {
				fprintf(stderr, "current no support audio sample fmt %d!\n", m_stCurStreamParameter.nAidioFmt);
				iRet = -1;
				break;
			}
			
			if (!check_sample_fmt(m_codec, m_context->sample_fmt)) {
				fprintf(stderr, "Encoder does not support sample format %s",
					av_get_sample_fmt_name(m_context->sample_fmt));
				iRet = -1;
				break;
			}

			m_context->sample_rate = m_stCurStreamParameter.nAudioSampleRate;
			m_context->channels = m_stCurStreamParameter.nAudioChannels;
			m_context->channel_layout = AV_CH_LAYOUT_MONO;

			//video
			m_context->width = m_stCurStreamParameter.nVideoWidth;
			m_context->height = m_stCurStreamParameter.nVideoHeight;
			m_context->time_base.den = m_stCurStreamParameter.nFPS;
			m_context->time_base.num = 1;
			m_context->gop_size = m_stCurStreamParameter.nGopSize;
			m_context->max_b_frames = m_stCurStreamParameter.nMaxBFrames;
			m_context->pix_fmt = m_stCurStreamParameter.enPixelFormat;

			/* open it */
			if (avcodec_open2(m_context, m_codec, NULL) < 0) {
				fprintf(stderr, "Could not open codec\n");
				exit(1);
			}

			m_icurCodecID = m_EncodeCodecID;

			if (m_AEncodeSamples) delete[] m_AEncodeSamples;
			m_AEncodeSamples = NULL;

			m_nSamplesSize = 0;
			m_nEncodeSampOffset = 0;
		}

		if ((strcmp(data->pstrMediumName, "AUDIO") == 0) || (strcmp(data->pstrMediumName, "audio") == 0)) {
			iRet = Encode_audio(data);
		}else {
			fprintf(stderr, "now no support encode this medium type! %s\n", data->pstrMediumName);
		}

	}while(0);
	
	if (bNeedFree) {
		ClearData( data );
		delete data;
	}

	return iRet;
}

int CFfmpegEnvoy::Encode_audio(RAWDATA * data)
{
	/* frame containing input raw audio */
	if (!m_avframe) {
		if (!(m_avframe = avcodec_alloc_frame())) {
				fprintf(stderr, "Could not allocate audio frame\n");
				return -1;
		}

		m_avframe->nb_samples     = m_context->frame_size;
		m_avframe->format         = m_context->sample_fmt;
		m_avframe->channel_layout = m_context->channel_layout;
	}
	
	if (!m_AEncodeSamples) {
		/* the codec gives us the frame size, in samples,
		 * we calculate the size of the samples buffer in bytes */
		m_nSamplesSize = av_samples_get_buffer_size(NULL, m_context->channels, m_context->frame_size,
												 m_context->sample_fmt, 0);

		m_AEncodeSamples = (BYTE *)av_malloc(m_nSamplesSize);
		if (NULL == m_AEncodeSamples) {
			fprintf(stderr, "%s::%s::%s::out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
	}

    int ret = avcodec_fill_audio_frame(m_avframe, m_context->channels, m_context->sample_fmt,
										(const uint8_t*)m_AEncodeSamples, m_nSamplesSize, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not setup audio frame\n");
		return -1;
	}

	PBYTE pos = data->pData;
	int len = (int)data->nDataLen;
	int got_output = 0;
	AVPacket pkt;

	while (len) {
		if ((m_nSamplesSize - m_nEncodeSampOffset) >= len) {
			memcpy(m_AEncodeSamples + m_nEncodeSampOffset, pos, len);
			pos += len;
			m_nEncodeSampOffset += len;
			len = 0;
		}else {
			memcpy(m_AEncodeSamples + m_nEncodeSampOffset, pos, (m_nSamplesSize - m_nEncodeSampOffset));
			len -= (m_nSamplesSize - m_nEncodeSampOffset);
			pos += (m_nSamplesSize - m_nEncodeSampOffset);
			m_nEncodeSampOffset = m_nSamplesSize;
		}

		if (m_nEncodeSampOffset == m_nSamplesSize) {
			av_init_packet(&pkt);
			pkt.data = NULL; // packet data will be allocated by the encoder
			pkt.size = 0;
			m_nEncodeSampOffset = 0;
			/* encode the samples */
			ret = avcodec_encode_audio2(m_context, &pkt, m_avframe, &got_output);
			if (ret < 0) {
				fprintf(stderr, "Error encoding audio frame\n");
				return -1;
			}
			
			if (got_output) {
				timeval ptime = {0};
				InformFrame(NULL, pkt.data, 0, 0, pkt.size, data->pstrMediumName, ptime);
				av_free_packet(&pkt);
			}
		}
	}

	return 0;
}

int CFfmpegEnvoy::Encode_free()
{
	/* get the delayed frames */
	AVPacket pkt;
	int got_output = 0, i = 0, ret = 0;

	if (m_context == NULL) return 0;

    for (got_output = 1; got_output; i++) {
		av_init_packet(&pkt);
		pkt.data = NULL; // packet data will be allocated by the encoder
		pkt.size = 0;

        ret = avcodec_encode_audio2(m_context, &pkt, NULL, &got_output);
        if (ret < 0) {
			fprintf(stderr, "%s::%s::Error encoding frame\n", __FILE__, __FUNCTION__);
            break;
        }

        if (got_output) {
			timeval ptime = {0};
			if (m_AEncodeSamples)
				InformFrame(NULL, pkt.data, 0, 0, pkt.size, "audio", ptime);
            av_free_packet(&pkt);
        }
    }

	if (m_AEncodeSamples) av_freep(&m_AEncodeSamples);
	m_AEncodeSamples = NULL;

	avcodec_free_frame(&m_avframe);
	avcodec_close(m_context);
    av_free(m_context);

	m_context = NULL;

	return 0;
}

int CFfmpegEnvoy::ClearData( RAWDATA * pstRawData )
{
	if (!pstRawData) return 0;

	if ( pstRawData->pData )
		delete[] pstRawData->pData;
	pstRawData->pData = NULL;

	if ( pstRawData->pstrMediumName )
		delete[] pstRawData->pstrMediumName;
	pstRawData->pstrMediumName = NULL;

	if ( pstRawData->pstrCodec )
		delete[] pstRawData->pstrCodec;
	pstRawData->pstrCodec = NULL;
	
	return 0;
}

int CFfmpegEnvoy::ClearDataManage()
{
	CAutoLock lock( &m_cs );

	if ( m_deDataManage.empty() )
		return 0;

	deque<RAWDATA *>::iterator iter = m_deDataManage.begin();
	for ( ; iter != m_deDataManage.end(); ++iter ) {
		ClearData( *iter );
		delete (*iter);
	}

	m_deDataManage.clear();

	return 0;
}
