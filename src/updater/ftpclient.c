/*FtpGetRun*/  
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <fcntl.h>  
#include <unistd.h>  
#include <stdarg.h> 
#include <stdio.h>  
#include <netdb.h>  
#include "md5.h"

/*FtpGetRun Variable*/    
FILE *pFtpIOFile = NULL;  
FILE *pFileCmdChmod;        //使用popen方式修改文件属性为可执行的文件指针  
FILE *pRunGetFile;  //使用popen方式执行文件的文件指针  
char aFtpBuffer[4096];  
/*Http Variable*/  
FILE *pFileCmdChmod;  
FILE *pRunGetFile; 
char aRequestHead[1000];  
char aResponseHead[1000];  
static int iSockHttpMark=-1;    
//int iGetRunMark;//用来标记是get模式，还是getrun模式1为get模式，2为getrun模式     
char acChmodCmd[50];//用来使用chmode命令行     
char acRunCmdLine[50];//用来运行程序     

extern int bNeedCheckFile;
static int FtpCmd(int iSockFtpCmd,char *cFmt,...)    
{     
	va_list vVaStartUse;     
	int iFtpCmdReturn;    
	int iFtpLength;      
	
	if (pFtpIOFile == NULL)     
	{     
		pFtpIOFile = fdopen(iSockFtpCmd,"r");    
		if (pFtpIOFile == NULL)    
		{    
			printf("The ERROR of pointer of pFtpIOFile");    
			return -1;    
		}     
	}      
	if (cFmt)     
	{     
		va_start(vVaStartUse,cFmt);     
		iFtpLength = vsprintf(aFtpBuffer,cFmt,vVaStartUse);     
		aFtpBuffer[iFtpLength++] = '\r';     
		aFtpBuffer[iFtpLength++]='\n';     
		write(iSockFtpCmd,aFtpBuffer,iFtpLength); //如同send     
	}      
	do     
	{     
		if (fgets(aFtpBuffer,sizeof(aFtpBuffer),pFtpIOFile) == NULL)     
		{    
			return -1;    
		}      
	} while(aFtpBuffer[3] == '-');    

	sscanf(aFtpBuffer,"%d",&iFtpCmdReturn);      
	return iFtpCmdReturn;    
}     
  
int FtpGet(char *host,unsigned short port, char *user,char *pass,char *filename,char *pcSaveFile)    
{     
	int iSockFtpCmd = -1;//用来socket接受调用后返回的套接口描述符号     
	int iSockFtpData = -1;//datasocket建立后返回的套接口描述符号     
	int iSockAccept = -1;     
	struct sockaddr_in addr;//定义socket结构      
	unsigned long hostip;//存放主机地址的变量      
	int iFtpLength;    
	int tmp;    
	int iFtpCmdReturn;     
	int retval = -1;     
	int iOpenReturn; //接收open函数的返回值     
	unsigned char *c;//用来指向data连接时候的主机地址     
	unsigned char *p;//用来指向data连接时候的端口     
	struct hostent *he;     
	hostip = inet_addr(host); //转换主机地址为网络排序模式     
	if (hostip == -1)     
	{     
		printf("\nHostIP is ERROR!!\n");    
	}     
  
	//建立socket     
	//设定相应的socket协议和地址     
	/**********************************************************/    
	iSockFtpCmd = socket(AF_INET,SOCK_STREAM,0);      

	if (iSockFtpCmd == -1)
	{
		retval = -2;		
		goto out;  
	}	

	addr.sin_family = PF_INET;     
	addr.sin_port = htons(port);     
	addr.sin_addr.s_addr = hostip;      

	/**********************************************************/     
	/*connect*/    
	if (connect(iSockFtpCmd,(struct sockaddr *)&addr,sizeof(addr)) == -1)     
	{
		retval = -3;
		goto out;   
	}	
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,NULL);     
	if (iFtpCmdReturn != 220)     
	{
		retval = -4;
		goto out;
	}	
 
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"USER %s",user);     
	if (iFtpCmdReturn != 331)     
	{
		retval = -5;
		goto out;
	}	
     
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"PASS %s",pass);     
	if (iFtpCmdReturn != 230)     
	{
		retval = -6;
		goto out;   
	}	
     
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"TYPE I");     
	if (iFtpCmdReturn != 200)     
	{
		retval = -7;
		goto out;   
	}	
      
	/*建立data socket*/    
	iSockFtpData = socket(AF_INET,SOCK_STREAM,0);     
      
	if (iSockFtpData == -1)     
	{
		retval = -8;
		goto out;  
	}	
      
	tmp = sizeof(addr);     
    
	getsockname(iSockFtpCmd,(struct sockaddr *)&addr,&tmp);     
	addr.sin_port = 0;     
	   
	/*绑定*/    
	if (bind(iSockFtpData,(struct sockaddr *)&addr,sizeof(addr)) == -1)     
	{
		retval = -9;
		goto out;   
	}	
      
	if (listen(iSockFtpData,1) == -1)     
	{
		retval = -10;
		goto out;    
	}	
     
	tmp = sizeof(addr);     
	getsockname(iSockFtpData,(struct sockaddr *)&addr,&tmp);     
	c = (unsigned char *)&addr.sin_addr;     
	p = (unsigned char *)&addr.sin_port;     
    
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"PORT %d,%d,%d,%d,%d,%d", c[0],c[1],c[2],c[3],p[0],p[1]);     
 
	if (iFtpCmdReturn != 200)     
	{
		retval = -11;
		goto out;
	}		
     
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"RETR %s",filename);     
	if (iFtpCmdReturn != 150)     
	{
		retval = -12;
		goto out;   
	}	
     
	tmp = sizeof(addr);     
	iSockAccept = accept(iSockFtpData,(struct sockaddr *)&addr,&tmp);     
      
	if (iSockAccept == -1)     
	{
		retval = -13;
		goto out;    
	}	
	//     
	iOpenReturn = open(pcSaveFile,O_WRONLY|O_CREAT,0644);     
	if (iOpenReturn == -1)     
	{
		retval = -14;
		goto out;    
	}	
	        
	retval = 0;     
	while ((iFtpLength=read(iSockAccept,aFtpBuffer,sizeof(aFtpBuffer)))>0)     
	{     
		write(iOpenReturn,aFtpBuffer,iFtpLength);     
		retval += iFtpLength;     
	};     
    
	close(iOpenReturn);    
 
   	//md5 check
	if(bNeedCheckFile != 0)
		if(CheckFileMd5(pcSaveFile) < 0)
		{
			retval = -1;
		}	
out:     
	close(iSockAccept);     
	close(iSockFtpData);     
	close(iSockFtpCmd);     
	if (pFtpIOFile)     
	{     
		fclose(pFtpIOFile);     
		pFtpIOFile = NULL;     
	}      
	return retval;    
}  

#if 0
int FtpPut(char *host,char *user,char *pass,char *filename,char *pcSaveFile)    
{     
	int iSockFtpCmd = -1;//用来socket接受调用后返回的套接口描述符号     
	int iSockFtpData = -1;//datasocket建立后返回的套接口描述符号     
	int iSockAccept = -1;     
	struct sockaddr_in addr;//定义socket结构      
	unsigned long hostip;//存放主机地址的变量      
	int iFtpLength;    
	int tmp;    
	int iFtpCmdReturn;     
	int retval = -1;     
	int iOpenReturn; //接收open函数的返回值     
	unsigned char *c;//用来指向data连接时候的主机地址     
	unsigned char *p;//用来指向data连接时候的端口     
	struct hostent *he;     
	hostip = inet_addr(host); //转换主机地址为网络排序模式     
	if (hostip == -1)     
	{     
		printf("\nHostIP is ERROR!!\n");    
	}     
	    
	//建立socket     
	//设定相应的socket协议和地址     
	/**********************************************************/    
	iSockFtpCmd = socket(AF_INET,SOCK_STREAM,0);     
	      
	if (iSockFtpCmd == -1)     
	goto out;     
	
	addr.sin_family = PF_INET;     
	addr.sin_port = htons(21);     
	addr.sin_addr.s_addr = hostip;     
	 
	/**********************************************************/     
	/*connect*/    
	if (connect(iSockFtpCmd,(struct sockaddr *)&addr,sizeof(addr)) == -1)     
		goto out;      
	
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,NULL);     
	if (iFtpCmdReturn != 220)     
		goto out;     
	      
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"USER %s",user);     
	if (iFtpCmdReturn != 331)     
		goto out;     
	     
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"PASS %s",pass);     
	if (iFtpCmdReturn != 230)     
		goto out;     
	      
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"TYPE I");     
	if (iFtpCmdReturn != 200)     
		goto out;     
	    
	/*建立data socket*/    
	iSockFtpData = socket(AF_INET,SOCK_STREAM,0);     
	     
	if (iSockFtpData == -1)     
		goto out;     
	     
	tmp = sizeof(addr);     
	   
	getsockname(iSockFtpCmd,(struct sockaddr *)&addr,&tmp);     
	addr.sin_port = 0;     
	   
	/*绑定*/    
	if (bind(iSockFtpData,(struct sockaddr *)&addr,sizeof(addr)) == -1)     
	goto out;     
	    
	if (listen(iSockFtpData,1) == -1)     
		goto out;     
	     
	tmp = sizeof(addr);     
	getsockname(iSockFtpData,(struct sockaddr *)&addr,&tmp);     
	c = (unsigned char *)&addr.sin_addr;     
	p = (unsigned char *)&addr.sin_port;     
	 
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"PORT %d,%d,%d,%d,%d,%d", c[0],c[1],c[2],c[3],p[0],p[1]);     
	
	if (iFtpCmdReturn != 200)     
		goto out;     
	    
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"STOR %s",filename);     
	if (iFtpCmdReturn != 150)     
		goto out;     
	    
	tmp = sizeof(addr);     
	iSockAccept = accept(iSockFtpData,(struct sockaddr *)&addr,&tmp);     
	     
	if (iSockAccept == -1)     
		goto out;     
	//     
	iOpenReturn = open(pcSaveFile,O_RDONLY,0644);     
	if (iOpenReturn == -1)     
		goto out;     
	       
	retval = 0;     
	   
	retval=read(iOpenReturn,aFtpBuffer,4096);    
	   
	while(retval!=0)    
	{    
		write(iSockAccept,aFtpBuffer,retval);    
		retval=read(iOpenReturn,aFtpBuffer,4096);    
	}     
	 
	close(iOpenReturn);    
	
out:     
	close(iSockAccept);     
	close(iSockFtpData);     
	close(iSockFtpCmd);     
	if (pFtpIOFile)     
	{     
		fclose(pFtpIOFile);     
		pFtpIOFile = NULL;     
	}  
	return retval;  
}

int main(int argc,char *argv[])     
{  
	//iGetRunMark=1;/*设定为get模式*/  
	if(strcmp("get",argv[1])==0)    
	{  
		printf("\nFTP protocol ...\n");  
		FtpGet(argv[2], 21, argv[3],argv[4],argv[5],argv[6]);  
	}     
#if 0
	if(strcmp("put",argv[1])==0)   
	{    
		printf("\nFTP protocol ...\n");    
		FtpPut(argv[2],argv[3],argv[4],argv[5],argv[6]);      
	}
#endif	
	return 0;  
}
#endif
