#pragma once

#include "ThreadBase.h"
#include "InterfaceClass.h"
#include "AutoLock.h"
#include <vector>
#include <WinSock2.h>
#include "Clock.h"
//#include <BasicUsageEnvironment.hh>
using namespace std;

typedef enum NetType_en
{
	NET_TYPE_TCP,
	NET_TYPE_UDP,
	NET_TYPE_MAX
}NET_TYPE;

typedef struct sock_info
{
	int recv_fd;
	int send_fd;
	struct sockaddr_in recv_addr;
	struct sockaddr_in send_addr;
	uint32_t	send_mcast_ip;
	uint32_t	nListenPort;
	uint32_t	nSendPort;
}sock_info;

#define COMMAND_BUFFER_SIZE	10240

class CNetCtrl : protected CThreadBase
{
public:
	CNetCtrl(void);
	virtual ~CNetCtrl(void);
	
	int OpenSearch();
	int CloseSearch();

	int Search(TIME_INFO & SystemTime);

	int OpenNetCtrl(uint16_t nDestPort, uint16_t nSelfPort, char * pDestIp, NET_TYPE type = NET_TYPE_UDP);
	int CloseNetCtrl();
	BOOL IsDisConnect();

	int GetHostSendIpAddr(unsigned long & ulIpAddr, unsigned short & usPort);

	int SendAudioData(BYTE * pAudioData, int nLen);

	int SendImageSet(IMAGE_SET & stImageSet);
	int SendGetPrivacyArea(unsigned int nPrivacyAreaNum = 0);
	int SendSetPrivacyArea(PRIVACY_AREA * stAreas, int nAreasCnt);
	int SendTakePic();
	int SendRecode();
	int SendStopRecode();
	int SendZoomInOut(uint8_t nZoom);
	int SendGetMotionDetectedAreas();
	int SendSetMotionDetectedAreas(MOTION_DETECT & stMotionDetected);
	int SendCameraMovement(CAMERAMOVEMENT movement);
	int SendTalk(AUDIOPARAMETER & stAudioParam);
	int SendISPCommand(BYTE * pCommandInfo, int nInfoLen);
	int SendGetFiles();
	int SendVolumeCtrl(VOLUME volumeCtrl);
	int SendGetIspInfo(int nInfoType);
	int SendHeartBeat();

	int RegisterServerRespond(IServerRespond * pIRespond);
	int UnregisterServerRespond(IServerRespond * pIRespond);
	
	int RegisterCommandRespond(ICommandRespond * pIRespond);
	int UnregisterCommandRespond(ICommandRespond * pIRespond);
	/*
	class ClientConnect
	{
	public:
		ClientConnect(CNetCtrl & ourCtrl, int recv_fd, struct sockaddr_in recv_addr);
		virtual ~ClientConnect();

		static void incomingCommandHandler(void*, int /*mask*//*);

		void incomingCommandHandler1();

	private:
		struct sockaddr_in m_RecvAddr;
		int m_RecvFd;
		CNetCtrl & m_ourCtrl;
		uint8_t m_RequestBuffer[COMMAND_BUFFER_SIZE];
	};*/

protected:
	virtual void Run();
	//virtual void Work();

private:
	unsigned int GetBroadcastIp();

	int InitSock();

	int InitUdpSock();

	int InitTcpSock();

	void ParseCommand(uint8_t * data, uint32_t len, struct sockaddr_in & sAddr);

	int SubCommandGetInfoParse(uint8_t * data, uint8_t subCommandType, uint32_t len, struct sockaddr_in & sAddr);
	int ServerOpenProcessRespond(uint8_t * data, uint8_t subCommandType, uint32_t len, struct sockaddr_in & sAddr);
	int ServerReturnParse(uint8_t * data, uint32_t len, struct sockaddr_in & sAddr);

	int InformServerRespond(SERVER_INFOEX & stExInfo);

	int InformGetInfoRespond(void * pstCommand, uint32_t len, uint8_t subCommandType);
	int InformOpenCommandRespond(void * pstCommand, uint8_t subCommandType);

	static unsigned int WINAPI thread_work(void * pParam);

private:
	NET_TYPE m_NetType;
	int	m_nWorkFunction, m_nRecvErrorCnt;
	sock_info m_stSockInfo;

	BOOL m_bStop, m_bDisConnect;
	vector<IServerRespond *>	m_vecServerRespond;
	vector<ICommandRespond *>	m_vecCommandRespond;
	CClock m_clock;
	uint64_t m_nLastMsSinceStart;

	CriticalSection m_cs4ServerResponds, m_cs4CommandResponds;
	/*
	char m_eventLoopWatchVariable;
	vector<ClientConnect *>		m_vecClientConnect;
	TaskScheduler*				m_Scheduler;
	UsageEnvironment*			m_Env;
	HANDLE						m_ThreadHandle;
	unsigned int				m_TID;
	*/
};


