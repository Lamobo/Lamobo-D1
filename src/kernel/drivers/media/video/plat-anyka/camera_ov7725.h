/**
 * @file camera_ov7725.h
 * @brief camera driver file
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-04-22
 * @version 1.0
 * @ref
 */
#ifndef __CAMERA_OV7725_H__
#define __CAMERA_OV7725_H__


#if defined (USE_CAMERA_OV7725) || defined (CONFIG_SENSOR_OV7725)

#undef DELAY_FLAG
#undef END_FLAG
#define DELAY_FLAG        0xFE   // first parameter is 0xfe, then 2nd parameter is delay time count
#define END_FLAG          0xFF   // first parameter is 0xff, then parameter table is over 


static const T_U8 INIT_TAB[] = 
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
    //MCLK 24MHz, 30fps/60fps; MCLK 32MHz, 40fps
    0x12, 0x80,
    0x3d, 0x03,

    0x17, 0x22,
    0x18, 0xa4, 
    0x32, 0x00,
    0x19, 0x07,
    0x1a, 0xf0,
    0x03, 0x00,

    0x29, 0xa0,
    0x2c, 0xf0,
    0x2a, 0x00,
    0x11, 0x01,//01, 30fps/40fps; //00, 60fps 
    0x33, 0x00,//30fps; 0x66, 25fps, dummy row
    0x42, 0x7f,
    0x4d, 0x09,
    0x63, 0xe0,
    0x64, 0xff,
    0x65, 0x20,
    0x66, 0x00,
    0x67, 0x48,
    0x13, 0xff,
    0x0d, 0x41,
    0x0f, 0xc5,
    0x14, 0x11,//21, //4x

    0x22, 0x97,//97, 30fps/60fps; //ca, 40fps
    0x23, 0x02,//02, 30fps; //01, 40fps/60fps
    0x24, 0x50,//70,//40,
    0x25, 0x40,//60,//30,
    0x26, 0xb2,//c3,//a1,

    0x2b, 0x00,
    0x6b, 0xaa,
    0x90, 0x05,
    0x91, 0x01,
    0x92, 0x03,
    0x93, 0x00,
    0x94, 0xb0,
    0x95, 0x9d,
    0x96, 0x13,
    0x97, 0x16,
    0x98, 0x7b,
    0x99, 0x91,
    0x9a, 0x1e,
    0x9b, 0x08,
    0x9c, 0x25,//20,
    0x9e, 0x81,
    0xa6, 0x04,
    0x7e, 0x0c,
    0x7f, 0x16,
    0x80, 0x2a,
    0x81, 0x4e,
    0x82, 0x61,
    0x83, 0x6f,
    0x84, 0x7b,
    0x85, 0x86,
    0x86, 0x8e,
    0x87, 0x97,
    0x88, 0xa4,
    0x89, 0xaf,
    0x8a, 0xc5,
    0x8b, 0xd7,
    0x8c, 0xe8,
    0x8d, 0x20,
    0x0c, 0x00,
    0x6b, 0x90,
    0xa7, 0x70,
    0xa8, 0x70,

    //LC
    0x47, 0x90,
    0x48, 0x14,
    0x4a, 0x00,
    0x49, 0x08, //07, //g
    0x4b, 0x09, //08, //b
    0x4c, 0x0b, //0d, //r
    0x46, 0x05,
    
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
    0x9c,0x08,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_2_TAB[] = 
{
    0x9c,0x10,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_3_TAB[] = 
{
    0x9c,0x15,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_4_TAB[] = 
{
    0x9c,0x20,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_5_TAB[] = 
{
    0x9c,0x24,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_6_TAB[] = 
{
    0x9c,0x28,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_7_TAB[] = 
{
    0x9c,0x40,
    END_FLAG, END_FLAG
};

/****************   Camera Saturation Table   ****************/
static const T_U8 SATURATION_1_TAB[] = 
{
    0xa6,0x06,
    0xa7,0x10,
    0xa8,0x30,
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_2_TAB[] = 
{
    0xa6,0x06,
    0xa7,0x40,
    0xa8,0x40,
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_3_TAB[] = 
{
    0xa6,0x04,
    0xa7,0x70,
    0xa8,0x70,
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_4_TAB[] = 
{
    0xa6,0x06,
    0xa7,0x80,
    0xa8,0x80,
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_5_TAB[] = 
{
    0xa6,0x06,
    0xa7,0x98,
    0xa8,0x98,
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

static const T_U8 AWB_NIGHT_TAB[] = 
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

static const T_U8 EFFECT_BWN_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera night/day mode   ****************/
static const T_U8 DAY_MODE_TAB[] = 
{
       0x0e,0x79,
    END_FLAG, END_FLAG
};

static const T_U8 NIGHT_MODE_TAB[] = 
{
       0x0e,0xF9,
    END_FLAG, END_FLAG
};

#endif
#endif
