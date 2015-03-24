#ifndef _AUDIO_PROCESS_H_
#define _AUDIO_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"
#include "audio_enc.h"

#define AUDIO_INFORM_LENGTH (8)

/**
* @brief  open the AK audio process
* @author dengzhou 
* @date 2013-04-25
* @param[in] NONE
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/

T_S32 audio_process_open( AudioEncIn * pstEncIn, AudioEncOut * pstEncOut, T_U32 oneBufsize );

/**
* @brief  close the AK audio process
* @author dengzhou 
* @date 2013-04-25
* @param[in] NONE
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_process_close();

/**
* @brief  write audio date to process
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 audio_process_writedata(T_pVOID pdata, T_U32 size);


#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif

