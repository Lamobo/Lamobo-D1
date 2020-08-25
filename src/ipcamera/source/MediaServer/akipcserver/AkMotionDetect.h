#ifndef LINUX_MEDIA_RECORDER_AkMotionDetect
#define LINUX_MEDIA_RECORDER_AkMotionDetect

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"

#define BUFFER_NUM 				5

typedef struct {
	//buffer manage
	T_pDATA pData;
	T_U32 size;
}T_MD_DATA;

typedef struct {

	//buffer manage
	T_MD_DATA Data[BUFFER_NUM];
	T_U32 wpos;
	T_U32 rpos;

}T_MD_PROC;


/**
* @brief  open the motion detect
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 OpenMotionDetect(T_U16 width, T_U16 height, T_U16 *ratio);

/**
* @brief  close the motion detect
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 CloseMotionDetect();

/**
* @brief  iupdate the video width and height
* @author hankejia
* @date 2012-07-05
* @param[int] nVideoWidth, nVideoHeight
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
//T_S32 SetVideoResolution( T_U32 nVideoWidth, T_U32 nVideoHeight, T_MOTION_DETECTOR_DIMENSION	*pDimension );

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
T_S32 UpdateCompareData( T_U8 * pData, int size );

T_S32 MotionDetect_writedata(T_pVOID pdata, T_U32 size);


#ifdef __cplusplus
}
#endif

#endif
