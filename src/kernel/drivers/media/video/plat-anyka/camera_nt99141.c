/**
 * @file camera_nt99141.c
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
#include "camera_nt99141.h"
#else 
#include "akdefine.h"
#include "cam_com_sensor.h"
#include "camera_nt99141.h"
#include "Gpio_config.h"
#endif

#if defined (USE_CAMERA_NT9914100) || defined (CONFIG_SENSOR_NT99141)

#define CAM_EN_LEVEL            0    
#define CAM_RESET_LEVEL         0
   
#define CAMERA_SCCB_ADDR       0x54
#define CAMERA_NT9914100_ID        0x1410

//#define CAMERA_MCLK_DIV         3              //192Mhz/(2*(3+1))=24Mhz

#define NT9914100_CAMERA_MCLK      24 //24, 30fps/60fps; //32, 40fps

static T_CAMERA_TYPE camera_nt9914100_type = CAMERA_1P3M;//CAMERA_2M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_nt99141_CurMode = CAMERA_MODE_VGA;
#if 0
static T_VOID camera_setbit(T_U16 reg, T_U8 bit, T_U8 value)
{
    T_U8 tmp;

    //sccb_read_data3(CAMERA_SCCB_ADDR, reg, &tmp, 1);
    tmp=sccb_read_short(CAMERA_SCCB_ADDR, reg);
    if (value == 1)
    {    
		tmp |= 0x1 << bit;
	}
    else
    {
	    tmp &= ~(0x1 << bit);
	}
    sccb_write_short(CAMERA_SCCB_ADDR, reg, &tmp, 1);
}
#endif

static T_BOOL camera_set_param(const T_U8 tabParameter[])
{ 
    int i = 0;
    T_U8 temp_value;
	T_U16 temp_reg;

    while (1)   
    {
        if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i + 1])\
			&& (END_FLAG == tabParameter[i + 2])) 
        {
            break;
        }
        else if ((DELAY_FLAG == tabParameter[i]) && (DELAY_FLAG == tabParameter[i + 1]))
        {
            mini_delay(tabParameter[i + 2]);
        }
        else
        {
            temp_reg = tabParameter[i];
			temp_reg = temp_reg*256 + tabParameter[i +1];
			
            sccb_write_short(CAMERA_SCCB_ADDR, temp_reg, (T_U8 *)(&tabParameter[i + 2]), 1);
              
            //sccb_read_data3(CAMERA_SCCB_ADDR, temp_reg, &temp_value, 1);
            temp_value = sccb_read_short(CAMERA_SCCB_ADDR, temp_reg);
            if (temp_value != tabParameter[i + 2])
            {
                akprintf(C1, M_DRVSYS, "set parameter error!\n");
                akprintf(C1, M_DRVSYS, "reg 0x%x write data is 0x%x, read data is 0x%x!\n", temp_reg, tabParameter[i + 2], temp_value);

                //return AK_FALSE;
            }
        }
        
        i += 3;
    }

    return AK_TRUE;
}

static T_VOID camera_setup(const T_U8 tabParameter[])
{
    int i = 0;
	T_U8 temp_value;
	T_U16 temp_reg;

    while (1)
    {
        if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i + 1])\
			&& (END_FLAG == tabParameter[i + 2]))  
        {
            break;
        }
        else if ((DELAY_FLAG == tabParameter[i]) && (DELAY_FLAG == tabParameter[i + 1]))
        {
            mini_delay(tabParameter[i + 2]);
        }
        else
        {
            temp_reg = tabParameter[i];
			temp_reg = temp_reg*256 + tabParameter[i +1];
            sccb_write_short(CAMERA_SCCB_ADDR, temp_reg, (T_U8 *)&tabParameter[i + 2], 1);

			    //sccb_read_data3(CAMERA_SCCB_ADDR, temp_reg, &temp_value, 1);
			    temp_value = sccb_read_short(CAMERA_SCCB_ADDR, temp_reg);
         /*   if (temp_value != tabParameter[i + 2])
            {
                akprintf(C1, M_DRVSYS, "set parameter error!\n");
                akprintf(C1, M_DRVSYS, "reg 0x%x write data is 0x%x, read data is 0x%x!\n", temp_reg, tabParameter[i + 2], temp_value);

                //return AK_FALSE;
            }*/
        }
        i += 3;
    }
}

static T_VOID cam_nt9914100_open(T_VOID)
{  	
	akprintf(C1, M_DRVSYS, "cam_nt99141_open!\n");
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

static T_BOOL cam_nt9914100_close(T_VOID)
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

static T_U32 cam_nt9914100_read_id(T_VOID)
{
    T_U8 value = 0x00;
    T_U32 id = 0;
akprintf(C1, M_DRVSYS, "cam_nt99141_read_id\r\n");    
    sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);        //init sccb first here!!
    
    //sccb_read_data3(CAMERA_SCCB_ADDR, 0x3000, &value, 1);
    value=sccb_read_short(CAMERA_SCCB_ADDR, 0x3000);
    id = value << 8;
    //sccb_read_data3(CAMERA_SCCB_ADDR, 0x3001, &value, 1);
    value=sccb_read_short(CAMERA_SCCB_ADDR, 0x3001);
    id |= value;    
    akprintf(C1, M_DRVSYS, "cam_nt99141_read_id= 0x%x\r\n",id   );    
            
    return id;
}

/**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author xia_wenting 
 * @date 2010-12-28
 * @return T_BOOL
 * @retval AK_TRUE if success, else AK_FALSE
 */
static T_BOOL cam_nt9914100_init(void)
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
static T_VOID cam_nt9914100_set_mode(T_CAMERA_MODE mode)
{
    s_nt99141_CurMode = mode;
	
	akprintf(C1, M_DRVSYS, "cam_nt99141_set_mode!\n");
    switch(mode)
    {
        case CAMERA_MODE_UXGA: 
            camera_setup(UXGA_MODE_TAB);
            break;    
        case CAMERA_MODE_SXGA: 
            camera_setup(SXGA_MODE_TAB);
            break; 
        case CAMERA_MODE_VGA:
		    akprintf(C1, M_DRVSYS, "cam mode is 640 x 480!\n");	
            camera_setup(VGA_MODE_TAB);
         // camera_setup(RECORD_720P_TAB);
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
			akprintf(C1, M_DRVSYS, "cam mode is prev_mode!\n");	
            camera_setup(PREV_MODE_TAB);
            
            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        case CAMERA_MODE_REC:
			akprintf(C1, M_DRVSYS, "cam mode is record_mode!\n");	
            camera_setup(RECORD_MODE_TAB);

            if (CAMERA_NIGHT_MODE == night_mode)    
            {    
                camera_setup(NIGHT_MODE_TAB);
            }
            break;     
        case CAMERA_MODE_720P:
			akprintf(C1, M_DRVSYS, "cam mode is 1280 x 720!\n");			      
            camera_setup(RECORD_720P_TAB);
            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        default:
            s_nt99141_CurMode = CAMERA_MODE_720P;//CAMERA_MODE_VGA;
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
static T_VOID cam_nt9914100_set_exposure(T_CAMERA_EXPOSURE exposure)
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
static T_VOID cam_nt9914100_set_brightness(T_CAMERA_BRIGHTNESS brightness)
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
static T_VOID cam_nt9914100_set_contrast(T_CAMERA_CONTRAST contrast)
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
static T_VOID cam_nt9914100_set_saturation(T_CAMERA_SATURATION saturation)
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
static T_VOID cam_nt9914100_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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

/**
 * @brief Set camera AWB mode 
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_nt9914100_set_AWB(T_CAMERA_AWB awb)
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
 * @author xia_wenting 
 * @date 2010-12-28
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_nt9914100_set_mirror(T_CAMERA_MIRROR mirror)
{
    switch(mirror)
    {
        case CAMERA_MIRROR_V:
            camera_setup(MIRROR_V_TAB);//垂直镜相
            break;
        case CAMERA_MIRROR_H:
            camera_setup(MIRROR_H_TAB);//水平镜相
            break;
        case CAMERA_MIRROR_NORMAL:
            camera_setup(MIRROR_NORMAL_TAB);//正常
            break;
        case CAMERA_MIRROR_FLIP:
            camera_setup(MIRROR_FLIP_TAB);//垂直水平镜相
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
static T_VOID cam_nt9914100_set_effect(T_CAMERA_EFFECT effect)
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
 * @author  
 * @date 2010-07-30
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if fail
 */
static T_S32 cam_nt9914100_set_digital_zoom(T_U32 srcWidth, T_U32 srcHeight)
{
    return 1;
}

static T_VOID cam_nt9914100_set_night_mode(T_NIGHT_MODE mode)
{
    akprintf(C1, M_DRVSYS, "cam_nt99141_set_effect!\n");
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

static T_BOOL cam_nt9914100_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
{    
    T_CAMERA_MODE Cammode;

    akprintf(C1, M_DRVSYS, "cam_nt99141_set_to_cap!\n");
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
    else if ((srcWidth <= 1280) && (srcHeight <= 720))
    {
        Cammode = CAMERA_MODE_720P;
    }
    else if ((srcWidth <= 1600) && (srcHeight <= 1200))
    {
        Cammode = CAMERA_MODE_UXGA;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "nt99141 unsupport %d & %d mode!\n", srcWidth, srcHeight);
        return AK_FALSE;
    }
    
    if(Cammode == CAMERA_MODE_VGA)
    {
        camera_setup(CAP_VGA_MODE_TAB);
	}
	else if(Cammode == CAMERA_MODE_720P)
	{
        camera_setup(CAP_720P_MODE_TAB);
	}
	else
	{
    cam_nt9914100_set_mode(Cammode);
	}
    cam_nt9914100_set_digital_zoom(srcWidth, srcHeight);    
    mini_delay(300);
    return AK_TRUE;
}

static T_BOOL cam_nt9914100_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{    
    akprintf(C1, M_DRVSYS, "cam_nt99141_set_to_prev!\n");
    cam_nt9914100_set_mode(CAMERA_MODE_PREV);    
    cam_nt9914100_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(300);
    return AK_TRUE;
}

static T_BOOL cam_nt9914100_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{
    T_CAMERA_MODE Cammode = CAMERA_MODE_QQVGA;
    akprintf(C1, M_DRVSYS, "cam_nt99141_set_to_record!\n");
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
    akprintf(C1, M_DRVSYS, "cam mode is 720p 640 !\n");
        Cammode =  CAMERA_MODE_REC;
    }
    else if ((srcWidth <= 1280) && (srcHeight <= 720))
    {
    akprintf(C1, M_DRVSYS, "cam_ mode is 720p 720!\n");
         Cammode = CAMERA_MODE_720P;
    }
    cam_nt9914100_set_mode(Cammode);
    cam_nt9914100_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(300);
    return AK_TRUE;
}

static T_CAMERA_TYPE cam_nt9914100_get_type(T_VOID)
{
    return camera_nt9914100_type;
} 

static T_VOID cam_nt9914100_set_sensor_param(T_U32 cmd, T_U32 data)
{
	T_U8 value;

	value = (T_U8)data;
	sccb_write_short(CAMERA_SCCB_ADDR, (T_U16)cmd, &value, 1);
}

static T_CAMERA_FUNCTION_HANDLER nt9914100_function_handler = 
{
    NT9914100_CAMERA_MCLK,
    cam_nt9914100_open,
    cam_nt9914100_close,
    cam_nt9914100_read_id,
    cam_nt9914100_init,
    cam_nt9914100_set_mode,
    cam_nt9914100_set_exposure,
    cam_nt9914100_set_brightness,
    cam_nt9914100_set_contrast,
    cam_nt9914100_set_saturation,
    cam_nt9914100_set_sharpness,
	AK_NULL,
	AK_NULL,
    cam_nt9914100_set_AWB,
    cam_nt9914100_set_mirror,
    cam_nt9914100_set_effect,
    cam_nt9914100_set_digital_zoom,
    cam_nt9914100_set_night_mode,
    AK_NULL,
    AK_NULL,
    cam_nt9914100_set_to_cap,
    cam_nt9914100_set_to_prev,
    cam_nt9914100_set_to_record,
    cam_nt9914100_get_type,
    cam_nt9914100_set_sensor_param
};

#ifndef CONFIG_LINUX_AKSENSOR
static int camera_nt9914100_reg(void)
{
    camera_reg_dev(CAMERA_NT9914100_ID, &nt9914100_function_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_nt9914100_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#else

static const char * resolution_menu[] = {
	[0] = "1280x720",	   
	[1] = "640x480",
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

static int nt9914100_s_ctl(struct v4l2_ctrl *ctrl)
{
	int ret = -EINVAL;

	switch (ctrl->id) {
		case V4L2_CID_CONTRAST:
			if (nt9914100_function_handler.cam_set_contrast_func) {
				nt9914100_function_handler.cam_set_contrast_func(ctrl->val);
				ret = 0;
			}
			break;	
		case V4L2_CID_SATURATION:
			if (nt9914100_function_handler.cam_set_saturation_func) {
				nt9914100_function_handler.cam_set_saturation_func(ctrl->val);
				ret = 0;
			}
			break;	
		case V4L2_CID_HFLIP:
			if (nt9914100_function_handler.cam_set_mirror_func) {
				nt9914100_function_handler.cam_set_mirror_func( 
					ctrl->val ? CAMERA_MIRROR_H : CAMERA_MIRROR_NORMAL);
				ret = 0;
			}
			break;
		case V4L2_CID_VFLIP:
			if (nt9914100_function_handler.cam_set_mirror_func) {
				nt9914100_function_handler.cam_set_mirror_func(
					ctrl->val ? CAMERA_MIRROR_V : CAMERA_MIRROR_NORMAL);
				ret = 0;
			}
			break;
		case V4L2_CID_NIGHTMODE:
			if (nt9914100_function_handler.cam_set_night_mode_func) {
				nt9914100_function_handler.cam_set_night_mode_func(ctrl->val);
				ret = 0;
			}
			break;
		default:
			break;
	}

	return ret;	
};

static struct v4l2_ctrl_ops nt9914100_ctrl_ops = {
	.s_ctrl = nt9914100_s_ctl,
};

static const struct v4l2_ctrl_config nt9914100_ctrls[] = {
	{
		.ops		= &nt9914100_ctrl_ops,
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
		.ops		= &nt9914100_ctrl_ops,
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
		.ops		= &nt9914100_ctrl_ops,
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
		.ops		= &nt9914100_ctrl_ops,
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
		.ops		= &nt9914100_ctrl_ops,
		.id		= V4L2_CID_CONTRAST,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Contrast",
		.min		= 0,
		.max		= CAMERA_CONTRAST_NUM -1,
		.step		= 1,
		.def		= 0,
	},
	{
		.ops		= &nt9914100_ctrl_ops,
		.id		= V4L2_CID_SATURATION,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Saturation",
		.min		= 0,
		.max		= CAMERA_SATURATION_NUM -1,
		.step		= 1,
		.def		= 0,
	},
};


/*
 * supported format list
 */
static const struct aksensor_color_format nt9914100_formats[] = {
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

static const struct aksensor_win_size nt9914100_win[] = {
	{.name = "VGA",		.width = 640,	.height = 480},
	{.name = "720P",	.width = 1280,	.height = 720},
};


static struct sensor_info nt9914100_sensor_info = {
	.sensor_name = "nt9914100",
	.sensor_id = CAMERA_NT9914100_ID,
	.ctrls = nt9914100_ctrls,
	.nr_ctrls = ARRAY_SIZE(nt9914100_ctrls), 
	.formats = nt9914100_formats,
	.num_formats = ARRAY_SIZE(nt9914100_formats),	
	.resolution = nt9914100_win,
	.num_resolution = ARRAY_SIZE(nt9914100_win),
	.handler = &nt9914100_function_handler,
};

static int nt9914100_module_init(void)
{
	return register_sensor(&nt9914100_sensor_info);
}
module_init(nt9914100_module_init)
#endif

#endif
