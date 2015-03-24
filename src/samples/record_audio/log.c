
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

//#include <linux/stat.h>

#include "log.h"

static T_S32 log_fd = -1; //log file is fd
static T_S32 log_level = LOG_DEFAULT_LEVEL; //min out put level
#define LOG_BUF_MAX 512

/**
* @brief  set the out put level
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_VOID
* @retval NONE
*/
T_VOID log_set_level(T_S32 level) {
    log_level = level;
}

/**
* @brief  init the log writer
* @author hankejia
* @date 2012-07-05
* @param none
* @return T_VOID
* @retval NONE
*/
T_VOID log_init(T_VOID)
{
    static const char *name = "/dev/__kmsg__";
    if (mknod(name, S_IFCHR | 0600, (1 << 8) | 11) == 0) {
        log_fd = open(name, O_WRONLY);
        fcntl(log_fd, F_SETFD, FD_CLOEXEC);
        unlink(name);
    }
}


/**
* @brief  write the log
* @author hankejia
* @date 2012-07-05
* @param[in] level	log level.
* @param[in] fmt	string format.
* @param[in] ...	other param.
* @return T_VOID
* @retval NONE
*/
T_VOID log_write(T_S32 level, T_pCSTR fmt, ...)
{
    T_CHR buf[LOG_BUF_MAX];
    va_list ap;

    
    if (level < log_level) return;
    if (log_fd < 0) return;
    
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_MAX, fmt, ap);
    buf[LOG_BUF_MAX - 1] = 0;
    va_end(ap);
    
    
    write(log_fd, LOG_TAG,strlen(LOG_TAG));
	write(log_fd, "::", 3);
    write(log_fd, buf, strlen(buf));
}

