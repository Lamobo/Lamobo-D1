/**
 * @file camera_hm1375.h
 * @brief camera driver file
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author wudaochao 
 * @date 2013-04-26
 * @version 1.0
 * @ref
 */
#ifndef __CAMERA_OV2710_H__
#define __CAMERA_OV2710_H__


#if defined (USE_CAMERA_OV2710) || defined (CONFIG_SENSOR_OV2710)

#undef DELAY_FLAG
#undef END_FLAG
#define DELAY_FLAG        0xfd   // first parameter is 0xfe, then 2nd parameter is delay time count
#define END_FLAG          0xfe   // first parameter is 0xff, then parameter table is over 

static const T_U16 INIT_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 UXGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SXGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 VGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 CIF_MODE_TAB[] = 
{

    END_FLAG, END_FLAG
};

static const T_U16 QVGA_MODE_TAB[] = 
{
   
    END_FLAG, END_FLAG
};

static const T_U16 QCIF_MODE_TAB[] = 
{

    END_FLAG, END_FLAG
};

static const T_U16 QQVGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 PREV_MODE_TAB[] = 
{    
    END_FLAG, END_FLAG
};

static const T_U16 RECORD_MODE_TAB[] = 
{
	0x3103, 0x03,
	0x3008, 0x82, //reset
	0x3017, 0x7f,
	0x3018, 0xfc,
	0x3706, 0x61,
	0x3712, 0x0c,
	0x3630, 0x6d,
	0x3801, 0xb4,
	0x3621, 0x04,
	0x3604, 0x60,
	0x3603, 0xa7,
	0x3631, 0x26,
	0x3600, 0x04,
	0x3620, 0x37,
	0x3623, 0x00,
	0x3702, 0x9e,
	0x3703, 0x74,
	0x3704, 0x10,
	0x370d, 0x0f,
	0x3713, 0x8b,
	0x3714, 0x74,
	0x3710, 0x9e,
	0x3801, 0xc4,
	0x3605, 0x05,
	0x3606, 0x12,
	0x302d, 0x90,
	0x370b, 0x40,
	0x3716, 0x31,
	0x380d, 0x74,
	0x5181, 0x20,
	0x518f, 0x00,
	0x4301, 0xff,
	0x4303, 0x00,
	0x3a00, 0x78,
	0x300f, 0x88,
	0x3011, 0x28,
	0x3a1a, 0x06,
	0x3a18, 0x00,
	0x3a19, 0x7a,
	0x3a13, 0x54,
	0x382e, 0x0f,
	0x381a, 0x1a,
	//aec 
	0x3a0f, 0x40,
	0x3a10, 0x38,
	0x3a1b, 0x48,
	0x3a1e, 0x30,
	0x3a11, 0x90,
	0x3a1f, 0x10,

	//VGA_binning(640*480) Reference Setting 24M MCLK 30fps
	//window
	0x381c, 0x10,
	0x381d, 0x42,
	0x381e, 0x3,
	0x381f, 0xc8,
	0x3820, 0xa,
	0x3821, 0x29,
	0x3800, 0x2,
	0x3801, 0xd6,
	0x3802, 0x0,
	0x3803, 0x5,
	0x3804, 0x2,
	0x3805, 0x80,
	0x3806, 0x1,
	0x3807, 0xe0,
	0x3808, 0x2,
	0x3809, 0x80,
	0x380a, 0x1,
	0x380b, 0xe0,
	0x380c, 0xa,
	0x380d, 0x84,
	0x380e, 0x1,
	0x380f, 0xf0,
	0x3810, 0x8,
	0x3811, 0x2,
	//timing
	0x3818, 0xe1, //flip on, mirror on
	0x3621, 0xd4,
	0x3622, 0x8,
	0x370d, 0x4f,
	0x401c, 0x4,
	0x3012, 0x1,
	0x300f, 0x88,
	0x3011, 0x28,
	0x3010, 0x10,
	//banding
	0x3a0a, 0x12,
	0x3a0b, 0x99,
	0x3a08, 0xf,
	0x3a09, 0x80,
	0x3a0d, 0x01,
	0x3a0e, 0x00,

	END_FLAG, END_FLAG
};

static const T_U16 RECORD_720P_TAB[] = 
{
	0x3103, 0x03,
	0x3008, 0x82, //reset
	0x3017, 0x7f,
	0x3018, 0xfc,
	0x3706, 0x61,
	0x3712, 0x0c,
	0x3630, 0x6d,
	0x3801, 0xb4,
	0x3621, 0x04,
	0x3604, 0x60,
	0x3603, 0xa7,
	0x3631, 0x26,
	0x3600, 0x04,
	0x3620, 0x37,
	0x3623, 0x00,
	0x3702, 0x9e,
	0x3703, 0x74,
	0x3704, 0x10,
	0x370d, 0x0f,
	0x3713, 0x8b,
	0x3714, 0x74,
	0x3710, 0x9e,
	0x3801, 0xc4,
	0x3605, 0x05,
	0x3606, 0x12,
	0x302d, 0x90,
	0x370b, 0x40,
	0x3716, 0x31,
	0x380d, 0x74,
	0x5181, 0x20,
	0x518f, 0x00,
	0x4301, 0xff,
	0x4303, 0x00,
	0x3a00, 0x78,
	0x300f, 0x88,
	0x3011, 0x28,
	0x3a1a, 0x06,
	0x3a18, 0x00,
	0x3a19, 0x7a,
	0x3a13, 0x54,
	0x382e, 0x0f,
	0x381a, 0x1a,
	//aec 
	0x3a0f, 0x40,
	0x3a10, 0x38,
	0x3a1b, 0x48,
	0x3a1e, 0x30,
	0x3a11, 0x90,
	0x3a1f, 0x10,
	
	//720p(1280*720) Reference Setting 24M MCLK 20fps
	//window
	0x381c, 0x10,
	0x381d, 0xb8,
	0x381e, 0x2,
	0x381f, 0xdc,
	0x3820, 0xa,
	0x3821, 0x29,
	0x3800, 0x1,
	0x3801, 0xc4,
	0x3802, 0x0,
	0x3803, 0x09,
	0x3804, 0x5,
	0x3805, 0x0,
	0x3806, 0x2,
	0x3807, 0xd0,
	0x3808, 0x5,
	0x3809, 0x0,
	0x380a, 0x2,
	0x380b, 0xd0,
	0x380c, 0x7,
	0x380d, 0x0,
	0x380e, 0x2,
	0x380f, 0xe8,
	0x3810, 0x10,
	0x3811, 0x6,
	//timing
	0x3818, 0xe0, //flip on, mirror on
	0x3621, 0x14,
	0x3622, 0x8,
	0x370d, 0xf, 
	0x401c, 0x8,
	0x3012, 0x1,
	0x300f, 0x88,
	0x3011, 0x28,
	0x3010, 0x20,
	//banding
	0x3a0a, 0x9,
	0x3a0b, 0x4c,
	0x3a08, 0x7,
	0x3a09, 0xc0,
	0x3a0d, 0x04,
	0x3a0e, 0x05,

	END_FLAG, END_FLAG
};

/****************   Camera Exposure Table   ****************/
static const T_U16 EXPOSURE_WHOLE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EXPOSURE_CENTER_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EXPOSURE_MIDDLE_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Brightness Table   ****************/
static const T_U16 BRIGHTNESS_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 BRIGHTNESS_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 BRIGHTNESS_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 BRIGHTNESS_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 BRIGHTNESS_4_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 BRIGHTNESS_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 BRIGHTNESS_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Contrast Table   ****************/
static const T_U16 CONTRAST_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 CONTRAST_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 CONTRAST_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 CONTRAST_4_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 CONTRAST_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 CONTRAST_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 CONTRAST_7_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Saturation Table   ****************/
static const T_U16 SATURATION_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SATURATION_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SATURATION_3_TAB[] = 
{ 
    END_FLAG, END_FLAG
};

static const T_U16 SATURATION_4_TAB[] = 
{ 
    END_FLAG, END_FLAG
};

static const T_U16 SATURATION_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Sharpness Table   ****************/
static const T_U16 SHARPNESS_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SHARPNESS_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SHARPNESS_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SHARPNESS_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SHARPNESS_4_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SHARPNESS_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 SHARPNESS_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera hue Table   ****************/
static const T_U16 HUE_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 HUE_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 HUE_2_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 HUE_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 HUE_4_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 HUE_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 HUE_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 HUE_AUTO_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 HUE_AUTO_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera AWB Table   ****************/
static const T_U16 AWB_AUTO_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 AWB_SUNNY_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 AWB_CLOUDY_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 AWB_OFFICE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 AWB_HOME_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Effect Table   ****************/
static const T_U16 EFFECT_NORMAL_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EFFECT_SEPIA_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EFFECT_ANTIQUE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EFFECT_BLUISH_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EFFECT_GREENISH_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EFFECT_REDDISH_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EFFECT_NEGATIVE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 EFFECT_BW_TAB[] = 
{    
    END_FLAG, END_FLAG
};

/****************   Camera night/day mode   ****************/
static const T_U16 DAY_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U16 NIGHT_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};
#endif
#endif
