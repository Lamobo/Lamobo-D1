#ifndef HEADERS_H
#define HEADERS_H

/* turn off assert debugging */
#define NDEBUG

/* standard C headers */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define HAVE_POSIX_THREAD

/* required on AIX for FD_SET (requires bzero).
 * often this is the same as <string.h> */
#ifdef HAVE_STRINGS_H
    #include <strings.h>
#endif // HAVE_STRINGS_H

/* unix headers */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include <syslog.h>

#ifdef HAVE_POSIX_THREAD
    #include <pthread.h>
#endif // HAVE_POSIX_THREAD

#include "anyka_types.h"
#include "simulate_class.h"

#ifndef UNUSED
#define UNUSED(x)	((void)(x))	/* to avoid warnings */
#endif

#define FAKE_TIME_STAMP

//#define NOICE

#define VGA_WIDTH		640//640//352//480
#define VGA_HEIGHT		480//480//288//272
#define DEFAULT_FPS		25

//#define VIDEO_BUF_LEN	30720//61440//30720//30K//57344

//#define DEBUG
#define OUTPUT_TIME		0

//#define CONFIG_RESAMPLE
#define MAX_PATH	1024

typedef struct demo_setting {
	char *					strHelpString;
	unsigned long width;			//实际编码图像的宽度，能被4整除
	unsigned long height;			//实际编码图像的长度，能被2整除
	unsigned char kbpsmode; 
	long qpHdr;			//初始的QP的值
	long iqpHdr;			//初始的i帧的QP值
	long bitPerSecond;	//目标bps
	long minQp;
	long maxQp;
	long framePerSecond;
	long video_types;
	long audioType;	
	char bhasAudio;
	unsigned long abitsrate;
	unsigned long aSamplerate;

	unsigned long enc_time; //编码时间，单位秒;
	char *rec_path; //编码路径;
}demo_setting;
#ifdef __cplusplus
extern "C"{
#endif

extern T_S32 g_bNeedPrintf;

#ifdef __cplusplus
}
#endif


#endif /* HEADERS_H */

