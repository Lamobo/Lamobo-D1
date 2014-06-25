#ifndef IPERF_AKDEMO_CYCBUFFER
#define IPERF_AKDEMO_CYCBUFFER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <memory.h>
//#include <stdbool.h>

#include "headers.h"
#include "Condition.h"
#include "simulate_class.h"

typedef struct CCycBuffer
{
	//declare the constructor function is point
	DECLARE_CONSTRUCTOR_POINT( CCycBuffer );
	//declare the destructor fucntion is point
	DECLARE_DESTRUCTOR_POINT( CCycBuffer );

	//设置循环buffer的size
	T_S32 (*SetBufferSize)( T_pVOID pthis, T_S32 iSize );

	//创建循环buffer
	T_S32 (*CreateCycBuffer)( T_pVOID pthis );

	//前两函数的合并。
	T_S32 (*CreateCycBufferEx)( T_pVOID pthis, T_S32 iSize );

	//从循环Buffer中弹出指定长度的数据。改变循环Buffer的读写指针
	T_S32 (*Pop)( T_pVOID pthis, T_CHR * pBuffer, T_S32 iSize );

	//向循环Buffer中压入指定长度的数据。改变循环Buffer的读写指针
	T_S32 (*Push)( T_pVOID pthis, T_CHR * pBuffer, T_S32 iSize );

	//向循环Buffer中压入指定长度的数据。改变循环Buffer的读写指针
	//此接在LCD和mmc设备复用时起到当LCD上共享锁时，避免上层应用
	//的push线程被间接锁上，但只适用于只有一个线程Push,另一个线
	//程Pop的情况，如果有多个线程需要push，则不都能使用此接口，
	//否则冲乱读数据。
	T_S32 (*PushSingle)( T_pVOID pthis, T_CHR * pBuffer, T_S32 iSize );
	
	//将循环Buffer中从读指针开始的iSize个数据写入指定的文件中
	//此接口只能和PushSingle配合使用，只适用同PushSingle的情况下使用。
	T_S32 (*WriteToFs)( T_pVOID pthis, T_S32 fd, T_S32 iSize );

	//将循环buffer中当前所有的数据写入到文件中。
	T_S32 (*flush)( T_pVOID pthis, T_S32 fd );

	//获取循环Buffer的Size
	T_S32 (*GetBufferSize)( T_pVOID pthis );

	//获取循环Buffer还有多少空闲的空间。 (未使用过的空间)
	T_S32 (*GetIdleSize)( T_pVOID pthis );

	//获取循环Buffer中使用的空间。
	T_S32 (*GetUsedSize)( T_pVOID pthis );

	//循环Buffer是否为空。
	T_BOOL (*IsEmpty)( T_pVOID pthis );

	//clean 循环Buffer
	T_S32 (*Clean)( T_pVOID pthis );

	//强制所有在pop/push操作中等待的线程离开。
	T_S32 (*ForceQuit)( T_pVOID pthis );

	//恢复强制退出状态，在调用ForceQuit后如果想继续使用pop/push功能
	//请先调用此接口。
	T_S32 (*ResumeForceQuitState)( T_pVOID pthis );
	
	T_S32 (*DestroyCycBuffer)( T_pVOID pthis );	

	//将循环Buffer的状态修改成满状态。
	T_S32 (*FakePushFull)( T_pVOID pthis );
	
	T_pVOID	handle;
}CCycBuffer;

REGISTER_SIMULATE_CLASS_H( CCycBuffer );

#ifdef __cplusplus
}
#endif

#endif
