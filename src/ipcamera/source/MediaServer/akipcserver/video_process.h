#ifndef _VIDEO_PROCESS_H_
#define _VIDEO_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif
int video_process_start();
void video_process_stop();
int video_process_get_buf(void* buf, unsigned* nlen, int nNeedIFrame, struct timeval* ptv, int index);
int video_process_get_buf_formux(void* buf, unsigned int* nlen, unsigned int* ts);
void video_process_SetViewFlag(int nFlag);
void video_process_SetRecordFlag(int nFlag);
#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
