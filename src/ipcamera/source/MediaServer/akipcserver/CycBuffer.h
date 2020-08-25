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

	//����ѭ��buffer��size
	T_S32 (*SetBufferSize)( T_pVOID pthis, T_S32 iSize );

	//����ѭ��buffer
	T_S32 (*CreateCycBuffer)( T_pVOID pthis );

	//ǰ�������ĺϲ���
	T_S32 (*CreateCycBufferEx)( T_pVOID pthis, T_S32 iSize );

	//��ѭ��Buffer�е���ָ�����ȵ����ݡ��ı�ѭ��Buffer�Ķ�дָ��
	T_S32 (*Pop)( T_pVOID pthis, T_CHR * pBuffer, T_S32 iSize );

	//��ѭ��Buffer��ѹ��ָ�����ȵ����ݡ��ı�ѭ��Buffer�Ķ�дָ��
	T_S32 (*Push)( T_pVOID pthis, T_CHR * pBuffer, T_S32 iSize );

	//��ѭ��Buffer��ѹ��ָ�����ȵ����ݡ��ı�ѭ��Buffer�Ķ�дָ��
	//�˽���LCD��mmc�豸����ʱ�𵽵�LCD�Ϲ�����ʱ�������ϲ�Ӧ��
	//��push�̱߳�������ϣ���ֻ������ֻ��һ���߳�Push,��һ����
	//��Pop�����������ж���߳���Ҫpush���򲻶���ʹ�ô˽ӿڣ�
	//������Ҷ����ݡ�
	T_S32 (*PushSingle)( T_pVOID pthis, T_CHR * pBuffer, T_S32 iSize );
	
	//��ѭ��Buffer�дӶ�ָ�뿪ʼ��iSize������д��ָ�����ļ���
	//�˽ӿ�ֻ�ܺ�PushSingle���ʹ�ã�ֻ����ͬPushSingle�������ʹ�á�
	T_S32 (*WriteToFs)( T_pVOID pthis, T_S32 fd, T_S32 iSize );

	//��ѭ��buffer�е�ǰ���е�����д�뵽�ļ��С�
	T_S32 (*flush)( T_pVOID pthis, T_S32 fd );

	//��ȡѭ��Buffer��Size
	T_S32 (*GetBufferSize)( T_pVOID pthis );

	//��ȡѭ��Buffer���ж��ٿ��еĿռ䡣 (δʹ�ù��Ŀռ�)
	T_S32 (*GetIdleSize)( T_pVOID pthis );

	//��ȡѭ��Buffer��ʹ�õĿռ䡣
	T_S32 (*GetUsedSize)( T_pVOID pthis );

	//ѭ��Buffer�Ƿ�Ϊ�ա�
	T_BOOL (*IsEmpty)( T_pVOID pthis );

	//clean ѭ��Buffer
	T_S32 (*Clean)( T_pVOID pthis );

	//ǿ��������pop/push�����еȴ����߳��뿪��
	T_S32 (*ForceQuit)( T_pVOID pthis );

	//�ָ�ǿ���˳�״̬���ڵ���ForceQuit����������ʹ��pop/push����
	//���ȵ��ô˽ӿڡ�
	T_S32 (*ResumeForceQuitState)( T_pVOID pthis );
	
	T_S32 (*DestroyCycBuffer)( T_pVOID pthis );	

	//��ѭ��Buffer��״̬�޸ĳ���״̬��
	T_S32 (*FakePushFull)( T_pVOID pthis );
	
	T_pVOID	handle;
}CCycBuffer;

REGISTER_SIMULATE_CLASS_H( CCycBuffer );

#ifdef __cplusplus
}
#endif

#endif
