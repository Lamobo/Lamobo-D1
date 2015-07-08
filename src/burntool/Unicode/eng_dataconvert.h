#ifndef		__EXFUNCTION_H__
#define		__EXFUNCTION_H__

//#include "crt.h"
#include "file.h"

#ifndef _ANYKA_TYPES_H_
#include "anyka_types.h"
#endif

typedef enum {
	eRES_LANG_CHINESE_SIMPLE,			//简体中文
	eRES_LANG_CHINESE_TRADITION,		//繁体中文
	eRES_LANG_CHINESE_BIG5,				//BIG5码
	eRES_LANG_ENGLISH,					//英文
	eRES_LANG_NUM
}T_RES_LANGUAGE;

// #ifndef E_CODE
// typedef T_U32 E_CODE ;
// #endif

T_S32 UniStr2AnsiStr(const T_U16 *pUniStr, T_U32 UniStrLen, T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code);

T_S32 AnsiStr2UniStr(const T_pSTR pAnsiStr, T_U32 AnsiStrLen, T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code);

T_RES_LANGUAGE get_cur_language(T_VOID);

#endif