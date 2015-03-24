#ifndef __CAM_COM_SENSOR__
#define __CAM_COM_SENSOR__

#ifdef CONFIG_LINUX_AKSENSOR
#include <mach-anyka/anyka_types.h>
#endif

#define M_DRVSYS            "DRVLIB" /* module name */

#define C1 1    /*Fatal error message*/
#define C2 2    /*Error message*/
#define C3 3    /*Common message*/

typedef enum {
    CAMERA_WMODE_PREV = 0,    //预览模式
    CAMERA_WMODE_CAP,        // 拍照模式
    CAMERA_WMODE_REC        // 录制模式
} T_CAMERA_WORKMODE;

typedef enum {
    PIN_AVDD = 0,    //获取AVDD GPIO 值
    PIN_POWER,        //获取POWER GPIO 值
    PIN_RESET        //获取RESET GPIO 值
} T_CAMERA_PINTYPE;

/** @brief Camera type definition
 *
 *  This structure define the type of camera
 */
typedef enum
{
    CAMERA_P3M  = 0x00000001,
    CAMERA_1P3M = 0x00000002,
    CAMERA_2M   = 0x00000004,
    CAMERA_3M   = 0x00000008,
    CAMERA_4M   = 0x00000010,
    CAMERA_5M   = 0x00000020,
    CAMERA_ZOOM = 0x00000040
}T_CAMERA_TYPE;

/** @brief Camera Parameter Night Mode definition
 *
 *  This structure define the value of parameter Night Mode
 */
typedef enum
{
    CAMERA_DAY_MODE,
    CAMERA_NIGHT_MODE,
    CAMERA_NIGHT_NUM
}T_NIGHT_MODE;

typedef enum {
    CAMERA_MODE_QXGA = 0,    // 2048 X 1536
    CAMERA_MODE_UXGA,        // 1600 X 1200
    CAMERA_MODE_SXGA,       // 1280 X 1024
    CAMERA_MODE_XGA,        // 1024 X 768
    CAMERA_MODE_SVGA,        // 800  X 600
    CAMERA_MODE_VGA,        // 640  X 480 
    CAMERA_MODE_QSVGA,      // 400  X 300
    CAMERA_MODE_CIF,        // 352  X 288
    CAMERA_MODE_QVGA,       // 320  X 240
    CAMERA_MODE_QCIF,       // 176  X 144
    CAMERA_MODE_QQVGA,        // 160  X 120
    CAMERA_MODE_PREV,        // 640  X 480
    CAMERA_MODE_REC,        // 352    X 288
    CAMERA_MODE_720P,        // 1280 X 720
    CAMERA_MODE_800P,        // 1280 X 800
    CAMERA_MODE_960P,        // 1280 X 960
    CAMERA_MODE_D1,            // 720 X 480
    CAMERA_MODE_QSXGA,      //2592X1944
    CAMERA_MODE_QXGA_JPEG,  //2048X1536 jpeg
    CAMERA_MODE_QSXGA_JPEG, //2592X1944 jpeg
    CAMERA_MODE_VGA_JPEG,   // 640  X 480  jpeg
    CAMERA_MODE_NUM
} T_CAMERA_MODE;

/** @brief Camera Parameter Exposure definition
 *
 *  This structure define the value of parameter Exposure
 */
typedef enum
{
    EXPOSURE_WHOLE = 0,
    EXPOSURE_CENTER,
    EXPOSURE_MIDDLE,
    CAMERA_EXPOSURE_NUM
}T_CAMERA_EXPOSURE;

/** @brief Camera Parameter Brightness definition
 *
 *  This structure define the value of parameter Brightness
 */
typedef enum
{
    CAMERA_BRIGHTNESS_0 = 0,
    CAMERA_BRIGHTNESS_1,
    CAMERA_BRIGHTNESS_2,
    CAMERA_BRIGHTNESS_3,
    CAMERA_BRIGHTNESS_4,
    CAMERA_BRIGHTNESS_5,
    CAMERA_BRIGHTNESS_6,
    CAMERA_BRIGHTNESS_NUM
}T_CAMERA_BRIGHTNESS;

/** @brief Camera Parameter Contrast definition
 *
 *  This structure define the value of parameter Contrast
 */
typedef enum 
{
    CAMERA_CONTRAST_1 = 0,
    CAMERA_CONTRAST_2,
    CAMERA_CONTRAST_3,
    CAMERA_CONTRAST_4,
    CAMERA_CONTRAST_5,
    CAMERA_CONTRAST_6,
    CAMERA_CONTRAST_7,
    CAMERA_CONTRAST_NUM
}T_CAMERA_CONTRAST;

/** @brief Camera Parameter Saturation definition
 *
 *  This structure define the value of parameter Saturation
 */
typedef enum
{
    CAMERA_SATURATION_0 = 0,
    CAMERA_SATURATION_1,
    CAMERA_SATURATION_2,
    CAMERA_SATURATION_3,
    CAMERA_SATURATION_4,
    CAMERA_SATURATION_5,
    CAMERA_SATURATION_6,
    CAMERA_SATURATION_NUM
}T_CAMERA_SATURATION;

/** @brief Camera Parameter Sharpness definition
 *
 *  This structure define the value of parameter Sharpness
 */
typedef enum
{
    CAMERA_SHARPNESS_0 = 0,
    CAMERA_SHARPNESS_1,
    CAMERA_SHARPNESS_2,
    CAMERA_SHARPNESS_3,
    CAMERA_SHARPNESS_4,
    CAMERA_SHARPNESS_5,
    CAMERA_SHARPNESS_6,
    CAMERA_SHARPNESS_NUM
}T_CAMERA_SHARPNESS;

/** @brief Camera Parameter AWB definition
 *
 *  This structure define the value of parameter AWB
 */
typedef enum
{
    AWB_AUTO = 0,
    AWB_SUNNY,
    AWB_CLOUDY,
    AWB_OFFICE,
    AWB_HOME,
    AWB_NIGHT,
    AWB_NUM
}T_CAMERA_AWB;

/** @brief Camera Parameter Mirror definition
 *
 *  This structure define the value of parameter Mirror
 */
typedef enum
{
    CAMERA_MIRROR_NORMAL = 0,
    CAMERA_MIRROR_V,
    CAMERA_MIRROR_H,
    CAMERA_MIRROR_FLIP,
    CAMERA_MIRROR_NUM
}T_CAMERA_MIRROR;

/** @brief Camera Parameter Effect definition
 *
 *  This structure define the value of parameter Effect
 */
typedef enum
{
    CAMERA_EFFECT_NORMAL = 0,
    CAMERA_EFFECT_SEPIA = 2,
    CAMERA_EFFECT_ANTIQUE = 5,
    CAMERA_EFFECT_BLUE = 6,
    CAMERA_EFFECT_GREEN = 7,
    CAMERA_EFFECT_RED = 8,
    CAMERA_EFFECT_NEGATIVE = 3,
    CAMERA_EFFECT_BW = 1,
    CAMERA_EFFECT_BWN = 4,    
    CAMERA_EFFECT_AQUA = 9,    // PO1200 additional mode add by Liub 20060918
    CAMERA_EFFECT_COOL,
    CAMERA_EFFECT_WARM,
    CAMERA_EFFECT_NUM
}T_CAMERA_EFFECT;


/** @brief Camera Parameter CCIR601/656 protocol
 *
 *    This structure define the CMOS sensor compatible with CCIR601 or CCIR656 protocol
 */
typedef enum
{
    CAMERA_CCIR_601,
    CAMERA_CCIR_656,
    CAMERA_CCIR_NUM
}T_CAMERA_INTERFACE;

/** @brief Camera feature definition
 *
 *  This structure define the feature list of camera
 */
typedef enum {
    CAM_FEATURE_NIGHT_MODE = 0,
    CAM_FEATURE_EXPOSURE,
    CAM_FEATURE_AWB,
    CAM_FEATURE_BRIGHTNESS,
    CAM_FEATURE_CONTRAST,
    CAM_FEATURE_SATURATION,
    CAM_FEATURE_SHARPNESS,
    CAM_FEATURE_MIRROR,
    CAM_FEATURE_EFFECT,
    CAM_FEATURE_NUM
}T_CAMERA_FEATURE;


typedef struct
{
    T_U32            cam_mclk;
    T_VOID           (*cam_open_func)(T_VOID);
    T_BOOL           (*cam_close_func)(T_VOID);
    T_U32            (*cam_read_id_func)(T_VOID);
    T_BOOL           (*cam_init_func)(T_VOID);
    T_VOID           (*cam_set_mode_func)(T_CAMERA_MODE mode);
    T_VOID           (*cam_set_exposure_func)(T_CAMERA_EXPOSURE exposure);
    T_VOID           (*cam_set_brightness_func)(T_CAMERA_BRIGHTNESS brightness);
    T_VOID           (*cam_set_contrast_func)(T_CAMERA_CONTRAST contrast);
    T_VOID           (*cam_set_saturation_func)(T_CAMERA_SATURATION saturation);
    T_VOID           (*cam_set_sharpness_func)(T_CAMERA_SHARPNESS sharpness);
	T_VOID           (*cam_set_hue)(T_U32 value);
	T_VOID           (*cam_set_hue_auto)(T_U32 value);
    T_VOID           (*cam_set_AWB_func)(T_CAMERA_AWB awb);
    T_VOID           (*cam_set_mirror_func)(T_CAMERA_MIRROR mirror);
    T_VOID           (*cam_set_effect_func)(T_CAMERA_EFFECT effect);
    T_S32            (*cam_set_window_func)(T_U32 srcWidth, T_U32 srcHeight);
    T_VOID           (*cam_set_night_mode_func)(T_NIGHT_MODE mode);
    T_U32            (*cam_set_framerate_func)(float framerate);
    T_VOID           (*cam_set_anti_flicker_func)(T_U32 value);
    T_BOOL           (*cam_set_to_cap_func)(T_U32 srcWidth, T_U32 srcHeight);
    T_BOOL           (*cam_set_to_prev_func)(T_U32 srcWidth, T_U32 srcHeight);    
    T_BOOL           (*cam_set_to_record_func)(T_U32 srcWidth, T_U32 srcHeight);
    T_CAMERA_TYPE    (*cam_get_type)(T_VOID);
	T_VOID			 (*cam_set_sensor_param_func)(T_U32 cmd, T_U32 data);
	T_U16			 (*cam_get_sensor_param_func)(T_U32 cmd);
}T_CAMERA_FUNCTION_HANDLER;
#endif

