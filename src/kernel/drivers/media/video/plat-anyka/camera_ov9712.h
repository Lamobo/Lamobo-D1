/**
 * @file camera_ov9712.h
 * @brief camera driver file
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-10-26
 * @version 1.0
 * @ref
 */
#ifndef __CAMERA_OV9712_H__
#define __CAMERA_OV9712_H__


#if defined (USE_CAMERA_OV9712) || defined (CONFIG_SENSOR_OV9712)

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
	//Reset
	0x12, 0x80,
	0x09, 0x10,
	//Core Settings
	0x1e, 0x07,
	0x5f, 0x18,
	0x69, 0x04,
	0x65, 0x2a,
	0x68, 0x0a,
	0x39, 0x28,
	0x4d, 0x90,
	0xc1, 0x80,
	0x0c, 0x30,
	0x6d, 0x02,
	//DSP
	0x96, 0xf1, //DSP options enable
	0xbc, 0x68,
	//Resolution and Format
	0x12, 0x00,
	0x3b, 0x00,
	0x97, 0x80,
	0x17, 0x25,
	0x18, 0xA2,
	0x19, 0x01,
	0x1a, 0xCA,
	0x03, 0x03,
	0x04, 0xc8,//flip on, mirror on
	0x32, 0x07,
	0x98, 0x40,
	0x99, 0xA0,
	0x9a, 0x01,
	0x57, 0x00,
	0x58, 0x78,
	0x59, 0x50,
	0x4c, 0x13,
	0x4b, 0x36,
	0x3d, 0x3c,
	0x3e, 0x03,
	0xbd, 0x50,
	0xbe, 0x78,
	//AWB
	//Lens Correction
	//YAVG
	0x4e, 0x55,	//AVERAGE 
	0x4f, 0x55,	
	0x50, 0x55,
	0x51, 0x55,
	0x24, 0x50,	//Exposure windows
	0x25, 0x40,
	0x26, 0xa1,
	//Clock
	0x5c, 0x59,
	0x5d, 0x00,
	0x11, 0x01,
	0x2a, 0x98,
	0x2b, 0x06,
	0x2d, 0x00,
	0x2e, 0x00,
	//General
	0x13, 0x85,
	0x14, 0x40, //Gain Ceiling 8X
	0x09, 0x00,
	
    END_FLAG, END_FLAG
};

static const T_U8 RECORD_720P_TAB[] = 
{
	//Reset 
	0x12, 0x80,
	0x09, 0x10,
	//Core Settings
	0x1e, 0x07,
	0x5f, 0x18,
	0x69, 0x04,
	0x65, 0x2a,
	0x68, 0x0a,
	0x39, 0x28,
	0x4d, 0x90,
	0xc1, 0x80,
	0x0c, 0x30,
	0x6d, 0x02,
	//DSP
	0x96, 0xf9, //0x01 -->manual:0xf9
	//0x96, 0xcf, //0x01 -->manual:0xf9
	0xbc, 0x68,
	//Resolution and Format
	0x12, 0x00,
	0x3b, 0x00,
	0x97, 0x80,
	0x17, 0x25,
	0x18, 0xA2,
	0x19, 0x01,
	0x1a, 0xCA,
	0x03, 0x01,
	0x04, 0xc8, //flip on, mirror on
	0x32, 0x07,
	0x98, 0x00,
	0x99, 0x28,
	0x9a, 0x00,
	0x57, 0x00,
	0x58, 0xB9,
	0x59, 0xA0,
	0x4c, 0x13,
	0x4b, 0x36,
	0x3d, 0x3c,
	0x3e, 0x03,
	0xbd, 0xA0,
	0xbe, 0xb4,

	//YAVG
	0x4e, 0x55,
	0x4f, 0x55,
	0x50, 0x55,
	0x51, 0x55,
	0x24, 0x50,
	0x25, 0x40,
	0x26, 0xa1,

	//Clock
	0x5c, 0x52,  //0x52-->manual:0x59
	0x5d, 0x00,
	0x11, 0x01,
	0x2a, 0x98,
	0x2b, 0x06,
	0x2d, 0x00,
	0x2e, 0x00,	
	//General
//	0x13, 0xa5,  //0xa5 -->manual: 0x85
//	0x14, 0x40,	
	0x13, 0xad,
	0x14, 0x48,
	//Banding
	0x4a, 0x00,
	0x49, 0xfa,
	0x22, 0x03,
	0x09, 0x00,
#if 0
	//close AE_AWB
	0x13, 0x80,
	0x16, 0x00,
	0x10, 0xf0,
	0x00, 0x3f,
	0x38, 0x00,
	0x01, 0x40,
	0x02, 0x40,
	0x05, 0x40,
	0x06, 0x00,
	0x07, 0x00,
#endif
  #if 1
	//0x13, 0x80,
	0x13, 0x80,
	0x16, 0x00,
	0x10, 0xf0,
	0x00, 0x3f,
   #endif
    0x38,0x00,
	0x01,0x5b,
	0x06,0xfc,
	0x02,0x52,
	0x05,0x40,
	0x07,0x00,
	0x2a,0xec,
	0x2b,0x07,
	//BLC
	0x41, 0x84,

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
