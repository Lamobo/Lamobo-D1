/**
 * @file camera_h22.h
 * @brief camera driver file
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-10-26
 * @version 1.0
 * @ref
 */
#ifndef __CAMERA_H22_H__
#define __CAMERA_H22_H__


#if defined (USE_CAMERA_H22) || defined (CONFIG_SENSOR_H22)

#undef DELAY_FLAG
#undef END_FLAG
#define DELAY_FLAG        0xfd   // first parameter is 0xfe, then 2nd parameter is delay time count
#define END_FLAG          0xfe   // first parameter is 0xff, then parameter table is over 

static const T_U8 INIT_TAB[] = 
{	

    END_FLAG, END_FLAG
};

static const T_U8 UXGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SXGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 VGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 CIF_MODE_TAB[] = 
{

    END_FLAG, END_FLAG
};

static const T_U8 QVGA_MODE_TAB[] = 
{
   
    END_FLAG, END_FLAG
};

static const T_U8 QCIF_MODE_TAB[] = 
{

    END_FLAG, END_FLAG
};

static const T_U8 QQVGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 PREV_MODE_TAB[] = 
{    
    END_FLAG, END_FLAG
};

static const T_U8 RECORD_MODE_TAB[] = 
{
	//640*480 Reference Setting 24M MCLK 15fps
	
    END_FLAG, END_FLAG
};

static const T_U8 RECORD_720P_TAB[] = 
{
	0x12, 0x40,
	0x00, 0x00,
	0x01, 0x55,
	0x02, 0x03,
	0x0E, 0x19,
	0x0F, 0x04,
	0x10, 0x20,
	0x11, 0x01,
	0x18, 0xD5,
	0x19, 0x00,
	0x1D, 0xFF,
	0x1E, 0x1F,
	0x1F, 0x00,
	0x20, 0xDC,
	0x21, 0x05,
	0x22, 0x55,
	0x23, 0x03,
	0x24, 0x00,
	0x25, 0xD0,
	0x26, 0x25,
	0x27, 0xE9,
	0x28, 0x0D,
	0x29, 0x00,
	0x2A, 0xD4,
	0x2B, 0x10,
	0x2C, 0x00,
	0x2D, 0x0A,
	0x2E, 0xC2,
	0x2F, 0x20,
	0x37, 0x36,
	0x38, 0x98,
	0x12, 0x00,

    END_FLAG, END_FLAG
};


/****************   Camera Exposure Table   ****************/
static const T_U8 EXPOSURE_WHOLE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EXPOSURE_CENTER_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EXPOSURE_MIDDLE_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Brightness Table   ****************/
static const T_U8 BRIGHTNESS_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_4_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Contrast Table   ****************/
static const T_U8 CONTRAST_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_4_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_7_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Saturation Table   ****************/
static const T_U8 SATURATION_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_3_TAB[] = 
{ 
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_4_TAB[] = 
{ 
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Sharpness Table   ****************/
static const T_U8 SHARPNESS_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_4_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera hue Table   ****************/
static const T_U8 HUE_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 HUE_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 HUE_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 HUE_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 HUE_4_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 HUE_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 HUE_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 HUE_AUTO_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 HUE_AUTO_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera AWB Table   ****************/
static const T_U8 AWB_AUTO_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_SUNNY_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_CLOUDY_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_OFFICE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_HOME_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Effect Table   ****************/
static const T_U8 EFFECT_NORMAL_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_SEPIA_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_ANTIQUE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_BLUISH_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_GREENISH_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_REDDISH_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_NEGATIVE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_BW_TAB[] = 
{    
    END_FLAG, END_FLAG
};

/****************   Camera night/day mode   ****************/
static const T_U8 DAY_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 NIGHT_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera anti-flicker mode   ****************/
static const T_U8 ANTI_FLICKER_DISABLE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 ANTI_FLICKER_50HZ_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 ANTI_FLICKER_60HZ_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 ANTI_FLICKER_AUTO_TAB[] = 
{
    END_FLAG, END_FLAG
};

#endif
#endif
