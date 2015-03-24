#ifndef __log_h__
#define __log_h__

#ifdef __cplusplus
extern "C"{
#endif

#include "anyka_types.h"

T_VOID log_init();
T_VOID log_set_level(T_S32 level);
T_VOID log_write(T_S32 level, T_pCSTR fmt, ...);

#define LOG_LEVEL_DEBUG   0
#define LOG_LEVEL_INFO    1
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_ERROR   3

#define LOG_DEFAULT_LEVEL   LOG_LEVEL_INFO

#define  logd( fmt... )   log_write( LOG_LEVEL_DEBUG, fmt)
#define  logi( fmt... )   log_write( LOG_LEVEL_INFO, fmt)
#define  logw( fmt... )   log_write( LOG_LEVEL_WARN, fmt)
#define  loge( fmt... )   log_write( LOG_LEVEL_ERROR, fmt)

#define  LOGV( fmt... )	  log_write( LOG_LEVEL_DEBUG, fmt)
#define	 LOGE( fmt... )	  log_write( LOG_LEVEL_ERROR, fmt)

#ifndef LOG_TAG
#define LOG_TAG "Aimer39_VideoRecorder_Demo"
#endif

#ifdef __cplusplus
}
#endif

#endif
