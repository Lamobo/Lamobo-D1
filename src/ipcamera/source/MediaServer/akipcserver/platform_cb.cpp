/* @file platform_cb.cpp
 * @brief declare platform callback function
 *
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author
 * @date 2012-7-18
 * @version 1.0
 */
 
#include "akuio.h"
#include "platform_cb.h"

/*
 * 打印回调函数
 * 参数：
 * format:同printf参数定义
 * 返回值:
 * 无
 */
T_VOID libPrintf(T_pCSTR format, ...)
{
    printf(format);
}

/*
 * 分配内存回调函数
 * 参数:
 * size:分配内存的字节数
 * 返回值:
 * 同malloc函数的返回值
 */
T_pVOID libMalloc(T_U32 size)
{
    return malloc(size);
}

/*
 * 释放内存回调函数
 * 参数:
 * mem:释放的内存指针
 * 返回值:
 * 同free函数的返回值
 */
T_VOID libFree(T_pVOID mem)
{
    free(mem);
}

/*
 * 延时回调函数
 * 参数:
 * ticks:延时的tick数，每个tick是5ms
 * 返回值：
 * true:成功
 * false:失败
 */
T_BOOL lnx_delay(T_U32 ticks)
{
    return (usleep (ticks*5000) == 0);
}

/*
 * 延时回调函数,仅提供给视频解码使用
 * 参数:
 * ticks:延时的tick数，每个tick是5ms
 * 返回值：
 * true:成功
 * false:失败
 */
T_BOOL vd_delay(T_U32 ticks)
{
    akuio_wait_irq();
    return true;
}

/*
 * 获取当前时间
 * 参数：
 * 无
 * 返回值：
 * 时间值，单位ms
 */
unsigned long get_system_time_ms()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000 + now.tv_usec / 1000);
}
