#ifndef CAMERA_H
#define CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*FUNC_DMA_MALLOC)(unsigned long size);
typedef void (*FUNC_DMA_FREE)(void *point);

typedef struct {
	FUNC_DMA_MALLOC dma_malloc;
	FUNC_DMA_FREE	dma_free;
	unsigned long width;
	unsigned long height;
} T_CAMERA_INPUT;

/**
* @brief open camera to get picture
* @author dengzhou 
* @date 2013-04-25
* @T_U32 width
* @T_U32 height
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_open(unsigned long width, unsigned long height);


/**
* @brief get one frame from camera
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_getframe(void **pbuf, long *size, unsigned long *timeStamp);

/**
* @brief call this when camera buffer unused
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_usebufok(void *pbuf);

/**
* @brief close camera
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_close(void);

int SetGAMMA(int cid);
int SetOcc(int num, int x1, int y1, int x2, int y2, int enable);
int SaveOcc(int num, int x, int y, int endx, int endy, int enable);
int SetChannel(int width, int height, int enable);
int SetBrightness(int bright);
int Set_Zoom(int z);
int SetSATURATION(int sat);
int SetIsp(unsigned char *pbuf, int size);
int GetIsp_awb(void* awbbuf, int* awbbuflen);

#ifdef __cplusplus
} /* end extern "C" */
#endif
#endif // CAMERA_H
