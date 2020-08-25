#ifndef LINUX_MEDIA_RECORDER_AkRecordManager
#define LINUX_MEDIA_RECORDER_AkRecordManager

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"
#include "inisetting.h"
typedef enum
{
	REC_FUNC_TYPE_NORMAL,
	REC_FUNC_TYPE_CYC,
	REC_FUNC_TYPE_COUNT
}REC_FUNC_TYPE;

T_S32 SetMinRecordLimit( T_U32 nMinLimitSize );

T_S32 recmgr_open( T_pSTR pstrRecPath, T_U32 nFileBitRate, 
							   T_U32 nRecordDuration, T_BOOL bIsCyc );

T_S32 GetRecordFile( T_pSTR filename );

T_BOOL ReachLimit( T_U32 nFileLen, T_U32 nFileDuration );

REC_FUNC_TYPE GetRecordFunction();

T_VOID CloseRecordManager();

T_BOOL IsOurCycFile( T_pSTR pstrFileName );

int ChangFileName( void );
int delete_oldest_file();
#ifdef __cplusplus
}
#endif

#endif
