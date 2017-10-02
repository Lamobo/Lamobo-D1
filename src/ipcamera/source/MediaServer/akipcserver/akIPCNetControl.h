#ifndef _AK_IPC_NETCONTROL_
#define _AK_IPC_NETCONTROL_
#ifdef __cplusplus
extern "C"{
#endif

#include <errno.h>
#include <sys/socket.h>
#include "IPCameraCommand.h"
typedef struct 
{
	char strDeviceID[32];
	char strStreamName1[16];
	char strStreamName2[16];
	VIDEO_MODE vm1;
	VIDEO_MODE vm2;
	int nRtspPort;
	int nMainFps;
	int nSubFps;
}NetCtlSrvPar;

int startNetCtlServer(NetCtlSrvPar* ncsp);
void audio_dec_exit( void );

void InitMotionDetect( void );

void auto_record_file(void);

#ifdef __cplusplus
}
#endif

#endif
