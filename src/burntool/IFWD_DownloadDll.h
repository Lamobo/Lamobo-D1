

 /**
 *       Copyright (C) Infineon Technologies Denmark A/S. All rights reserved.
 *
 * This document contains proprietary information belonging to Infineon Technologies 
 * Denmark A/S. Passing on and copying of this document, use and communication
 * of its contents is not permitted without prior written authorisation.
 *
 * Description:
 *   Main inferface file for the IFWD download DLL
 *
 * Revision Information:
 *   File name: \dwdtoolsrc\download_dll\DLL_source\IFWD_DownloadDll.h
 *   Version: \main\55
 *   Date: 2007-06-15 13:25:07
 *   Responsible: Kofod
 *   Comment:
 *     Ver 2.25
 */


#ifndef _INC_IFDW_DL_DLL
#define _INC_IFDW_DL_DLL

#include "IFWD_std_types.h"

/*---------------------------------------------*/
/* Enumeration                                 */
/*---------------------------------------------*/
typedef enum 
{
  IFWD_DL_OK = 0,
  IFWD_DL_Error,
  IFWD_DL_ProgressUpdated,
  IFWD_DL_ProcessOutlineUpdated,
  IFWD_DL_ProcessDetailUpdated,
  IFWD_DL_PhoneOn,
  IFWD_DL_AT_Command,  
  IFWD_DL_status_force_U32_enum = 0xfffff
} IFWD_DL_status_enum;

typedef enum
{
  IFWD_DL_text_class_ifx_error_code = 0,
  IFWD_DL_text_class_os_api_error_code,
  IFWD_DL_text_class_process_info
} IFWD_DL_text_class_enum;

typedef enum
{
  IFWD_DL_mco_no_change = 0,
  IFWD_DL_mco_set_to_0,
  IFWD_DL_mco_set_to_1,
  IFWD_DL_mco_pulse_0,   /* Set before boot and reset when tagat and PC is synchronized */
  IFWD_DL_mco_pulse_1
} IFWD_DL_modem_control_output_enum;

typedef enum
{
  IFWD_DL_mci_is_0 = 0,
  IFWD_DL_mci_is_1
} IFWD_DL_modem_control_input_enum;

typedef enum
{
  IFWD_DL_dll_parameter_stay_in_function = 0,
  IFWD_DL_dll_parameter_boot_process_timeout,
  IFWD_DL_dll_parameter_comm_timeout,
  IFWD_DL_dll_parameter_use_pre_erase,
  IFWD_DL_dll_parameter_return_unformatted_text,
  IFWD_DL_dll_parameter_skip_empty_blocks,
  IFWD_DL_dll_parameter_faster_crc_method,
  IFWD_DL_dll_parameter_lower_multichannel_CPU_load,
  IFWD_DL_dll_parameter_erase_mode,  /* IFWD_DL_dll_parameter_erase_mode_enum */
  IFWD_DL_dll_parameter_check_file_size,
  IFWD_DL_dll_parameter_erase_sector_verify,
  IFWD_DL_dll_parameter_flash_debug,
  IFWD_DL_dll_parameter_force_area_erase
} IFWD_DL_dll_parameter_enum;

typedef enum
{
  IFWD_DL_dll_get_parameter_filechecksum = 0
} IFWD_DL_dll_get_parameter_enum;



typedef enum
{
  IFWD_DL_target_erase_before_write = 0,
  IFWD_DL_target_erase_all,
  IFWD_DL_target_erase_nothing,
  IFWD_DL_target_erase_last_valid = IFWD_DL_target_erase_nothing /* used for valid range check */
} IFWD_DL_dll_parameter_erase_mode_enum;

typedef enum
{
  IFWD_DL_target_boot_mode_normal = 0, /* The phone start in normal mode after EEP file download */
  IFWD_DL_target_boot_mode_test        /* The phone start in test mode after EEP file download */
} IFWD_DL_target_boot_mode_enum;

typedef enum
{
  IFWD_DL_dffs_load_sel_none = 0, /* No part of the DFFS file is downloaded.  */
  IFWD_DL_dffs_load_sel_static,   /* Only the static part of the DFFS file is downloaded */
  IFWD_DL_dffs_load_sel_dynamic,  /* Only the dynamic part of the DFFS file is downloaded */ 
  IFWD_DL_dffs_load_sel_both,     /* Both parts are downloaded */
  IFWD_DL_dffs_load_sel_nand_disk
} IFWD_DL_dffs_load_sel_enum;

/*---------------------------------------------*/
/* Structures                                  */
/*---------------------------------------------*/



typedef struct
{
  U8 DTR;  /* Out: IFWD_DL_modem_control_output_enum */
  U8 RTS;  /* Out: IFWD_DL_modem_control_output_enum */
  U8 CTS;  /*  In: IFWD_DL_modem_control_input_enum  */
  U8 DSR;  /*  In: IFWD_DL_modem_control_input_enum  */
  U8 RING; /*  In: IFWD_DL_modem_control_input_enum  */
  U8 RLSD; /*  In: IFWD_DL_modem_control_input_enum  */
} IFWD_DL_modem_control_signals_type;
 
/*---------------------------------------------*/
/* External defines                            */
/*---------------------------------------------*/

/*---------------------------------------------*/
/* External declarations                       */
/*---------------------------------------------*/

typedef struct
{
  void *(*fopen)( const char *filename, const char *mode );
  int (*fseek)(void *stream, long offset, int origin );
  long (*ftell)(void *stream );
  int (*fclose)(void *stream );
  int (*Feof)(void *stream );
  size_t (*fread)( void *buffer, size_t size, size_t count, void *stream );
  size_t (*fwrite)( const void *buffer, size_t size, size_t count, void *stream );
  int (*remove)( const char *path );
  char (*fgets)(char *line, int maxline, void *stream); 
}IFWD_ExtFileHandleType;

extern IFWD_ExtFileHandleType *IFWD_GLOBAL_ExtFileHandle;

/*---------------------------------------------*/
/* External functions                          */
/*---------------------------------------------*/
#ifdef TEST_MMI
  #define DECLSPEC  __declspec(dllimport) 
#else
  #ifdef __BORLANDC__	
    #define DECLSPEC  __declspec(dllexport)  __cdecl
  #else
    #define DECLSPEC
  #endif
#endif

#ifndef __BORLANDC__
#ifdef DECLSPEC
#undef DECLSPEC
#endif
#define DECLSPEC
#endif


#ifdef TEST_MMI
#ifdef __cplusplus
extern "C" {
#endif
#endif
          IFX_CHAR *DECLSPEC IFWD_DL_get_dll_version(void);
IFWD_DL_status_enum DECLSPEC IFWD_DL_get_sw_version_data(U8 channel,IFX_CHAR *fls_file_name,U8 *file_data,U8 *taget_data,U16 data_size,IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_set_dll_parameter(/*IFWD_DL_dll_parameter_enum*/ U8 parameter_code, U32 parameter_value, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_dll_get_parameter(/*IFWD_DL_dll_get_parameter_enum*/ U8 parameter_code, U32 *result_output, IFX_CHAR *input_parameter);
IFWD_DL_status_enum DECLSPEC IFWD_DL_init_callback(void (*HostCallBack)(U8 channel, IFWD_DL_status_enum status, IFX_CHAR *status_text), IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_open_comm_port(U8 channel, IFX_CHAR *comm_port_name, IFX_CHAR *ctrl_port_name, U32 baud_rate, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_close_comm_port(U8 channel, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_boot_target(U8 channel, IFX_CHAR *ref_file, IFWD_DL_modem_control_signals_type *modem_control_signals, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_boot_bin_target(U8 channel, U8 *ref_file_header, IFWD_DL_modem_control_signals_type *modem_control_signals, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_set_get_control_signal(U8 channel, IFWD_DL_modem_control_signals_type *modem_control_signals, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_download_fls_file(U8 channel, IFX_CHAR *fls_file_name, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_download_eep_file(U8 channel, IFX_CHAR *eep_file_name,  /*IFWD_DL_target_boot_mode_enum*/ U8 boot_mode, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_download_dffs_file(U8 channel, IFX_CHAR *dffs_file_name, /*IFWD_DL_dffs_load_sel_enum*/ U8 load_selection, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_download_cust_file(U8 channel, IFX_CHAR *cust_file_name, IFX_CHAR *Status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_upload_eep_file(U8 channel, IFX_CHAR *template_file_name, IFX_CHAR *uploaded_eep_file_name, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_update_static_eeprom(U8 channel, IFX_CHAR *dep_file_name, IFX_CHAR *project_prefix_string, IFX_CHAR *ref_eep_file_name, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_upload_bin_image(U8 channel, U32 image_start_address, U32 image_length, IFX_CHAR *bin_file_name, IFX_CHAR *ref_file_name, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_download_bin_image(U8 channel, U32 image_start_address, U32 image_length, U32 image_offset, IFX_CHAR *bin_file_name, IFX_CHAR *ref_file_name, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_erase_image(U8 channel, U32 image_start_address, U32 image_length,IFX_CHAR *ref_file_name, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_verify_flash_image(U8 channel, U32 image_start_address, U32 image_length, IFX_CHAR *ref_file_name, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_abort_process(U8 channel, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_force_target_reset(U8 channel, U8 mode, U8 p1, U8 p2, U8 p3, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_end_of_process(U8 channel, IFX_CHAR *status);

/* AT-Mode functions: */
IFWD_DL_status_enum DECLSPEC IFWD_DL_start_AT_mode(U8 channel, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_stop_AT_mode(U8 channel, IFX_CHAR *status);
IFWD_DL_status_enum DECLSPEC IFWD_DL_AT_send(U8 channel, IFX_CHAR *data, U16 length, IFX_CHAR *status);

/* Memory based file handling emulation: */
               void DECLSPEC IFWD_DL_set_external_file_handling(IFWD_ExtFileHandleType *ExtFileHandlers);
/* NOTE: No files in any channels are allowed to be open when calling this function.
         Failure to comply with this can lead to memory leaks and unclosed files, and invalid handles.  */

#ifdef TEST_MMI
#ifdef __cplusplus
 }
#endif
#endif


#endif


