#ifndef __AKFONT_LIB_H__
#define __AKFONT_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "headers.h"

typedef T_U8 T_FONT_CODE;

typedef enum  {
    FONT_SIZE_16,
    FONT_SIZE_32,
} T_eFONT_SIZE;
T_U8 *FontLib_GetDataByCode(T_FONT_CODE unicode, T_eFONT_SIZE fontsize);


#ifdef __cplusplus
}
#endif
#endif



