/**
 * @file camera_tw9912.c
 * @brief camera driver file
 * Copyright (C) 2017 SPF Dipol, Ltd.
 * @author Andrew Tamila  
 * @date 2017-03-30
 * @version 1.0
 * @ref
 */ 
#ifdef CONFIG_LINUX_AKSENSOR

#include <plat-anyka/aksensor.h>
#include <plat-anyka/wrap_sensor.h>
#include <plat-anyka/cam_com_sensor.h>
#include "camera_tw9912.h"

#else 

#include "cam_com_sensor.h"
#include "camera_tw9912.h" 

#endif

#if defined (USE_CAMERA_TW9912) || defined (CONFIG_SENSOR_TW9912)

#define CAM_EN_LEVEL            0    
#define CAM_RESET_LEVEL         0

#define CAMERA_SCCB_ADDR        0x88
#define CAMERA_TW9912_ID 		0x60

#define TW9912_CAMERA_MCLK      54

static T_CAMERA_TYPE camera_tw9912_type = CAMERA_2M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_tw9912_CurMode = CAMERA_MODE_VGA;

#if 0
static T_VOID camera_setbit(T_U16 reg, T_U8 bit, T_U8 value)
{
    T_U8 tmp;

    tmp = sccb_read_data(CAMERA_SCCB_ADDR, reg);
    if (value == 1)
    {    
        tmp |= 0x1 << bit;
    }
    else
    {
        tmp &= ~(0x1 << bit);
    }
    
    sccb_write_data(CAMERA_SCCB_ADDR, reg, &tmp, 1);
}
#endif
static T_U32 cam_tw9912_read_id(T_VOID)
{
#if 0
    T_U8 value = 0x00;
    T_U32 id = 0;

    sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);        //init sccb first here!!
    
    value = sccb_read_data(CAMERA_SCCB_ADDR, 0x00);
    //id = value << 8;
    //value = sccb_read_short(CAMERA_SCCB_ADDR, 0x0002);
    id |= value;    

    return id;
#else
	return CAMERA_TW9912_ID;
#endif
}

static T_BOOL camera_set_param(const T_U16 tabParameter[])
{ 
    int i = 0;
    //T_U8 temp_value;
   	

    while (1)
    {
        if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i + 1])) 
        {
            break;
        }
        else if (DELAY_FLAG == tabParameter[i])
        {
            mini_delay(tabParameter[i + 1]);
        }
        else
        {
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)(&tabParameter[i + 1]), 1);
/*
            if (((tabParameter[i] != 0xFF) || (tabParameter[i] != 0xFE) || (tabParameter[i] != 0xFD)))
            {                
                temp_value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);
                if (temp_value != tabParameter[i + 1])
                {
                    akprintf(C1, M_DRVSYS, "set parameter error!\n");
                    akprintf(C1, M_DRVSYS, "reg 0x%x write data is 0x%x, read data is 0x%x!\n", tabParameter[i], tabParameter[i + 1], temp_value);

                    return AK_FALSE;
                }
            }
*/	    
        }
        
        i += 2;
    }

    return AK_TRUE;
}

#if 0
static T_VOID read_camera_reg(const T_U16 tabParameter[])
{
	unsigned short data;
	int i = 0;
	
	while (1) {
		if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i + 1])) {
            break;
        } else if (DELAY_FLAG != tabParameter[i]) {
			data = sccb_read_word(CAMERA_SCCB_ADDR, tabParameter[i]);
			//printk("read: [0x%04x 0x%04x]\n", tabParameter[i], data);
       	}
		i += 2;
	}
}
#endif

static T_VOID camera_setup(const T_U16 tabParameter[])
{
    int i = 0;
    	

    while (1)
    {
        if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i + 1])) 
        {
            break;
        }
        else if (DELAY_FLAG == tabParameter[i])
        {
            mini_delay(tabParameter[i + 1]);
        }
        else
        {
        //	data = tabParameter[i + 1];
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i],(T_U8 *)&tabParameter[i+1], 1);
        }
        i += 2;
    }
}

static T_VOID cam_tw9912_open(T_VOID)
{  
    gpio_set_pin_dir(GPIO_CAMERA_AVDD, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));   

    gpio_set_pin_as_gpio(GPIO_CAMERA_CHIP_ENABLE);
    gpio_set_pin_dir(GPIO_CAMERA_CHIP_ENABLE, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, CAM_EN_LEVEL);    
    mini_delay(10);

    gpio_set_pin_as_gpio(GPIO_CAMERA_RESET);
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_RESET, CAM_RESET_LEVEL);
    mini_delay(10);
    gpio_set_pin_level(GPIO_CAMERA_RESET, !CAM_RESET_LEVEL);

    mini_delay(20);
}

static T_BOOL cam_tw9912_close(T_VOID)
{
    //sccb software standby mode
//    T_U8 Reg0x3d = 0x48;
//    T_U8 Reg0xc3 = 0x00;
    
//    sccb_write_word(CAMERA_SCCB_ADDR, 0x3d, &Reg0x3d, 1);
//    sccb_write_word(CAMERA_SCCB_ADDR, 0xc3, &Reg0xc3, 1);

    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, !CAM_EN_LEVEL);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, !gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));    
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);

    gpio_set_pin_dir(GPIO_I2C_SCL, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SCL, GPIO_LEVEL_LOW);
    gpio_set_pin_dir(GPIO_I2C_SDA, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SDA, GPIO_LEVEL_LOW);
    
    return AK_TRUE;
}

/**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author cao_lianming 
 * @date 2013-07-31
 * @return T_BOOL
 * @retval AK_TRUE if success, else AK_FALSE
 */
static T_BOOL cam_tw9912_init(void)
{
    if (!camera_set_param(INIT_TAB)) //null init tab in header
    {
        return AK_FALSE;
    }
    else
    {        
        night_mode = CAMERA_DAY_MODE;
        return AK_TRUE;
    }        
}

/**
 * @brief Set camera mode to specify image quality, SXGA/VGA/CIF etc 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] mode mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_mode(T_CAMERA_MODE mode)
{
    s_tw9912_CurMode = mode;
    switch(mode) {
    case CAMERA_MODE_UXGA: 			// 1600x1200
        camera_setup(UXGA_MODE_TAB);
        break;    
    case CAMERA_MODE_SXGA: 			// 1280x1024
        camera_setup(SXGA_MODE_TAB);
        break; 
    case CAMERA_MODE_XGA: 			// 1024x768
        camera_setup(XGA_MODE_TAB);
        break; 
    case CAMERA_MODE_VGA:			 // 640x480
        camera_setup(VGA_MODE_TAB);
        break;
    case CAMERA_MODE_SVGA:			 // 640x480
        camera_setup(VGA_MODE_TAB);
        break;
	case CAMERA_MODE_DVC:			 // 640x480
        camera_setup(VGA_MODE_TAB);
        break;
    case CAMERA_MODE_CIF:
        camera_setup(CIF_MODE_TAB);
        break;
    case CAMERA_MODE_QVGA:
        camera_setup(QVGA_MODE_TAB);
        break;
    case CAMERA_MODE_QCIF:
        camera_setup(QCIF_MODE_TAB);
        break;
    case CAMERA_MODE_QQVGA:
        camera_setup(QQVGA_MODE_TAB);
        break;
    case CAMERA_MODE_PREV:
        camera_setup(PREV_MODE_TAB);
        break;
    case CAMERA_MODE_REC:
        camera_setup(RECORD_MODE_TAB);
        break;
    case CAMERA_MODE_720P:
        camera_setup(RECORD_720P_TAB);
        break;
    default:
        s_tw9912_CurMode = CAMERA_MODE_VGA;
        akprintf(C1, M_DRVSYS, "set camera mode parameter error!\n");
        break;
    }
#if 0
	if (mode == CAMERA_MODE_720P)
		read_camera_reg(RECORD_720P_TAB);
#endif
}

/**
 * @brief Set camera exposure mode 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_exposure(T_CAMERA_EXPOSURE exposure)
{
    switch(exposure)
    {
        case EXPOSURE_WHOLE:
            camera_setup(EXPOSURE_WHOLE_TAB);
            break;
        case EXPOSURE_CENTER:
            camera_setup(EXPOSURE_CENTER_TAB);
            break;
        case EXPOSURE_MIDDLE:
            camera_setup(EXPOSURE_MIDDLE_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set exposure parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera brightness level 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_brightness(T_CAMERA_BRIGHTNESS brightness)
{
    switch(brightness)
    {
        case CAMERA_BRIGHTNESS_0:
            camera_setup(BRIGHTNESS_0_TAB);
            break;
        case CAMERA_BRIGHTNESS_1:
            camera_setup(BRIGHTNESS_1_TAB);
            break;
        case CAMERA_BRIGHTNESS_2:
            camera_setup(BRIGHTNESS_2_TAB);
            break;
        case CAMERA_BRIGHTNESS_3:
            camera_setup(BRIGHTNESS_3_TAB);
            break;
        case CAMERA_BRIGHTNESS_4:
            camera_setup(BRIGHTNESS_4_TAB);
            break;
        case CAMERA_BRIGHTNESS_5:
            camera_setup(BRIGHTNESS_5_TAB);
            break;
        case CAMERA_BRIGHTNESS_6:
            camera_setup(BRIGHTNESS_6_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set brightness parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera contrast level 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_contrast(T_CAMERA_CONTRAST contrast)
{
    switch(contrast)
    {
        case CAMERA_CONTRAST_1:
            camera_setup(CONTRAST_1_TAB);
            break;
        case CAMERA_CONTRAST_2:
            camera_setup(CONTRAST_2_TAB);
            break;
        case CAMERA_CONTRAST_3:
            camera_setup(CONTRAST_3_TAB);
            break;
        case CAMERA_CONTRAST_4:
            camera_setup(CONTRAST_4_TAB);
            break;
        case CAMERA_CONTRAST_5:
            camera_setup(CONTRAST_5_TAB);
            break;
        case CAMERA_CONTRAST_6:
            camera_setup(CONTRAST_6_TAB);
            break;
        case CAMERA_CONTRAST_7:
            camera_setup(CONTRAST_7_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set contrast parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera saturation level 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_saturation(T_CAMERA_SATURATION saturation)
{
    switch(saturation)
    {
        case CAMERA_SATURATION_1:
            camera_setup(SATURATION_1_TAB);
            break;
        case CAMERA_SATURATION_2:
            camera_setup(SATURATION_2_TAB);
            break;
        case CAMERA_SATURATION_3:
            camera_setup(SATURATION_3_TAB);
            break;
        case CAMERA_SATURATION_4:
            camera_setup(SATURATION_4_TAB);
            break;
        case CAMERA_SATURATION_5:
            camera_setup(SATURATION_5_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set saturation parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera sharpness level 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_sharpness(T_CAMERA_SHARPNESS sharpness)
{
    switch(sharpness)
    {
        case CAMERA_SHARPNESS_0:
            camera_setup(SHARPNESS_0_TAB);
            break;
        case CAMERA_SHARPNESS_1:
            camera_setup(SHARPNESS_1_TAB);
            break;
        case CAMERA_SHARPNESS_2:
            camera_setup(SHARPNESS_2_TAB);
            break;
        case CAMERA_SHARPNESS_3:
            camera_setup(SHARPNESS_3_TAB);
            break;
        case CAMERA_SHARPNESS_4:
            camera_setup(SHARPNESS_4_TAB);
            break;
        case CAMERA_SHARPNESS_5:
            camera_setup(SHARPNESS_5_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set sharpness parameter error!\n");
            break;
    }
}

static T_VOID cam_tw9912_set_hue(T_U32 value)
{
    switch(value)
    {
        case CAMERA_SHARPNESS_0:
            camera_setup(HUE_0_TAB);
            break;
        case CAMERA_SHARPNESS_1:
            camera_setup(HUE_1_TAB);
            break;
        case CAMERA_SHARPNESS_2:
            camera_setup(HUE_2_TAB);
            break;
        case CAMERA_SHARPNESS_3:
            camera_setup(HUE_3_TAB);
            break;
        case CAMERA_SHARPNESS_4:
            camera_setup(HUE_4_TAB);
            break;
        case CAMERA_SHARPNESS_5:
            camera_setup(HUE_5_TAB);
            break;
		case CAMERA_SHARPNESS_6:
            camera_setup(HUE_6_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set hue parameter error!\n");
            break;
    }
}

static T_VOID cam_tw9912_set_hue_auto(T_U32 value)
{
    switch(value)
    {
        case CAMERA_SHARPNESS_0:
            camera_setup(HUE_AUTO_0_TAB);
            break;
        case CAMERA_SHARPNESS_1:
            camera_setup(HUE_AUTO_1_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set hue auto parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera AWB mode 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_AWB(T_CAMERA_AWB awb)
{
    switch(awb)
    {
        case AWB_AUTO:
            camera_setup(AWB_AUTO_TAB);
            break;
        case AWB_SUNNY:
            camera_setup(AWB_SUNNY_TAB);
            break;
        case AWB_CLOUDY:
            camera_setup(AWB_CLOUDY_TAB);
            break;
        case AWB_OFFICE:
            camera_setup(AWB_OFFICE_TAB);
            break;
        case AWB_HOME:
            camera_setup(AWB_HOME_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set AWB mode parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera mirror mode 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_mirror(T_CAMERA_MIRROR mirror)
{
    switch(mirror)
    {
        case CAMERA_MIRROR_V:
            break;
        case CAMERA_MIRROR_H:
            break;
        case CAMERA_MIRROR_NORMAL:
            break;
        case CAMERA_MIRROR_FLIP:
            break;
        default:
            akprintf(C1, M_DRVSYS, "set mirror parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera effect mode 
 * @author cao_lianming 
 * @date 2013-07-31
 * @param[in] effect effect mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_tw9912_set_effect(T_CAMERA_EFFECT effect)
{
    switch(effect)
    {
        case CAMERA_EFFECT_NORMAL:
            camera_setup(EFFECT_NORMAL_TAB);
            break;
        case CAMERA_EFFECT_SEPIA:
            camera_setup(EFFECT_SEPIA_TAB);
            break;
        case CAMERA_EFFECT_ANTIQUE:
            camera_setup(EFFECT_ANTIQUE_TAB);
            break;
        case CAMERA_EFFECT_BLUE:
            camera_setup(EFFECT_BLUISH_TAB);
            break;
        case CAMERA_EFFECT_GREEN:
            camera_setup(EFFECT_GREENISH_TAB);
            break;
        case CAMERA_EFFECT_RED:
            camera_setup(EFFECT_REDDISH_TAB);
            break;
        case CAMERA_EFFECT_NEGATIVE:
            camera_setup(EFFECT_NEGATIVE_TAB);
            break;
        case CAMERA_EFFECT_BW:
            camera_setup(EFFECT_BW_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set camer effect parameter error!\n");
            break;
    }
}

/**
 * @brief set camera window
 * @author cao_lianming  
 * @date 2013-07-31
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if failed
 */
static T_S32 cam_tw9912_set_digital_zoom(T_U32 srcWidth, T_U32 srcHeight)
{
    return 1;
}

static T_VOID cam_tw9912_set_night_mode(T_NIGHT_MODE mode)
{
    switch(mode)
    {
        case CAMERA_DAY_MODE:
            camera_setup(DAY_MODE_TAB);
            night_mode = CAMERA_DAY_MODE;
            break;
        case CAMERA_NIGHT_MODE:
            camera_setup(NIGHT_MODE_TAB);
            night_mode = CAMERA_NIGHT_MODE;
            break;
        default:
            akprintf(C1, M_DRVSYS, "set night mode parameter error!\n");
            break;
    }
}

static T_VOID cam_tw9912_set_anti_flicker(T_U32 value)
{
    switch(value) {
    case V4L2_CID_POWER_LINE_FREQUENCY_DISABLED:
        //camera_setup(ANTI_FLICKER_DISABLE_TAB);
		akprintf(C1, M_DRVSYS, "Anti-flicker not support 'Disable', Error."
		" please select other frequency!\n");
        break;
    case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
        camera_setup(ANTI_FLICKER_50HZ_TAB);
        break;
	case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
        camera_setup(ANTI_FLICKER_60HZ_TAB);
        break;
    case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
        //camera_setup(ANTI_FLICKER_AUTO_TAB);
		akprintf(C1, M_DRVSYS, "Anti-flicker not support 'Auto', Error."
		" please select other frequency!\n");
        break;
    default:
        akprintf(C1, M_DRVSYS, "set Anti-flicker parameter error!\n");
        break;
    }
}

static T_BOOL cam_tw9912_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
{    
    T_CAMERA_MODE Cammode;

    if ((srcWidth <= 160) && (srcHeight <= 120))
    {
        Cammode = CAMERA_MODE_QQVGA;
    }
    else if ((srcWidth <= 176) && (srcHeight <= 144))
    {
        Cammode = CAMERA_MODE_QCIF;
    }
    else if ((srcWidth <= 320) && (srcHeight <= 240))
    {
        Cammode = CAMERA_MODE_QVGA;
    }
    else if ((srcWidth <= 352) && (srcHeight <= 288))
    {
        Cammode = CAMERA_MODE_CIF;
    }
    else if ((srcWidth <= 640) && (srcHeight <= 480))
    {
        Cammode = CAMERA_MODE_VGA;
    }
    else if ((srcWidth <= 800) && (srcHeight <= 600))
    {
        Cammode = CAMERA_MODE_SVGA;
    }
    else if ((srcWidth <= 800) && (srcHeight <= 600))
    {
        Cammode = CAMERA_MODE_DVC;
    }   
    else if ((srcWidth <= 960) && (srcHeight <= 720))
    {
        Cammode = CAMERA_MODE_XGA;
    }
    else if ((srcWidth <= 1280) && (srcHeight <= 720))
    {
        Cammode = CAMERA_MODE_720P;
    }    
    else if ((srcWidth <= 1280) && (srcHeight <= 1024))
    {
        Cammode = CAMERA_MODE_SXGA;
    }
    else if ((srcWidth <= 1600) && (srcHeight <= 1200))
    {
        Cammode = CAMERA_MODE_UXGA;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "tw9912 unsupport %d & %d mode!\n", srcWidth, srcHeight);
        return AK_FALSE;
    }
    
    cam_tw9912_set_mode(Cammode);
    cam_tw9912_set_digital_zoom(srcWidth, srcHeight);    
    mini_delay(300);
    return AK_TRUE;
}

static T_BOOL cam_tw9912_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{    
    cam_tw9912_set_mode(CAMERA_MODE_PREV);    
    cam_tw9912_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(300);
    return AK_TRUE;
}

static T_BOOL cam_tw9912_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{    
    T_CAMERA_MODE Cammode;
    if ((srcWidth <= 640) && (srcHeight <= 480))
    {
        Cammode = CAMERA_MODE_REC;
    }
    else if ((srcWidth <= 800) && (srcHeight <= 600))
    {
        Cammode = CAMERA_MODE_SVGA;
    }
    else if ((srcWidth <= 960) && (srcHeight <= 720))
    {
        Cammode = CAMERA_MODE_DVC;
    }
    else if ((srcWidth <= 1024) && (srcHeight <= 768))
    {
         Cammode = CAMERA_MODE_XGA;
    }  
    else if ((srcWidth <= 1280) && (srcHeight <= 720))
    {
         Cammode = CAMERA_MODE_720P;
    } 
    else
    {
        akprintf(C1, M_DRVSYS, "200W camera dose not support such mode");
        return AK_FALSE;
    }
	
    cam_tw9912_set_mode(Cammode);
    cam_tw9912_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(300);
    return AK_TRUE;
}

static T_CAMERA_TYPE cam_tw9912_get_type(T_VOID)
{
    return camera_tw9912_type;
} 

static T_VOID cam_tw9912_set_sensor_param(T_U32 cmd, T_U32 data)
{
	T_U8 value;

	value = (T_U8)data;
	sccb_write_data(CAMERA_SCCB_ADDR, (T_U8)cmd, &value, 1);
}

static T_U16 cam_tw9912_get_sensor_param(T_U32 cmd)
{
	return sccb_read_data(CAMERA_SCCB_ADDR, (T_U8)cmd);
}

static T_CAMERA_FUNCTION_HANDLER tw9912_function_handler = 
{
    TW9912_CAMERA_MCLK,
    cam_tw9912_open,
    cam_tw9912_close,
    cam_tw9912_read_id,
    cam_tw9912_init,
    cam_tw9912_set_mode,
    cam_tw9912_set_exposure,
    cam_tw9912_set_brightness,
    cam_tw9912_set_contrast,
    cam_tw9912_set_saturation,
    cam_tw9912_set_sharpness,
    cam_tw9912_set_hue,
    cam_tw9912_set_hue_auto,
    cam_tw9912_set_AWB,
    cam_tw9912_set_mirror,
    cam_tw9912_set_effect,
    cam_tw9912_set_digital_zoom,
    cam_tw9912_set_night_mode,
    AK_NULL,
    cam_tw9912_set_anti_flicker,
    cam_tw9912_set_to_cap,
    cam_tw9912_set_to_prev,
    cam_tw9912_set_to_record,
    cam_tw9912_get_type,
    cam_tw9912_set_sensor_param,
    cam_tw9912_get_sensor_param,
};

#ifndef CONFIG_LINUX_AKSENSOR
static int camera_tw9912_reg(void)
{
    camera_reg_dev(CAMERA_TW9912_ID, &tw9912_function_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_tw9912_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#else
static const char * awb_menu[] = {
	[AWB_AUTO] = "auto",
	[AWB_SUNNY] = "sunny",
	[AWB_CLOUDY] = "cloudy",
	[AWB_OFFICE] = "office",
	[AWB_HOME] = "home",
	[AWB_NIGHT] = "night",
};

static const char * effect_menu[] = {
	[CAMERA_EFFECT_NORMAL] = "normal",
	[CAMERA_EFFECT_SEPIA] = "sepia",
	[CAMERA_EFFECT_ANTIQUE] = "antique",
	[CAMERA_EFFECT_BLUE] = "blue",
	[CAMERA_EFFECT_GREEN] = "green",
	[CAMERA_EFFECT_RED] = "red",
	[CAMERA_EFFECT_NEGATIVE] = "negative",
	[CAMERA_EFFECT_BW] = "bw",
	[CAMERA_EFFECT_BWN] = "bwn",
	[CAMERA_EFFECT_AQUA] = "aqua",
	[CAMERA_EFFECT_COOL] = "cool",
	[CAMERA_EFFECT_WARM] = "warm",	
};

static const char * resolution_menu[] = {
	[0] = "1280x960",
	[1] = "1280x720",
	[2] = "640x480",
	[3] = "1024x768",
	[4] = "800x600",
	[5] = "960x720",
};

static const char * hflip_menu[] = {
	[0] = "normal",
	[1] = "horizontal flip",
};

static const char * vflip_menu[] = {
	[0] = "normal",
	[1] = "vertical flip",
};

static const char * night_menu[] = {
	[CAMERA_DAY_MODE] = "daylight",
	[CAMERA_NIGHT_MODE] = "night",
};

static const char * anti_flicker_menu[] = {
	[V4L2_CID_POWER_LINE_FREQUENCY_DISABLED] = "Disable",
	[V4L2_CID_POWER_LINE_FREQUENCY_50HZ] = "50Hz",
	[V4L2_CID_POWER_LINE_FREQUENCY_60HZ] = "60Hz",
	[V4L2_CID_POWER_LINE_FREQUENCY_AUTO] = "Auto",
};

static int tw9912_s_ctl(struct v4l2_ctrl *ctrl)
{
	int ret = -EINVAL;

	switch (ctrl->id) {
	case V4L2_CID_AUTO_WHITE_BALANCE:
		if (tw9912_function_handler.cam_set_AWB_func) {
			tw9912_function_handler.cam_set_AWB_func(ctrl->val);
			ret = 0;
		}
		break;
	case V4L2_CID_COLORFX:
		if (tw9912_function_handler.cam_set_effect_func) {
			tw9912_function_handler.cam_set_effect_func(ctrl->val);
			ret = 0;
		}
		break;	

	case V4L2_CID_BRIGHTNESS:
		if (tw9912_function_handler.cam_set_brightness_func) {
			tw9912_function_handler.cam_set_brightness_func(ctrl->val);
			ret = 0;
		}
		break;	
	case V4L2_CID_CONTRAST:
		if (tw9912_function_handler.cam_set_contrast_func) {
			tw9912_function_handler.cam_set_contrast_func(ctrl->val);
			ret = 0;
		}
		break;	
	case V4L2_CID_SATURATION:
		if (tw9912_function_handler.cam_set_saturation_func) {
			tw9912_function_handler.cam_set_saturation_func(ctrl->val);
			ret = 0;
		}
		break;	
	case V4L2_CID_SHARPNESS:
		if (tw9912_function_handler.cam_set_sharpness_func) {
			tw9912_function_handler.cam_set_sharpness_func(ctrl->val);
			ret = 0;
		}
		break;	
	case V4L2_CID_HUE:
		if (tw9912_function_handler.cam_set_hue) {
			tw9912_function_handler.cam_set_hue(ctrl->val);
			ret = 0;
		}
		break;	
	case V4L2_CID_HUE_AUTO:
		if (tw9912_function_handler.cam_set_hue_auto) {
			tw9912_function_handler.cam_set_hue_auto(ctrl->val);
			ret = 0;
		}
		break;
	case V4L2_CID_HFLIP:
		if (tw9912_function_handler.cam_set_mirror_func) {
			tw9912_function_handler.cam_set_mirror_func( 
				ctrl->val ? CAMERA_MIRROR_H : CAMERA_MIRROR_NORMAL);
			ret = 0;
		}
		break;
	case V4L2_CID_VFLIP:
		if (tw9912_function_handler.cam_set_mirror_func) {
			tw9912_function_handler.cam_set_mirror_func(
				ctrl->val ? CAMERA_MIRROR_V : CAMERA_MIRROR_NORMAL);
			ret = 0;
		}
		break;
	case V4L2_CID_NIGHTMODE:
		if (tw9912_function_handler.cam_set_night_mode_func) {
			tw9912_function_handler.cam_set_night_mode_func(ctrl->val);
			ret = 0;
		}
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		if (tw9912_function_handler.cam_set_anti_flicker_func) {
			tw9912_function_handler.cam_set_anti_flicker_func(ctrl->val);
			ret = 0;
		}
		break;
	default:
		break;
	}

	return ret;	
};

static struct v4l2_ctrl_ops tw9912_ctrl_ops = {
	.s_ctrl = tw9912_s_ctl,
};

static const struct v4l2_ctrl_config tw9912_ctrls[] = {
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_AUTO_WHITE_BALANCE,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "AWB",
		.min		= 0,
		.max		= ARRAY_SIZE(awb_menu) - 1,
		.step		= 0,
		.def		= 0,
		.flags		= 0,
		.menu_skip_mask = 0,
		.qmenu		= awb_menu,
	},	
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_COLORFX,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "Effect",
		.min		= 0,
		.max		= ARRAY_SIZE(effect_menu) - 1,
		.step		= 0,
		.def		= 0,
		.flags		= 0,
		.menu_skip_mask = 0,
		.qmenu		= effect_menu,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_HFLIP,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "Horizontal Flip",
		.min		= 0,
		.max		= ARRAY_SIZE(hflip_menu) - 1,
		.step		= 0,
		.def		= 0,
		.flags		= 0,
		.menu_skip_mask = 0,
		.qmenu		= hflip_menu,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_VFLIP,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "Vetical Flip",
		.min		= 0,
		.max		= ARRAY_SIZE(vflip_menu) - 1,
		.step		= 0,
		.def		= 0,
		.flags		= 0,
		.menu_skip_mask = 0,
		.qmenu		= vflip_menu,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_PICTURE,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "Resolution",
		.min		= 0,
		.max		= ARRAY_SIZE(resolution_menu) - 1,
		.step		= 0,
		.def		= 0,
		.flags		= 0,
		.menu_skip_mask = 0,
		.qmenu		= resolution_menu,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_NIGHTMODE,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "Night mode",
		.min		= 0,
		.max		= ARRAY_SIZE(night_menu) - 1,
		.step		= 0,
		.def		= 0,
		.flags		= 0,
		.menu_skip_mask = 0,
		.qmenu		= night_menu,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_BRIGHTNESS,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Brightness",
		.min		= 0,
		.max		= CAMERA_BRIGHTNESS_NUM -1,
		.step		= 1,
		.def		= 0,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_CONTRAST,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Contrast",
		.min		= 0,
		.max		= CAMERA_CONTRAST_NUM -1,
		.step		= 1,
		.def		= 0,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_SATURATION,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Saturation",
		.min		= 0,
		.max		= CAMERA_SATURATION_NUM -1,
		.step		= 1,
		.def		= 0,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id		= V4L2_CID_SHARPNESS,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Sharpness",
		.min		= 0,
		.max		= CAMERA_SHARPNESS_NUM -1,
		.step		= 1,
		.def		= 0,
	},
	{
		.ops		= &tw9912_ctrl_ops,
		.id			= V4L2_CID_POWER_LINE_FREQUENCY,
		.type		= V4L2_CTRL_TYPE_MENU,
		.name		= "anti flicker",
		.min		= V4L2_CID_POWER_LINE_FREQUENCY_DISABLED,
		.max		= V4L2_CID_POWER_LINE_FREQUENCY_AUTO,
		.step		= 0,
		.def		= V4L2_CID_POWER_LINE_FREQUENCY_50HZ,
		.flags		= 0,
		.menu_skip_mask = V4L2_CID_POWER_LINE_FREQUENCY_DISABLED,
		.qmenu		= anti_flicker_menu,
	}
};


/*
 * supported format list
 */
static const struct aksensor_color_format tw9912_formats[] = {
	
	{
		.code = V4L2_MBUS_FMT_YUYV8_2X8,		//0x2008
		.colorspace = V4L2_COLORSPACE_JPEG,
	},
	
	{
		.code = V4L2_MBUS_FMT_VYUY8_2X8, 		//0x2007
		.colorspace = V4L2_COLORSPACE_JPEG,
	},
	
	
};

static const struct aksensor_win_size tw9912_win[] = {
	{.name = "VGA",		.width = 640,	.height = 480},
	{.name = "SVGA",	.width = 800,	.height = 600},
	{.name = "DVC",		.width = 960,	.height = 720},
	{.name = "XGA",		.width = 1024,	.height = 768},
	{.name = "720P",	.width = 1280,	.height = 720},
	{.name = "960P",	.width = 1280,	.height = 960},
};

static struct sensor_info tw9912_sensor_info = {
	.sensor_name = "tw9912",
	.sensor_id = CAMERA_TW9912_ID,
	.ctrls = tw9912_ctrls,
	.nr_ctrls = ARRAY_SIZE(tw9912_ctrls), 
	.formats = tw9912_formats,
	.num_formats = ARRAY_SIZE(tw9912_formats),
	.resolution = tw9912_win,
	.num_resolution = ARRAY_SIZE(tw9912_win),
	.handler = &tw9912_function_handler,
};

static int tw9912_module_init(void)
{
	return register_sensor(&tw9912_sensor_info);
}
module_init(tw9912_module_init)
#endif

#endif


