#include "StdAfx.h"
#include "Aimer39RTSPClient.h"
#include "GroupsockHelper.hh"

#define IS_VIDEO_SUBS_R( subsession, ret ) \
		if ( strcmp( subsession->mediumName(), "video" ) != 0 &&   	\
			 strcmp( subsession->mediumName(), "VIDEO" ) != 0 ) {	\
			return ret;												\
		}

uint64_t CAimer39RTSPClient::m_nCount = 0;
vector<CAimer39RTSPClient *> CAimer39RTSPClient::vecAllClient; 
CriticalSection CAimer39RTSPClient::m_cs4Create;

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

// Implementation of "StreamClientState":
StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}

// Implementation of "DummySink":
// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE (720 * 576 / 2)

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, CAimer39RTSPClient * AimerClient, char const* streamId) {
  return new DummySink(env, subsession, AimerClient, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, CAimer39RTSPClient * AimerClient, char const* streamId)
  : MediaSink(env),
    fSubsession(subsession),
	fAimerClient(AimerClient){
  fStreamId = strDup(streamId);
  fLassNPT = 0.0;
  fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
  fIsPause = FALSE;
  fSyncTime.tv_sec = fSyncTime.tv_usec = 0;
  fSyncTimestamp = 0;
}

DummySink::~DummySink() {
  delete[] fReceiveBuffer;
  delete[] fStreamId;
}

void DummySink::pause()
{
	fIsPause = TRUE;
}

void DummySink::play()
{
	fIsPause = FALSE;
}

void DummySink::calcNPTandInform()
{
	if ((fSubsession.rtpSource() == NULL) || 
		(fSubsession.rtpSource()->timestampFrequency() == 0) || 
		(NULL == fAimerClient->m_pPlayNPTCB)) return;

	u_int32_t rtpTimestamp = fSubsession.rtpSource()->curPacketRTPTimestamp();

	if (fSyncTime.tv_sec == 0 && fSyncTime.tv_usec == 0) {
		struct timeval timeNow;
		gettimeofday(&timeNow, NULL);
		fSyncTime = timeNow;

		fSyncTimestamp = rtpTimestamp;
	}

	unsigned timestampFrequency = fSubsession.rtpSource()->timestampFrequency();
	int timestampDiff = rtpTimestamp - fSyncTimestamp;
	double timeDiff = timestampDiff/(double)timestampFrequency;

	unsigned const million = 1000000;
	unsigned seconds, uSeconds;
	if (timeDiff >= 0.0) {
		seconds = fSyncTime.tv_sec + (unsigned)(timeDiff);
		uSeconds = fSyncTime.tv_usec
			+ (unsigned)((timeDiff - (unsigned)timeDiff)*million);
		if (uSeconds >= million) {
			uSeconds -= million;
			++seconds;
		}
	} else {
		timeDiff = -timeDiff;
		seconds = fSyncTime.tv_sec - (unsigned)(timeDiff);
		uSeconds = fSyncTime.tv_usec
		  - (unsigned)((timeDiff - (unsigned)timeDiff)*million);
		if ((int)uSeconds < 0) {
			uSeconds += million;
			--seconds;
		}
	}

	struct timeval resultPresentationTime;

	resultPresentationTime.tv_sec = seconds;
	resultPresentationTime.tv_usec = uSeconds;

	fSyncTimestamp = rtpTimestamp;
	fSyncTime = resultPresentationTime;

	double dNPT = fSubsession.getNormalPlayTime(resultPresentationTime);
	
	if (((double)abs(dNPT - fLassNPT) >= fAimerClient->m_dNPTInformInterval) && (fAimerClient->m_pPlayNPTCB)) {
		fAimerClient->m_pPlayNPTCB(dNPT, fSubsession.mediumName(), fAimerClient->m_pPlayNPTCBP, fAimerClient);
		fLassNPT = dNPT;
	}
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  DummySink* sink = (DummySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
//#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1
void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) 
{
	// We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
	if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
	envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
	if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
	char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
	sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
	envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
	if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
		envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
	}
#ifdef DEBUG_PRINT_NPT
	envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
	envir() << "\n";
#endif

	calcNPTandInform();
	
	fAimerClient->m_nRecvBytes += frameSize;
	fAimerClient->m_nRecvBytesSec += frameSize;

	if ( strcmp( fSubsession.mediumName(), "VIDEO" ) == 0 ||
		 strcmp( fSubsession.mediumName(), "video" ) == 0 ) {
#if 0
		LARGE_INTEGER stfreq;
		QueryPerformanceFrequency(&stfreq);
		static double m_dfFreq = (double)stfreq.QuadPart;
		
		LARGE_INTEGER startTime;
		static LONGLONG m_nStartTime = 0;
		static LONGLONG nNowTime = 0;
		
		QueryPerformanceCounter(&startTime);
		m_nStartTime = (LONGLONG)startTime.QuadPart;
		
		double dfMinus = (double)(m_nStartTime - nNowTime);
		LONGLONG nMs = (uint64_t)((dfMinus / m_dfFreq) * 1000);

		fprintf(stderr, "diif = %lld\n", nMs);

		nNowTime = m_nStartTime;
#endif
		fAimerClient->m_nRecvVideoBytes += frameSize;
		vector<IDataSink *>::iterator iter = fAimerClient->m_aVecDataSink[STREAM_VIDEO].begin();
		for ( ; iter != fAimerClient->m_aVecDataSink[STREAM_VIDEO].end(); ++iter ) {
			(*iter)->SendData( fReceiveBuffer, frameSize, presentationTime, 
							   fSubsession.mediumName(), fSubsession.codecName() );
		}

		//fprintf(stderr, "Video time %ds, %dms\n", presentationTime.tv_sec, (presentationTime.tv_usec / 1000));
	}

	if ( strcmp( fSubsession.mediumName(), "AUDIO" ) == 0 ||
		 strcmp( fSubsession.mediumName(), "audio" ) == 0 ) {
		vector<IDataSink *>::iterator iter = fAimerClient->m_aVecDataSink[STREAM_AUDIO].begin();
		for ( ; iter != fAimerClient->m_aVecDataSink[STREAM_AUDIO].end(); ++iter ) {
			(*iter)->SendData( fReceiveBuffer, frameSize, presentationTime, 
							fSubsession.mediumName(), fSubsession.codecName(), fSubsession.fmtp_config() );
		}

		//fprintf(stderr, "Audio time %ds, %dms\n", presentationTime.tv_sec, (presentationTime.tv_usec / 1000));
	}
	// Then continue, to request the next frame of data:
	continuePlaying();
}

Boolean DummySink::continuePlaying() 
{
	if (fSource == NULL) return False; // sanity check (should not happen)

	// Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
	fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
						afterGettingFrame, this,
						onSourceClosure, this);
	return True;
}


#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

// Implementation of "ourRTSPClient":
ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum) {
}

ourRTSPClient::~ourRTSPClient() {
}

CAimer39RTSPClient * CAimer39RTSPClient::findClient( ourRTSPClient * pkey )
{
	if ( vecAllClient.empty() || !pkey ) return NULL;

	vector<CAimer39RTSPClient *>::iterator iter = vecAllClient.begin();
	for ( ; iter != vecAllClient.end(); ++iter )
		if ( (*iter)->m_pRTSPClient == pkey )
			return *iter;

	return NULL;
}

// Implementation of the RTSP 'response handlers':
void CAimer39RTSPClient::continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
		CAimer39RTSPClient * arc = findClient( (ourRTSPClient*)rtspClient );
		arc->m_bIsPrepare = false;

		if ( NULL == arc ) {
			arc->m_errorCode = -1;
			env << "some how the system in to a dangerous situation!" << "\n";
			break;
		}
		
		arc->m_errorCode = resultCode;
		if (resultCode != 0) {
			arc->m_lastErrorMsg = resultString;
			env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
			delete[] resultString;
			break;
		}

		char* const sdpDescription = resultString;
		env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

		// Create a media session object from this SDP description:
		scs.session = MediaSession::createNew(env, sdpDescription);
		delete[] sdpDescription; // because we don't need it anymore
		if (scs.session == NULL) {
			env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
			arc->m_errorCode = -1;
			arc->m_lastErrorMsg = env.getResultMsg();
			break;
		} else if (!scs.session->hasSubsessions()) {
			env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
			arc->m_errorCode = 2;
			arc->m_lastErrorMsg = "This session has no media subsessions (i.e., no \"m=\" lines)\n";
			break;
		}
		
		arc->m_bIsPrepare = true;
		return;
	} while (0);
	
	// An unrecoverable error occurred with this stream.
	shutdownStream(rtspClient);
}

void CAimer39RTSPClient::setupNextSubsession(RTSPClient* rtspClient) 
{
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
	CAimer39RTSPClient * arc = findClient( (ourRTSPClient*)rtspClient );

	if ( NULL == arc ) {
		arc->m_errorCode = -1;
		env << "some how the system in to a dangerous situation!" << "\n";
		return;
	}

	scs.subsession = scs.iter->next();
	if (scs.subsession != NULL) {
		vector<MediaSubsession *>::iterator iter = arc->m_vecNoNeedStream.begin();
		for ( ; iter != arc->m_vecNoNeedStream.end(); ++iter ) {
			if ( *iter == scs.subsession ) {
				setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
				return;
			}
		}

		if (!scs.subsession->initiate()) {
			env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
			arc->m_errorCode = 3;
			arc->m_lastErrorMsg = env.getResultMsg();
			setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
		} else {
			env << *rtspClient << "Initiated the \"" << *scs.subsession
			<< "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

			// Continue setting up this subsession, by sending a RTSP "SETUP" command:
			rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP);
		}
		return;
	}

	// We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
	if (scs.session->absStartTime() != NULL) {
		// Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
	} else {
		scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
	}
}

void CAimer39RTSPClient::continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
		CAimer39RTSPClient * arc = findClient( (ourRTSPClient*)rtspClient );

		if ( NULL == arc ) {
			env << "some how the system in to a dangerous situation!" << "\n";
			break;
		}
		
		arc->m_errorCode = resultCode;
		if (resultCode != 0) {
			arc->m_lastErrorMsg = resultString;
			env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
			break;
		}

		env << *rtspClient << "Set up the \"" << *scs.subsession
		<< "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

		// Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
		// (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
		// after we've sent a RTSP "PLAY" command.)

		scs.subsession->sink = DummySink::createNew(env, *scs.subsession, arc, rtspClient->url());
		  // perhaps use your own custom "MediaSink" subclass instead
		if (scs.subsession->sink == NULL) {
			env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
			<< "\" subsession: " << env.getResultMsg() << "\n";
			break;
		}

		env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
		scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
		scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
						   subsessionAfterPlaying, scs.subsession);
		// Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
		if (scs.subsession->rtcpInstance() != NULL) {
			scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
		}
	} while (0);
	delete[] resultString;

	// Set up the next subsession, if any:
	setupNextSubsession(rtspClient);
}

void CAimer39RTSPClient::continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	Boolean success = False;

	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
		CAimer39RTSPClient * arc = findClient( (ourRTSPClient*)rtspClient );

		if ( NULL == arc ) {
			env << "some how the system in to a dangerous situation!" << "\n";
			break;
		}
		
		arc->m_errorCode = resultCode;
		if (resultCode != 0) {
			arc->m_lastErrorMsg = resultString;
			env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
			break;
		}

		// Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
		// using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
		// 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
		// (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
		/*if (scs.duration > 0) {
			unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
			scs.duration += delaySlop;
			unsigned uSecsToDelay = (unsigned)((scs.duration - scs.session->playStartTime())*1000000);
			if (scs.streamTimerTask)
				env.taskScheduler().unscheduleDelayedTask(scs.streamTimerTask);
			scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
		}*/

		env << *rtspClient << "Started playing session";
		if (scs.duration > 0) {
			env << " (for up to " << scs.duration << " seconds)";
		}
		env << "...\n";

		if (arc->m_pStartPlayCB)
			arc->m_pStartPlayCB(scs.duration, scs.session->playStartTime(), arc->m_pStartPlayCBP, arc);

		success = True;
	} while (0);
	delete[] resultString;

	if (!success) {
		// An unrecoverable error occurred with this stream.
		shutdownStream(rtspClient);
		return;
	}
}

// Implementation of the other event handlers:
void CAimer39RTSPClient::subsessionAfterPlaying(void* clientData) 
{
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

	// Begin by closing this subsession's stream:
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);
	while ((subsession = iter.next()) != NULL) {
		if (subsession->sink != NULL) return; // this subsession is still active
	}

	// All subsessions' streams have now been closed, so shutdown the client:
	shutdownStream(rtspClient);
}

void CAimer39RTSPClient::subsessionByeHandler(void* clientData)
{
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
	UsageEnvironment& env = rtspClient->envir(); // alias

	env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

	// Now act as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

void CAimer39RTSPClient::streamTimerHandler(void* clientData)
{
	ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
	StreamClientState& scs = rtspClient->scs; // alias

	scs.streamTimerTask = NULL;

	// Shut down the stream:
	shutdownStream(rtspClient);
}

void CAimer39RTSPClient::shutdownStream( RTSPClient* rtspClient ) 
{
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

	CAimer39RTSPClient * arc = findClient( (ourRTSPClient*)rtspClient );

	if ( NULL == arc ) {
		env << "some how the system in to a dangerous situation!" << "\n";
		return;
	}

	// First, check whether any subsessions have still to be closed:
	if (scs.session != NULL) { 
		Boolean someSubsessionsWereActive = False;
		MediaSubsessionIterator iter(*scs.session);
		MediaSubsession* subsession;

		while ((subsession = iter.next()) != NULL) {
			if (subsession->sink != NULL) {
				Medium::close(subsession->sink);
				subsession->sink = NULL;

				if (subsession->rtcpInstance() != NULL) {
					subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
				}

				someSubsessionsWereActive = True;
			}
		}

		if (someSubsessionsWereActive) {
			// Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
			// Don't bother handling the response to the "TEARDOWN".
			rtspClient->sendTeardownCommand(*scs.session, NULL);
		}
	}

	env << *rtspClient << "Closing the stream.\n";
	arc->m_bIsShutDown = true;
	if (arc->m_pFinishCallback)	arc->m_pFinishCallback(arc->m_pCallBackParam, arc); //call back inform stream over
	Medium::close(rtspClient);
	// Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.
}

CAimer39RTSPClient * CAimer39RTSPClient::CreateNew()
{
	CAimer39RTSPClient * ret = new CAimer39RTSPClient();
	if ( ret != NULL ) {
		CAutoLock lock(&m_cs4Create);
		vecAllClient.push_back( ret );
	}

	return ret;
}

CAimer39RTSPClient::CAimer39RTSPClient(void)
: CThreadBase(), m_eventLoopWatchVariable(0), m_bIsPrepare(false), m_bIsPlay(false)
, m_bIsShutDown(false), m_errorCode(0), m_nRecvBytes(0), m_nLastMsSinceStart(0), m_nRecvVideoBytes(0)
, m_nRecvBytesSec(0), m_pFinishCallback(NULL), m_pCallBackParam(NULL), m_pDisConnCallback(NULL)
, m_pDCCallBackParam(NULL), m_pStartPlayCB(NULL), m_pStartPlayCBP(NULL), m_pPlayNPTCB(NULL), m_pPlayNPTCBP(NULL)
, m_pRTSPClient(NULL), m_dNPTInformInterval(0.0), m_dBitsRatePerSec(0.0), m_Scheduler(NULL), m_Env(NULL)
{
	++m_nCount;
	m_nDisConnLimmtMSec = -1;
	m_nID = m_nCount;
}

CAimer39RTSPClient::~CAimer39RTSPClient(void)
{
	if ( !vecAllClient.empty() ) {
		CAutoLock lock(&m_cs4Create);
		vector<CAimer39RTSPClient *>::iterator iter = vecAllClient.begin();
		for ( ; iter != vecAllClient.end(); ++iter ) 
			if ( *iter == this ) {
				iter = vecAllClient.erase( iter );
				break;
			}
	}
}

uint64_t CAimer39RTSPClient::GetID()
{
	return m_nID;
}

int CAimer39RTSPClient::OpenURL( const char * strURL )
{
	// the rtsp client already begin.
	if ( m_pRTSPClient )
		return 1;
	
	m_vecNoNeedStream.clear();
	m_eventLoopWatchVariable = 0;
	m_rtspURL = strURL;
	m_bIsPrepare = m_bIsPlay = m_bIsShutDown = false;
	
	// Begin by setting up our usage environment:
	m_Scheduler = BasicTaskScheduler::createNew();
	m_Env = BasicUsageEnvironment::createNew(*m_Scheduler);

	m_pRTSPClient = ourRTSPClient::createNew( *m_Env, m_rtspURL.c_str(), RTSP_CLIENT_VERBOSITY_LEVEL, "Anykia IP Camera" );

	m_pRTSPClient->sendDescribeCommand( continueAfterDESCRIBE );

	return 	Start();
}

int CAimer39RTSPClient::Close()
{
	m_eventLoopWatchVariable = 1;
	
	//wait for thread return
	Join();

	if ( !m_bIsShutDown ) {
		shutdownStream( m_pRTSPClient );
	}
	
	m_Env->reclaim();
	m_Env = NULL;

    delete m_Scheduler;
	m_Scheduler = NULL;
	
	m_pRTSPClient = NULL;

	m_bIsPrepare = m_bIsPlay = m_bIsShutDown = false;
	
	return m_errorCode;
}

int CAimer39RTSPClient::GetURL(string & strURL)
{
	if (m_rtspURL.empty()) return -1;
	
	strURL = m_rtspURL;
	return 0;
}

int CAimer39RTSPClient::IsPrepare( bool & bIsPrepare )
{
	bIsPrepare = m_bIsPrepare;
	return m_errorCode;
}

const char * CAimer39RTSPClient::GetLastError()
{
	return m_lastErrorMsg.c_str();
}

double CAimer39RTSPClient::GetBitsRate()
{
	uint64_t nStartTime = 0;
	m_StaticClock.GetMsSinceStart(nStartTime);
	uint64_t startSecond = nStartTime / 1000;
	if (startSecond == 0) startSecond = 1;
	double dBitsRate = (m_nRecvBytes * 8) / startSecond;
	return dBitsRate;
}

double CAimer39RTSPClient::GetBitsRatePerSec()
{
	return m_dBitsRatePerSec;
}

int CAimer39RTSPClient::GetStreamCount( unsigned int & nStreamCnt )
{
	StreamClientState& scs = m_pRTSPClient->scs; // alias
	MediaSubsessionIterator iter(*scs.session);
	
	nStreamCnt = 0;

	iter.reset();
	while ( iter.next() != NULL )
		++nStreamCnt;
	iter.reset();

	return 0;
}

int CAimer39RTSPClient::GetStreamType( unsigned int nStreamNum, STREAM_TYPE & Type )
{
	StreamClientState& scs = m_pRTSPClient->scs; // alias
	MediaSubsessionIterator iter(*scs.session);
	MediaSubsession* subsession = NULL;
	int iStreamCnt = 0;

	iter.reset();

	while ( ( subsession = iter.next() ) != NULL ) {
		if ( strcmp( subsession->mediumName(), "video" ) == 0 ||
			 strcmp( subsession->mediumName(), "VIDEO" ) == 0 ) {
			Type = STREAM_VIDEO;
		} else if ( strcmp( subsession->mediumName(), "audio" ) == 0 ||
					strcmp( subsession->mediumName(), "AUDIO" ) == 0 ) {
			Type = STREAM_AUDIO;
		} else {
			Type = STREAM_UNKNOWN;
		}

		if (nStreamNum == iStreamCnt) break;

		++iStreamCnt;
	}

	iter.reset();
	
	return 0;
}

int CAimer39RTSPClient::GetVideoHeight( unsigned int nStreamNum )
{
	MediaSubsession * subsession = findSubSessionByStreamNum( nStreamNum );

	if ( subsession == NULL ) return -1;

	IS_VIDEO_SUBS_R( subsession, -1 );
	
	return (int)subsession->videoHeight();
}

int CAimer39RTSPClient::GetVideoWidth( unsigned int nStreamNum )
{
	MediaSubsession * subsession = findSubSessionByStreamNum( nStreamNum );

	if ( NULL == subsession ) return -1;

	IS_VIDEO_SUBS_R( subsession, -1 );
	
	return (int)subsession->videoWidth();
}

int CAimer39RTSPClient::GetVideoFPS( unsigned int nStreamNum )
{
	MediaSubsession * subsession = findSubSessionByStreamNum( nStreamNum );

	if ( NULL == subsession ) return -1;

	IS_VIDEO_SUBS_R( subsession, -1 );

	return (int)subsession->videoFPS();
}

int CAimer39RTSPClient::NoNeedStream( unsigned int nStreamNum )
{
	MediaSubsession * subsession = findSubSessionByStreamNum( nStreamNum );
	if ( NULL == subsession ) return 1;

	m_vecNoNeedStream.push_back( subsession );

	if ( m_bIsPlay )
		m_pRTSPClient->sendTeardownCommand( *subsession, NULL );

	return 0;
}

int CAimer39RTSPClient::Play()
{
	// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
	// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
	// (Each 'subsession' will have its own data source.)
	m_pRTSPClient->scs.iter = new MediaSubsessionIterator(*(m_pRTSPClient->scs.session));
	setupNextSubsession( m_pRTSPClient );

	m_bIsPlay = true;
	m_nRecvBytes = 0;
	m_nRecvVideoBytes = 0;
	
	timeval stTime = {0};
	m_StaticClock.start(stTime);
	
	return m_errorCode;
}

int CAimer39RTSPClient::StartPlay(double dDuration)
{
	if (!m_pRTSPClient || !m_bIsPrepare) return -1;

	StreamClientState& scs = m_pRTSPClient->scs; // alias
	MediaSubsessionIterator iter(*scs.session);
	MediaSubsession* subsession = NULL;

	iter.reset();

	while ( ( subsession = iter.next() ) != NULL ) {
		((DummySink*)(subsession->sink))->play();
	}

	m_pRTSPClient->sendPlayCommand(*(m_pRTSPClient->scs.session), continueAfterPLAY, dDuration);
	return 0;
}

int CAimer39RTSPClient::PausePlay()
{
	if (!m_pRTSPClient || !m_bIsPrepare) return -1;

	m_pRTSPClient->sendPauseCommand(*(m_pRTSPClient->scs.session), NULL, 0);

	StreamClientState& scs = m_pRTSPClient->scs; // alias
	MediaSubsessionIterator iter(*scs.session);
	MediaSubsession* subsession = NULL;

	iter.reset();

	while ( ( subsession = iter.next() ) != NULL ) {
		((DummySink*)(subsession->sink))->pause();
	}

	return 0;
}

int CAimer39RTSPClient::StopPlay()
{
	return 0;
}

int CAimer39RTSPClient::RegisterSink( STREAM_TYPE type, IDataSink * pISink )
{
	m_aVecDataSink[type].push_back( pISink );
	return 0;
}

int CAimer39RTSPClient::UnregisterSink( IDataSink * pISink )
{
	bool bFind = false;
	vector<IDataSink *>::iterator iter;

	for ( int i = 0; i < STREAM_TYPE_CNT; ++i ) {
		iter = m_aVecDataSink[i].begin();
		for ( ; iter != m_aVecDataSink[i].end(); ++iter ) {
			if ( *iter == pISink ) {
				iter = m_aVecDataSink[i].erase( iter );
				bFind = true;
				break;
			}
		}

		if ( bFind ) return 0;
	}

	return -1;
}

#define MAX_UNRECV_CNT	500
#define MAX_UNRECV_MS	4000 //4s

void CAimer39RTSPClient::Run()
{
	if (m_bIsShutDown)	return;
	
	uint64_t nLastRecvBytes = m_nRecvVideoBytes;
	int nNoRecvAnyCnt = 0;

	CClock clock;
	BOOL bIsStartClock = FALSE;
	uint64_t nMs = 0;
	UsageEnvironment & env = m_pRTSPClient->envir();
	// All subsequent activity takes place within the event loop:
	//env.taskScheduler().doEventLoop( &m_eventLoopWatchVariable );

	if (m_nDisConnLimmtMSec < 0) m_nDisConnLimmtMSec = MAX_UNRECV_MS;

	while(1) {
		if (m_eventLoopWatchVariable != 0) break;
		
		if (m_bIsPlay) {
			if (((uint64_t)m_nRecvVideoBytes) == ((uint64_t)nLastRecvBytes)) {
				if (!bIsStartClock) {
					clock.ReInit();
					timeval stTime;
					ZeroMemory(&stTime, sizeof(timeval));
					clock.start(stTime);
					bIsStartClock = TRUE;
					continue;
				}
				
				clock.GetMsSinceStart(nMs);
				if (nMs > (uint64_t)(m_nDisConnLimmtMSec)) {
					bIsStartClock = FALSE;
					if (m_pDisConnCallback) m_pDisConnCallback(m_pDCCallBackParam, this);
				}

			}else {
				bIsStartClock = FALSE;
				nLastRecvBytes = m_nRecvVideoBytes;
			}
		}
		
		((BasicTaskScheduler0*)(&(env.taskScheduler())))->SingleStep();
		CalcBitsRatePerSec();
	}
}

MediaSubsession * CAimer39RTSPClient::findSubSessionByStreamNum( unsigned int nStreamNum )
{
	StreamClientState& scs = m_pRTSPClient->scs; // alias
	MediaSubsessionIterator iter(*scs.session);
	MediaSubsession* subsession = NULL;
	int iCnt = 0;

	iter.reset();
	while ( (( subsession = iter.next() ) != NULL) && (++iCnt != nStreamNum) );
	iter.reset();
	
	return subsession;
}

void CAimer39RTSPClient::CalcBitsRatePerSec()
{
	uint64_t nStartTime = 0;
	m_StaticClock.GetMsSinceStart(nStartTime);

	uint64_t diff = nStartTime - m_nLastMsSinceStart;

	if (diff >= 1000 && diff < 1030) {
		m_dBitsRatePerSec = m_nRecvBytesSec * 8;
	}else if (diff >= 1030) {
		m_dBitsRatePerSec = (m_nRecvBytesSec * 8) / (double)(diff / 1000);
	}else {
		return;
	}

	m_nRecvBytesSec = 0;
	m_nLastMsSinceStart = nStartTime;
}
