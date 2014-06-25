/******* http客户端程序 httpclient.c ************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/fcntl.h>

#define CAMERA			"/etc/jffs2/camera.ini"

#define SEM_PROJ_ID	0x23
#define SEM_NUMS		6

#define KERNEL_UPDATE_OP						0
#define KERNEL_UPDATE_OP_DOWNLOAD		1
#define KERNEL_UPDATE_SD						2
#define KERNEL_UPDATE_DOWNLOAD			3
#define KERNEL_UPDATE_CAN_BURN_KER	4
#define KERNEL_UPDATE_FILE					5

key_t sem_key;
int sem_id;

union semun{
	int			val;		//semctl SETVAL
	struct semid_ds *	buf;		//semctl IPC_STAT, IPC_SET
	unsigned short	*	array; 	//Array for GETALL, SETALL
	struct seminfo	*	 __buf;	//IPC_INFO
};

int g_bIsSem = 0;

//////////////////////////////httpclient.c 开始///////////////////////////////////////////

/********************************************
功能：搜索字符串右边起的第一个匹配字符
********************************************/
static char * Rstrchr(char * s, char x)
{
	int i = strlen(s);
  if(!(*s))  
  	return 0;
  while(s[i-1]) 
  	if(strchr(s + (i - 1), x))  
  		return (s + (i - 1));  
  	else i--;
  return 0;
}

/********************************************
功能：把字符串转换为全小写
********************************************/
static void ToLowerCase(char * s)  {
	while(*s)
	{
		printf("%c\n", *s);
		*s=tolower(*s);
		s++;
	}
}

/**************************************************************
功能：从字符串src中分析出网站地址和端口，并得到用户要下载的文件
***************************************************************/
static void GetHost(char * src, char * web, char * file, int * port)  {
  char * pA;
  char * pB;
  memset(web, 0, sizeof(web));
  memset(file, 0, sizeof(file));
  *port = 0;
  if(!(*src))  
  	return;
  pA = src;
  
  if(!strncmp(pA, "http://", strlen("http://")))  
  	pA = src+strlen("http://");
  else if(!strncmp(pA, "https://", strlen("https://")))  
  	pA = src+strlen("https://");
  pB = strchr(pA, '/');
  
  if(pB)
  {
    memcpy(web, pA, strlen(pA) - strlen(pB));
    if(pB+1)
    {
      memcpy(file, pB + 1, strlen(pB) - 1);
      file[strlen(pB) - 1] = 0;
    }
  }
  else  
  	memcpy(web, pA, strlen(pA));
  
  if(pB)  
  	web[strlen(pA) - strlen(pB)] = 0;
  else  
  	web[strlen(pA)] = 0;
  pA = strchr(web, ':');
  
  if(pA)  
  	*port = atoi(pA + 1);
  else 
  	*port = 80;
}


int HttpDownload(char* url, char* filename)
{
	int sockfd;
  char buffer[1024];
  struct sockaddr_in server_addr;
  struct hostent *host;
  int portnumber,nbytes;
  char host_addr[256];
  char host_file[1024];
  char local_file[256];
  char local_file_tmp[256];
  FILE * fp;
  char request[1024];
  int send, totalsend;
  int i, err = 0;
  char * pt;
  
  printf("parameter.1 is: %s\n", url);
  ToLowerCase(url);/*将参数转换为全小写*/
  printf("lowercase parameter.1 is: %s\n", url);

  GetHost(url, host_addr, host_file, &portnumber);/*分析网址、端口、文件名等*/
  printf("webhost:%s\n", host_addr);
  printf("hostfile:%s\n", host_file);
  printf("portnumber:%d\n\n", portnumber);

  if((host=gethostbyname(host_addr))==NULL)/*取得主机IP地址*/
  {
    fprintf(stderr,"Gethostname error, %s\n", strerror(errno));
    //exit(1);
  	return -1;
  }

  /* 客户程序开始建立 sockfd描述符 */
  if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)/*建立SOCKET连接*/
  {
    fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
    //exit(1);
  	return -1;
  }

  /* 客户程序填充服务端的资料 */
  bzero(&server_addr,sizeof(server_addr));
  server_addr.sin_family=AF_INET;
  server_addr.sin_port=htons(portnumber);
  server_addr.sin_addr=*((struct in_addr *)host->h_addr);

  /* 客户程序发起连接请求 */
  if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)/*连接网站*/
  {
    fprintf(stderr,"Connect Error:%s\a\n",strerror(errno));
    return -1;
	//exit(1);
  }

  sprintf(request, "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\n\
User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n\
Host: %s:%d\r\nConnection: Close\r\n\r\n", host_file, host_addr, portnumber);
  printf("%s", request);/*准备request，将要发送给主机*/

  /*取得真实的文件名*/
  if(/*host_file && */*host_file)  
		  pt = Rstrchr(host_file, '/');
  else 
		  pt = 0;

  memset(local_file, 0, sizeof(local_file));
  if(pt && *pt)
  {
    if((pt + 1) && *(pt+1))  
    	strcpy(local_file, pt + 1);
    else  
    	memcpy(local_file, host_file, strlen(host_file) - 1);
 
	}
  else if(/*host_file && */*host_file)  
	{ 
		strcpy(local_file, host_file);
	} 
 else  
  	strcpy(local_file, "index.html");
	  
 	sprintf(local_file_tmp, "/mnt/%s.tmp", local_file);
printf("local filename to write:%s\n\n", local_file_tmp);
	unlink(local_file_tmp);

  /*发送http请求request*/
  send = 0;totalsend = 0;
  nbytes=strlen(request);
  while(totalsend < nbytes)
  {
    send = write(sockfd, request + totalsend, nbytes - totalsend);
    if(send==-1)  
    {
    	printf("send error!%s\n", strerror(errno));
    	return -1;
		//exit(0);
    }
    totalsend+=send;
    printf("%d bytes send OK!\n", totalsend);
  }

  fp = fopen(local_file_tmp, "a");
  if(!fp)
  {
    printf("create file error! %s\n", strerror(errno));
    return -1;
  }
  printf("\nThe following is the response header:\n");
  i=0;
  /* 连接成功了，接收http响应，response */
  while((nbytes=read(sockfd,buffer,1))==1)
  {
    if(i < 4)
    {
      if(buffer[0] == '\r' || buffer[0] == '\n')
      	i++;
      else 
      	i = 0;
      printf("%c", buffer[0]);/*把http头信息打印在屏幕上*/
    }
    else
    {
      while (fwrite(buffer, 1, 1, fp) != 1) {/*将http主体信息写入文件*/
      	err = ferror(fp);
      	if (err == EAGAIN) continue;
      	
      	printf("can't write the download data to the file, error = %d\n", err);
      	fclose(fp);
      	close(sockfd);
      	return -1;
      }
      
      i++;
      if(i%1024 == 0)
	  {
		printf("download %d bytes\n", i);
      	if ((fflush(fp) != 0) && errno != EAGAIN) {/*每1K时存盘一次*/
      		printf("can't fflush the data to the file, error = %d\n", errno);
      		fclose(fp);
      		close(sockfd);
      		return -1;
      	}
    	}
	 }
  }
  fclose(fp);
  /* 结束通讯 */
  close(sockfd);
	if( i < 1000000 )
	{
		printf("download err \n");
		return -1;
	}
  
  if(1)
	{
  char cmd[256];
  sprintf(cmd, "mv %s /mnt/%s", local_file_tmp, filename);
  system(cmd);
 // strcpy(filename, local_file);
 // sprintf(cmd, "cp %s %s", )
  return 0;
	}
  else
  {
  	return -1;
  }
  //exit(0);
}

/**
 * *  @brief       system V semaphore pv operation
 * *  @author      
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static int semaphore_pv(int sem_id, int sem_index, int op)
{
	struct sembuf sbuf;
	sbuf.sem_num = sem_index;
	sbuf.sem_op = op;
	sbuf.sem_flg = SEM_UNDO;
	
	if (semop(sem_id, &sbuf, 1) < 0) {
		printf("%s->%s:semop fail, error %d!", __FILE__, __func__, errno);
		return -1;
	}
	
	return 0;
}

/**
 * *  @brief       init the system V semaphore
 * *  @author      
 * *  @date        2013-5-9
 * *  @param[in]   none
 * *  @return	   	none
 * */
static int init_systemv_sem()
{
	union semun seminfo;
	unsigned short array[SEM_NUMS] = {1, 1, 1, 1, 0, 1};
	
	sem_key = ftok(CAMERA, SEM_PROJ_ID); //use camera.ini to generate the key
	if (sem_key < 0) {
		printf("%s->%s:ftok fail!", __FILE__, __func__);
		return -1;
	}
	
	if ((sem_id = semget(sem_key, 0, 0)) < 0) {
		if ((sem_id = semget(sem_key, SEM_NUMS, IPC_CREAT | 0666)) < 0) {
			printf("%s->%s:semget fail, error %d!", __FILE__, __func__, errno);
			return -1;
		}
		
		seminfo.array = array;
		if (semctl(sem_id, 0, SETALL, seminfo) < 0) {
			printf("%s->%s:semctl fail, error %d!", __FILE__, __func__, errno);
			return -1;
		}
	}
	
	return 0;
}

int main( int argc, char **argv )
{
	struct stat statbuf;

	if(argc < 2)
	{
		printf("please input download add \n");
		return -1;
	}
	printf("%s", argv[1]);
	
	if (!init_systemv_sem()) g_bIsSem = 1;
	
	if (stat("/dev/mmcblk0", &statbuf) == -1) {
		if (g_bIsSem)
			semctl(sem_id, KERNEL_UPDATE_SD, SETVAL, 0);
		return 0;
	}
	
	if (g_bIsSem)	
		semctl(sem_id, KERNEL_UPDATE_SD, SETVAL, 1);
	
	if(HttpDownload( argv[1], "zImage") == 0)
	{
		if (g_bIsSem) {
			semctl(sem_id, KERNEL_UPDATE_DOWNLOAD, SETVAL, 1);
			semctl(sem_id, KERNEL_UPDATE_CAN_BURN_KER, SETVAL, 1);
			semaphore_pv(sem_id, KERNEL_UPDATE_OP_DOWNLOAD, -1);
			if (semctl(sem_id, KERNEL_UPDATE_OP_DOWNLOAD, GETVAL) == 0)
				printf("-----now begin to burn the kernel to the spi flash----------\n");
			else {
				semctl(sem_id, KERNEL_UPDATE_OP_DOWNLOAD, SETVAL, 0);
				printf("-----user cancel the update kernel----------\n");
				return 0;
			}
		}
		
		char cmd[256];
  	sprintf(cmd, "/etc/init.d/update.sh /mnt/%s", "zImage");
  	system(cmd);
		return 0;
	}
	
	if (g_bIsSem){
		semctl(sem_id, KERNEL_UPDATE_DOWNLOAD, SETVAL, 0);
		semctl(sem_id, KERNEL_UPDATE_CAN_BURN_KER, SETVAL, 0);
	}
}

