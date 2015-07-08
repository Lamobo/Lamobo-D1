#pragma once
#include <map>
#include <list>
#include <vector>
#include "NetCtrl.h"
#include "AutoLock.h"
#include <string>
using namespace std;

#define MAX_ID_LEN			32
#define MAX_IP_LEN			20
#define MAX_STREAM_CNT		10
#define MAX_STREAM_NAME		16

class CServerSearch : IServerRespond
{
public:
	CServerSearch(void);
	virtual ~CServerSearch(void);
	
	int Search();
	
	virtual int ServerRespond(SERVER_INFOEX & stSInfo);

	int GetServerCount();

	int GetServer(int iServerNum, IServer ** ppIServer);

	int DeleteAllServer();

	class Server : public IServer, protected ICommandRespond, protected IServerRespond
	{
	public:
		Server(void);
		virtual ~Server(void);

		virtual int GetServerID(char * pstrServerID, unsigned int * pnIDlen);
		virtual int GetServerIp(char * pstrServerIP, unsigned int * pnIPLen);
		virtual int GetServerIp(unsigned int & nIP);
		virtual int GetServerStreamPort(unsigned int & nPort);
		virtual int GetServerCommandPort(unsigned int & nPort);
		virtual int GetServerStreamCnt(unsigned int & nCnt);
		virtual int GetServerStreamMode(unsigned int nStreamNum, STREAMMODE & enStreamMode);
		virtual int GetServerStreamName(unsigned int nStreamNum, char * pstrStreamName, unsigned int * pnNameLen);
		virtual int GetServerImageSet(IMAGE_SET & stServerImageSet);
		virtual int GetServerFileDir(string & strDir);
		virtual int GetStreamFps(unsigned int nStreamNum, int & nFps);
		virtual int GetServerRespondComplete(BOOL & bIsRespond);

		virtual int Connect();
		virtual int DisConnect();
		virtual BOOL IsDisConnect();

		virtual int SetCurrentPlayURL(const char * pstrURL);
		virtual int GetCurrentPlayURL(char * pstrURL, unsigned int & nStrLen);

		virtual int	SendImageSet(IMAGE_SET & stImageSet);
		virtual int SendSetPrivacyArea(PRIVACY_AREA * stAreas, int nAreasCnt);
		virtual int	SendGetPrivacyArea(unsigned int nPrivacyAreaNum = 0);
		virtual int SendTakePic();
		virtual int SendRecode();
		virtual int SendStopRecode();
		virtual int SendZoomInOut(uint8_t nZoom);
		virtual int SendGetMotionDetectedAreas();
		virtual int SendSetMotionDetectedAreas(MOTION_DETECT & stMotionDetected);
		virtual int SendCameraMovement(CAMERAMOVEMENT movement);
		virtual int SendTalk(AUDIOPARAMETER & stAudioParam);
		virtual int SendISPCommand(BYTE * pCommandInfo, int nInfoLen);
		virtual int SendGetFiles();
		virtual int SendVolumeCtrl(VOLUME volumeCtrl);
		virtual int SendGetIspInfo(int nInfoType);
		virtual int SendGetServerInfo();

		virtual int GetRespondComplete(int nMainType, int nSubType, BOOL & bIsComplete);
		virtual int GetPrivacyAreaCount(unsigned int & nCount);
		virtual int GetPrivacyArea(unsigned int nIndex, PRIVACY_AREA & stPrivacyArea);

		virtual int GetMotionDetectedSensitivity(unsigned int & nSensitivity);
		virtual int GetMotionDetectedAreas(uint64_t & nMDAreas);

		virtual int SetTalkKickOutCallBack(TALKKICKOUTCB pTalkKickOutCB, void * pParam);

		virtual int SetFileListAddCallBack(FILELISTADDCB pFileListAddCB, void * pParam);

		virtual int SetIspInfoParamCallBack(ISPINFOPARAMCB pIspInfoCB, void * pParam);

		virtual int SetServerRetCallBack(SERVERRETCB pServerRetCB, void * pParam);

	protected:
		virtual int ServerRespond(SERVER_INFOEX & stSInfo);
		virtual int PrivacyAreaRespond(PRIVACY_AREA & stPrivacyArea);
		virtual int MotionDetectedAreaRespond(MOTION_DETECT & stMotionDetected);
		virtual int TalkKickOutRespond(unsigned long ulIpAddr, unsigned short usPort);
		virtual int FilesList(char * strFilesName, unsigned int nLen);
		virtual int IspInfoParamRespond(BYTE * pInfoParam, unsigned int nlen);
		virtual int ServerReturnInfo(RETINFO & stRetInfo);

	private:
		friend class CServerSearch;
		char m_strServerID[MAX_ID_LEN], m_strServerIP[MAX_IP_LEN], 
			m_strServerStreamName[MAX_STREAM_CNT][MAX_STREAM_NAME];

		int m_nStreamCnt, m_nStreamPort, m_nCommandPort, m_nFps[MAX_STREAM_CNT];
		STREAMMODE m_enStreamMode[MAX_STREAM_CNT];
		IMAGE_SET m_stImageSet;
		CNetCtrl m_NetCtrl;
		
		BOOL m_bIsPrivacyAreaComplete, m_bIsServerRespond;
		
		string m_playURL;
		vector<PRIVACY_AREA> m_vecPrivacyArea;
		list<string> m_FilesList;
		string m_fileDirectory;

		UINT		m_nSensitivity;
		uint64_t	m_nMDAreas;

		TALKKICKOUTCB	m_pTalkKickOutCB;
		void *			m_pTalkKickOutCBP;

		FILELISTADDCB	m_pFileListAddCB;
		void *			m_pFileListAddCBP;

		ISPINFOPARAMCB	m_pInfoCB;
		void *			m_pInfoCBP;

		SERVERRETCB		m_pServerRetCB;
		void *			m_pServerRetCBP;

		CriticalSection m_cs, m_cs4PAreas, m_cs4MDAreas, m_cs4TalkKickOut, m_cs4FilesListUpdate, m_cs4Awb, m_cs4SRet;
	};

private:
	int MakeServer(SERVER_INFOEX & stSInfo);

private:
	CNetCtrl				m_NetCtrl;
	map<string, Server*>	m_mapServer;
	CriticalSection			m_cs;
	BOOL					m_bIsOpen;
};
