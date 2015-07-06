#include <sys/socket.h>
#include <linux/netlink.h>

#include "Thread.h"
#include "SDcard.h"

#define 	SDDEV 		("/dev/mmcblk0")

static int g_StopListenTh = 0;
static nthread_t SDTid;
static T_S32 hotplug_sock;
int sd_mount = 0;
int sd_remove = 0;
extern int times;
extern int close_flag;

static void * ListenSD( void * this );

/**
* @brief  init the sd card hot-plug/pull listen function
* @author hankejia
* @date 2012-07-05
* @param[in] this  			the pointer point to the AkMediaRecorder.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 InitListenSD( void )
{
	
	struct sockaddr_nl snl;
	const T_S32 buffersize = 16 * 1024 * 1024;
	T_S32 retval;

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	//open socket
	hotplug_sock = socket( PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT );
	if ( hotplug_sock == -1 )
	{        
		printf( "error getting socket: %s\n", strerror(errno) );
		return -1;
	}
	
	/* set receive buffersize */   
	setsockopt( hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize) ); 
	retval = bind( hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl) );
	if ( retval < 0 )  
	{	 
		printf( "bind failed: %s\n", strerror(errno) );   
		close(hotplug_sock);   
		hotplug_sock = -1;     
		return -1;   
	}
	g_StopListenTh = 0;
	if ( pthread_create( ( pthread_t *)&SDTid, NULL, ListenSD, NULL ) != 0 ) {
		printf( "unable to create a thread for listen SD hot plug !\n" );
		return -1;
	}
	
	return 0;
}

void CloseListenSD( void )
{
	g_StopListenTh = 1;
	pthread_join(SDTid, NULL);
	SDTid = thread_zeroid();
	close( hotplug_sock );
}


/*
*@BRIEF  call sys command mount the sd card
*@AUTHOR Li_Qing
*@DATA 2012-08-08
*/

void mount_sd(void)
{ 
	if (access("/dev/mmcblk0p1", R_OK) == 0)
	{
		system("mount /dev/mmcblk0p1 /mnt");
	}
	else
	{
		system("mount /dev/mmcblk0 /mnt");
	}
    printf("*** mount the sd to /mnt ***\n");
    
#if 1
    system("mount /mnt/local /usr/local");
#endif
	
	sd_mount = 1;

}
/*
*@BRIEF  call sys command umount the sd card
*@AUTHOR Li_Qing
*@DATA 2012-08-08
*/

void umount_sd(void)
{
	system("umount /mnt");
	//system("rm /sd_test -rf");
	printf("*** umount the sd ***\n");
	sd_mount = 0;
}


//由于当前最终是在3751b上使用此Demo，3751B没有Nandflash并且
//只有一个SD卡槽，所以这里实现上只要检测到SD卡拔出就
//直接退出程序了。但以后如果有多个SD卡槽或NandFlask存储
//录像文件。则还需要加入判断录制目录是否在被拔出的SD
//卡上。
static void * ListenSD( void * this )
{
	
	T_CHR buf[SD_EVENT_BUF_SIZE * 2] = {0};    
	T_CHR temp_buf[20];
	//T_pSTR *p;
	T_S32 i, ret = 0;


	while( AK_TRUE )
	{
		if(g_StopListenTh == 1)
		{
			break;
		}

		memset( buf, 0, sizeof(buf) ); 
		//recv hot-plug/pull uevent
		ret = recv( hotplug_sock, &buf, sizeof(buf), MSG_DONTWAIT );
		//printf("recv buf = %s\n", buf);
		//printf("recv the data leng = %d", ret);
		if ( ret <= 0 ) {
			sleep( 1 );
			continue;
		}
		//p = strrchr( buf, '/' );

		//get event string
		for (i = 0; buf[i] != '@' && buf[i] != 0; i++)
		{     
			temp_buf[i] = buf[i];
		}
		
		temp_buf[i] = 0;
		//if ((!strcmp(p, "/mmcblk")) && (!strcmp(temp_buf, "remove")))
        /*
		if(strstr(buf, "remove") != NULL)
		{
			//close_flag = 1;
			sd_remove = 1;
			times = 0;
			
		}
        */
		
	}
	//umount_sd();

	return NULL;
}

int check_sdcard( void )
{
	if (access(SDDEV, R_OK) < 0)
	{
		//no sd card
		printf("no SDsard exit \n");
		return -1;
	}
	else
	{
		printf("install sdcard \n");
		mount_sd();
		sd_remove = 0;
		InitListenSD();
		return 0;
	}
}
	

