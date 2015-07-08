#pragma once

#include "IPCameraCommand.h"
#include <string>
using namespace std;

typedef struct ServerInfoEx_t
{
	char		astrServerIpAddr[20];
	SERVER_INFO stServerInfo;
}SERVER_INFOEX;

class IDataSink
{
public:
	virtual ~IDataSink(){};
	virtual int SendData( PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime, 
						  const char * pstrMediumName, const char * pstrCodec, const char * pstrConfig = NULL ) = 0;
};

class IAudioSink
{
public:
	virtual ~IAudioSink(){};
	virtual int SendAudio( PBYTE pData, unsigned int nDataLen, struct timeval sPresentationTime, 
						   unsigned int nAudioChannels, unsigned int nAudioSampleRate, unsigned int nAudioFmt ) = 0;
};

class IVideoSink
{
public:
	virtual ~IVideoSink(){};
	virtual int SendVideo( PBYTE pY, unsigned int nYlineSize, PBYTE pU, unsigned int nUlineSize, 
						   PBYTE pV, unsigned int nVlineSize, struct timeval sPresentationTime, 
						   unsigned int nVideoHeight, unsigned int nVideoWidth ) = 0;
};

class IServerRespond
{
public:
	virtual ~IServerRespond(){};
	virtual int ServerRespond(SERVER_INFOEX & stSInfo) = 0;
};

class ICommandRespond
{
public:
	virtual ~ICommandRespond(){};
	virtual int PrivacyAreaRespond(PRIVACY_AREA & stPrivacyArea) = 0;
	virtual int MotionDetectedAreaRespond(MOTION_DETECT & stMotionDetected) = 0;
	virtual int TalkKickOutRespond(unsigned long ulIpAddr, unsigned short usPort) = 0;
	virtual int FilesList(char * strFilesName, unsigned int nLen) = 0;
	virtual int IspInfoParamRespond(BYTE * pInfoParam, unsigned int nlen) = 0;
	virtual int ServerReturnInfo(RETINFO & stRetInfo) = 0;
};

typedef enum StreamMode_en 
{
	STREAM_MODE_AUDIO_AAC = 0,
	STREAM_MODE_VIDEO_720P,
	STREAM_MODE_VIDEO_D1,
	STREAM_MODE_VIDEO_VGA,
	STREAM_MODE_VIDEO_QVGA,
	STREAM_MODE_MAX
}STREAMMODE;

class IServer;

typedef void (__stdcall *TALKKICKOUTCB)(IServer * pIServer, unsigned long ulIpAddr, unsigned short usPort, void * pClassParam);
typedef void (__stdcall *FILELISTADDCB)(string * strFileName, IServer * pIServer, void * pClassParam);
typedef void (__stdcall *ISPINFOPARAMCB)(IServer * pIServer, BYTE * pInfoParam, unsigned int nlen, void * pClassParam);
typedef void (__stdcall *SERVERRETCB)(IServer * pIServer, RETINFO * pstRetInfo, void * pClassParam);

class IServer
{
public:
	virtual ~IServer(){};
	virtual int GetServerID(char * pstrServerID, unsigned int * pnIDlen) = 0;
	virtual int GetServerIp(char * pstrServerIP, unsigned int * pnIPLen) = 0;
	virtual int GetServerIp(unsigned int & nIP) = 0;
	virtual int GetServerStreamPort(unsigned int & nPort) = 0;
	virtual int GetServerCommandPort(unsigned int & nPort) = 0;
	virtual int GetServerStreamCnt(unsigned int & nCnt) = 0;
	virtual int GetServerStreamMode(unsigned int nStreamNum, STREAMMODE & enStreamMode) = 0;
	virtual int GetServerStreamName(unsigned int nStreamNum, char * pstrStreamName, unsigned int * pnNameLen) = 0;
	virtual int GetServerImageSet(IMAGE_SET & stServerImageSet) = 0;
	virtual int GetServerFileDir(string & strDir) = 0;
	virtual int GetStreamFps(unsigned int nStreamNum, int & nFps) = 0;
	virtual int GetServerRespondComplete(BOOL & bIsRespond) = 0;

	virtual int Connect() = 0;
	virtual int DisConnect() = 0;
	virtual BOOL IsDisConnect() = 0;

	virtual int SetCurrentPlayURL(const char * pstrURL) = 0;
	virtual int GetCurrentPlayURL(char * pstrURL, unsigned int & nStrLen) = 0;

	virtual int	SendImageSet(IMAGE_SET & stImageSet) = 0;
	virtual int SendSetPrivacyArea(PRIVACY_AREA * stAreas, int nAreasCnt) = 0;
	virtual int	SendGetPrivacyArea(unsigned int nPrivacyAreaNum = 0) = 0;
	virtual int SendTakePic() = 0;
	virtual int SendRecode() = 0;
	virtual int SendStopRecode() = 0;
	virtual int SendZoomInOut(uint8_t nZoom) = 0;
	virtual int SendGetMotionDetectedAreas() = 0;
	virtual int SendSetMotionDetectedAreas(MOTION_DETECT & stMotionDetected) = 0;
	virtual int SendCameraMovement(CAMERAMOVEMENT movement) = 0;
	virtual int SendTalk(AUDIOPARAMETER & stAudioParam) = 0;
	virtual int SendISPCommand(BYTE * pCommandInfo, int nInfoLen) = 0;
	virtual int SendGetFiles() = 0;
	virtual int SendVolumeCtrl(VOLUME volumeCtrl) = 0;
	virtual int SendGetIspInfo(int nInfoType) = 0;
	virtual int SendGetServerInfo() = 0;

	virtual int GetRespondComplete(int nMainType, int nSubType, BOOL & bIsComplete) = 0;
	virtual int GetPrivacyAreaCount(unsigned int & nCount) = 0;
	virtual int GetPrivacyArea(unsigned int nIndex, PRIVACY_AREA & stPrivacyArea) = 0;

	virtual int GetMotionDetectedSensitivity(unsigned int & nSensitivity) = 0;
	virtual int GetMotionDetectedAreas(uint64_t & nMDAreas) = 0;

	virtual int SetTalkKickOutCallBack(TALKKICKOUTCB pTalkKickOutCB, void * pParam) = 0;
	virtual int SetFileListAddCallBack(FILELISTADDCB pFileListAddCB, void * pParam) = 0;
	virtual int SetIspInfoParamCallBack(ISPINFOPARAMCB pIspInfoCB, void * pParam) = 0;
	virtual int SetServerRetCallBack(SERVERRETCB pServerRetCB, void * pParam) = 0;
};

class IAudioCaptureEvent
{
public:
	virtual long OnDSCapturedData(unsigned char* data, unsigned long size, unsigned long samplerate) = 0;
};
