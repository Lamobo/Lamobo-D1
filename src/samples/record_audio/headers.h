/* 
 * @file header.h
 * @brief
 *
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author
 * @date 2012-7-18
 * @version 1.0
 */
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

/*
 * define the chip type
 */
typedef enum
{
	MACH_UNKNOW,
	MACH_AK98,
	MACH_AK37,
}T_eAK_MACH_TYPE;


/*
 * this to avoid waring
 */
#ifndef UNUSED
#define UNUSED(x)	((void)(x))	
#endif

#define CHIP_ID_AK37XX

#define bzero(a, b) memset((a), 0, (b))

#define AUDIO_NR
#undef  AUDIO_NR

#define AUDIO_AGC
#undef  AUDIO_AGC

#define AUDIO_RESAMPLE
#undef  AUDIO_RESAMPLE

#endif /* HEADERS_H */

