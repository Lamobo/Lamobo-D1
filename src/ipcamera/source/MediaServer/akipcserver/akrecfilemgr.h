#ifndef LINUX_MEDIA_RECORDER_AkRecordManager
#define LINUX_MEDIA_RECORDER_AkRecordManager

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"

#define SIGN_FILE_NAME							".LMR_RecordManger_Sign"
#define FILE_SUFFIX								".avi"
#define FILE_NAME_DIF							"(New)"
#define DIR_SEPARATOR							'/'
#define CYC_FILE_PREFIX							"CYC_DV_"
#define REC_FILE_PREFIX							"DV_"

typedef enum
{
	REC_FUNC_TYPE_NORMAL,
	REC_FUNC_TYPE_CYC,
	REC_FUNC_TYPE_COUNT
}REC_FUNC_TYPE;

T_S32 RecFile_setMinSize( T_U32 nMinLimitSize );

T_S32 RecFile_open( T_pSTR pstrRecPath, T_U32 nFileBitRate, 
							   T_U32 nRecordDuration, T_BOOL bIsCyc );

T_S32 RecFile_openFile(T_pSTR *filename);

T_BOOL RecFile_limitSize(T_U32 nFileLen, T_U32 nFileDuration );
T_S32 RecFile_removeOldFile(T_pSTR path, T_BOOL bNeedLast );

T_VOID RecFile_free(T_VOID);
T_BOOL RecFile_isCycRec(T_VOID);

T_BOOL RecFile_isCycFile(T_pSTR pFileName);

int RecFile_rename(T_pSTR fileName, T_pSTR newName);

#ifdef __cplusplus
}
#endif

#endif
