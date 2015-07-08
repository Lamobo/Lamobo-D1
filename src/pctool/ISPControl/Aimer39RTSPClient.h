#pragma once

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <string>
#include <vector>

#include "ThreadBase.h"
#include "AutoLock.h"
#include "InterfaceClass.h"
#include "Clock.h"

using namespace std;
class CAimer39RTSPClient;

typedef enum StreamType_en {
	STREAM_UNKNOWN = -1,
	STREAM_AUDIO = 0,
	STREAM_VIDEO,
	STREAM_TYPE_CNT
}STREAM_TYPE, *PSTREAM_TYPE;

class StreamClientState {
public:
  StreamClientState();
  virtual ~StreamClientState();

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:
class ourRTSPClient: public RTSPClient {
public:
  static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);

protected:
  ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
  virtual ~ourRTSPClient();

public:
  StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.
class DummySink: public MediaSink
{
public:
	static DummySink* createNew(UsageEnvironment& env,
				  MediaSubsession& subsession, // identifies the kind of data that's being received
				  CAimer39RTSPClient * AimerClient,
				  char const* streamId = NULL); // identifies the stream itself (optional)

	void play();

	void pause();

private:
	DummySink(UsageEnvironment& env, MediaSubsession& subsession, CAimer39RTSPClient * AimerClient, char const* streamId);
	// called only by "createNew()"
	virtual ~DummySink();

	static void afterGettingFrame(void* clientData, unsigned frameSize,
								unsigned numTruncatedBytes,
								struct timeval presentationTime,
								unsigned durationInMicroseconds);

	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

	void calcNPTandInform();

private:
	// redefined virtual functions:
	virtual Boolean continuePlaying();

private:
	u_int8_t* fReceiveBuffer;
	MediaSubsession& fSubsession;
	CAimer39RTSPClient * fAimerClient;
	double fLassNPT;
	char* fStreamId;
	Boolean fIsPause;
	struct timeval fSyncTime;
	u_int32_t fSyncTimestamp;
};

typedef unsigned char	BYTE;
typedef unsigned char *	PBYTE;
typedef void (__stdcall *CLIENTCALLBACK)(void * pLParam, void * pRParam);

typedef void (__stdcall *STARTPLAYCB)(double duration, double startDuration, void * pLParam, void * pRParam);

typedef void (__stdcall *PLAYNPTCB)(double sNPT, const char * mediumName, void * pLParam, void * pRParam);

class CAimer39RTSPClient : protected CThreadBase
{
public:
	static CAimer39RTSPClient * CreateNew();
	
	virtual ~CAimer39RTSPClient(void);
	
	uint64_t GetID();

	/*
	open the rtsp url
	eg: rtsp://192.168.x.x/WebCamera720p
	*/
	int OpenURL( const char * strURL );

	int Close();

	int GetURL(string & strURL);
	
	/*
	Is our client connect server success
	@ret if return 0 success, otherwise the error code identifier, hint why failed to connect server.
	you can use GetLastError to receive the error string message.
	*/
	int IsPrepare( bool & bIsPrepare );

	const char * GetLastError();
	
	int Play();
	
	int StartPlay(double dDuration);
	int PausePlay();
	int StopPlay();

	/*
	get net bits rate per second
	*/
	double GetBitsRate();

	double GetBitsRatePerSec();
	
	/*
	get this client received how many stream from server
	*/
	int GetStreamCount( unsigned int & nStreamCnt );

	/*
	get stream is type
	@arg nStreamNum[in] the stream number
	@arg pType[out] the type of stream
	*/
	int GetStreamType( unsigned int nStreamNum, STREAM_TYPE & Type );

	int GetVideoHeight( unsigned int nStreamNum );

	int GetVideoWidth( unsigned int nStreamNum );
	
	int GetVideoFPS( unsigned int nStreamNum );

	int NoNeedStream( unsigned int nStreamNum );

	/*
	register the data sink
	@arg type[in] this sink will receive what type of stream
	@arg pISink[in] the sink interface point
	*/
	int RegisterSink( STREAM_TYPE type, IDataSink * pISink );

	int UnregisterSink( IDataSink * pISink );

	void RegisterFinishCallback(CLIENTCALLBACK pCallback, void * pParam)
	{
		m_pFinishCallback = pCallback;
		m_pCallBackParam = pParam;
	}

	void RegisterDisConnCallback(CLIENTCALLBACK pCallback, void * pParam, int nDisConnLimmitMSec = -1)
	{
		m_pDisConnCallback = pCallback;
		m_pDCCallBackParam = pParam;
		m_nDisConnLimmtMSec = nDisConnLimmitMSec;
	}

	void RegistrerStartPlayCallback(STARTPLAYCB pCallback, void * pParam)
	{
		m_pStartPlayCB = pCallback;
		m_pStartPlayCBP = pParam;
	}

	void RegistrerPlayNPTCallback(PLAYNPTCB pCallback, void * pParam, double dUpdateInterval)
	{
		m_pPlayNPTCB = pCallback;
		m_pPlayNPTCBP = pParam;
		m_dNPTInformInterval = dUpdateInterval;
	}

private:
	static CAimer39RTSPClient * findClient( ourRTSPClient * pkey );

	static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);

	static void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);

	static void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

	static void setupNextSubsession(RTSPClient* rtspClient);

	static void shutdownStream( RTSPClient* rtspClient );

	static void subsessionAfterPlaying(void* clientData);

	static void subsessionByeHandler(void* clientData);

	static void streamTimerHandler(void* clientData);

	MediaSubsession * findSubSessionByStreamNum( unsigned int nStreamNum );

	void CalcBitsRatePerSec();

protected:
	CAimer39RTSPClient(void);

	virtual void Run();

private:
	friend class DummySink;
	char m_eventLoopWatchVariable;
	bool m_bIsPrepare, m_bIsPlay, m_bIsShutDown;
	int m_errorCode;
	uint64_t m_nRecvBytes, m_nRecvVideoBytes, m_nLastMsSinceStart, m_nRecvBytesSec, m_nID;

	string m_rtspURL;
	string m_lastErrorMsg;

	CLIENTCALLBACK m_pFinishCallback;
	void *		   m_pCallBackParam;

	CLIENTCALLBACK m_pDisConnCallback;
	void *		   m_pDCCallBackParam;
	int			   m_nDisConnLimmtMSec;

	STARTPLAYCB	m_pStartPlayCB;
	void *		m_pStartPlayCBP;

	PLAYNPTCB	m_pPlayNPTCB;
	void *		m_pPlayNPTCBP;
	double		m_dNPTInformInterval;
	double		m_dBitsRatePerSec;

	
	ourRTSPClient * m_pRTSPClient;
	TaskScheduler * m_Scheduler;
	UsageEnvironment * m_Env;
	vector<IDataSink *> m_aVecDataSink[STREAM_TYPE_CNT];
	vector<MediaSubsession *> m_vecNoNeedStream;

	CriticalSection m_cs;
	CClock m_StaticClock;

	static uint64_t m_nCount;
	static CriticalSection m_cs4Create;
	static vector<CAimer39RTSPClient *> vecAllClient;
};
