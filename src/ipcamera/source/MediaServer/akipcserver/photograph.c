#include <sys/file.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "video_stream_lib.h"
#include "muxer.h"
#include "cgi_anyka.h"
#include "Tool.h"
#include "akuio.h"
#include "SDcard.h"
#include "inisetting.h"
#include "encode.h"
#include "Thread.h"

extern init_parse parse;
extern int sd_mount;
#define 	SDDEV 		("/dev/mmcblk0")
T_pVOID outbuf;
T_pVOID encbuf;
#define ENCMEM (256*1024)

T_pVOID h_eid;
int index_file = 1;
static Condition		g_conMangerPh;
static Condition		g_conMetex;
static pthread_t 		g_phid;
static int 				g_phsize = 0;
static int 				g_busy = 0;
static T_pSTR MakeFileName( )
{
	//T_U32 	nSuffizeIndex = 0, nFileIndex = 0, nLastLen = 0;
	T_U32 	nFileIndex = 0;
	//T_pSTR	strTime = NULL;
	T_pSTR	strFileName = NULL;
	//T_pSTR	strNumer = NULL;
	char 	name[20];
	char 	file[20];
	char 	num[5];
	struct tm *tnow = GetCurTime();
	//assert( handle );
	
	//strTime = GetCurTimeStr();
	memset(name, 0x00, 20);
	sprintf( name, "/mnt/DC%4d%02d%02d/", 1900 + tnow->tm_year, tnow->tm_mon + 1, tnow->tm_mday );
	CompleteCreateDirectory(name);
	memset(file, 0x00, 20);
	sprintf(file, "DC%02d%02d%02d", tnow->tm_hour, tnow->tm_min, tnow->tm_sec);
	{
		strFileName = (T_pSTR)malloc( strlen(file) + 30 );
		if ( NULL == strFileName ) {
			goto err;
		}
		memset(strFileName, 0x00, strlen(file) + 30);
		strcat(strFileName, name);
		strcat( strFileName, file );
		
		strcat( strFileName, ".jpeg" );

		//free( strTime );
		//strTime = NULL;
	}
	while ( AK_TRUE ) 
	{
		if ( IsExists( strFileName ) ) 
		{
			++nFileIndex;
			if ( nFileIndex > 99 ) {
				printf( "MakeFileName::too many file have same name!\n" );
				goto err;
			}
			memset(strFileName, 0x00, strlen(file) + 30);
			sprintf(num, "_%02d", ( int )nFileIndex);
			strcat(strFileName, name);
			strcat( strFileName, file );
			strcat(strFileName, num);
			strcat( strFileName, ".jpeg" );
			//strNumer = strFileName + nLastLen;
			//sprintf( strNumer, "(%02d)", (int)(nFileIndex++) );
			continue;
		}

		break;
	}
	
	return strFileName;
err:
	/*
	if ( strTime ) {
		free( strTime );
	}
	*/
	if ( strFileName ) {
		free( strFileName );
		strFileName = NULL;
	}
	
	return strFileName;
}




int frame_encode(T_pVOID hvs1, void *pinbuf, void **poutbuf, unsigned long *size)
{
	T_VIDEOLIB_ENC_IO_PAR video_enc_io_param1;

	if(1 == g_busy)
	{
		printf("write file busy \n");
		return -1;
	}
	video_enc_io_param1.QP = 0;
	video_enc_io_param1.mode = 0;				//编码类型I/P帧,0，i，1，p
	video_enc_io_param1.p_curr_data = (T_U8*)pinbuf;		//yuv输入地址
	video_enc_io_param1.p_vlc_data = (T_U8*)encbuf;			//码流输出地址
	video_enc_io_param1.out_stream_size = ENCMEM;	//输出码流的大小

	VideoStream_Enc_Encode(hvs1, AK_NULL, &video_enc_io_param1, NULL);
	*poutbuf = video_enc_io_param1.p_vlc_data;
	*size = video_enc_io_param1.out_stream_size;
	//printf("enc size is %d", *size);
	
	return 0;
}

static T_pVOID open_encode(int width, int height, int real_width, int real_height)
{
	T_VIDEOLIB_ENC_OPEN_INPUT open_input;
	T_U32 temp;
	
	outbuf = akuio_alloc_pmem(ENCMEM);
	if (AK_NULL == outbuf)
	{
		return AK_NULL;
	}
	
	temp = akuio_vaddr2paddr(outbuf) & 7;
	//编码buffer 起始地址必须8字节对齐
	encbuf = ((T_U8 *)outbuf) + ((8-temp)&7);

	
	open_input.encFlag = VIDEO_DRV_MJPEG;
	open_input.encMJPEGPar.frameType = ENC_YUV420_PLANAR;//JPEGENC_YUV420_PLANAR;
	open_input.encMJPEGPar.format = ENC_THUMB_JPEG;
	open_input.encMJPEGPar.thumbWidth = 0;
	open_input.encMJPEGPar.thumbHeight = 0;
	open_input.encMJPEGPar.thumbData = NULL;
	open_input.encMJPEGPar.thumbDataLen = 0;
	open_input.encMJPEGPar.qLevel = 7;
	open_input.encMJPEGPar.width = width;
	open_input.encMJPEGPar.height = height;
	open_input.encMJPEGPar.lumWidthSrc = real_width;
	open_input.encMJPEGPar.lumHeightSrc = real_height;
	open_input.encMJPEGPar.horOffsetSrc = (real_height-height)/2;
	open_input.encMJPEGPar.verOffsetSrc = (real_height-height)/2;
	

	return  VideoStream_Enc_Open(&open_input);
}
#define MIN_FREE_SPACE_SIZE  3145728

static void *photograph_thread(void *param)
{
	//int mount_falg = 0;
	T_S32 bavail, bsize;
	signed long long DiskSize = 0;
	T_pSTR filename;
	
	while(1)
	{
	
		Condition_Lock( g_conMangerPh );

		Condition_Wait( &g_conMangerPh );
		if(g_phsize == 0)
			continue;
		
		Condition_Unlock( g_conMangerPh );
		if(sd_mount == 0)
		{
			if (access(SDDEV, R_OK) < 0)
			{
				//no sd card
				printf("no SDsard exit \n");
				continue;
			}
			else
			{
				//mount_sd();
			}
			//mount_falg =1;
		}

		
		DiskFreeSize( "/mnt", &bavail, &bsize);
		DiskSize = (T_S64)(T_U32)(bavail) * (T_S64)(T_U32)(bsize);
		
		if ( DiskSize < (T_S64)MIN_FREE_SPACE_SIZE ) 
		{
			printf( "get %s disk size error!\n", "/mnt" );
			//umount_sd();
			goto err;
		}

		filename = MakeFileName();
		printf("filename is %s \n", filename);
		if(filename == NULL)
		{
			printf("filename is NULL \n");
			goto err;
		}

		long fid = open(filename, O_RDWR | O_CREAT | O_TRUNC);
		if(fid <= 0)
		{
			printf("open file err \n");
			free(filename);
			goto err;
		}

		Condition_Lock( g_conMetex );
		g_busy = 1;
		Condition_Unlock( g_conMetex );
		
		write( fid, encbuf, g_phsize);

		Condition_Lock( g_conMetex );
		g_busy = 0;
		Condition_Unlock( g_conMetex );

		close(fid);

		free(filename);	
err:
		g_phsize = 0;
		
		
		//if( mount_falg == 1 )
		//{
			//umount_sd();
		//}
		
		//printf("save file ok\n");
			
	}
	return NULL;
}

void Init_photograph( void )
{
	IniSetting_init();
	struct picture_info *picture = IniSetting_GetPictureInfo();
	index_file = atoi( picture->video_index );
	IniSetting_destroy();
	int width, height, real_width, real_height;
	
	if( 2 == index_file )
	{
		width = parse.width2;
		height = parse.height2;
		real_width = parse.real_width2;
		real_height = parse.real_height2;
	}
	else 
	{
		width = parse.width;
		real_width = parse.width;
		height = parse.height;
		real_height = parse.height;
		
	}

	Condition_Initialize( &g_conMangerPh );
	Condition_Initialize( &g_conMetex );
	//encode_init();
	h_eid = open_encode(width, height, real_width, real_height);
	if(h_eid == NULL)
	{
		printf("open encode err \n");
		return ;
	}

	pthread_create( &g_phid, NULL, photograph_thread, NULL);
	
}



int photograph( void *pbuf, int size)
{

	unsigned long outsize;
	int ret;

	VideoStream_Enc_Reset();
	
	//printf("%d\n", h_eid);
	void * buf;
	if( 2 == index_file )
		ret = frame_encode(h_eid , pbuf+(parse.width*parse.height*3/2), &buf, &outsize);
	else
		ret = frame_encode(h_eid, pbuf, &buf, &outsize);

	if(ret == 0)
	{
		Condition_Lock( g_conMangerPh );
		g_phsize = outsize;
		Condition_Signal( &g_conMangerPh );
		Condition_Unlock( g_conMangerPh );
	}

	VideoStream_Enc_Reset();
	
	return 0;
}

void close_encode( void )
{
	Condition_Destroy( &g_conMangerPh );
	Condition_Destroy( &g_conMetex );

	VideoStream_Enc_Close(h_eid);
	if(outbuf != NULL)
		akuio_free_pmem(outbuf);
}

