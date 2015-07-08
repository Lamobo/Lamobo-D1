#include "StdAfx.h"
#include "NetCtrl.h"
#include <ws2tcpip.h>

#define BROADCAST_PORT		8045

enum WorkFunction_en{
	WF_SEARCH = 0,
	WF_NORMAL,
	WF_UNKNOWN
};

CNetCtrl::CNetCtrl(void)
: m_NetType(NET_TYPE_MAX), m_nWorkFunction(WF_UNKNOWN), m_nRecvErrorCnt(0), m_bStop(FALSE), m_bDisConnect(FALSE)
/*, m_Scheduler(NULL), m_Env(NULL), m_ThreadHandle(NULL), m_TID(0), m_eventLoopWatchVariable(0) */
{
	ZeroMemory(&m_stSockInfo, sizeof(m_stSockInfo));
}

CNetCtrl::~CNetCtrl(void)
{

}

int CNetCtrl::OpenSearch()
{
	if (m_nWorkFunction != WF_UNKNOWN) {
		//AfxMessageBox(L"already work in Search or Normal mode!", 0, 0);
		return 0;
	}
	
	m_bStop = FALSE;
	m_nWorkFunction = WF_SEARCH;
	m_NetType = NET_TYPE_UDP; //search function only can work in udp mode

	unsigned int nBroadcastIp = GetBroadcastIp();
	if (nBroadcastIp == 0) {
		AfxMessageBox(L"can't get broadcast ip address!", 0, 0);
		m_nWorkFunction = WF_UNKNOWN;
		return -1;
	}
	
	m_stSockInfo.send_mcast_ip = nBroadcastIp;
	m_stSockInfo.nSendPort = BROADCAST_PORT;
	m_stSockInfo.nListenPort = BROADCAST_PORT;

	if (InitSock() < 0) {
		m_nWorkFunction = WF_UNKNOWN;
		return -1;
	}

	return Start();
}

int CNetCtrl::CloseSearch()
{
	m_bStop = TRUE;
	Join();
	
	closesocket(m_stSockInfo.send_fd);

	return 0;
}

int CNetCtrl::OpenNetCtrl(uint16_t nDestPort, uint16_t nSelfPort, char * pDestIp, NET_TYPE type)
{
	if (m_nWorkFunction != WF_UNKNOWN) {
		//AfxMessageBox(L"already work in Search or Normal mode!", 0, 0);
		return 0;
	}
	
	m_bStop = FALSE;
	//m_eventLoopWatchVariable = 0;
	m_nWorkFunction = WF_NORMAL;

	if (nDestPort)	m_stSockInfo.nSendPort = nDestPort;
	if (nSelfPort)	m_stSockInfo.nListenPort = nSelfPort;

	m_stSockInfo.send_addr.sin_addr.S_un.S_addr = inet_addr(pDestIp);

	m_NetType = type;
	
	m_clock.ReInit();

	if (InitSock() < 0) {
		m_nWorkFunction = WF_UNKNOWN;
		return -1;
	}

	return 0;
}

int CNetCtrl::InitTcpSock()
{
	m_stSockInfo.send_fd = socket(AF_INET, SOCK_STREAM, 0);
	m_stSockInfo.send_addr.sin_family = AF_INET;

	m_stSockInfo.send_addr.sin_port = htons(m_stSockInfo.nSendPort);

	//connect非阻塞方式
	unsigned long ul = 1;
	int result = ioctlsocket(m_stSockInfo.send_fd, FIONBIO, &ul);
	if (result == SOCKET_ERROR) {
		closesocket(m_stSockInfo.send_fd);
		m_bDisConnect = TRUE;
		fprintf(stderr, "set socket FIONBIO 1 error %d!\n", WSAGetLastError());
		return -1;
	}
	
	result = connect(m_stSockInfo.send_fd, (struct sockaddr*)(&m_stSockInfo.send_addr), sizeof(m_stSockInfo.send_addr));
	struct timeval timeout;
	fd_set fd_con;

	FD_ZERO(&fd_con);
	FD_SET(m_stSockInfo.send_fd, &fd_con);
	timeout.tv_sec = 2; //2s 连接超时
	timeout.tv_usec = 0;
	
	result = select(m_stSockInfo.send_fd + 1, 0, &fd_con, 0, &timeout);
	if (result <= 0) {
		closesocket(m_stSockInfo.send_fd);
		m_bDisConnect = TRUE;
		fprintf(stderr, "connect filed with error %d!\n", WSAGetLastError());
		return -1;
	}
	
	m_stSockInfo.recv_fd = m_stSockInfo.send_fd;
	
	//设回阻塞方式
	ul = 0;
	if (ioctlsocket(m_stSockInfo.send_fd, FIONBIO, &ul) < 0) {
		closesocket(m_stSockInfo.send_fd);
		m_bDisConnect = TRUE;
		fprintf(stderr, "set socket FIONBIO 0 error %d!\n", WSAGetLastError());
		return -1;	
	}
	
	/*
	m_stSockInfo.recv_fd = socket(AF_INET, SOCK_STREAM, 0);
	m_stSockInfo.recv_addr.sin_family = AF_INET;
	m_stSockInfo.recv_addr.sin_port = htons(m_stSockInfo.nListenPort);
	m_stSockInfo.recv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	result = bind(m_stSockInfo.recv_fd, (struct sockaddr* )&m_stSockInfo.recv_addr, sizeof(m_stSockInfo.recv_addr));
	if(result < 0) {
		closesocket(m_stSockInfo.recv_fd);
		fprintf(stderr, "can't bind the socket in port = %d, error = %d!\n", m_stSockInfo.nListenPort, WSAGetLastError());
		return 0;
	}
	
	result = listen(m_stSockInfo.recv_fd, SOMAXCONN);
	if(result < 0) {
		closesocket(m_stSockInfo.recv_fd);
		fprintf(stderr, "can't listen the socket in port = %d, error = %d!\n", m_stSockInfo.nListenPort, WSAGetLastError());
		return 0;
	}
	*/

	Start();

	return 0;
}


int CNetCtrl::CloseNetCtrl()
{
	m_bDisConnect = TRUE;

	if (m_bStop) return 0;

	m_bStop = TRUE;
	/*
	m_eventLoopWatchVariable = 1;
	
	if (m_ThreadHandle)
		::WaitForSingleObject( m_ThreadHandle, -1 );
	*/

	Join();
	
	/*
	for (int i = 0; i < m_vecClientConnect.size(); ++i) {
		if (m_vecClientConnect[i])
			delete m_vecClientConnect[i];
		m_vecClientConnect[i] = NULL;
	}

	m_vecClientConnect.clear();
	*/

	if (closesocket(m_stSockInfo.send_fd) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSANOTINITIALISED)
			fprintf(stderr, "[socket close]close socket error! WSAStartup call must occur before!\n");
		else if (error == WSAENETDOWN)
			fprintf(stderr, "[socket close]close socket error! network subsystem has failed!\n");
		else if (error == WSAENOTSOCK)
			fprintf(stderr, "[socket close]close socket error! this descriptor is not a socket!\n");
		else if (error == WSAEINPROGRESS)
			fprintf(stderr, "[socket close]close socket error! a blocking windows socket 1.1 call is in progress, or \
							the service provider is still processing a callback function!\n");
		else if (error == WSAEINTR)
			fprintf(stderr, "[socket close]close socket error! the windows socket 1.1 call was canceled through WSACancelBlockingCall!\n");
		else if (error == WSAEWOULDBLOCK)
			fprintf(stderr, "[socket close]close socket error! ths socket is marked as nonblocking, but the l_onoff member of the linger structure \
							is set to nonzero and the l_linger member of linger structure is set to a nonzero timeout value!\n");
		else 
			fprintf(stderr, "[socket close]close socket error! unknown error!\n");
	}

	m_nWorkFunction = WF_UNKNOWN;

	return 0;
}

int CNetCtrl::GetHostSendIpAddr(unsigned long & ulIpAddr, unsigned short & usPort)
{
	ulIpAddr = 0;
	usPort = 0;

	if (m_nWorkFunction == WF_UNKNOWN) return -1;

	struct sockaddr_in stAddr;
	int nLen = sizeof(stAddr);
	if (getsockname(m_stSockInfo.send_fd, (struct sockaddr*)&stAddr, &nLen) < 0) {
		fprintf(stderr, "%s::getsockname filed! error = %d", __FUNCTION__, WSAGetLastError());
		return -1;
	}

	ulIpAddr = stAddr.sin_addr.S_un.S_addr;
	usPort = stAddr.sin_port;

	return 0;
}

BOOL CNetCtrl::IsDisConnect()
{
	return m_bDisConnect;
}

int CNetCtrl::RegisterServerRespond(IServerRespond * pIRespond)
{
	CAutoLock lock(&m_cs4ServerResponds);
	m_vecServerRespond.push_back(pIRespond);
	return 0;
}

int CNetCtrl::UnregisterServerRespond(IServerRespond * pIRespond)
{
	CAutoLock lock(&m_cs4ServerResponds);

	vector<IServerRespond *>::iterator iter = m_vecServerRespond.begin();

	for (; iter != m_vecServerRespond.end(); ++iter) {
		if (*iter == pIRespond) {
			iter = m_vecServerRespond.erase( iter );
			return 0;
		}
	}

	return -1;
}

int CNetCtrl::RegisterCommandRespond(ICommandRespond * pIRespond)
{
	CAutoLock lock(&m_cs4CommandResponds);
	m_vecCommandRespond.push_back(pIRespond);
	return 0;
}

int CNetCtrl::UnregisterCommandRespond(ICommandRespond * pIRespond)
{
	CAutoLock lock(&m_cs4CommandResponds);

	vector<ICommandRespond *>::iterator iter = m_vecCommandRespond.begin();

	for (; iter != m_vecCommandRespond.end(); ++iter) {
		if (*iter == pIRespond) {
			iter = m_vecCommandRespond.erase( iter );
			return 0;
		}
	}

	return -1;
}

//Search命令发送
int CNetCtrl::Search(TIME_INFO & SystemTime)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_GET_INFO;
	stCH.subCommandType = GET_COMM_SERVER_INFO;
	stCH.nLen = sizeof(TIME_INFO);
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(TIME_INFO) + 4;

	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &SystemTime, sizeof(TIME_INFO));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		fprintf(stderr, "send search info error!");
		delete[] data;
		return -1;
	}
	
	delete[] data;
	return 0;
}

//直接发送Audio数据
int CNetCtrl::SendAudioData(BYTE * pAudioData, int nLen)
{
	int ret = sendto(m_stSockInfo.send_fd, (char *)pAudioData, nLen, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		fprintf(stderr, "send audio data to server error data len = %d, error = %d!\n", nLen, WSAGetLastError());
		return -1;
	}

	return 0;
}

//发送设置图像饱和度、亮度，和对比度的命令
int CNetCtrl::SendImageSet(IMAGE_SET & stImageSet)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_SET_PARAMETER;
	stCH.subCommandType = SET_COMM_IMAGE;
	stCH.nLen = sizeof(IMAGE_SET);
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(IMAGE_SET) + 4;

	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &stImageSet, sizeof(IMAGE_SET));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send image set error!", 0, 0);
		delete[] data;
		return -1;
	}
	
	delete[] data;
	return 0;
}

//发送获取当前服务端隐私遮挡的区域
int CNetCtrl::SendGetPrivacyArea(unsigned int nPrivacyAreaNum)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_GET_INFO;
	stCH.subCommandType = GET_COMM_PRIVACY_AREA;
	stCH.nLen = 0;

	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + 4;
	stSH.nLen = len;
	
	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send get privacy error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送设置隐私遮挡区域命令
int CNetCtrl::SendSetPrivacyArea(PRIVACY_AREA * stAreas, int nAreasCnt)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = nAreasCnt;

	int len = sizeof(SYSTEM_HEADER) + ((sizeof(COMMAND_HEADER) + sizeof(PRIVACY_AREA)) * nAreasCnt) + 4;
	stSH.nLen = len;
	
	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;

	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);

	COMMAND_HEADER stCH;
	for (int i = 0; i < nAreasCnt; ++ i) {
		ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
		stCH.CommandType = COMM_TYPE_SET_PARAMETER;
		stCH.subCommandType = SET_COMM_PRIVACY_AREA;
		stCH.nLen = sizeof(PRIVACY_AREA);
		
		memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
		pos += sizeof(COMMAND_HEADER);

		memcpy(pos, stAreas + i, sizeof(PRIVACY_AREA));
		pos += stCH.nLen;
	}

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send set privacy areas error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送拍照命令
int CNetCtrl::SendTakePic()
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_OPEN_SERVICE;
	stCH.subCommandType = OPEN_COMM_TAKE_PIC;
	stCH.nLen = 0;

	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send take picture error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送录像命令
int CNetCtrl::SendRecode()
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_OPEN_SERVICE;
	stCH.subCommandType = OPEN_COMM_RECODE;
	stCH.nLen = 0;
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send start recode error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送停止录像命令
int CNetCtrl::SendStopRecode()
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_OPEN_SERVICE;
	stCH.subCommandType = OPEN_COMM_RECODE_CLOSE;
	stCH.nLen = 0;
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send start recode error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送缩放命令
int CNetCtrl::SendZoomInOut(uint8_t nZoom)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_SET_PARAMETER;
	stCH.subCommandType = SET_COMM_ZOOM;
	stCH.nLen = sizeof(ZOOM);
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(ZOOM) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &nZoom, sizeof(ZOOM));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send Zoom In/Out error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送获取服务端当前移动侦测的区域命令
int CNetCtrl::SendGetMotionDetectedAreas()
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_GET_INFO;
	stCH.subCommandType = GET_COMM_MOTION_DETECT_AREAS;
	stCH.nLen = 0;

	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send get motion detected areas error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送设置移动侦测的区域命令
int CNetCtrl::SendSetMotionDetectedAreas(MOTION_DETECT & stMotionDetected)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_OPEN_SERVICE;
	stCH.subCommandType = OPEN_COMM_MOTION_DETECT;
	stCH.nLen = sizeof(MOTION_DETECT);

	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(MOTION_DETECT) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &stMotionDetected, sizeof(MOTION_DETECT));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send get motion detected areas error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送云台控制命令
int CNetCtrl::SendCameraMovement(CAMERAMOVEMENT movement)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_SET_PARAMETER;
	stCH.subCommandType = SET_COMM_CAMERA_MOVEMENT;
	stCH.nLen = sizeof(CAMERAMOVEMENT);

	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(CAMERAMOVEMENT) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &movement, sizeof(CAMERAMOVEMENT));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		fprintf(stderr, "movement = %d\n", movement);
		AfxMessageBox(L"send set camera movement error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送打开双向对讲命令
int CNetCtrl::SendTalk(AUDIOPARAMETER & stAudioParam)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_OPEN_SERVICE;
	stCH.subCommandType = OPEN_COMM_TALK;
	stCH.nLen = sizeof(AUDIOPARAMETER);

	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(AUDIOPARAMETER) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &stAudioParam, sizeof(AUDIOPARAMETER));
	
	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send the talk command error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;
	return 0;
}

//发送设置ISP参数命令
int CNetCtrl::SendISPCommand(BYTE * pCommandInfo, int nInfoLen)
{
	if (!pCommandInfo || nInfoLen <= 0) return -1;

	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_SET_PARAMETER;
	stCH.subCommandType = SET_COMM_ISP;
	stCH.nLen = nInfoLen;

	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + nInfoLen + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;

	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, pCommandInfo, nInfoLen);

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send the ISP command info error!", 0, 0);
		delete[] data;
		return -1;
	}
	
	delete[] data;
	return 0;
}

//发送获取文件列表的命令
int CNetCtrl::SendGetFiles()
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_GET_INFO;
	stCH.subCommandType = GET_COMM_FILES_LIST;
	stCH.nLen = 0;

	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send start recode error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;

	return 0;
}

//发送设置声音大小的命令
int CNetCtrl::SendVolumeCtrl(VOLUME volumeCtrl)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_SET_PARAMETER;
	stCH.subCommandType = SET_COMM_VOLUME;
	stCH.nLen = sizeof(VOLUME);
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(VOLUME) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &volumeCtrl, sizeof(VOLUME));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send volume control error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;

	return 0;
}

//发送获取当前服务器中ISP配置的命令
int CNetCtrl::SendGetIspInfo(int nInfoType)
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_GET_INFO;
	stCH.subCommandType = GET_COMM_ISP_INFO;
	stCH.nLen = sizeof(int);
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(int) + 4;
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &nInfoType, sizeof(int));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send volume control error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;

	return 0;
}

//发送心跳包
int CNetCtrl::SendHeartBeat()
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;

	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_HEART_BEAT;
	stCH.subCommandType = 0;
	stCH.nLen = sizeof(int);
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER);
	stSH.nLen = len;

	uint8_t * data = new uint8_t[len];
	uint8_t * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));

	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send volume control error!", 0, 0);
		delete[] data;
		return -1;
	}

	delete[] data;

	return 0;
}

unsigned int CNetCtrl::GetBroadcastIp()
{
	char szHostName[50] = {0};
	
	if (gethostname(szHostName, sizeof(szHostName)) == 0) {
		hostent * hostInfo = gethostbyname(szHostName);
		if (NULL == hostInfo){
			AfxMessageBox(L"can't get host ip address!", 0, 0);
			return 0;
		}
		
		struct in_addr * hAddr = (struct in_addr *)*(hostInfo->h_addr_list);
		return hAddr->S_un.S_addr;
	}

	return 0;
}

int CNetCtrl::InitSock()
{
	m_nRecvErrorCnt = 0;
	m_bDisConnect = FALSE;
	if (m_NetType == NET_TYPE_UDP) return InitUdpSock();
	else if (m_NetType == NET_TYPE_TCP) return InitTcpSock();
	else return -1;
}

int CNetCtrl::InitUdpSock()
{
	int optval = 1;
	/*
	if (m_stSockInfo.send_mcast_ip != 0) {
		m_stSockInfo.recv_fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (m_stSockInfo.recv_fd < 0) {
			AfxMessageBox(L"can't create the sock!", 0, 0);
			return -1;	
		}
		
		if (setsockopt(m_stSockInfo.recv_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(int)) < 0) {
			AfxMessageBox(L"setsockopt fail!", 0, 0);
			closesocket(m_stSockInfo.recv_fd);
			return -1;
		}

		int i = SOCKET_ERROR;

		memset(&m_stSockInfo.recv_addr, 0, sizeof(struct sockaddr_in));  
		m_stSockInfo.recv_addr.sin_family = AF_INET;       
		m_stSockInfo.recv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
		m_stSockInfo.recv_addr.sin_port = htons(m_stSockInfo.nListenPort);
		
		/*bind the socket*//*
		if (bind(m_stSockInfo.recv_fd, (struct sockaddr *) &m_stSockInfo.recv_addr, sizeof (m_stSockInfo.recv_addr)) < 0) {
			AfxMessageBox(L"bind socket to port fail!", 0, 0);
			closesocket(m_stSockInfo.recv_fd);
			return -1;
		}
	}*/

	if((m_stSockInfo.send_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		AfxMessageBox(L"create send socket fail", 0, 0);
		return -1;
	}
	
	optval = 1;
	if (setsockopt(m_stSockInfo.send_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(int)) < 0) {
		AfxMessageBox(L"send setsockopt fail!", 0, 0);
		closesocket(m_stSockInfo.send_fd);
		return -1;
	}

	optval = 1;
    if (m_stSockInfo.send_mcast_ip != 0) {
		if (setsockopt(m_stSockInfo.send_fd, SOL_SOCKET, SO_BROADCAST, (const char *)&optval, sizeof(int)) < 0) {
            AfxMessageBox(L"send_sock udp setsockopt fail", 0, 0);
			closesocket(m_stSockInfo.send_fd);
			return -1;
		}

		m_stSockInfo.send_addr.sin_addr.s_addr  = htonl(INADDR_BROADCAST);
	}

	m_stSockInfo.recv_fd = m_stSockInfo.send_fd;

	m_stSockInfo.send_addr.sin_family = AF_INET;
	m_stSockInfo.send_addr.sin_port = htons(m_stSockInfo.nSendPort);
	
	return 0;
}

/*
unsigned int CNetCtrl::thread_work( void * pParam )
{
	CNetCtrl * pThread = static_cast<CNetCtrl *>(pParam);
	pThread->Work();
	return 0;
}

void CNetCtrl::ClientConnect::incomingCommandHandler(void* instance, int /*mask*//*) {
  ClientConnect* Connect = (ClientConnect*)instance;
  Connect->incomingCommandHandler1();
}


CNetCtrl::ClientConnect::ClientConnect(CNetCtrl & ourCtrl, int recv_fd, sockaddr_in recv_addr)
: m_RecvAddr(recv_addr), m_RecvFd(recv_fd), m_ourCtrl(ourCtrl)
{
	ZeroMemory(m_RequestBuffer, sizeof(m_RequestBuffer));
	m_ourCtrl.m_Env->taskScheduler().setBackgroundHandling(m_RecvFd, SOCKET_READABLE|SOCKET_EXCEPTION,
						(TaskScheduler::BackgroundHandlerProc*)&incomingCommandHandler, this);
}

void CNetCtrl::ClientConnect::incomingCommandHandler1() {
	struct sockaddr_in dummy; // 'from' address, meaningless in this case

	ZeroMemory(m_RequestBuffer, sizeof(m_RequestBuffer));
	socklen_t len = sizeof(struct sockaddr_in);

	int bytesRead = recvfrom(m_RecvFd, (char *)m_RequestBuffer, COMMAND_BUFFER_SIZE, 0, (struct sockaddr *)&dummy, &len);

	m_ourCtrl.ParseCommand(m_RequestBuffer, bytesRead, m_RecvAddr);
}*/

#define TIME_OUT				500
#define MAX_BUFFER_LEN			1024 * 10
#define COMMAND_HEADER_LEN		12
#define MAX_RECV_ERROR_CNT		10

void CNetCtrl::Run()
{
	uint8_t * data = NULL;
	struct timeval timeout;
    struct sockaddr_in sAddr;

	socklen_t len = sizeof(struct sockaddr_in);
	int ret = 0;
	
	ZeroMemory(&sAddr, len);
	
	data = new uint8_t[MAX_BUFFER_LEN];

	timeout.tv_sec = TIME_OUT / 1000;
	timeout.tv_usec = (TIME_OUT % 1000) * 1000;

	timeval stTime = {0};
	m_clock.start(stTime);

	m_nLastMsSinceStart = 0;
	
	int nReady = 0;
	while( TRUE ) {
		if (m_bStop) break;

		fd_set rset;
		
		FD_ZERO(&rset);
		FD_SET(m_stSockInfo.recv_fd, &rset);
		
		nReady = select(m_stSockInfo.recv_fd + 1, &rset, NULL, NULL, &timeout);
		if(nReady <= 0) { //select error or timeout.
			if ((nReady == 0) && (m_nWorkFunction == WF_NORMAL)) {
				uint64_t nStartTime = 0;
				m_clock.GetMsSinceStart(nStartTime);

				uint64_t diff = nStartTime - m_nLastMsSinceStart;
				if ((nStartTime < 100) || diff > 5000) {//每隔5s给服务端发送一个心跳包，客户端不使用心跳包，服务端不必给客户端回复心跳包。
					SendHeartBeat();
					m_nLastMsSinceStart = nStartTime;
				}
			}

			continue;
		}else {
			//if (m_NetType == NET_TYPE_UDP) {
			ret = recvfrom(m_stSockInfo.recv_fd, (char *)data, MAX_BUFFER_LEN, 0, (struct sockaddr *)&sAddr, &len);
			if (ret <= 0){
				++m_nRecvErrorCnt;
				if (m_nRecvErrorCnt >= MAX_RECV_ERROR_CNT) {//连续recvfrom返回进行断线处理
					m_nRecvErrorCnt = 0;
					m_bDisConnect = TRUE;
					m_bStop = TRUE;
					closesocket(m_stSockInfo.send_fd);
					m_nWorkFunction = WF_UNKNOWN;
					goto End; 
				}

				fprintf(stderr, "recv from net error! error = %d\n", WSAGetLastError());
				continue;
			}
			
			if ((m_stSockInfo.send_mcast_ip) && (sAddr.sin_addr.S_un.S_addr == m_stSockInfo.send_mcast_ip)) { //from host
				continue;
			}

			ParseCommand(data, ret, sAddr);
			
			if (m_nWorkFunction == WF_NORMAL){
				uint64_t nStartTime = 0;
				m_clock.GetMsSinceStart(nStartTime);

				uint64_t diff = nStartTime - m_nLastMsSinceStart;
				if ((nStartTime < 100) || diff > 5000) {//每隔5s给服务端发送一个心跳包，客户端不使用心跳包，服务端不必给客户端回复心跳包。
					SendHeartBeat();
					m_nLastMsSinceStart = nStartTime;
				}
			}
			/*}else if (m_NetType == NET_TYPE_TCP) { //listen the port
				struct sockaddr_in ser_addr;
				socklen_t len = sizeof(struct sockaddr_in);

				int ServerSock = accept(m_stSockInfo.recv_fd, (struct sockaddr*)&ser_addr, &len);
				sock_info stInfo = {0};
				stInfo.recv_addr = ser_addr;
				stInfo.recv_fd = ServerSock;
				
				if (NULL == m_Env) {
					m_Scheduler = BasicTaskScheduler::createNew();
					m_Env = BasicUsageEnvironment::createNew(*m_Scheduler);

					m_ThreadHandle = (HANDLE)_beginthreadex( NULL, 0, thread_work, this, 0, &m_TID );
				}
				
				ClientConnect * Connect = new ClientConnect(*this, ServerSock, ser_addr);
				m_vecClientConnect.push_back(Connect);
			}else
				continue;*/
		}
	}

End:
	delete[] data;
}

/*
void CNetCtrl::Work()
{
	m_Env->taskScheduler().doEventLoop( &m_eventLoopWatchVariable );

	m_Env->reclaim();
    delete m_Scheduler;
}*/

void CNetCtrl::ParseCommand(uint8_t * data, uint32_t len, struct sockaddr_in & sAddr)
{
	uint8_t * pos = data;
	uint8_t * EndPos = data + len;

	while (pos < EndPos) {
		if (len < sizeof(SYSTEM_HEADER)) {
			fprintf(stderr, "the data recv from net is wrong!\n");
			return;
		}

		SYSTEM_HEADER stSystemHeader = *(SYSTEM_HEADER*)(pos);
		char strSystemTag[5];
		ZeroMemory(strSystemTag, sizeof strSystemTag);
		CON_SYSTEM_TAG(&stSystemHeader.nSystemTag, strSystemTag);

		if (strcmp(strSystemTag, SYSTEM_TAG) != 0) {
			fprintf(stderr, "no our command!\n");
			return;
		}

		pos += sizeof(SYSTEM_HEADER);
		if (pos >= EndPos) {
			fprintf(stderr, "the data recv from net no any command content!\n");
			return;
		}
		
		for (unsigned int i = 0; i < stSystemHeader.nCommandCnt; ++i) {
			COMMAND_HEADER stCommandHeader = *(COMMAND_HEADER *)pos;
			pos += sizeof(COMMAND_HEADER);
			if (pos >= EndPos) {
				fprintf(stderr, "the data recv from net no any sub command!\n");
				return;
			}

			if (pos + stCommandHeader.nLen > EndPos) {
				fprintf(stderr, "the data recv from net have a invalid sub command type = %d!", stCommandHeader.subCommandType);
				return;
			}

			switch(stCommandHeader.CommandType) {
			case COMM_TYPE_OPEN_SERVICE: //server respond the command process status
				//MessageBox(NULL, L"waring", L"aaa", MB_OK); 
				Sleep(1000);
				ServerOpenProcessRespond(pos, stCommandHeader.subCommandType, stCommandHeader.nLen, sAddr);
				break;
			case COMM_TYPE_GET_INFO:
				SubCommandGetInfoParse(pos, stCommandHeader.subCommandType, stCommandHeader.nLen, sAddr);
				break;
			case COMM_TYPE_SERVER_PROCESS_RETURN:
				ServerReturnParse(pos, stCommandHeader.nLen, sAddr);
				break;
			default:
				fprintf(stderr, "no support %d command", stCommandHeader.CommandType);
				break;
			}

			pos += stCommandHeader.nLen;
			len -= (pos - data);
		}
	}
	
	return;
}

int CNetCtrl::SubCommandGetInfoParse(uint8_t * data, uint8_t subCommandType, uint32_t len, struct sockaddr_in & sAddr)
{
	uint8_t * pos = data;
	
	switch(subCommandType) {
	case GET_COMM_SERVER_INFO:
		if (len != sizeof(SERVER_INFO)) {
			fprintf(stderr, "the data recv from net have a invalid get server info command!\n");
			return -1;
		}
		
		SERVER_INFOEX stExInfo;
		stExInfo.stServerInfo = *(SERVER_INFO *)(pos);
		strcpy(stExInfo.astrServerIpAddr, inet_ntoa(sAddr.sin_addr));

		InformServerRespond(stExInfo);

		break;
	case GET_COMM_PRIVACY_AREA:
		if (len != sizeof(PRIVACY_AREA)) {
			fprintf(stderr, "the data recv from net have a invalid get privacy area command!\n");
			return -1;
		}

		PRIVACY_AREA stPrivacyArea;
		stPrivacyArea = *(PRIVACY_AREA *)(pos);
		InformGetInfoRespond((void*)&stPrivacyArea, len, subCommandType);
		
		break;
	case GET_COMM_MOTION_DETECT_AREAS:
		if (len != sizeof(MOTION_DETECT)) {
			fprintf(stderr, "the data recv from net have a invalid get motion detected areas command!\n");
			return -1;
		}

		MOTION_DETECT stMDetect;
		stMDetect = *(MOTION_DETECT *)(pos);
		InformGetInfoRespond((void*)&stMDetect, len, subCommandType);

		break;
	case GET_COMM_FILES_LIST:
	case GET_COMM_ISP_INFO:
		InformGetInfoRespond((void*)pos, len, subCommandType);
		break;
	default:
		fprintf(stderr, "SubCommandGetInfoParse no support %d sub command", subCommandType);
		break;
	}

	return 0;
}

int CNetCtrl::ServerOpenProcessRespond(uint8_t * data, uint8_t subCommandType, uint32_t len, struct sockaddr_in & sAddr)
{
	if (subCommandType != OPEN_COMM_TALK) {
		fprintf(stderr, "now no support other open command is respond!\n");
		return 0;
	}
	
	//当前如果收到Server返回的Talk命令，则表明Server需要和其他客户端进行双向对讲，将原先的Talk踢出。
	//此处需要修改，当对应了其他Open命令的状态返回处理。
	if (len == 8)
		InformOpenCommandRespond(data, subCommandType);
	else 
		InformOpenCommandRespond(NULL, subCommandType);

	return 0;
}

int CNetCtrl::ServerReturnParse(uint8_t * data, uint32_t len, struct sockaddr_in & sAddr)
{
	if (len < sizeof(RETINFO)) {
		fprintf(stderr, "the data recv from net have a invalid return type structure!\n");
		return -1;
	}

	RETINFO * pRetInfo = (RETINFO *)data;
	for (unsigned int i = 0; i < m_vecCommandRespond.size(); ++i) {
		m_vecCommandRespond[i]->ServerReturnInfo(*pRetInfo);
	}

	return 0;
}

int CNetCtrl::InformServerRespond(SERVER_INFOEX& stExInfo)
{
	for (unsigned int i = 0; i < m_vecServerRespond.size(); ++i)
		m_vecServerRespond[i]->ServerRespond(stExInfo);

	return 0;
}

int CNetCtrl::InformGetInfoRespond(void * pstCommand, uint32_t len, uint8_t subCommandType)
{
	for (unsigned int i = 0; i < m_vecCommandRespond.size(); ++i) {
		if (subCommandType == GET_COMM_PRIVACY_AREA)
			m_vecCommandRespond[i]->PrivacyAreaRespond(*((PRIVACY_AREA *)(pstCommand)));
		else if (subCommandType == GET_COMM_MOTION_DETECT_AREAS)
			m_vecCommandRespond[i]->MotionDetectedAreaRespond(*((MOTION_DETECT *)(pstCommand)));
		else if (subCommandType == GET_COMM_FILES_LIST)
			m_vecCommandRespond[i]->FilesList((char *)pstCommand, len);
		else if (subCommandType == GET_COMM_ISP_INFO) {
			m_vecCommandRespond[i]->IspInfoParamRespond((BYTE *)pstCommand, len);
		}
		else
			fprintf(stderr, "%s:%s::unknown get info is sub command type! can't inform.", __FILE__, __FUNCTION__);
	}

	return 0;
}

int CNetCtrl::InformOpenCommandRespond(void * pstCommand, uint8_t subCommandType)
{
	for (unsigned int i = 0; i < m_vecCommandRespond.size(); ++i) {
		if (subCommandType == OPEN_COMM_TALK) {
			unsigned long ulIpAddr = 0, ulPort = 0;
			if (pstCommand != NULL) {
				ulPort = *((unsigned long *)pstCommand);
				ulIpAddr = *((unsigned long *)pstCommand + 1);
			}

			m_vecCommandRespond[i]->TalkKickOutRespond(ulIpAddr, (unsigned short)ulPort);
		}else 
			fprintf(stderr, "%s:%s::unknown open command is sub command type! can't inform.", __FILE__, __FUNCTION__);
	}

	return 0;
}
