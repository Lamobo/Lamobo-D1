
//#include "AkMediaRecLib.h"
#include "AkMotionDetect.h"
#include <motion_detector_lib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/time.h>
#include "Thread.h"

#include "log.h"
#include "muxer.h"
#include "photograph.h"

#define THRESHOLD_MIN   		10
#define THRESHOLD_MAX   		40

#define WIDTH_720		1280
#define HEIGHT_720		720
#define WIDTH_VGA		640
#define HEIGHT_VGA		480
#define WIDTH_SVGA		800
#define HEIGHT_SVGA		600

#define THRESHOLD	35
#define DIM_HORNUM	8
#define DIM_VERNUM	8


static T_VOID * hMdetector 		= NULL;
static T_U8 * 	LatestData		= NULL;

T_MD_PROC 		MDData;
nthread_t		TID;
int 			motiondetect_exit = 0;
extern 			init_parse parse;
extern 		int pic;
//allocate memory callback
static T_pVOID md_malloc(T_U32 size, T_pSTR filename, T_U32 line);
//free memory callback
static T_pVOID md_free(T_pVOID mem);
void* thread_MotinDetec(void *usr);

/**
* @brief  open the motion detect
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 OpenMotionDetect(T_U16 width, T_U16 height, T_U16 *ratio)
{
	printf("%s\n", __func__);
	T_MOTION_DETECTOR_OPEN_PARA openParam;

	if (hMdetector)
	{
		//T_MOTION_DETECTOR_DIMENSION dim;		
		
		logi( "motion detect already open!\n" );
		
		//dim.m_uHoriNum = DIM_HORNUM;
		//dim.m_uVeriNum = DIM_VERNUM;
		
		Motion_Detector_SetRatio(hMdetector, ratio);
		//Motion_Detector_SetDimension(hMdetector, width, height,	&dim);
		return 0;
	}
	
	for (int i = 0; i< BUFFER_NUM; i++)
	{
		MDData.Data[i].pData = malloc(width*height);
	}
	
	MDData.rpos = 0;
	MDData.wpos = 0;
	
	//bzero( CompareData, sizeof( CompareData ) );
	bzero( &openParam, sizeof( T_MOTION_DETECTOR_OPEN_PARA ) );
	
	
	printf("width = %d, height = %d \n", width, height);
	openParam.m_uWidth 		= (T_U16)width;	//any
	openParam.m_uHeight 	= (T_U16)height;	//any
		
	if (width > WIDTH_SVGA)
		openParam.m_uIntervalHoriNum = 1;
	else
		openParam.m_uIntervalHoriNum = 0;

	if (height > HEIGHT_SVGA)
		openParam.m_uIntervalVeriNum = 1;
	else
		openParam.m_uIntervalVeriNum = 0;

	openParam.m_uThreshold	= (T_U16)THRESHOLD;	//10
	openParam.m_Dimension.m_uHoriNum = DIM_HORNUM;
	openParam.m_Dimension.m_uVeriNum = DIM_VERNUM;
	
	openParam.m_CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	openParam.m_CBFunc.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)md_malloc;
	openParam.m_CBFunc.m_FunFree   = (MEDIALIB_CALLBACK_FUN_FREE)md_free;

	//open the detector
	hMdetector = Motion_Detector_Open( &openParam );
	if (!hMdetector) 
	{
		printf( "motion detector open failed!\n" );
		if (LatestData != NULL)
			free(LatestData);
		
		return -1;
	}
	
	Motion_Detector_SetRatio(hMdetector, ratio);

	motiondetect_exit = 0;
	pthread_create(&TID, NULL, thread_MotinDetec, NULL);
	
	return 0;
}

/**
* @brief  close the motion detect
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 CloseMotionDetect()
{
	printf("%s\n", __func__);
	if ( !hMdetector ) {
		logi( "motion detect already close!\n" );
		return 0;
	}
	motiondetect_exit = 1;
	//wait capture thread return
	pthread_join(TID, NULL);
	TID	= thread_zeroid();

	Motion_Detector_Close( hMdetector );
	hMdetector = NULL;

	#if 0
	if(NULL != LatestData)
	{
		free(LatestData);
	}
	#endif
	//bzero( CompareData, sizeof( CompareData ) );
	for (int i = 0; i< BUFFER_NUM; i++)
	{
		free(MDData.Data[i].pData);
	}
	
	return 0;
}

/**
* @brief  iupdate the video width and height
* @author hankejia
* @date 2012-07-05
* @param[int] nVideoWidth, nVideoHeight
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
#if 0
T_S32 SetVideoResolution( T_U32 nVideoWidth, T_U32 nVideoHeight, T_MOTION_DETECTOR_DIMENSION	*pDimension )
{
	if ( !hMdetector ) {
		loge( "open motion detect first!\n" );
		return -1;
	}

	if ( ( nWidth == nVideoWidth ) && ( nHeight == nVideoHeight ) ) {
		return 0;
	}

	//change the video width and height, Motion_Detector_Handle will use these new width and
	//height to compare the data
	if ( Motion_Detector_SetDimension( hMdetector, nVideoWidth, nVideoHeight, pDimension ) ) {
		nWidth = nVideoWidth;
		nHeight = nVideoHeight;
		return 0;
	}

	loge( "motion detector can't set the dimension!\n" );
	return -1;
}
#endif

/**
* @brief  update the compare video data
* @author hankejia
* @date 2012-07-05
* @param[in] pData	video data
* @return T_S32
* @retval	if return 1 success and update the first data
*		if return 2 success and update the second data, do the compare.
*		and the compare result is no motion beteen these two video frame.
*		if return 3 success and update the second data, do the compare.
*		and the compare result is have motion happen beteen these two video frame.
*		otherwise failed
*/


T_S32 UpdateCompareData( T_U8 * pData, int size )
{
	T_BOOL MotionHappen = AK_FALSE;
	
	if ( !hMdetector ) {
		loge( "open motion detect first!\n" );
		return -1;
	}

//	printf("i = %d \n", ++i);
	
	if (!LatestData)
	{

		LatestData = (T_U8 *)malloc(size);
		
		if (!LatestData)
		{
			printf("malloc buff err \n");
			return -1;
		}
		else
		{
			
			memcpy(LatestData, pData, size);
			return 0;
		}
	}

	MotionHappen = Motion_Detector_Handle(hMdetector, LatestData, pData);
	
	if (MotionHappen) 
	{
#if 0	
		long fd = -1;
		char fileName[20];
		
		
		sprintf(fileName, "/mnt/motion_%da", i);
		fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC);
		if (fd > 0)
		{
			write(fd, LatestData, size);
			close(fd);
		}

		sprintf(fileName, "/mnt/motion_%db", i);
		fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC);
		if (fd > 0)
		{
			write(fd, pData, size);
			close(fd);
		}
#endif		
		//printf("@@@ Motion @@@: %d\n", size);
				
		memcpy(LatestData, pData, size);
		return 3;
	}
	
	memcpy(LatestData, pData, size);
	return 2;
}

/*
 * @brief		allocate memory
 * @param	size[in] ,filename[in], line[in]
 * @return	T_pVOID
 * @retval	NULL for error,the handle of memory allocated if succeed.
 */
static T_pVOID md_malloc(T_U32 size, T_pSTR filename, T_U32 line)
{
	return malloc(size);
}

/*
 * @brief		free memory
 * @param	mem[in]
 * @return	T_pVOID
 * @retval	the handle of the freed memory
 */
static T_pVOID md_free(T_pVOID mem)
{
	free(mem);
	return mem;
}

T_S32 MotionDetect_writedata(T_pVOID pdata, T_U32 size)
{	
	if (((MDData.wpos+1)%BUFFER_NUM) == MDData.rpos)
	{
		return -1;
	}

	memcpy(MDData.Data[MDData.wpos].pData, pdata, size);
	MDData.Data[MDData.wpos].size = size;
	MDData.wpos++;
	if (MDData.wpos >= BUFFER_NUM)
	{
		MDData.wpos = 0;
	}
	return 0;
}


T_S32 MotionDetect_getpdata(T_pDATA *pdata, T_U32 *size)
{	
	if (MDData.wpos == MDData.rpos)
	{
		return -1;
	}

	*pdata = MDData.Data[MDData.rpos].pData;
	*size = MDData.Data[MDData.rpos].size;
	return 0;
}

T_S32 audio_process_useok()
{	
	MDData.rpos++;
	
	if (MDData.rpos >= BUFFER_NUM)
	{
		MDData.rpos = 0;
	}
	
	return 0;
}

void* thread_MotinDetec(void *usr)
{
	T_pDATA pbuf = NULL;
	T_U32  size;
	//struct timeval tv, tv1;
	
	while(1)
	{
		if( 1 == motiondetect_exit )
		{
			printf("motiondetect exit \n");
			break;
		}
		if (MotionDetect_getpdata(&pbuf, &size) < 0)
		{
			usleep(10000);
			continue;
		}
		
		//printf("pbuf = %p, size is = %d\n", pbuf, size);
		//gettimeofday(&tv, NULL);
		if (3== UpdateCompareData(pbuf, size))
		{
			pic = 1;
			//printf("Motin \n");
			//photograph(pbuf, offset);
		}
		
		//gettimeofday(&tv1, NULL);
		//printf("T: %d\n", (tv1.tv_sec-tv.tv_sec)*1000 + (tv1.tv_usec-tv.tv_usec)/1000);
		audio_process_useok();
	}

	return NULL;
}
