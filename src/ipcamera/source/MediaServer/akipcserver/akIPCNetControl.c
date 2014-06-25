#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "akIPCNetControl.h"
#include "muxer.h"
#include "AkMotionDetect.h"
#include "AkAlsaHardware.h"
#include "akaudioDecode.h"
#include "sdcodec.h"
#include "SDcard.h"
#include "camera.h"
#include "AkRecordManager.h"
#include "PTZControl.h"
#include "audio.h"
#include "inisetting.h"
#include "sdfilter.h"

#define BUFLEN 1024

#define UDPPORT 8045
#define TCPLISTENPORT 8046
#define AUDIOLISTENPORT 7710
#define MAXCLIENT 5 + 1
#define RATIO_NUM				65
#define DEF_RATIO	1000
int g_InitMotion = 0;
extern init_parse 		parse;
extern AkAlsaHardware	stAlsaModule;
static alsa_handle_t	stAlsaHandle;
static media_info 		media;
static int 				audio_fd = 0;
static int				thread_num = 0;
static int 				deocde_exit = 0;
static int 				audio_write_exit = 0;
static int				cmd_fd;

MOTION_DETECT 			g_motion_detect;




static int parseAndHandleCMD(int* fd, struct sockaddr* addr, void* buf, unsigned int nlen);
static int handleGetSrvInfo(int fd, struct sockaddr* addr, TIME_INFO* tInfo);
static int handleGetRecordFile();
static int handleGetPriArea(int fd);
static int handleGetMotionDetectAreas(int fd);
static int handleGetIspInfo_awb(int fd);

static int handleGetFilesList(int fd);
static int handleOpenRecord();
static int handleOpenTalk(AUDIOPARAMETER * audiopara, int fd);
static int handleOpenTakePic();
static int handleOpenMotionDetect();
static int handleSetImage(IMAGE_SET* imgSet);
static int handleSetIsp(int fd, unsigned char *pbuf, int nlen);
static int handleSetPrivacyArea(int fd, PRIVACY_AREA* priArea);
static int handleSetZoom(ZOOM* zm);
static int handleSetVolume(VOLUME* vol);
static int handleSetMovementType(CAMERA_MOVEMENT_TYPE* pmovement);
static T_S32 OpenSDFilter( int nChannels, int nActuallySR, int nSampleRate );

static void* process_thread(void* param);
void* audio_rec_thread(void* param);
void* audio_dec_thread(void* param);
void *audio_write_thread(void *param);
extern T_S32 Openaec( void );
extern T_S32 Closeaec( void );


static SERVER_INFO srvInfoSave;
//static IMAGE_SET imgSetSave;
PRIVACY_AREA priAreaSave[4];

typedef struct
{
	int fd;
	struct sockaddr_in addr;
}CLIENT_INFO_T;
static CLIENT_INFO_T clients[MAXCLIENT];
static int conn_amount;    // current connection amount
static int startflag = 0;

void auto_record_file(){
    printf("[###]aaaaauto reeeeeeeeeeecord....\n");
    handleOpenRecord();
}

int startNetCtlServer(NetCtlSrvPar* ncsp)
{
	int i;
	int fd;
	int ret;

	if(ncsp == NULL)
		return -1;
	for(i = 0; i < MAXCLIENT; i ++)
	{
		memset(clients + i, 0, sizeof(CLIENT_INFO_T));
	}
#if 1
	for(i = 0; i < 4; i ++)
	{
		//memset(priAreaSave + i, 0, sizeof(PRIVACY_AREA));
		priAreaSave[i].nNumber = i + 1;
	}
#endif	
	if( IniSetting_aki() < 0 )
	{
		printf("Init OCC fail \n");
		return -1;
	}
	
	struct isp_info *isp = IniSetting_GetispInfo();
	if(isp == NULL)
		printf("read isp err \n");

	memset(&srvInfoSave, 0, sizeof(srvInfoSave));
	strcpy(srvInfoSave.strDeviceID, ncsp->strDeviceID);
	srvInfoSave.nRtspPort = ncsp->nRtspPort;
	srvInfoSave.nCommandPort= TCPLISTENPORT;
	strcpy(srvInfoSave.strMainAddr, ncsp->strStreamName1);
	strcpy(srvInfoSave.strSubAddr, ncsp->strStreamName2);
	srvInfoSave.enMainStream = ncsp->vm1;
	srvInfoSave.enSubStream = ncsp->vm2;
	srvInfoSave.nMainFps = ncsp->nMainFps;
	srvInfoSave.nSubFps = ncsp->nSubFps;
	srvInfoSave.stImageSet.nContrast = atoi(isp->nContrast);
	srvInfoSave.stImageSet.nSaturation = atoi(isp->nSaturation);
	srvInfoSave.stImageSet.nBrightness = atoi(isp->nBrightness);

	IniSetting_akidestroy();
	
	pthread_attr_t SchedAttr;
	pthread_t ntid;
	struct sched_param	SchedParam;
	memset(&SchedAttr, 0, sizeof(pthread_attr_t));
	memset(&SchedParam, 0, sizeof(SchedParam));					
	pthread_attr_init( &SchedAttr );				
	SchedParam.sched_priority = 60;	
	pthread_attr_setschedparam( &SchedAttr, &SchedParam );
	pthread_attr_setschedpolicy( &SchedAttr, SCHED_RR );
	if ( ( ret = pthread_create( &ntid, &SchedAttr, process_thread, (void*)&fd) ) != 0 ) {
		pthread_attr_destroy(&SchedAttr);
		printf( "unable to create a thread for read data ret = %d!\n", ret );
		return -1;	
	}
#if 1
	if ( ( ret = pthread_create( &ntid, &SchedAttr, audio_rec_thread, (void*)&fd) ) != 0 ) {
		pthread_attr_destroy(&SchedAttr);
		printf( "unable to create a thread for read data ret = %d!\n", ret );
		return -1;	
	}

	pthread_create( &ntid, &SchedAttr, audio_write_thread, NULL);

	pthread_attr_destroy(&SchedAttr);

	media.m_nChannels = 0;
#endif
	return 0;
}


int parseAndHandleCMD(int* fd, struct sockaddr* addr, void* buf, unsigned int nlen)
{
	if(buf == NULL || nlen > BUFLEN)
	{
		printf("parseAndHandleCmd buf===NULL  || nlen !!\n");
		return -1;
	}
	unsigned char* pbuf = buf;
	int structsize = 0;
	
	structsize = sizeof(SYSTEM_HEADER);
	printf("structsize:%d\n", structsize);
	if(nlen < structsize)
	{
		printf("system header error\n");
		return -1;	
	}
	
	/*SYSTEM_HEADER* psysHeader = (SYSTEM_HEADER*)pbuf*/;
	pbuf += structsize;
	nlen -= structsize;
	
	structsize = sizeof(COMMAND_HEADER);
	printf("cmd structsize:%d\n", structsize);
	if(nlen < structsize)
	{
		printf("command header error\n");
		return -1;	
	}
	COMMAND_HEADER* pcmdHeader = (COMMAND_HEADER*)pbuf;
	pbuf += structsize;
	nlen -= structsize;
	
	switch(pcmdHeader->CommandType)
	{
		case COMM_TYPE_OPEN_SERVICE:
		switch(pcmdHeader->subCommandType)
		{
			case OPEN_COMM_RECODE:
				handleOpenRecord();
				break;
			case OPEN_COMM_TALK:
				structsize = sizeof(AUDIOPARAMETER);
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("set Talk error\n");
					return -1;	
				}
				AUDIOPARAMETER* TalkSet = (AUDIOPARAMETER*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				handleOpenTalk(TalkSet, *fd);
				break;
			case OPEN_COMM_TAKE_PIC:
				handleOpenTakePic();
				break;
			case OPEN_COMM_MOTION_DETECT:
				structsize = sizeof(MOTION_DETECT);
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("set MotionSet error\n");
					return -1;	
				}
				MOTION_DETECT* MotionSet = (MOTION_DETECT*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				handleOpenMotionDetect(MotionSet);
				break;
			default:
				printf("comm type open service error\n");
				return -1;
		}
		break;
		
		case COMM_TYPE_SET_PARAMETER:
		switch(pcmdHeader->subCommandType)
		{
			case SET_COMM_IMAGE:
				structsize = sizeof(IMAGE_SET);
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("set image net recv error\n");
					return -1;	
				}
				IMAGE_SET* pimgSet = (IMAGE_SET*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				handleSetImage(pimgSet);
				break;
			case SET_COMM_ISP:
				
				handleSetIsp(*fd, pbuf, pcmdHeader->nLen);
				break;
			case SET_COMM_PRIVACY_AREA:
				structsize = sizeof(PRIVACY_AREA);
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("set privacyarea net recv error\n");
					return -1;	
				}
				PRIVACY_AREA* pPriArea = (PRIVACY_AREA*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				handleSetPrivacyArea(*fd, pPriArea);
				break;
			
			case SET_COMM_ZOOM:
				structsize = sizeof(ZOOM);
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("set zoom net recv error\n");
					return -1;	
				}
				ZOOM* pZoom = (ZOOM*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				handleSetZoom(pZoom);
				break;
			case SET_COMM_VOLUME:
				structsize = sizeof(VOLUME);
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("set volume recverror\n");
					return -1;	
				}
				VOLUME* pVolume = (VOLUME*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				handleSetVolume(pVolume);
				break;
			case SET_COMM_CAMERA_MOVEMENT:
				structsize = /*sizeof(CAMERA_MOVEMENT_TYPE)*/1;
				//printf("%u, %u, %u\n", structsize, nlen, pcmdHeader->nLen);
					//													4        5      1
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("set camera movement recv error\n");
					return -1;	
				}
				
				unsigned char utmp = *pbuf;
				CAMERA_MOVEMENT_TYPE movement= (CAMERA_MOVEMENT_TYPE)utmp;
				//CAMERA_MOVEMENT_TYPE* pmovement = (CAMERA_MOVEMENT_TYPE*)(unsigned char*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				handleSetMovementType(&movement);
				break;
			default:
				printf("comm type set parameter error\n");
				return -1;	
		}
		break;
		case COMM_TYPE_GET_INFO:
		switch(pcmdHeader->subCommandType)
		{
			case GET_COMM_RECODE_FILE:
				handleGetRecordFile();
				break;
			case GET_COMM_SERVER_INFO:
				structsize = sizeof(TIME_INFO);
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("get server info recv error\n");
					return -1;	
				}
				TIME_INFO* pTimeInfo = (TIME_INFO*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				
				handleGetSrvInfo(*fd, addr, pTimeInfo);
				break;
			case GET_COMM_PRIVACY_AREA:
				handleGetPriArea(*fd);
				break;
			case GET_COMM_IMAGE_SET:
				break;
			case GET_COMM_FILES_LIST:
				handleGetFilesList(*fd);
				break;
			case GET_COMM_MOTION_DETECT_AREAS:
				handleGetMotionDetectAreas( *fd );
				break;
			case GET_COMM_ISP_INFO:
				structsize = sizeof(int);
				if(nlen < structsize || pcmdHeader->nLen != structsize)
				{
					printf("get isp info rece error\n");
					return -1;		
				}
				int nIspType = *(int*)pbuf;
				pbuf += structsize;
				nlen -= structsize;
				if(nIspType == 0x67)
					handleGetIspInfo_awb(*fd);
				break;
			default:
				printf("comm type get info error\n");
				return -1;	
		}
		break;
		default:
			printf("command type error\n");
			return -1;	
	}
	
	return 0;
}

void* process_thread(void* param)
{
	startflag = 1;
	int sock_fd, new_fd, udp_fd;  // listen on sock_fd, new connection on new_fd,receieve broasdcast packet on udp_fd
	struct sockaddr_in server_addr;    // server address information
	struct sockaddr_in client_addr; // connector's address information
	socklen_t sin_size;
	int yes = 1;
	char buf[BUFLEN];
	int ret;
	int i;

	//udp listen broadcast 
	udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in dest_addr;
	bzero(&dest_addr, sizeof(dest_addr));

	socklen_t addrlen;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(UDPPORT);
	dest_addr.sin_addr.s_addr = INADDR_ANY;

	bind(udp_fd, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

	addrlen = sizeof(struct sockaddr);


	//tcp listen client
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		return NULL;
	}

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
	{
		perror("setsockopt");
		return NULL;
	}
    
	server_addr.sin_family = AF_INET;         // host byte order
	server_addr.sin_port = htons(TCPLISTENPORT);     // short, network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

	if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
	{
		perror("bind");
		return NULL;
	}

	if (listen(sock_fd, MAXCLIENT) == -1) 
	{
		perror("listen");
		return NULL;
	}

	printf("listen port %d\n", TCPLISTENPORT);

	fd_set fdsr;
	int maxsock;
	struct timeval tv;

	conn_amount = 0;
	sin_size = sizeof(client_addr);
	maxsock = (sock_fd > udp_fd) ? sock_fd:udp_fd;
	while (1) 
	{
		if(startflag == 0)
			break;
		// initialize file descriptor set
		FD_ZERO(&fdsr);
		FD_SET(sock_fd, &fdsr);
		FD_SET(udp_fd, &fdsr);

		// timeout setting
		tv.tv_sec = 3;
		tv.tv_usec = 0;

		// add active connection to fd set
		for (i = 0; i < MAXCLIENT; i++) 
		{
			if (clients[i].fd != 0) 
			{
				FD_SET(clients[i].fd, &fdsr);
			}
		}

		ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
		if (ret < 0) 
		{
			perror("select");
			break;
		} 
		else if (ret == 0) 
		{
			continue;
		}

		// check every fd in the set
		for (i = 0; i < MAXCLIENT; i++) 
		{
			if (FD_ISSET(clients[i].fd, &fdsr))
			{
				ret = recv(clients[i].fd, buf, sizeof(buf), 0);
				if (ret <= 0) 
				{        
					// client close
					printf("client[%d] close\n", i);
					close(clients[i].fd);
					FD_CLR(clients[i].fd, &fdsr);
					clients[i].fd = 0;
					conn_amount --;
				} 
				else 
				{   // receive data
					if (ret < BUFLEN)
						memset(&buf[ret], '\0', 1);
					printf("tcp client[%d] send:%s\n", i, buf);
					if(strcmp(buf, "bye") == 0)
					{
						startflag = 0;
					}
					else
					{
						//printf("recv data = %d \n", ret);
						parseAndHandleCMD(&clients[i].fd, NULL, buf, ret);
					}
				}
			}
		}

    // check whether a new connection comes
		if (FD_ISSET(sock_fd, &fdsr)) 
		{
			new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
			if (new_fd <= 0) 
			{
				perror("accept");
				continue;
			}
			// add to fd queue
			if (conn_amount < MAXCLIENT) 
			{
				for(i = 0;i < MAXCLIENT;i++)
				{
					if(clients[i].fd == 0)
					{
						clients[i].fd = new_fd;
						memcpy(&clients[i].addr, &client_addr, sizeof(client_addr));
						break;
					}		
				}
				conn_amount++;

				printf("new connection client[%d] %s:%u\n", i,
		                (char*)inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		
				if (new_fd > maxsock)
					maxsock = new_fd;
			}
			else 
			{
				printf("*****max connections arrive, refuse\n");
				//send(new_fd, "bye", 4, 0);
				close(new_fd);
				new_fd = 0;
				continue;
			}
		}

		//check whether a udp broadcast packet come
		if (FD_ISSET(udp_fd, &fdsr))
		{
			ret = recvfrom(udp_fd, buf, BUFLEN, 0, (struct sockaddr*)&dest_addr, &addrlen);
		
			if(ret < 0)
			{
				perror("recvfrom");
				break;
			}
			else
			{
				if (ret < BUFLEN)
					memset(&buf[ret], '\0', 1);
				printf("udp client[] send:%s\n",  buf);
				if(strcmp(buf, "bye") == 0)
					startflag = 0;
				else
				{
					parseAndHandleCMD(&udp_fd, (struct sockaddr*)&dest_addr, buf, ret);
				}
			}
		}
	}

	// close other connections
	for (i = 0; i < MAXCLIENT; i++) 
	{
		if (clients[i].fd != 0) 
		{
			close(clients[i].fd);
			clients[i].fd = 0;
		}
	}

	close(sock_fd);
	sock_fd = 0;
	close(udp_fd);
	udp_fd = 0;

	printf("process_thread end\n");
	return 0;
}

int handleGetSrvInfo(int fd, struct sockaddr* addr, TIME_INFO* tInfo)
{
	printf("%s\n", __func__);
	printf("datetime:%u-%u-%u %u:%u:%u\n", tInfo->nYear + 1970, tInfo->nMon, tInfo->nDay, tInfo->nHour, tInfo->nMin, tInfo->nSecond);

	static int nForSetOnce = 0;
	if(nForSetOnce == 0)
	{
		char  timebuf[128];
		memset(timebuf, 0, 128);
		sprintf(timebuf, "date -s \"%d-%d-%d %d:%d:%d\"", tInfo->nYear + 1970, tInfo->nMon, tInfo->nDay,
					tInfo->nHour, tInfo->nMin, tInfo->nSecond);
		system(timebuf);
		nForSetOnce = 1;
	}

	SYSTEM_HEADER sysHeader;
	CON_SYSTEM_TAG(SYSTEM_TAG, &sysHeader.nSystemTag);
	sysHeader.nCommandCnt = 1;
	sysHeader.nLen = sizeof(SERVER_INFO) + sizeof(COMMAND_HEADER);
	
	COMMAND_HEADER cmdHeader;
	cmdHeader.CommandType = COMM_TYPE_GET_INFO;
	cmdHeader.subCommandType = GET_COMM_SERVER_INFO;
	cmdHeader.nLen = sizeof(SERVER_INFO);
	unsigned char buf[BUFLEN];
	int nSend = 0;
	memset(buf, 0, BUFLEN);
	memcpy(buf, &sysHeader, sizeof(sysHeader));
	nSend += sizeof(sysHeader);
	memcpy(buf + nSend, &cmdHeader, sizeof(cmdHeader));
	nSend += sizeof(cmdHeader);


	if( IniSetting_aki() < 0 )
	{
		printf("Init OCC fail \n");
		return -1;
	}
	
	struct isp_info *isp = IniSetting_GetispInfo();
	if(isp == NULL)
		printf("read isp err \n");


	int i = -1;
	if(conn_amount == MAXCLIENT)
	{
		//达到了最大连接限制
		for(i = 0; i < MAXCLIENT; i ++)
		{
			if(clients[i].fd == fd)
			{
				break;	
			}	
		}
		
		srvInfoSave.stImageSet.nContrast = 255;
		srvInfoSave.stImageSet.nSaturation = 255;
		srvInfoSave.stImageSet.nBrightness = 255;
		srvInfoSave.stImageSet.nReserve	= 255;
	}
	else
	{
		srvInfoSave.stImageSet.nContrast = atoi(isp->nContrast);
		srvInfoSave.stImageSet.nSaturation = atoi(isp->nSaturation);
		srvInfoSave.stImageSet.nBrightness = atoi(isp->nBrightness);
	}
	IniSetting_akidestroy();

	memcpy(buf + nSend, &srvInfoSave, sizeof(srvInfoSave));
	nSend += sizeof(srvInfoSave);

	int ret;
	unsigned char* pbuf = buf;
	while(nSend > 0)
	{
		ret = sendto(fd, pbuf, nSend, 0, addr, sizeof(struct sockaddr));
		if(ret < 0)
		{
			printf("send error\n");
			return -1;
		}
			
		nSend -= ret;
		pbuf += ret;
	}
		
	if(i >=0 && i < MAXCLIENT)
	{
		close(clients[i].fd);
		clients[i].fd = 0;
		conn_amount --;	
		printf("max connections arrive, refuse\n");
	}
	return 0;
}
int handleGetRecordFile()
{
	printf("%s\n", __func__);
	return 0;
}

int handleGetMotionDetectAreas(int fd)
{
	printf("%s\n", __func__);
	SYSTEM_HEADER sysHeader;
	CON_SYSTEM_TAG(SYSTEM_TAG, &sysHeader.nSystemTag);
	sysHeader.nCommandCnt = 1;
	sysHeader.nLen = sizeof(SERVER_INFO) + sizeof(COMMAND_HEADER);

	COMMAND_HEADER cmdHeader;
	cmdHeader.CommandType = COMM_TYPE_GET_INFO;
	cmdHeader.subCommandType = GET_COMM_MOTION_DETECT_AREAS;
	cmdHeader.nLen = sizeof(MOTION_DETECT);
	unsigned char buf[BUFLEN];
	int nSend = 0;

	memset(buf, 0, BUFLEN);
	memcpy(buf, &sysHeader, sizeof(sysHeader));
	nSend += sizeof(sysHeader);
	memcpy(buf + nSend, &cmdHeader, sizeof(cmdHeader));
	nSend += sizeof(cmdHeader);
	memcpy(buf + nSend, &g_motion_detect, sizeof(g_motion_detect));
	nSend += sizeof(g_motion_detect);


	int ret;
	unsigned char* pbuf = buf;
	while(nSend > 0)
	{
		ret = send(fd, pbuf, nSend, 0);
		if(ret < 0)
		{
			printf("send error\n");
			return -1;
		}
			
		nSend -= ret;
		pbuf += ret;
	}
	return 0;

	
}
int handleGetPriArea(int fd)
{
	printf("%s\n", __func__);
	SYSTEM_HEADER sysHeader;
	CON_SYSTEM_TAG(SYSTEM_TAG, &sysHeader.nSystemTag);
	sysHeader.nCommandCnt = 4;
	sysHeader.nLen = sizeof(PRIVACY_AREA) + sizeof(COMMAND_HEADER) * sysHeader.nCommandCnt;
	
	COMMAND_HEADER cmdHeader;
	cmdHeader.CommandType = COMM_TYPE_GET_INFO;
	cmdHeader.subCommandType = GET_COMM_PRIVACY_AREA;
	cmdHeader.nLen = sizeof(PRIVACY_AREA);
	unsigned char buf[BUFLEN];
	
	int nSend = 0;
	memset(buf, 0, BUFLEN);
	
	memcpy(buf, &sysHeader, sizeof(sysHeader));
	nSend += sizeof(sysHeader);
	
	for(int i = 0; i < sysHeader.nCommandCnt; i ++)
	{
		memcpy(buf + nSend, &cmdHeader, sizeof(cmdHeader));
		nSend += sizeof(cmdHeader);
		memcpy(buf + nSend, priAreaSave + i, sizeof(srvInfoSave));
		nSend += sizeof(PRIVACY_AREA);
	}
	
	int ret;
	unsigned char* pbuf = buf;
	while(nSend > 0)
	{
		ret = send(fd, pbuf, nSend, 0);
		if(ret < 0)
		{
			printf("send error\n");
			return -1;
		}
			
		nSend -= ret;
		pbuf += ret;
	}
	return 0;
}

static int handleGetIspInfo_awb(int fd)
{
	int awbbuflen  = 256;
	unsigned char awbbuf[awbbuflen];
	memset(awbbuf, 0, awbbuflen);
	if(GetIsp_awb(awbbuf, &awbbuflen) == 0)
	{
		SYSTEM_HEADER sysHeader;
		CON_SYSTEM_TAG(SYSTEM_TAG, &sysHeader.nSystemTag);
		sysHeader.nCommandCnt = 1;
		sysHeader.nLen = awbbuflen + sizeof(COMMAND_HEADER) * sysHeader.nCommandCnt;
	
		COMMAND_HEADER cmdHeader;
		cmdHeader.CommandType = COMM_TYPE_GET_INFO;
		cmdHeader.subCommandType = GET_COMM_ISP_INFO;
		cmdHeader.nLen = awbbuflen;
		unsigned char buf[BUFLEN];
	
		int nSend = 0;
		memset(buf, 0, BUFLEN);
	
		memcpy(buf, &sysHeader, sizeof(sysHeader));
		nSend += sizeof(sysHeader);
			
		memcpy(buf + nSend, &cmdHeader, sizeof(cmdHeader));
		nSend += sizeof(cmdHeader);
				
		memcpy(buf + nSend, awbbuf, awbbuflen);
		nSend += awbbuflen;
	
		int ret;
		unsigned char* pbuf = buf;
		while(nSend > 0)
		{
			ret = send(fd, pbuf, nSend, 0);
			if(ret < 0)
			{
				printf("send error\n");
				return -1;
			}
			
			nSend -= ret;
			pbuf += ret;
		}
		return 0;
	}
	return 0;
}

static int sendFileList(int fd, unsigned char* filebuf, int buflen)
{
	SYSTEM_HEADER sysHeader;
	CON_SYSTEM_TAG(SYSTEM_TAG, &sysHeader.nSystemTag);
	sysHeader.nCommandCnt = 1;
	sysHeader.nLen = buflen + sizeof(COMMAND_HEADER) * sysHeader.nCommandCnt;
	
	COMMAND_HEADER cmdHeader;
	cmdHeader.CommandType = COMM_TYPE_GET_INFO;
	cmdHeader.subCommandType = GET_COMM_FILES_LIST;
	cmdHeader.nLen = buflen;
	
	unsigned char buf[BUFLEN];
	int nSend = 0;
	memset(buf, 0, BUFLEN);
	
	memcpy(buf, &sysHeader, sizeof(sysHeader));
	nSend += sizeof(sysHeader);

	memcpy(buf + nSend, &cmdHeader, sizeof(cmdHeader));
	nSend += sizeof(cmdHeader);

	memcpy(buf + nSend, filebuf, buflen);
	nSend += buflen;
	
	int ret;
	unsigned char* pbuf = buf;
	while(nSend > 0)
	{
		ret = send(fd, pbuf, nSend, 0);
		if(ret < 0)
		{
			printf("send error\n");
			return -1;
		}
			
		nSend -= ret;
		pbuf += ret;
	}
	
	return 0;
}

int handleGetFilesList(int fd)
{
	printf("%s\n", __func__);
	DIR 			*dirp = NULL;
	struct dirent 	*direntp = NULL; 
	char* pstrRecPath = "/mnt";
	char* pstrRecPathT = "mnt";
	if( ( dirp = opendir(pstrRecPath) ) == NULL ) {  
		printf( "Open Directory %s Error: %s\n", pstrRecPath, strerror(errno) );
		return -1;
	}

	unsigned char filenamebuf[BUFLEN];
	const int maxfilecount = 20;
	int filecount = 0;
	int buflen = 0;
	
	sprintf((char*)filenamebuf, "[%s];", pstrRecPathT);
	buflen = strlen((const char*)filenamebuf);
	while ( ( direntp = readdir( dirp ) ) != NULL )
	{
		if ( direntp->d_name == NULL ) {
			continue;
		}
		
		if ( !( strcmp( direntp->d_name, "." ) ) || 
			 !( strcmp( direntp->d_name, ".." ) )/* ||
			 !( strcmp( direntp->d_name, SIGN_FILE_NAME ) )*/ ) {
			continue;
		}

		if ( !(IsOurCycFile( direntp->d_name )) ) {
			continue;
		}
		strcpy((char*)filenamebuf + buflen, direntp->d_name);
		buflen += strlen(direntp->d_name);
		filenamebuf[buflen] = ';';
		buflen ++;
		
		filecount ++;
		if(filecount == maxfilecount)
		{
			filecount = 0;
			if(sendFileList(fd, filenamebuf, buflen) < 0)
			{
				break;	
			}
			sprintf((char*)filenamebuf, "[%s];", pstrRecPathT);
			buflen = strlen((const char*)filenamebuf);
		}
	}
	
	if(filecount != 0)
	{
		sendFileList(fd, filenamebuf, buflen);
	}

	if ( closedir( dirp ) != 0 ) {
		printf( "close Directory %s Error:%s\n ", pstrRecPath, strerror(errno) );
	}
	return 0;
}


int handleCloseTalk(int fd)
{
	printf("Close Talk fd = %d \n", fd);
	cmd_fd = 0;
	AUDIOPARAMETER audiopara;
	audiopara.nChannels = 0;
	audiopara.nEncodeType = 0;
	audiopara.nSampleFmt = 0;
	audiopara.nSampleRateType = 0;

	COMMAND_HEADER cmdHeader;
	cmdHeader.CommandType = COMM_TYPE_OPEN_SERVICE;
	cmdHeader.subCommandType = OPEN_COMM_TALK;
	cmdHeader.nLen = sizeof(AUDIOPARAMETER);

	SYSTEM_HEADER sysHeader;
	CON_SYSTEM_TAG(SYSTEM_TAG, &sysHeader.nSystemTag);
	sysHeader.nCommandCnt = 1;
	sysHeader.nLen = sizeof(AUDIOPARAMETER) + sizeof(COMMAND_HEADER) * sysHeader.nCommandCnt;


	unsigned char buf[BUFLEN];
	
	int nSend = 0;
	memset(buf, 0, BUFLEN);
	
	memcpy(buf, &sysHeader, sizeof(sysHeader));
	nSend += sizeof(sysHeader);

	memcpy(buf+nSend, &cmdHeader, sizeof(cmdHeader));
	nSend += sizeof(cmdHeader);

	memcpy(buf+nSend, &audiopara, sizeof(audiopara));
	nSend += sizeof(audiopara);

	int ret;
	unsigned char* pbuf = buf;
	while(nSend > 0)
	{
		ret = send(fd, pbuf, nSend, 0);
		if(ret < 0)
		{
			printf("send error\n");
			return -1;
		}
			
		nSend -= ret;
		pbuf += ret;
	}
	return 0;
	
}

//extern T_MUX_INPUT mux_input;
extern int Recordflag; 

int handleOpenRecord()
{
	printf("%s\n", __func__);
	
	if (Recordflag == 0)
	{
		if (check_sdcard() < 0)
		{
			printf("no sd card \n");
			return -1;
		}
		
		start_record(1);
	}
	
	return 0;
}

int handleOpenTalk(AUDIOPARAMETER * audiopara, int fd)
{
	//int ret;
	int SampleRate, Channels, Bits, type;
	printf("%s\n", __func__);
	if(thread_num > 0 && audiopara->nChannels == 0)
	{
		Closeaec();
		printf("@@@@encode type =%d, channel =%d sample =%d samplefmt =%d\n", 
			audiopara->nEncodeType, audiopara->nChannels, audiopara->nSampleRateType, audiopara->nSampleFmt);
		deocde_exit	= 1;
		audio_fd = 0;
		return 0;
	}
	printf("get talk fd = %d \n", fd);
	if(cmd_fd != 0)
	{
		usleep(10000);
	}
	cmd_fd = fd;
	printf("encode type =%d, channel =%d sample =%d samplefmt =%d\n", 
			audiopara->nEncodeType, audiopara->nChannels, audiopara->nSampleRateType, audiopara->nSampleFmt);
	switch(audiopara->nEncodeType)
	{
		case AUDIO_EN_AAC:
			type = _SD_MEDIA_TYPE_AAC;
			break;
		default:
			type = _SD_MEDIA_TYPE_AAC;
			
	}
	Channels = audiopara->nChannels;
	switch(audiopara->nSampleRateType)
	{
		case AUDIO_SAMPLERATE_8K:
			SampleRate = 8000;
			break;
		case AUDIO_SAMPLERATE_11K:
			SampleRate = 11025;
			break;
		default:
			SampleRate = 8000;
	}

	switch(audiopara->nSampleFmt)
	{
		case AUDIO_SAMPLE_FMT_U8:
			Bits = 8;
			break;
		case AUDIO_SAMPLE_FMT_S16:
			Bits = 16;
			break;
		default:
			Bits = 16;
	}
	media.m_AudioType = type;
	media.m_nChannels = Channels;
	media.m_nSamplesPerSec = SampleRate;
	media.m_wBitsPerSample = Bits;
	Openaec();
	return 0;
}
extern int pic;
int handleOpenTakePic()
{
	printf("%s\n", __func__);
	pic = 1;
	return 0;
}


void InitMotionDetect( void )
{
	uint64_t mdareas = 0;
	uint32_t high = 0;
	uint32_t low = 0;
	if( IniSetting_aki() < 0 )
	{
		printf("Init OCC fail \n");
		return;
	}
	
	struct md_info * md = IniSetting_GetmdInfo();
	if(NULL == md)
	{
		return;
	}
	high = atoi(md->matrix_high);
	low =  atoi(md->matrix_low);
	mdareas =  mdareas | high;
	mdareas = (mdareas <<32) | low;
	g_motion_detect.nMDAreas = mdareas;
	g_motion_detect.nSensitivity = atoi(md->nSensitivity);
	g_motion_detect.nReserve = 0;

//	IniSetting_akidestroy();
	
	if( 1 == atoi(md->on))
	{
		handleOpenMotionDetect( &g_motion_detect );
	}
//	printf("%lld \n", atoll(md->matrix));
//	printf("%d \n", atoi(md->nSensitivity));
//	printf("%d \n", atoi(md->on));

	
}
	

int handleOpenMotionDetect(MOTION_DETECT* MotionSet)
{
	T_U16 ratios[RATIO_NUM]={250};
	T_U16 ratio;
	uint32_t matrix_low, matrix_high;
	printf("%s\n", __func__);
	printf("sensitivity: %d\nZONE: ", MotionSet->nSensitivity);
	memcpy( &g_motion_detect, MotionSet, sizeof(MOTION_DETECT));
	uint64_t matrix = MotionSet->nMDAreas;
	matrix_low = matrix;
	matrix_high = matrix>>32;
	int on;


	if( IniSetting_aki() < 0 )
	{
		printf("Init OCC fail \n");
		return -1;
	}
	struct md_info * md = IniSetting_GetmdInfo();
	if(NULL == md)
	{
		return -1;
	}

	if ((matrix_low | matrix_high) == 0)
	{
		 on = 0;
	}
	else
		 on = 1;
	
	sprintf(md->matrix_high, "%d", matrix_high);
	printf("%s\n", md->matrix_high);
	sprintf(md->matrix_low, "%d", matrix_low);
	printf("%s\n", md->matrix_low);
	sprintf(md->nSensitivity, "%d", ( int )MotionSet->nSensitivity);
	printf("%d\n", MotionSet->nSensitivity);
	sprintf(md->on, "%d", on);
	printf("%s\n", md->on);
	IniSetting_akisave();
	
//	IniSetting_akidestroy();

	if ((matrix_low | matrix_high) == 0)
	{
		g_InitMotion = 0;
		usleep(35000);
		CloseMotionDetect();
		return 0;
	}

#if 0
	if(1 == g_InitMotion)
	{
		g_InitMotion = 0;
		usleep(35000);
		CloseMotionDetect();
		usleep(35000);
	}
#endif	

	
	switch (MotionSet->nSensitivity)
	{
	case 6:
		ratio = 1;
		break;

	case 5:
		ratio = 50;
		break;

	case 4:
		ratio = 120;
		break;

	case 3:
		ratio = 200;
		break;

	case 2:
		ratio = 300;
		break;

	case 1:
		ratio = 500;
		break;

	default:
		ratio = DEF_RATIO;
		break;

	}

	printf("0x%x, ", matrix_low);
	printf("0x%x\n\n", matrix_high);
	

	
	for (int i = 1; i < RATIO_NUM; i++)
	{
		if( matrix & 0x1 )
		{
			ratios[i] = ratio;
			// printf("#");
		}
		else
		{
			ratios[i] = DEF_RATIO;
			// printf(".");
		}
		
		// if (i%8 == 0)
		// 	printf("\n");
		
		matrix >>= 1;
	}
	
	OpenMotionDetect(parse.width, parse.height, ratios);
	g_InitMotion = 1;
	
	return 0;
}

int handleSetImage(IMAGE_SET* imgSet)
{
	printf("%s\n", __func__);
	if(imgSet == NULL)
		return -1;
	printf("contrast:%u,saturation:%u,brightness:%u\n",imgSet->nContrast, imgSet->nSaturation, imgSet->nBrightness);
	SetGAMMA(imgSet->nContrast);
	SetSATURATION(imgSet->nSaturation);
	SetBrightness(imgSet->nBrightness);

	if( IniSetting_aki() < 0 )
	{
		printf("Init OCC fail \n");
		return -1;
	}
	struct isp_info * isp = IniSetting_GetispInfo();
	if(NULL == isp)
	{
		return -1;
	}

	sprintf(isp->nContrast, "%d", imgSet->nContrast);
	printf("%s\n", isp->nContrast);
	sprintf(isp->nSaturation, "%d", imgSet->nSaturation);
	printf("%s\n", isp->nSaturation);
	sprintf(isp->nBrightness, "%d", imgSet->nBrightness);
	printf("%s\n", isp->nBrightness);
	IniSetting_akisave();


	return 0;
}

int handleSetIsp(int fd, unsigned char *pbuf, int nlen)
{
	printf("%s\n", __func__);
	printf("datalneg = %d \n", nlen);
	for(int i = 0; i< nlen; i++)
	{	
		printf(" %d", *(pbuf+i));
	}
	printf("\n");
	SetIsp(pbuf, nlen);
	return 0;
}

int handleSetPrivacyArea(int fd, PRIVACY_AREA* priArea)
{
	printf("%s\n", __func__);
	if(priArea == NULL)
		return -1;
	printf("Num:%u,LTX:%u,LTY:%u,Wid:%u,Hei:%u\n",\
	priArea->nNumber,priArea->nLeftTopX,priArea->nLeftTopY,priArea->nWidth,priArea->nHeight);
	
	if(priArea->nNumber == 0)
	{
		//clear all
	}
	else
	{
		for(int i = 0; i < 4; i ++)
		{
			if(priArea->nNumber == priAreaSave[i].nNumber)
			{
				memcpy(priAreaSave + i, priArea, sizeof(*priArea));
				//then set it
				int endx = priAreaSave[i].nLeftTopX + priAreaSave[i].nWidth;
				int endy = priAreaSave[i].nLeftTopY +priAreaSave[i].nHeight;
				if(priAreaSave[i].nLeftTopX == 0 && 
					priAreaSave[i].nLeftTopY == 0 &&
					priAreaSave[i].nWidth == 0 &&
					priAreaSave[i].nHeight == 0)
				{
					SetOcc(priAreaSave[i].nNumber, priAreaSave[i].nLeftTopX, priAreaSave[i].nLeftTopY, endx, endy, 0);	
					SaveOcc(priAreaSave[i].nNumber, priAreaSave[i].nLeftTopX, priAreaSave[i].nLeftTopY, endx, endy, 0);
				}
				else
				{
					SetOcc(priAreaSave[i].nNumber, priAreaSave[i].nLeftTopX, priAreaSave[i].nLeftTopY, endx, endy, 1);
					SaveOcc(priAreaSave[i].nNumber, priAreaSave[i].nLeftTopX, priAreaSave[i].nLeftTopY, endx, endy, 1);
				}
				break;	
			}	
		}
	}
	return 0;
}


int handleSetZoom(ZOOM* zm)
{
	printf("%s\n", __func__);
	if(zm == NULL)
		return -1;
		
	printf("zoom:%u\n", *zm);
	Set_Zoom( *zm );
	return 0;
}

int handleSetVolume(VOLUME* vol)
{
	printf("%s\n", __func__);
	if(vol == NULL)
		return -1;
	
	printf("volume:%u\n", *vol);
	int volume = GetAudioMicVolume();
	printf( "%d \n", volume );
	if( *vol == 2 )
	{
		volume += 16;
		if(volume <= 80 )
		{
			printf("set the volume %d \n", volume);
			SetAudioMicVolume( volume );
		}
		else
		{
			SetAudioMicVolume( 80 );
		}
	}
	else if( *vol == 1 )
	{
		volume --;
		if( volume >= 0 )
		{
			SetAudioMicVolume( volume );
		}
		else
		{
			SetAudioMicVolume( 0 );
		}
	}
	return 0;
}

int handleSetMovementType(CAMERA_MOVEMENT_TYPE* pmovement)
{
	printf("%s, %d\n", __func__, *pmovement);
	switch(*pmovement)
	{
		case CMT_STEP_UP:
			PTZControlStepUp();
		break;
		case CMT_STEP_DOWN:
			PTZControlStepDown();
		break;
		case CMT_STEP_LEFT:
			PTZControlStepLeft();
		break;
		case CMT_STEP_RIGHT:
			PTZControlStepRight();
		break;
		case CMT_RUN_UP_DOWN:
			PTZControlUpDown();
		break;
		case CMT_RUN_LEFT_RIGHT:
			PTZControlLeftRight();
		break;
		case CMT_RUN_REPOSITION:
			PTZControlRunPosition();
		break;
		case CMT_SET_REPOSITION:
			PTZControlSetPosition();
		break;
		default:
		printf("cmd set movement type error\n");
	}
	return 0;
}
void* audio_rec_thread(void* param)
{
	startflag = 1;
	int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd,receieve broasdcast packet on udp_fd
	struct sockaddr_in server_addr;    // server address information
	struct sockaddr_in client_addr; // connector's address information
	socklen_t sin_size;
    int yes = 1;
    //char buf[BUFLEN];
    int ret;
    //int i;

	//tcp listen client
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
        perror("socket");
        return NULL;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
	{
        perror("setsockopt");
        return NULL;
    }
    
    server_addr.sin_family = AF_INET;         // host byte order
    server_addr.sin_port = htons(AUDIOLISTENPORT);     // short, network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
	{
        perror("bind");
        return NULL;
    }

    if (listen(sock_fd, 2) == -1) 
	{
        perror("listen");
        return NULL;
    }

    printf("listen port %d\n", AUDIOLISTENPORT);

	OpenSDFilter( 1, 8000, 7990 );

    //fd_set fdsr;
    int maxsock;
    //struct timeval tv;
#if 0
	ret = stAlsaModule.OpenDA(&stAlsaHandle, NULL);
	if ( ret < 0 ) {
		printf( "StartRec::can't open AD!\n" );
		return NULL;
	}

	media.m_AudioType = _SD_MEDIA_TYPE_AAC;
	media.m_nChannels = 1;
	media.m_nSamplesPerSec = 8000;
	media.m_wBitsPerSample = 16;

	//ret = stAlsaModule.SetParams(&stAlsaHandle, media.m_nSamplesPerSec, media.m_wBitsPerSample, media.m_nChannels, 0);
	ret = stAlsaModule.SetParams(&stAlsaHandle, 8000, 16,  1, 0);
	if ( ret < 0 ) {
		printf( "StartRec::set audio params failed!\n" );
		return NULL;
	}

	ret = decode_open( &media );
	if( ret < 0)
	{
		printf( "open decode failed \n" );
		return NULL;
	}

#endif
    conn_amount = 0;
    sin_size = sizeof(client_addr);
    maxsock =  sock_fd;
	while (1) 
	{

       	// check whether a new connection comes
#if 1
			printf("accept net \n ");
            new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
            if (new_fd <= 0) 
			{
                perror("accept");
                continue;
            }
			if(thread_num >= 2)
			{
				printf("thread_num >= 2 \n");
				close(new_fd);
				new_fd = 0;
				continue;
			}
			printf("######audio_fd = %d \n", audio_fd);
			if(audio_fd != 0)
			{
				//deocde_exit = 1;
				handleCloseTalk(cmd_fd);
				usleep(10000);
				close(audio_fd);
			}
			audio_fd = new_fd;
			pthread_attr_t SchedAttr;
			//pthread_t ntid;
			struct sched_param	SchedParam;
			memset(&SchedAttr, 0, sizeof(pthread_attr_t));
			memset(&SchedParam, 0, sizeof(SchedParam));					
			pthread_attr_init( &SchedAttr );				
			SchedParam.sched_priority = 60;	
			pthread_attr_setschedparam( &SchedAttr, &SchedParam );
			pthread_attr_setschedpolicy( &SchedAttr, SCHED_RR );
			{
				pthread_t ntid;
				if ( ( ret = pthread_create( &ntid, &SchedAttr, audio_dec_thread, NULL) ) != 0 ) 
				{
					printf( "unable to create a thread for read data ret = %d!\n", ret );
					return /*-1*/ NULL;	
				}
			}
			pthread_attr_destroy(&SchedAttr);
			printf("newfd is %d \n", new_fd);

#endif
		
    }


	close(sock_fd);
	sock_fd = 0;


	printf("process_thread end\n");
    return 0;
}

static T_pVOID					pSdFilter = NULL;

static T_S32 OpenSDFilter( int nChannels, int nActuallySR, int nSampleRate )
{
	T_AUDIO_FILTER_INPUT 	s_ininfo;

	// open
	s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
	s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;

	
	s_ininfo.m_info.m_BitsPerSample = 16;

	
	s_ininfo.m_info.m_Channels = nChannels;
	s_ininfo.m_info.m_SampleRate = nActuallySR; //我们ADC的实际采样率

	s_ininfo.m_info.m_Type = _SD_FILTER_RESAMPLE;
	s_ininfo.m_info.m_Private.m_resample.maxinputlen = 0;
	s_ininfo.m_info.m_Private.m_resample.outSrindex = 0; 
	s_ininfo.m_info.m_Private.m_resample.outSrFree = nSampleRate; // 需要转换成的采样率
	s_ininfo.m_info.m_Private.m_resample.reSampleArithmetic = RESAMPLE_ARITHMETIC_1; //1是新算法，比较省内存；0是老算法，比较耗内存。

	pSdFilter = _SD_Filter_Open(&s_ininfo);
	if ( AK_NULL == pSdFilter ) {
		printf( "can't open the sd filter!\n" );
		return -1;
	}

	return 0;
}


void* audio_dec_thread(void* param)
{
	int ret = -1;
	int size = 0;
	int fd = audio_fd;

	
	printf("audio_dec_thread run \n");
	char * netbuf = (char *) malloc(2048);
	if(NULL == netbuf)
	{
		printf("malloc buf failed \n");
		return NULL;
	}
	
	thread_num++;
	printf("recv data num %d\n", thread_num);
	while(thread_num >= 2)
	{
		//ret = recv( fd , netbuf, 512, 0);
		usleep(1000);
	}
	printf("recv data %d\n", thread_num);
	printf("Channels = 0 \n");
	while(media.m_nChannels == 0)
	{
		
		usleep(1000);
	}
#if 1	
	ret = stAlsaModule.OpenDA(&stAlsaHandle, NULL);
	if ( ret < 0 ) {
		printf( "StartRec::can't open AD!\n" );
		return NULL;
	}

	ret = stAlsaModule.SetParams(&stAlsaHandle, media.m_nSamplesPerSec, media.m_wBitsPerSample, media.m_nChannels, 0);
	//ret = stAlsaModule.SetParams(&stAlsaHandle, 8000, 16,  2, 0);
	if ( ret < 0 ) {
		printf( "StartRec::set audio params failed!\n" );
		return NULL;
	}

	ret = decode_open( &media );
	if( ret < 0)
	{
		printf( "open decode failed \n" );
		return NULL;
	}
#endif
	printf("client audiofd is %d, %d\n", audio_fd, fd);

	struct timeval tv_out;
	tv_out.tv_sec = 3;
	tv_out.tv_usec = 0;
	setsockopt( fd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
	ret = recv( fd , netbuf, 512, 0);
	//fd_set fdsr;

	
	while(1)
	{
		if(deocde_exit == 1)
		{
			printf("audio_dec_thread exit \n ");
			break;
		}
		size = GetFillSize();
		//printf("fill size is %d \n", size);
		if(size > 1024)
			size = 1024;
		else if(size < 512)
			size = 0;
		if(size > 0)
		{
			ret = recv( fd , netbuf, size, 0);
			//printf("read net data %d \n", ret);
			
			if (ret < 0) 
			{        
				// client close
            	printf("audio_fd close\n" );
              	close(fd);
				Closeaec();
		    	break;	
            } 
			else 
			{   
				if(ret <= size)
					FillAudioData( netbuf, ret);
            }
		}
		else
		{
			usleep(10000);
		}

		//usleep(5000);
	}
	
	decode_close();
	
	stAlsaModule.Close(&stAlsaHandle, AK_FALSE);

	free(netbuf);
	deocde_exit = 0;
	thread_num--;
	
	return NULL;
}

extern T_VOID		*pAudioCodec;
#define MAX_VOLUME_FOR_AK39         0xa186   /* -4.00dB                  */

void *audio_write_thread(void *param)
{
	int size = 0;
	int i = 0;
	int j = 0;
//	int temp = 0;
	char * decbuf = (char *) malloc(4096);
	T_AUDIO_FILTER_BUF_STRC fbuf_strc;
	int nReSRDataLen;
	if(NULL == decbuf)
	{
		printf("malloc buf failed \n");
		return NULL;
	}
	T_U8 * bata = (T_U8 *) malloc(1024);
	//int file_fd = open("playback.pcm", O_RDWR | O_CREAT | O_TRUNC);
	while(1)
	{
		if(audio_write_exit == 1)
		{
			break;
		}
		if(pAudioCodec == NULL)
		{
			usleep(10000);
		}
#if 1
		else
		{
			size = audioDecode(decbuf, 2048);

			if( size == 0 )
			{
				usleep( 50000 );
			}
		#if 1
			//双通道改单通道
			for( i =0, j=0; i< size; i+=4) 
			{
				*(bata+j) = *(decbuf+i);
				j++;
				*(bata+j) = *(decbuf +i+1);
				j++;
			}
		#endif
#if 1
			fbuf_strc.buf_in = decbuf;
			fbuf_strc.buf_out = decbuf; //输入输出允许同buffer，但是要保证buffer够大，因为输出数据会比输入数据长
			fbuf_strc.len_in = size/2; // 这里指输入ibuf中的有效数据长度
			fbuf_strc.len_out = 4096;	 // 这里指输出buffer 大小，库里面做判断用，防止写buffer越界。
			nReSRDataLen = _SD_Filter_Control( pSdFilter, &fbuf_strc );
#endif			
#if 0
			//printf("decode size = %d \n", size );
			short *tempbuf = (short *)decbuf;
			for(i=0; i < nReSRDataLen; i++)
	        {
	            temp = (long)(tempbuf[i]);
	           // temp = (temp * MAX_VOLUME_FOR_AK39)>>16;
	            temp = (temp * 1024) >> 10;
	            tempbuf[i] = (short)temp;
	        }
#endif			
			if(size > 0)
			{
				
				stAlsaModule.WriteDA(&stAlsaHandle, bata, (T_U32)nReSRDataLen);
				//write(file_fd, bata, size/2);
			}

		}
#endif

	}
	//close(file_fd);
	free(decbuf);
	return NULL;
}

void audio_dec_exit( void )
{
	Closeaec();
	if(thread_num > 0)
	{
		audio_write_exit = 1;
		deocde_exit	= 1;
	}
#if 0
	decode_close();
	
	stAlsaModule.Close(&stAlsaHandle, AK_FALSE);
#endif
	if ( pSdFilter ) 
	{
		_SD_Filter_Close( pSdFilter );
		pSdFilter = NULL;
	}
}



