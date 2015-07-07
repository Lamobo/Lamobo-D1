#ifndef CAMERA_H
#define CAMERA_H
#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"

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
int camera_open(demo_setting * Setting);


/**
* @brief get one frame from camera
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_getframe(void **ppBuf, unsigned long *size, unsigned long *timeStamp);

/**
* @brief call this when camera buffer unused
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_usebufok(void);

/**
* @brief close camera
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_close(void);

int camera_start(void);


#ifdef __cplusplus
} /* end extern "C" */
#endif
#endif // CAMERA_H
