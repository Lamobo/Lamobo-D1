#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>

#define ERROR_FD  stderr
#define LOG_ERROR "E/"
#define LOGE(...) fprintf(ERROR_FD, LOG_ERROR LOG_TAG "  :  " __VA_ARGS__)

#define WARN_FD   stderr
#define LOG_WARN  "W/"
#define LOGW(...) fprintf(WARN_FD, LOG_WARN LOG_TAG "  :  " __VA_ARGS__)

#define INFO_FD   stdout
#define LOG_INFO  "I/"
#define LOGI(...) fprintf(INFO_FD, LOG_INFO LOG_TAG "  :  " __VA_ARGS__)

#define DEBUG_FD  stdout
#define LOG_DEBUG "D/"
#define LOGD(...) //fprintf(DEBUG_FD, LOG_DEBUG LOG_TAG "  :  " __VA_ARGS__)

#endif /* __LOG_H */
