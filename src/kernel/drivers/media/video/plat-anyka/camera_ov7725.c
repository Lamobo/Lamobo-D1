/**
 * @file camera_ov7725.c
 * @brief camera driver file
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2010-12-28
 * @version 1.0
 * @ref
 */ 
#ifdef CONFIG_LINUX_AKSENSOR
#include <plat-anyka/aksensor.h>
#include <plat-anyka/wrap_sensor.h>
#include <plat-anyka/cam_com_sensor.h>
#include "camera_ov7725.h"
#else 
#include "akdefine.h"
#include "cam_com_sensor.h"
#include "camera_ov7725.h"
#include "Gpio_config.h"
#endif

#if defined (USE_CAMERA_OV7725) || defined (CONFIG_SENSOR_OV7725)

#define CAM_EN_LEVEL            0    
#define CAM_RESET_LEVEL         0
   
#define CAMERA_SCCB_ADDR        0x42
#define CAMERA_OV7725_ID        0x7721

#define CAMERA_MCLK_DIV         3              //192Mhz/(2*(3+1))=24Mhz

#define OV7725_CAMERA_MCLK      24 //24, 30fps/60fps; //32, 40fps

static T_CAMERA_TYPE camera_ov7725_type = CAMERA_P3M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_ov7725_CurMode = CAMERA_MODE_VGA;

static T_VOID camera_setbit(T_U8 reg, T_U8 bit, T_U8 value)
{
    T_U8 tmp;

    tmp = sccb_read_data(CAMERA_SCCB_ADDR, reg);
    if (value == 1)
        tmp |= 0x1 << bit;
    else
        tmp &= ~(0x1 << bit);
    sccb_write_data(CAMERA_SCCB_ADDR, reg, &tmp, 1);
}

static T_BOOL camera_set_param(const T_U8 tabParameter[])
{ 
    int i = 0;
    T_U8 temp_value;

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

            if (!((tabParameter[i] == 0x12) && (tabParameter[i + 1] & 0x80))
                && !((tabParameter[i] == 0xc9) && (tabParameter[i + 1] & 0x60)))
            {                
                temp_value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);
                if (temp_value != tabParameter[i + 1])
                {
                    akprintf(C1, M_DRVSYS, "set parameter error!\n");
                    akprintf(C1, M_DRVSYS, "reg 0x%x write data is 0x%x, read data is 0x%x!\n", tabParameter[i], tabParameter[i + 1], temp_value);

                    return AK_FALSE;
                }
            }
        }
        i += 2;
    }

    return AK_TRUE;
}

static T_VOID camera_setup(const T_U8 tabParameter[])
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
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)&tabParameter[i + 1], 1);
        }
        i += 2;
    }
}

static T_VOID cam_ov7725_open(T_VOID)
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

static T_BOOL cam_ov7725_close(T_VOID)
{
    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, !CAM_EN_LEVEL);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, !gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));    
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);

    gpio_set_pin_dir(GPIO_I2C_SCL, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SCL, GPIO_LEVEL_LOW);
    gpio_set_pin_dir(GPIO_I2C_SDA, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SDA, GPIO_LEVEL_LOW);
    
    return AK_TRUE;
}

static T_U32 cam_ov7725_read_id(T_VOID)
{
    T_U8 value = 0x00;
    T_U32 id = 0;

    sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);        //init sccb first here!!
    
    value = sccb_read_data(CAMERA_SCCB_ADDR, 0x0a);
    id = value << 8;
    value = sccb_read_data(CAMERA_SCCB_ADDR, 0x0b);
    id |= value;    
            
    return id;
}

/**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author xia_wenting 
 * @date 2010-12-28
 * @return T_BOOL
 * @retval AK_TRUE if success, else AK_FALSE
 */
static T_BOOL cam_ov7725_init(void)
{
    if (!camera_set_param(INIT_TAB))
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
 * @brief Set camera mode to specify image quality, SXGA/VGA/CIF/ etc 
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] mode mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_mode(T_CAMERA_MODE mode)
{
    s_ov7725_CurMode = mode;
    switch(mode)
    {
        case CAMERA_MODE_VGA:
            camera_setup(VGA_MODE_TAB);
            
            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        case CAMERA_MODE_CIF:
//            camera_setup(CIF_MODE_TAB);
            break;
        case CAMERA_MODE_QVGA:
//            camera_setup(QVGA_MODE_TAB);
            break;
        case CAMERA_MODE_QCIF:
//            camera_setup(QCIF_MODE_TAB);
            break;
        case CAMERA_MODE_QQVGA:
//            camera_setup(QQVGA_MODE_TAB);
            break;
        case CAMERA_MODE_PREV:                              //preview mode
            camera_setup(PREV_MODE_TAB);
            
            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        case CAMERA_MODE_REC:              //record mode
            camera_setup(RECORD_MODE_TAB);

            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        default:
            s_ov7725_CurMode = CAMERA_MODE_VGA;
            akprintf(C1, M_DRVSYS, "set camera mode parameter error!\n");
            break;
        }
}

/**
 * @brief Set camera exposure mode 
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_exposure(T_CAMERA_EXPOSURE exposure)
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
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_brightness(T_CAMERA_BRIGHTNESS brightness)
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
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_contrast(T_CAMERA_CONTRAST contrast)
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
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_saturation(T_CAMERA_SATURATION saturation)
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
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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
        case CAMERA_SHARPNESS_6:
            camera_setup(SHARPNESS_6_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set sharpness parameter error!\n");
            break;
    }
}

static T_VOID cam_ov7725_set_hue(T_U32 value)
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

static T_VOID cam_ov7725_set_hue_auto(T_U32 value)
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
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_AWB(T_CAMERA_AWB awb)
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
        case AWB_NIGHT:
            camera_setup(AWB_NIGHT_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set AWB mode parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera mirror mode 
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_mirror(T_CAMERA_MIRROR mirror)
{
    switch(mirror)
    {
        case CAMERA_MIRROR_V:
            camera_setbit(0x1e, 4, 1);
            camera_setbit(0x1e, 5, 0);
            break;
        case CAMERA_MIRROR_H:
            camera_setbit(0x1e, 4, 0);
            camera_setbit(0x1e, 5, 1);
            break;
        case CAMERA_MIRROR_NORMAL:
            camera_setbit(0x1e, 4, 0);
            camera_setbit(0x1e, 5, 0);
            break;
        case CAMERA_MIRROR_FLIP:
            camera_setbit(0x1e, 4, 1);
            camera_setbit(0x1e, 5, 1);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set mirror parameter error!\n");
            break;
    }
}


/**
 * @brief Set camera effect mode 
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] effect effect mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7725_set_effect(T_CAMERA_EFFECT effect)
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
        case CAMERA_EFFECT_BWN:
            camera_setup(EFFECT_BWN_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set camer effect parameter error!\n");
            break;
    }
}

/**
 * @brief set camera window
 * @author  
 * @date 2010-07-30
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if fail
 */
static T_S32 cam_ov7725_set_digital_zoom(T_U32 srcWidth, T_U32 srcHeight)
{
    T_U16 hrefstart = 0, hrefstop = 0, vrefstart = 0, vrefstop = 0;
    T_U8 hbit = 0, lstartbit = 0, lstopbit = 0;
    T_CAMERA_MODE Cammode = s_ov7725_CurMode;
    T_U8 Camera_window_table[] =
    {
        0x17, 0,
        0x18, 0,
        0x32, 0,
        0x19, 0,
        0x1a, 0,
        0x03, 0,
        END_FLAG, END_FLAG
    };
    
    akprintf(C1, M_DRVSYS, "set window size %d, %d, %d\r\n", Cammode, srcWidth, srcHeight);
    
    if(Cammode == CAMERA_MODE_VGA )//VGA_MODE
    {
        if((srcWidth == 640) && (srcHeight == 480))
        {
            Camera_window_table[1] = 0x22;//0x13;    //0x17;
            Camera_window_table[3] = 0xa4;//0x01;    //0x18
            Camera_window_table[5] = 0x00;//0xb6;     //0x92;    //0x32 from 0x92 to 0xb6 by lujie @061009
            Camera_window_table[7] = 0x07;    //0x19
            Camera_window_table[9] = 0xf0;    //0x1a
            Camera_window_table[11] = 0x00;   //0x00;    //0x03 //0x00 from 0x0a to 0xb6 by lujie @061009
            if (camera_set_param(Camera_window_table)  == AK_TRUE)
            {
                return 1;
            }
            else
            {    
                return -1;
            } 
        }
        else
        {
            hrefstart = 158 + (640 - srcWidth) / 2;   // by lujie 154 to 156 @061009
            hrefstop = hrefstart + srcWidth;

            vrefstart = 8 + (480 - srcHeight) / 2;
            vrefstop = vrefstart + srcHeight; 
        }
    }
    else if(Cammode == CAMERA_MODE_QVGA )//QVGA_MODE
    {
        if((srcWidth == 320) && (srcHeight == 240))
        {
            Camera_window_table[1] = 0x15;    //0x17;
            Camera_window_table[3] = 0x03;    //0x18
            Camera_window_table[5] = 0x36;    //0x32
            Camera_window_table[7] = 0x02;    //0x19
            Camera_window_table[9] = 0x7a;    //0x1a
            Camera_window_table[11]= 0x0a;    //0x03     
            if (camera_set_param(Camera_window_table) == AK_TRUE)
            {
                return 1;
            }
            else
            {
                return -1;    
            }               
        }
        else
        {
            hrefstart = 282 + (320 - srcWidth) / 2;//196
            hrefstop = hrefstart + srcWidth;     //836

            vrefstart = 8 + (240 - srcHeight) / 2 ;
            vrefstop = vrefstart + srcHeight;
        }
    }
    else if(Cammode == CAMERA_MODE_QQVGA )      //QQVGA_MODE
    {
        if((srcWidth == 160) && (srcHeight == 120))
        {
            cam_ov7725_set_mode(CAMERA_MODE_QQVGA);
            return 1;
        }
        hrefstart = 282 + (160 - srcWidth) * 2;
        hrefstop = hrefstart + srcWidth;

        vrefstart = 8 + (120 - srcHeight) / 2 ;
        vrefstop = vrefstart + srcHeight;
    }
    else
    {
        return 0;
    }

    hbit = hrefstart >> 3;         //Horizontal Frame start high 8-bit
    lstartbit = hrefstart & 0x7;   //Horizontal Frame start low 3-bit

    Camera_window_table[1] = hbit;

    if(Cammode != CAMERA_MODE_VGA && Cammode != CAMERA_MODE_SXGA)
    {
        if (hrefstop > 800)
        {
            hrefstop -=800;
        }
    }
    hbit = hrefstop >> 3;          //Horizontal Frame end high 8-bit
    lstopbit = hrefstop & 0x7;     //Horizontal Frame end low 3-bit

    Camera_window_table[3] = hbit;

    Camera_window_table[5] = 0x80 | lstartbit | (lstopbit << 3) ;

    hbit = vrefstart >> 2;         //Vertical Frame start high 8-bit
    lstartbit = vrefstart & 0x2;   //Vertical Frame start low 2-bit

    Camera_window_table[7] = hbit;

    hbit = vrefstop >> 2;         //Vertical Frame end high 8-bit
    lstopbit = vrefstop & 0x2;    //Vertical Frame end low 2-bit

    Camera_window_table[9] = hbit;

    Camera_window_table[11] = 0x0 | lstartbit | (lstopbit << 2);

    if (camera_set_param(Camera_window_table) == AK_TRUE)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

static T_VOID cam_ov7725_set_night_mode(T_NIGHT_MODE mode)
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

static T_VOID cam_ov7725_set_anti_flicker(T_U32 value)
{
    switch(value) {
    case V4L2_CID_POWER_LINE_FREQUENCY_DISABLED:
        camera_setup(ANTI_FLICKER_DISABLE_TAB);
		//akprintf(C1, M_DRVSYS, "Anti-flicker not support 'Disable', Error."
		//" please select other frequency!\n");
        break;
    case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
        camera_setup(ANTI_FLICKER_50HZ_TAB);
        break;
	case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
        camera_setup(ANTI_FLICKER_60HZ_TAB);
        break;
    case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
        camera_setup(ANTI_FLICKER_AUTO_TAB);
		//akprintf(C1, M_DRVSYS, "Anti-flicker not support 'Auto', Error."
		//" please select other frequency!\n");
        break;
    default:
        akprintf(C1, M_DRVSYS, "set Anti-flicker parameter error!\n");
        break;
    }
}

static T_BOOL cam_ov7725_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
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
    else
    {
        akprintf(C1, M_DRVSYS, "30W camera dose not support such mode");
        return AK_FALSE;
    }
    
    cam_ov7725_set_mode(Cammode);
    cam_ov7725_set_digital_zoom(srcWidth, srcHeight);    
    mini_delay(200);
    return AK_TRUE;
}

static T_BOOL cam_ov7725_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{    
    cam_ov7725_set_mode(CAMERA_MODE_PREV);    
    cam_ov7725_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(200);
    return AK_TRUE;
}

static T_BOOL cam_ov7725_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{
    cam_ov7725_set_mode(CAMERA_MODE_REC);
    cam_ov7725_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(200);
    return AK_TRUE;
}

static T_CAMERA_TYPE cam_ov7725_get_type(T_VOID)
{
    return camera_ov7725_type;
} 

static T_VOID cam_ov7725_set_sensor_param(T_U32 cmd, T_U32 data)
{
	T_U8 value;

	value = (T_U8)data;
	sccb_write_data(CAMERA_SCCB_ADDR, (T_U8)cmd, &value, 1);
}

static T_U16 cam_ov7725_get_sensor_param(T_U32 cmd)
{
	return sccb_read_data(CAMERA_SCCB_ADDR, (T_U8)cmd);
}

static T_CAMERA_FUNCTION_HANDLER ov7725_function_handler = 
{
    OV7725_CAMERA_MCLK,
    cam_ov7725_open,
    cam_ov7725_close,
    cam_ov7725_read_id,
    cam_ov7725_init,
    cam_ov7725_set_mode,
    cam_ov7725_set_exposure,
    cam_ov7725_set_brightness,
    cam_ov7725_set_contrast,
    cam_ov7725_set_saturation,
    cam_ov7725_set_sharpness,
    cam_ov7725_set_hue,
    cam_ov7725_set_hue_auto,
    cam_ov7725_set_AWB,
    cam_ov7725_set_mirror,
    cam_ov7725_set_effect,
    cam_ov7725_set_digital_zoom,
    cam_ov7725_set_night_mode,
    AK_NULL,
    cam_ov7725_set_anti_flicker,
    cam_ov7725_set_to_cap,
    cam_ov7725_set_to_prev,
    cam_ov7725_set_to_record,
    cam_ov7725_get_type,
    cam_ov7725_set_sensor_param,
    cam_ov7725_get_sensor_param
};

#ifndef CONFIG_LINUX_AKSENSOR
static int camera_ov7725_reg(void)
{
    camera_reg_dev(CAMERA_OV7725_ID, &ov7725_function_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_ov7725_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#else

static const char * resolution_menu[] = {
      [0] = "640x480",
};

static const char * hflip_menu[] = {
	[CAMERA_MIRROR_NORMAL] = "Normal",
	[CAMERA_MIRROR_V] = "VFlip",
    [CAMERA_MIRROR_H] = "Mirror",
    [CAMERA_MIRROR_FLIP] = "H/VFlip",
};

static const char * vflip_menu[] = {
	[CAMERA_MIRROR_NORMAL] = "Normal",
	[CAMERA_MIRROR_V] = "VFlip",
    [CAMERA_MIRROR_H] = "Mirror",
    [CAMERA_MIRROR_FLIP] = "H/VFlip",
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

static int ov7725_s_ctl(struct v4l2_ctrl *ctrl)
{
	int ret = -EINVAL;

	switch (ctrl->id) {
		case V4L2_CID_AUTO_WHITE_BALANCE:
			if (ov7725_function_handler.cam_set_AWB_func) {
				ov7725_function_handler.cam_set_AWB_func(ctrl->val);
				ret = 0;
			}
			break;
		case V4L2_CID_COLORFX:
			if (ov7725_function_handler.cam_set_effect_func) {
				ov7725_function_handler.cam_set_effect_func(ctrl->val);
				ret = 0;
			}
			break;
			
		case V4L2_CID_BRIGHTNESS:
			if (ov7725_function_handler.cam_set_brightness_func) {
				ov7725_function_handler.cam_set_brightness_func(ctrl->val);
				ret = 0;
			}
			break;	
		case V4L2_CID_CONTRAST:
			if (ov7725_function_handler.cam_set_contrast_func) {
				ov7725_function_handler.cam_set_contrast_func(ctrl->val);
				ret = 0;
			}
			break;	
		case V4L2_CID_SATURATION:
			if (ov7725_function_handler.cam_set_saturation_func) {
				ov7725_function_handler.cam_set_saturation_func(ctrl->val);
				ret = 0;
			}
			break;	
		case V4L2_CID_SHARPNESS:
			if (ov7725_function_handler.cam_set_sharpness_func) {
				ov7725_function_handler.cam_set_sharpness_func(ctrl->val);
				ret = 0;
			}
			break;
		case V4L2_CID_HUE:
			if (ov7725_function_handler.cam_set_hue) {
				ov7725_function_handler.cam_set_hue(ctrl->val);
				ret = 0;
			}
			break;	
		case V4L2_CID_HUE_AUTO:
			if (ov7725_function_handler.cam_set_hue_auto) {
				ov7725_function_handler.cam_set_hue_auto(ctrl->val);
				ret = 0;
			}
			break;	
		case V4L2_CID_HFLIP:
			if (ov7725_function_handler.cam_set_mirror_func) {
				ov7725_function_handler.cam_set_mirror_func( 
					ctrl->val ? CAMERA_MIRROR_H : CAMERA_MIRROR_NORMAL);
				ret = 0;
			}
			break;
		case V4L2_CID_VFLIP:
			if (ov7725_function_handler.cam_set_mirror_func) {
				ov7725_function_handler.cam_set_mirror_func(
					ctrl->val ? CAMERA_MIRROR_V : CAMERA_MIRROR_NORMAL);
				ret = 0;
			}
			break;
		case V4L2_CID_NIGHTMODE:
			if (ov7725_function_handler.cam_set_night_mode_func) {
				ov7725_function_handler.cam_set_night_mode_func(ctrl->val);
				ret = 0;
			}
			break;
		case V4L2_CID_POWER_LINE_FREQUENCY:
			if (ov7725_function_handler.cam_set_anti_flicker_func) {
				ov7725_function_handler.cam_set_anti_flicker_func(ctrl->val);
				ret = 0;
			}
			break;
		default:
			break;
	}

	return ret;	
};

static struct v4l2_ctrl_ops ov7725_ctrl_ops = {
	.s_ctrl = ov7725_s_ctl,
};

static const struct v4l2_ctrl_config ov7725_ctrls[] = {
	{
		.ops		= &ov7725_ctrl_ops,
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
		.ops		= &ov7725_ctrl_ops,
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
		.ops		= &ov7725_ctrl_ops,
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
		.ops		= &ov7725_ctrl_ops,
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
		.ops		= &ov7725_ctrl_ops,
		.id		= V4L2_CID_CONTRAST,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Contrast",
		.min		= 0,
		.max		= CAMERA_CONTRAST_NUM -1,
		.step		= 1,
		.def		= 0,
	},
	{
		.ops		= &ov7725_ctrl_ops,
		.id		= V4L2_CID_SATURATION,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Saturation",
		.min		= 0,
		.max		= CAMERA_SATURATION_NUM -1,
		.step		= 1,
		.def		= 0,
	},
	{
		.ops		= &ov7725_ctrl_ops,
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
static const struct aksensor_color_format ov7725_formats[] = {
	{
		.code = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
	},
	{
		.code		= V4L2_MBUS_FMT_YVYU8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
	},
	{
		.code		= V4L2_MBUS_FMT_UYVY8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
	},
};

static const struct aksensor_win_size ov7725_win[] = {
	{.name = "VGA",		.width = 640,	.height = 480},
};


static struct sensor_info ov7725_sensor_info = {
	.sensor_name = "ov7725",
	.sensor_id = CAMERA_OV7725_ID,
	.ctrls = ov7725_ctrls,
	.nr_ctrls = ARRAY_SIZE(ov7725_ctrls), 
	.formats = ov7725_formats,
	.num_formats = ARRAY_SIZE(ov7725_formats),	
	.resolution = ov7725_win,
	.num_resolution = ARRAY_SIZE(ov7725_win),
	.handler = &ov7725_function_handler,
};

static int ov7725_module_init(void)
{
	return register_sensor(&ov7725_sensor_info);
}
module_init(ov7725_module_init)
#endif

#endif
