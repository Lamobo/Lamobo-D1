#pragma once
#include <deque>
#include <vector>
#include "interfaceclass.h"
#include "ThreadBase.h"
#include "AutoLock.h"
using namespace std;

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include "D3D9RenderImpl.h"

typedef struct RawData_st
{
	PBYTE pData;
	unsigned int nDataLen;
	struct timeval sPresentationTime;
	char * pstrMediumName;
	char * pstrCodec;
}RAWDATA;

typedef struct StreamParameter_st
{
	//video
	unsigned int nVideoHeight;
	unsigned int nVideoWidth;
	unsigned int nFPS;
	enum AVPixelFormat enPixelFormat;
	unsigned int nGopSize;/* emit one intra frame every nGopSize frames */
	unsigned int nMaxBFrames;

	//audio
	unsigned int nAudioChannels;
	unsigned int nAidioFmt;
	unsigned int nAudioSampleRate;

	//audio video
	unsigned int nBitsRate;
}STREAMPARAMETER;

typedef enum functionType_en
{
	FUN_DECODER,
	FUN_ENCODER
}FUCTIONTYPE;

enum SinkType{
	SINK_VIDEO,
	SINK_AUDIO
};

class CFfmpegEnvoy :
	public IDataSink, protected CThreadBase 
{
public:
	static CFfmpegEnvoy * createNew();

	virtual ~CFfmpegEnvoy(void);
	
	int OpenFfmpeg(bool bAsync = true, FUCTIONTYPE enFType = FUN_DECODER);

	int SetEncodeCodecID(enum AVCodecID enCodecID);
	
	virtual int SendData(PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime,
						  const char * pstrMediumName, const char * pstrCodec, const char * pstrConfig = NULL);
	
	int getStreamParameter(STREAMPARAMETER & stStreamParam);

	int setStreamParameter(const STREAMPARAMETER & stStreamParam);

	int RegisterSink(void * pISink, int sinktype); 

	int UnregisterSink(void * pISink, int sinktype);

	int Start();

protected:
	CFfmpegEnvoy(void);
	virtual void Run();

private:
	int FindStartCode(const char * pstrCodec, const char * pstrConfig, int frameLen, uint8_t * startCode, int & iCodeLen);

	int ProcessData(PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime,
					 const char * pstrMediumName, const char * pstrCodec, const char * pstrConfig = NULL);

	int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt);
	
	int Decode(RAWDATA * data = NULL);
	int Decode_video(RAWDATA * data);
	int Decode_audio(RAWDATA * data);
	int Decode_free();

	int Encode(RAWDATA * data = NULL);
	int Encode_audio(RAWDATA * data);
	int Encode_free();

	int FindCodecIdFromCodecName(char const * strCodec);

	int InformFrame(AVFrame *frame, PBYTE pExtendData, int nWidth, int nHeight, int nSize, char const * strMediumName, struct timeval sPresentationTime);
	
	int ClearData(RAWDATA * pstRawData);
	int ClearDataManage();

private:
	deque<RAWDATA *>		m_deDataManage;
	vector<IAudioSink *>	m_vecASink;
	vector<IVideoSink *>	m_vecVSink;
	STREAMPARAMETER			m_stCurStreamParameter;
	int						m_icurCodecID;
	bool					m_bAsync;
	FUCTIONTYPE				m_enFunType;
	CriticalSection			m_cs;
	int						m_numTruncatedBytes;

	HANDLE					m_hEvent[2];
	
	enum AVCodecID			m_EncodeCodecID;
	BYTE *					m_AEncodeSamples;
	int						m_nSamplesSize;
	int						m_nEncodeSampOffset;

	AVCodec *				m_codec;
	AVCodecContext *		m_context;
	AVCodecParserContext *	m_pc;
	bool					m_bIsFirst;
	struct SwrContext *		m_audioConv;
	AVFrame *				m_avframe;
};
