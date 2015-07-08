#include "StdAfx.h"
#include "ServerSearch.h"
#include <time.h>

CServerSearch::CServerSearch(void)
: m_bIsOpen(FALSE)
{
}

CServerSearch::~CServerSearch(void)
{
	DeleteAllServer();
	m_NetCtrl.CloseSearch();
}

int CServerSearch::Search()
{
	if (!m_bIsOpen) {
		m_NetCtrl.OpenSearch();
		m_NetCtrl.RegisterServerRespond(this);
		m_bIsOpen = TRUE;
	}

	time_t t = time(0);
	struct tm * ptNow = NULL;
	
	ptNow = localtime(&t);
	TIME_INFO stSystemTime = {0};

	stSystemTime.nYear = (ptNow->tm_year + 1900) - 1970;
	stSystemTime.nMon = ptNow->tm_mon + 1;
	stSystemTime.nDay = ptNow->tm_mday;
	stSystemTime.nHour = ptNow->tm_hour;
	stSystemTime.nMin = ptNow->tm_min;
	stSystemTime.nSecond = ptNow->tm_sec;

	return m_NetCtrl.Search(stSystemTime);
}

int CServerSearch::ServerRespond(SERVER_INFOEX & stSInfo)
{
	return MakeServer(stSInfo);
}

int CServerSearch::GetServerCount()
{
	CAutoLock lock(&m_cs);
	return m_mapServer.size();
}

int CServerSearch::GetServer(int iServerNum, IServer ** ppIServer)
{
	CAutoLock lock(&m_cs);
	if (((unsigned int)iServerNum >= m_mapServer.size()) || (iServerNum < 0)) return -1;
	
	map<string, Server*>::iterator iterService = m_mapServer.begin();

	for (int i = 0; i != iServerNum; ++i) ++iterService;

	*ppIServer = iterService->second;

	return 0;
}

int CServerSearch::MakeServer(SERVER_INFOEX & stSInfo)
{
	Server * ser = new Server();
	
	strncpy(ser->m_strServerID, stSInfo.stServerInfo.strDeviceID, MAX_ID_LEN);
	strncpy(ser->m_strServerIP, stSInfo.astrServerIpAddr, MAX_IP_LEN);
	
	if (strlen(stSInfo.stServerInfo.strMainAddr)) {
		ser->m_nStreamCnt += 1;
		strncpy(ser->m_strServerStreamName[0], stSInfo.stServerInfo.strMainAddr, MAX_STREAM_NAME);
		ser->m_nFps[0] = stSInfo.stServerInfo.nMainFps;

		switch(stSInfo.stServerInfo.enMainStream) {
		case VIDEO_MODE_QVGA:
			ser->m_enStreamMode[0] = STREAM_MODE_VIDEO_QVGA;
			break;
		case VIDEO_MODE_VGA:
			ser->m_enStreamMode[0] = STREAM_MODE_VIDEO_VGA;
			break;
		case VIDEO_MODE_D1:
			ser->m_enStreamMode[0] = STREAM_MODE_VIDEO_D1;
			break;
		case VIDEO_MODE_720P:
			ser->m_enStreamMode[0] = STREAM_MODE_VIDEO_720P;
			break;
		default:
			fprintf(stderr, "main stream %d mode unsupport\n", stSInfo.stServerInfo.enMainStream);
			ser->m_enStreamMode[0] = STREAM_MODE_MAX;
			break;
		}
	}

	if (strlen(stSInfo.stServerInfo.strSubAddr)) {
		ser->m_nStreamCnt += 1;
		strncpy(ser->m_strServerStreamName[1], stSInfo.stServerInfo.strSubAddr, MAX_STREAM_NAME);
		
		ser->m_nFps[1] = stSInfo.stServerInfo.nSubFps;
		switch(stSInfo.stServerInfo.enSubStream) {
		case VIDEO_MODE_QVGA:
			ser->m_enStreamMode[1] = STREAM_MODE_VIDEO_QVGA;
			break;
		case VIDEO_MODE_VGA:
			ser->m_enStreamMode[1] = STREAM_MODE_VIDEO_VGA;
			break;
		case VIDEO_MODE_D1:
			ser->m_enStreamMode[1] = STREAM_MODE_VIDEO_D1;
			break;
		case VIDEO_MODE_720P:
			ser->m_enStreamMode[1] = STREAM_MODE_VIDEO_720P;
			break;
		default:
			fprintf(stderr, "sub stream %d mode unsupport\n", stSInfo.stServerInfo.enSubStream);
			ser->m_enStreamMode[1] = STREAM_MODE_MAX;
			break;
		}
	}

	ser->m_nStreamPort = stSInfo.stServerInfo.nRtspPort;
	ser->m_nCommandPort = stSInfo.stServerInfo.nCommandPort;
	ser->m_stImageSet = stSInfo.stServerInfo.stImageSet;
	
	CAutoLock lock(&m_cs);
	pair<map<string, Server*>::iterator, bool> insertRet = m_mapServer.insert(pair<string, Server*>(ser->m_strServerIP, ser));
	if (!(insertRet.second)) {
		fprintf(stderr, "the server %s already been searched\n", ser->m_strServerIP);
		delete ser;
		return 1;
	}
	
	//ser->m_NetCtrl.OpenNetCtrl(ser->m_nCommandPort, 0, ser->m_strServerIP, NET_TYPE_TCP);

	return 0;
}

int CServerSearch::DeleteAllServer()
{
	CAutoLock lock(&m_cs);
	if (m_mapServer.empty()) return 0;

	map<string, Server*>::iterator iterService = m_mapServer.begin();
	for (; iterService != m_mapServer.end(); ++iterService) {
		if (iterService->second)
			delete iterService->second;
	}

	m_mapServer.clear();
	return 0;
}

CServerSearch::Server::Server()
:m_nStreamCnt(0), m_nStreamPort(0), m_nCommandPort(0), m_bIsPrivacyAreaComplete(FALSE)
{
	ZeroMemory(m_strServerID, sizeof(m_strServerID));
	ZeroMemory(m_strServerIP, sizeof(m_strServerIP));
	ZeroMemory(m_strServerStreamName, sizeof(m_strServerStreamName));
	ZeroMemory(&m_stImageSet, sizeof(IMAGE_SET));
	memset(m_enStreamMode, 0xff, sizeof(m_enStreamMode));
	
	for (int i = 0; i < MAX_STREAM_CNT; ++i) m_nFps[i] = 30;

	m_NetCtrl.RegisterCommandRespond(this);
	m_NetCtrl.RegisterServerRespond((IServerRespond *)this);
	m_playURL = "";

	m_nSensitivity = 0;
	m_nMDAreas = 0;

	m_bIsServerRespond = TRUE;

	m_pTalkKickOutCB = NULL;
	m_pTalkKickOutCBP = NULL;

	m_pFileListAddCB = NULL;
	m_pFileListAddCBP = NULL;
}


CServerSearch::Server::~Server()
{
	m_FilesList.clear();
	m_NetCtrl.CloseNetCtrl();
}

int CServerSearch::Server::GetServerID(char * pstrServerID, unsigned int * pnIDlen)
{
	if (*pnIDlen < strlen(m_strServerID) || pstrServerID == NULL) return -1;
	
	CAutoLock lock(&m_cs);

	*pnIDlen = strlen(m_strServerID);
	strncpy(pstrServerID, m_strServerID, *pnIDlen);

	return 0;
}

int CServerSearch::Server::GetServerIp(char * pstrServerIP, unsigned int * pnIPLen)
{
	if (*pnIPLen < strlen(m_strServerIP) || pstrServerIP == NULL) return -1;
	
	*pnIPLen = strlen(m_strServerIP);
	strncpy(pstrServerIP, m_strServerIP, *pnIPLen);

	return 0;
}

int CServerSearch::Server::GetServerIp(unsigned int & nIP)
{
	nIP = inet_addr(m_strServerIP);
	return 0;
}

int CServerSearch::Server::GetServerStreamPort(unsigned int & nPort)
{
	CAutoLock lock(&m_cs);
	nPort = m_nStreamPort;
	return 0;
}

int CServerSearch::Server::GetServerCommandPort(unsigned int & nPort)
{
	nPort = m_nCommandPort;
	return 0;
}

int CServerSearch::Server::GetServerStreamCnt(unsigned int & nCnt)
{
	nCnt = m_nStreamCnt;
	return 0;
}

int CServerSearch::Server::GetServerStreamMode(unsigned int nStreamNum, STREAMMODE & enStreamMode)
{
	if (nStreamNum > MAX_STREAM_CNT) return -1;
	
	CAutoLock lock(&m_cs);
	enStreamMode = m_enStreamMode[nStreamNum];
	return 0;
}

int CServerSearch::Server::GetServerStreamName(unsigned int nStreamNum, char * pstrStreamName, unsigned int * pnNameLen)
{
	if (nStreamNum > MAX_STREAM_CNT) return -1;
	if (*pnNameLen < strlen(m_strServerStreamName[nStreamNum])) return -1;
	
	*pnNameLen = strlen(m_strServerStreamName[nStreamNum]);
	strncpy(pstrStreamName, m_strServerStreamName[nStreamNum], *pnNameLen);
	return 0;
}

int CServerSearch::Server::GetServerImageSet(IMAGE_SET & stServerImageSet)
{
	CAutoLock lock(&m_cs);
	stServerImageSet = m_stImageSet;
	return 0;
}

int CServerSearch::Server::GetServerFileDir(string & strDir)
{
	strDir = m_fileDirectory;
	return 0;
}

int CServerSearch::Server::GetStreamFps(unsigned int nStreamNum, int & nFps)
{
	if (nStreamNum > MAX_STREAM_CNT) return -1;
	CAutoLock lock(&m_cs);

	nFps = m_nFps[nStreamNum];
	return 0;
}

int CServerSearch::Server::GetServerRespondComplete(BOOL & bIsRespond)
{
	bIsRespond = m_bIsServerRespond;
	return 0;
}

int CServerSearch::Server::Connect()
{
	return m_NetCtrl.OpenNetCtrl(m_nCommandPort, 0, m_strServerIP, NET_TYPE_TCP);
}

int CServerSearch::Server::DisConnect()
{
	return m_NetCtrl.CloseNetCtrl();
}

int CServerSearch::Server::IsDisConnect()
{
	return m_NetCtrl.IsDisConnect();
}

int	CServerSearch::Server::SendImageSet(IMAGE_SET & stImageSet)
{
	int ret = m_NetCtrl.SendImageSet(stImageSet);
	if (ret == 0){
		CAutoLock lock(&m_cs);
		m_stImageSet = stImageSet;
	}
	return ret;
}

int	CServerSearch::Server::SendGetPrivacyArea(unsigned int nPrivacyAreaNum)
{
	m_bIsPrivacyAreaComplete = FALSE;
	int ret = m_NetCtrl.SendGetPrivacyArea(nPrivacyAreaNum);
	return ret;
}

int	CServerSearch::Server::SendSetPrivacyArea(PRIVACY_AREA * stAreas, int nAreasCnt)
{
	int ret = m_NetCtrl.SendSetPrivacyArea(stAreas, nAreasCnt);
	
	if (ret != 0) return ret;
	
	PRIVACY_AREA stDeleteAreaContent;
	ZeroMemory(&stDeleteAreaContent, sizeof(PRIVACY_AREA));

	CAutoLock lock(&m_cs4PAreas);

	for (int i = 0; i < nAreasCnt; ++i) {
		vector<PRIVACY_AREA>::iterator iter = m_vecPrivacyArea.begin();
		for (; iter != m_vecPrivacyArea.end(); ++iter) {
			if (iter->nNumber == stAreas[i].nNumber) {
				stDeleteAreaContent.nNumber = stAreas[i].nNumber;
				if (memcmp(stAreas + i, &stDeleteAreaContent, sizeof(PRIVACY_AREA)) == 0)
					iter = m_vecPrivacyArea.erase(iter);
				else
					*iter = stAreas[i];
			}

			break;
		}

		if (iter == m_vecPrivacyArea.end()) {
			stDeleteAreaContent.nNumber = stAreas[i].nNumber;
			if (memcmp(stAreas + i, &stDeleteAreaContent, sizeof(PRIVACY_AREA)) != 0)
				m_vecPrivacyArea.push_back(stAreas[i]);
		}
	}

	return 0;
}

int CServerSearch::Server::SendTakePic()
{
	int ret = m_NetCtrl.SendTakePic();
	return ret;
}

int CServerSearch::Server::SendRecode()
{
	int ret = m_NetCtrl.SendRecode();
	return ret;
}

int CServerSearch::Server::SendStopRecode()
{
	return m_NetCtrl.SendStopRecode();
}

int CServerSearch::Server::SendZoomInOut(uint8_t nZoom)
{
	int ret = m_NetCtrl.SendZoomInOut(nZoom);
	return ret;
}

int CServerSearch::Server::SendGetMotionDetectedAreas()
{
	m_nSensitivity = 0;
	int ret = m_NetCtrl.SendGetMotionDetectedAreas();
	return ret;
}

int CServerSearch::Server::SendSetMotionDetectedAreas(MOTION_DETECT & stMotionDetected)
{
	int ret = m_NetCtrl.SendSetMotionDetectedAreas(stMotionDetected);
	if (ret != 0) return ret;

	CAutoLock lock(&m_cs4MDAreas);
	m_nSensitivity = stMotionDetected.nSensitivity;
	m_nMDAreas = stMotionDetected.nMDAreas;

	return 0;
}

int CServerSearch::Server::SendCameraMovement(CAMERAMOVEMENT movement)
{
	int ret = m_NetCtrl.SendCameraMovement(movement);
	return ret;
}

int CServerSearch::Server::SendTalk(AUDIOPARAMETER & stAudioParam)
{
	int ret = m_NetCtrl.SendTalk(stAudioParam);
	return ret;
}

int CServerSearch::Server::SendISPCommand(BYTE * pCommandInfo, int nInfoLen)
{
	return m_NetCtrl.SendISPCommand(pCommandInfo, nInfoLen);
}

int CServerSearch::Server::SendGetFiles()
{	
	{
		CAutoLock lock(&m_cs4FilesListUpdate);
		m_FilesList.clear();
	}

	return m_NetCtrl.SendGetFiles();
}

int CServerSearch::Server::SendVolumeCtrl(VOLUME volumeCtrl)
{
	return m_NetCtrl.SendVolumeCtrl(volumeCtrl);
}

int CServerSearch::Server::SendGetIspInfo(int nInfoType)
{
	return m_NetCtrl.SendGetIspInfo(nInfoType);
}

int CServerSearch::Server::SendGetServerInfo()
{
	m_bIsServerRespond = FALSE;

	time_t t = time(0);
	struct tm * ptNow = NULL;
	
	ptNow = localtime(&t);
	TIME_INFO stSystemTime = {0};

	stSystemTime.nYear = (ptNow->tm_year + 1900) - 1970;
	stSystemTime.nMon = ptNow->tm_mon + 1;
	stSystemTime.nDay = ptNow->tm_mday;
	stSystemTime.nHour = ptNow->tm_hour;
	stSystemTime.nMin = ptNow->tm_min;
	stSystemTime.nSecond = ptNow->tm_sec;

	return m_NetCtrl.Search(stSystemTime);
}

int CServerSearch::Server::GetRespondComplete(int nMainType, int nSubType, BOOL & bIsComplete)
{
	switch(nMainType) {
	case COMM_TYPE_GET_INFO:
		switch(nSubType) {
		case GET_COMM_PRIVACY_AREA:
			bIsComplete = m_bIsPrivacyAreaComplete;
			break;
		default:
			bIsComplete = FALSE;
		}
	default:
		bIsComplete = FALSE;
	}

	return 0;
}

int CServerSearch::Server::GetPrivacyAreaCount(unsigned int & nCount)
{
	CAutoLock lock(&m_cs4PAreas);
	nCount = m_vecPrivacyArea.size();
	return 0;
}

int CServerSearch::Server::GetPrivacyArea(unsigned int nIndex, PRIVACY_AREA & stPrivacyArea)
{
	CAutoLock lock(&m_cs4PAreas);
	
	if ((nIndex >= m_vecPrivacyArea.size()) || (nIndex < 0)) return -1; 

	stPrivacyArea = m_vecPrivacyArea[nIndex];
	return 0;
}

int CServerSearch::Server::PrivacyAreaRespond(PRIVACY_AREA & stPrivacyArea)
{
	CAutoLock lock(&m_cs4PAreas);
	
	if (stPrivacyArea.nNumber == 0) {
		m_bIsPrivacyAreaComplete = TRUE;
		return 0;
	}
	
	PRIVACY_AREA stEmptyArea;
	ZeroMemory(&stEmptyArea, sizeof(PRIVACY_AREA));

	for (unsigned int i = 0; i < m_vecPrivacyArea.size(); ++i) {
		if (m_vecPrivacyArea[i].nNumber == stPrivacyArea.nNumber) {
			m_vecPrivacyArea[i] = stPrivacyArea;
			return 0;
		}
	}

	stEmptyArea.nNumber = stPrivacyArea.nNumber;
	
	if (memcmp(&stEmptyArea, &stPrivacyArea, sizeof(PRIVACY_AREA)) != 0)
		m_vecPrivacyArea.push_back(stPrivacyArea);

	return 0;
}

int CServerSearch::Server::MotionDetectedAreaRespond(MOTION_DETECT & stMotionDetected)
{
	CAutoLock lock(&m_cs4MDAreas);
	m_nSensitivity = stMotionDetected.nSensitivity;
	m_nMDAreas = stMotionDetected.nMDAreas;

	return 0;
}

int CServerSearch::Server::TalkKickOutRespond(unsigned long ulIpAddr, unsigned short usPort)
{
	CAutoLock lock(&m_cs4TalkKickOut);

	if (m_pTalkKickOutCB)
		m_pTalkKickOutCB((IServer*)this, ulIpAddr, usPort, m_pTalkKickOutCBP);
	
	return 0;
}

#define NAME_SEPARATOR			';'
#define STRING_SEPARATOR		0
#define DIRECTORY_SEPARATOR		"[]"

int CServerSearch::Server::FilesList(char * strFilesName, unsigned int nLen)
{
	string strName;
	char * pEnd = strFilesName + nLen, * pWhere = NULL, * pNameStart = strFilesName;
	BOOL bHasSeparator = TRUE;
	BOOL bIsDir = FALSE;

	if (strFilesName[nLen - 1] != STRING_SEPARATOR)
		bHasSeparator = FALSE;
	
	while ((pNameStart < pEnd) && (*pNameStart != STRING_SEPARATOR)) {
		bIsDir = FALSE;
		pWhere = strchr(pNameStart, NAME_SEPARATOR);

		if (pWhere == NULL) {
			if (!bHasSeparator) {
				char * pstrName = new char[pEnd - pNameStart + 1];
				int i = 0;

				for (; i < pEnd - pNameStart; ++i) {
					pstrName[i] = pNameStart[i];
				}
				pstrName[i] = STRING_SEPARATOR;
				strName = pstrName;
			}else {
				strName = pNameStart;
			}

			if (strName.at(0) == DIRECTORY_SEPARATOR[0] && 
				strName.at(strName.length()-1) == DIRECTORY_SEPARATOR[1]) {
					strName.erase(0, 1);
					strName.erase(strName.length()-1, 1);
					bIsDir = TRUE;
			}
			
			if (bIsDir) {
				m_fileDirectory = strName;
				break;
			}

			CAutoLock lock(&m_cs4FilesListUpdate);
			if (m_pFileListAddCB)
				m_pFileListAddCB(&strName, (IServer*)this, m_pFileListAddCBP);

			m_FilesList.push_back(strName);
			break;
		}

		*pWhere = STRING_SEPARATOR;
		strName = pNameStart;
		if (strName.at(0) == DIRECTORY_SEPARATOR[0] && 
			strName.at(strName.length()-1) == DIRECTORY_SEPARATOR[1]) {
				strName.erase(0, 1);
				strName.erase(strName.length()-1, 1);
				bIsDir = TRUE;
		}
	
		*pWhere = NAME_SEPARATOR;

		pNameStart = ++pWhere;

		if (bIsDir) {
			m_fileDirectory = strName;
			continue;
		}

		CAutoLock lock(&m_cs4FilesListUpdate);
		if (m_pFileListAddCB)
			m_pFileListAddCB(&strName, (IServer*)this, m_pFileListAddCBP);

		m_FilesList.push_back(strName);
	}

	return 0;
}

int CServerSearch::Server::IspInfoParamRespond(BYTE * pInfoParam, unsigned int nlen)
{
	CAutoLock lock(&m_cs4Awb);
	if (m_pInfoCB)
		m_pInfoCB((IServer*)this, pInfoParam, nlen, m_pInfoCBP);
	return 0;
}

int CServerSearch::Server::SetCurrentPlayURL(const char * pstrURL)
{
	m_playURL = pstrURL;
	return 0;
}

int CServerSearch::Server::GetCurrentPlayURL(char * pstrURL, unsigned int & nStrLen)
{
	if (nStrLen < m_playURL.length() + 1) return -1;

	if (m_playURL.empty()) {
		nStrLen = 0;
		return 0;
	}
	
	nStrLen = m_playURL.length() + 1;
	memcpy(pstrURL, m_playURL.c_str(), nStrLen);
	return 0;
}

int CServerSearch::Server::GetMotionDetectedSensitivity(unsigned int & nSensitivity)
{
	nSensitivity = m_nSensitivity;
	return 0;
}

int CServerSearch::Server::GetMotionDetectedAreas(uint64_t & nMDAreas)
{
	CAutoLock lock(&m_cs4MDAreas);
	nMDAreas = m_nMDAreas;

	return 0;
}

int CServerSearch::Server::SetTalkKickOutCallBack(TALKKICKOUTCB pTalkKickOutCB, void * pParam)
{
	CAutoLock lock(&m_cs4TalkKickOut);
	m_pTalkKickOutCB = pTalkKickOutCB;
	m_pTalkKickOutCBP = pParam;
	return 0;
}

int CServerSearch::Server::SetFileListAddCallBack(FILELISTADDCB pFileListAddCB, void * pParam)
{
	CAutoLock lock(&m_cs4FilesListUpdate);
	m_pFileListAddCB = pFileListAddCB;
	m_pFileListAddCBP = pParam;
	return 0;
}

int CServerSearch::Server::SetIspInfoParamCallBack(ISPINFOPARAMCB pIspInfoCB, void * pParam)
{
	CAutoLock lock(&m_cs4Awb);
	m_pInfoCB = pIspInfoCB;
	m_pInfoCBP = pParam;
	return 0;
}

int CServerSearch::Server::ServerRespond(SERVER_INFOEX & stSInfo)
{
	CAutoLock lock(&m_cs);

	strncpy(m_strServerID, stSInfo.stServerInfo.strDeviceID, MAX_ID_LEN);
	
	if (strlen(stSInfo.stServerInfo.strMainAddr)) {
		m_nStreamCnt += 1;
		strncpy(m_strServerStreamName[0], stSInfo.stServerInfo.strMainAddr, MAX_STREAM_NAME);
		m_nFps[0] = stSInfo.stServerInfo.nMainFps;

		switch(stSInfo.stServerInfo.enMainStream) {
		case VIDEO_MODE_QVGA:
			m_enStreamMode[0] = STREAM_MODE_VIDEO_QVGA;
			break;
		case VIDEO_MODE_VGA:
			m_enStreamMode[0] = STREAM_MODE_VIDEO_VGA;
			break;
		case VIDEO_MODE_D1:
			m_enStreamMode[0] = STREAM_MODE_VIDEO_D1;
			break;
		case VIDEO_MODE_720P:
			m_enStreamMode[0] = STREAM_MODE_VIDEO_720P;
			break;
		default:
			fprintf(stderr, "main stream %d mode unsupport\n", stSInfo.stServerInfo.enMainStream);
			m_enStreamMode[0] = STREAM_MODE_MAX;
			break;
		}
	}

	if (strlen(stSInfo.stServerInfo.strSubAddr)) {
		m_nStreamCnt += 1;
		strncpy(m_strServerStreamName[1], stSInfo.stServerInfo.strSubAddr, MAX_STREAM_NAME);
		
		m_nFps[1] = stSInfo.stServerInfo.nSubFps;
		switch(stSInfo.stServerInfo.enSubStream) {
		case VIDEO_MODE_QVGA:
			m_enStreamMode[1] = STREAM_MODE_VIDEO_QVGA;
			break;
		case VIDEO_MODE_VGA:
			m_enStreamMode[1] = STREAM_MODE_VIDEO_VGA;
			break;
		case VIDEO_MODE_D1:
			m_enStreamMode[1] = STREAM_MODE_VIDEO_D1;
			break;
		case VIDEO_MODE_720P:
			m_enStreamMode[1] = STREAM_MODE_VIDEO_720P;
			break;
		default:
			fprintf(stderr, "sub stream %d mode unsupport\n", stSInfo.stServerInfo.enSubStream);
			m_enStreamMode[1] = STREAM_MODE_MAX;
			break;
		}
	}

	m_nStreamPort = stSInfo.stServerInfo.nRtspPort;
	m_nCommandPort = stSInfo.stServerInfo.nCommandPort;
	m_stImageSet = stSInfo.stServerInfo.stImageSet;

	m_bIsServerRespond = TRUE;

	return 0;
}

int CServerSearch::Server::ServerReturnInfo(RETINFO & stRetInfo)
{
	CAutoLock lock(&m_cs4SRet);
	if (m_pServerRetCB)
		m_pServerRetCB((IServer*)this, &stRetInfo, m_pServerRetCBP);

	return 0;
}

int CServerSearch::Server::SetServerRetCallBack(SERVERRETCB pServerRetCB, void * pParam)
{
	CAutoLock lock(&m_cs4SRet);
	m_pServerRetCB = pServerRetCB;
	m_pServerRetCBP = pParam;

	return 0;
}