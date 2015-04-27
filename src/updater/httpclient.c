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
#include "md5.h"

extern int bNeedCheckFile;
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
  int i;
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
	  
 	sprintf(local_file_tmp, "%s.tmp", local_file);
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
      fwrite(buffer, 1, 1, fp);/*将http主体信息写入文件*/
      i++;
      if(i%1024 == 0)
	  {
		printf("download %d bytes\n", i);
      	fflush(fp);/*每1K时存盘一次*/
    	}
	 }
  }
  fclose(fp);
  /* 结束通讯 */
  close(sockfd);
  if(bNeedCheckFile != 0)
  {
  	if(CheckFileMd5(local_file_tmp) == 0)
	{
  		char cmd[256];
  		sprintf(cmd, "mv %s %s", local_file_tmp, local_file);
  		system(cmd);
  		strcpy(filename, local_file);
  		return 0;
	}
  	else
  	{
  		return -1;
  	}
  }
  else
  {
  	return 0;
  }
  //exit(0);
}

#if 0
/*********************************************************************
*filename: httpclient.c
*purpose: HTTP协议客户端程序，可以用来下载网页
*wrote by: zhoulifa(zhoulifa@163.com) 周立发(http://zhoulifa.bokee.com)
           Linux爱好者 Linux知识传播者 SOHO族 开发者 最擅长C语言
*date time:2006-03-11 21:49:00
*Note: 任何人可以任意复制代码并运用这些代码，当然包括你的商业用途
*                         但请遵循GPL
*********************************************************************/
int main(int argc, char *argv[])
{
	char filename[256];
	memset(filename, 0, 256);
	HttpDownload(argv[1], filename);
	printf("%s\n", filename);
	return 0;
	#if 0
  int sockfd;
  char buffer[1024];
  struct sockaddr_in server_addr;
  struct hostent *host;
  int portnumber,nbytes;
  char host_addr[256];
  char host_file[1024];
  char local_file[256];
  FILE * fp;
  char request[1024];
  int send, totalsend;
  int i;
  char * pt;

  if(argc!=2)
  {
    fprintf(stderr,"Usage:%s web-address\a\n",argv[0]);
    exit(1);
  }
  
  printf("parameter.1 is: %s\n", argv[1]);
  ToLowerCase(argv[1]);/*将参数转换为全小写*/
  printf("lowercase parameter.1 is: %s\n", argv[1]);

  GetHost(argv[1], host_addr, host_file, &portnumber);/*分析网址、端口、文件名等*/
  printf("webhost:%s\n", host_addr);
  printf("hostfile:%s\n", host_file);
  printf("portnumber:%d\n\n", portnumber);

  if((host=gethostbyname(host_addr))==NULL)/*取得主机IP地址*/
  {
    fprintf(stderr,"Gethostname error, %s\n", strerror(errno));
    exit(1);
  }

  /* 客户程序开始建立 sockfd描述符 */
  if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)/*建立SOCKET连接*/
  {
    fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
    exit(1);
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
    exit(1);
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
  	strcpy(local_file, host_file);
  else  
  	strcpy(local_file, "index.html");
  printf("local filename to write:%s\n\n", local_file);

  /*发送http请求request*/
  send = 0;totalsend = 0;
  nbytes=strlen(request);
  while(totalsend < nbytes)
  {
    send = write(sockfd, request + totalsend, nbytes - totalsend);
    if(send==-1)  
    {
    	printf("send error!%s\n", strerror(errno));
    	exit(0);
    }
    totalsend+=send;
    printf("%d bytes send OK!\n", totalsend);
  }

  fp = fopen(local_file, "a");
  if(!fp)
  {
    printf("create file error! %s\n", strerror(errno));
    return 0;
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
      fwrite(buffer, 1, 1, fp);/*将http主体信息写入文件*/
      i++;
      if(i%1024 == 0)  
      	fflush(fp);/*每1K时存盘一次*/
    }
  }
  fclose(fp);
  /* 结束通讯 */
  close(sockfd);
  exit(0);
  #endif
}
#endif
//////////////////////////////httpclient.c 结束///////////////////////////////////////////
