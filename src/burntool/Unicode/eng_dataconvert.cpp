
#include "stdafx.h"

extern "C"
{
#include "unicode_api.h"
#include "eng_dataconvert.h"
}

static T_RES_LANGUAGE cur_language = eRES_LANG_CHINESE_SIMPLE;

T_RES_LANGUAGE get_cur_language(T_VOID)
{
	return cur_language;//当前语言
}


T_S32 WideCharToMultiByte(T_RES_LANGUAGE lang, 
						  const T_U16 *unicode, 
						  T_U32 ucLen, T_U32 *readedBytes, 
						  T_S8 *pDestBuf, T_U32 destLen, 
						  const T_S8 *defaultChr)
{
	const union cptable *table;
    T_CODE_PAGE code_page = CP_936;
    T_S32 ret;
    int UsedDefCh = 0;
    int *pUsedDefCh = (int *)AK_NULL;
    
    
    switch (lang)
    {
        case eRES_LANG_CHINESE_SIMPLE://简体中文
            code_page = CP_936;
            break;
        case eRES_LANG_CHINESE_TRADITION://繁体中文
            code_page = CP_936;
            break;
	    case eRES_LANG_CHINESE_BIG5://BIG5
            code_page = CP_950;
            break;
	    case eRES_LANG_ENGLISH://英文
            code_page = CP_936;
            break;
        default:
            code_page = CP_936;//
            break;
    }

    table = cp_get_table( code_page );//
    
    pUsedDefCh = (int *)((defaultChr) ? (&UsedDefCh) : AK_NULL);
    ret = cp_wcstombs( table, 0,
                      unicode, ucLen,
                      (char *)pDestBuf, destLen, (const char *)defaultChr, pUsedDefCh );

    if (destLen && pDestBuf)
    {
        if (ret >= 0)
        {
            if (ret < (T_S32)destLen)
                pDestBuf[ret] = 0;
        }
        else
        {
            pDestBuf[0] = 0;
        }
    }

    return ret;
}

T_S32 MultiByteToWideChar(T_RES_LANGUAGE lang, const T_S8 *src, T_U32 srcLen, T_U32 *readedBytes, T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr)
{
	const union cptable *table;
    T_CODE_PAGE code_page = CP_936;
    T_S32 ret;
    
    switch (lang)
    {
        case eRES_LANG_CHINESE_SIMPLE://简体
            code_page = CP_936;
            break;
        case eRES_LANG_CHINESE_TRADITION://繁体
            code_page = CP_936;
            break;
	    case eRES_LANG_CHINESE_BIG5://BIG5
            code_page = CP_950;
            break;
	    case eRES_LANG_ENGLISH://英文
            code_page = CP_936;
            break;
        default:
            code_page = CP_936;//
            break;
    }

    table = cp_get_table( code_page );
    
    ret = cp_mbstowcs( table, 0,
                      (const char *)src, srcLen,
                      ucBuf, ucBufLen, defaultUChr );

    if (ucBufLen && ucBuf)
    {
        if (ret >= 0)
        {
            if (ret < (T_S32)ucBufLen)
                ucBuf[ret] = 0;
        }
        else
        {
            ucBuf[0] = 0;
        }
    }
    
    return ret;
}

/**
 * @brief Convert UNICODE string to ansi string, just for file system lib 
 * @param[in]   UniStr     source UNICODE string
 * @param[in]   UniStrLen       the length of source string, in UNICODE char unit
 * @param[out]  pAnsibuf        the output ansi string buffer
 * @param[in]   AnsiBufLen indicate the output ansi string buffer size, in bytes
 * @param[in]   code      language
 * @return T_S32
 * @retval if AnsiBufLen is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string
 * @retval if AnsiBufLen is not zero, the return value is the number of bytes written to the buffer pointed to by pAnsi
 */
T_S32 UniStr2AnsiStr(const T_U16 *pUniStr, T_U32 UniStrLen, T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code)
{
    return WideCharToMultiByte((T_RES_LANGUAGE)code, pUniStr, UniStrLen, (T_U32 *)AK_NULL, pAnsibuf, AnsiBufLen, (const T_S8 *)AK_NULL);
}

/**
 * @brief Convert ansi string to UNICODE string, just for file system lib 
 * @param[in] pAnsiStr       source ansi string
 * @param[in] AnsiStrLen    the length of source string, in bytes
 * @param[out] pUniBuf    the output UNICODE string buffer
 * @param[in] UniBufLen  indicate the output UNICODE string buffer size, in UNICODE unit
 * @return T_S32
 * @retval if ucBufLen is zero, the return value is the required size, in UNICODE char, for a buffer that can receive the translated string
 * @retval if ucBufLen is not zero, the return value is the number of UNICODE chars written to the buffer pointed to by ucBuf
 */
T_S32 AnsiStr2UniStr(const T_pSTR pAnsiStr, T_U32 AnsiStrLen, T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code)
{
    return MultiByteToWideChar((T_RES_LANGUAGE)code, pAnsiStr, AnsiStrLen, (T_U32 *)AK_NULL, pUniBuf, UniBufLen, (const T_U16 *)AK_NULL);
}
