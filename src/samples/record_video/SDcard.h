#ifndef __SDCARD_H__
#define __SDCARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SD_EVENT_BUF_SIZE  				4096

T_S32 InitListenSD( void );
void CloseListenSD( void );
/*
*@BRIEF  call sys command mount the sd card
*@AUTHOR Li_Qing
*@DATA 2012-08-08
*/
void mount_sd(void);





#ifdef __cplusplus
}
#endif

#endif
