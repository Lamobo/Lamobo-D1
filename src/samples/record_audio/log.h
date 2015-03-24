/* 
 * @file log.h
 * @brief
 *
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author
 * @date 2012-7-18
 * @version 1.0
 */
#ifndef __log_h__
#define __log_h__

#ifdef __cplusplus
extern "C"{
#endif

#include "anyka_types.h"

/*
 * init the log
 * input param:
 * none
 * return value:
 * none
 */
T_VOID log_init();

/*
 * set log leavel
 * input param:
 * level:
 * log leavel
 * return value:
 * none
 */
T_VOID log_set_level(T_S32 level);

/*
 * write log
 * input param:
 * level: level 
 * fmt: format string
 * return value:
 * none
 */
T_VOID log_write(T_S32 level, T_pCSTR fmt, ...);

#define LOG_LEVEL_DEBUG   0
#define LOG_LEVEL_INFO    1
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_ERROR   3

#define LOG_DEFAULT_LEVEL   LOG_LEVEL_DEBUG

#define  logd( fmt... )   log_write( LOG_LEVEL_DEBUG, fmt)
#define  logi( fmt... )   log_write( LOG_LEVEL_INFO, fmt)
#define  logw( fmt... )   log_write( LOG_LEVEL_WARN, fmt)
#define  loge( fmt... )   log_write( LOG_LEVEL_ERROR, fmt)

#define  LOGV( fmt... )	  log_write( LOG_LEVEL_DEBUG, fmt)
#define	 LOGE( fmt... )	  log_write( LOG_LEVEL_ERROR, fmt)

#ifndef LOG_TAG
#define LOG_TAG "Linux_AudioDemo"
#endif

#ifdef __cplusplus
}
#endif

#endif
