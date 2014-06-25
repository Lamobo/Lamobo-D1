#ifndef __SDCARD_H__
#define __SDCARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SD_EVENT_BUF_SIZE  				4096

T_S32 InitListenSD( void );
void CloseListenSD( void );

int check_sdcard( void );
void umount_sd(void);

void mount_sd(void);



#ifdef __cplusplus
}
#endif

#endif
