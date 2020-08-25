/**
 * @file camera_tw9912.h
 * @brief camera driver file
 * Copyright (C) 2017 SPF Dipol, Ltd.
 * @author Andrew Tamila 
 * @date 2017-03-30
 * @version 1.0
 * @ref
 */
#ifndef __CAMERA_TW9912_H__
#define __CAMERA_TW9912_H__


#if defined (USE_CAMERA_TW9912) || defined (CONFIG_SENSOR_TW9912)

#undef DELAY_FLAG
#undef END_FLAG
#define DELAY_FLAG        0xfd   // first parameter is 0xfe, then 2nd parameter is delay time count
#define END_FLAG          0xfe   // first parameter is 0xff, then parameter table is over 

static const T_U16 INIT_TAB[] = 
{


	 0xFF , 0x00  ,  //; Page 00  
   
     //0x02 , 0x40  ,
     0x03 , 0x26  ,  // data group,pclk output, sync tri-state
    
     0x05 , 0x1A ,  //0x1A- Progressive BT.656, HSY&VSY Deinterlacer, HSO no invert
     //0x06 , 0x00  ,  // C & V ADC power down
    
	 0x07, 0x26,
	 0x08, 0x12,	
	 0x09, 0x40,
	 0x0A, 0xFF,
	 0x0B, 0xD0,
     
     
     0x0C , 0xCC  ,
     0x0D , 0x15  ,
     
     0x33 , 0x05  ,
     0x34 , 0x9C  ,  //ID Detection Control
     0x35 , 0x00  ,
     0x36 , 0xE2  ,  //De-interlacer control
     
     0x37 , 0x28 ,  //De-interlacer delay control
     0x38 , 0xAF  ,  //De-interlacer sync control
    
     0xC0 , 0x01  ,  //ADC clock select oscillator clock
 
     0xE1 , 0x49  ,  //GPLL control
     0xE2 , 0xD9  ,
      
     0xE6 , 0x00  ,
     0xE7 , 0x2A  ,
     0xE8 , 0x0F  ,  // Y C V decoder mode
     0xE9 , 0x4F ,  // 4f clock control deinterlacing mode 0x43 default
   
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
static const T_U16 XGA_MODE_TAB[] = 
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

    END_FLAG, END_FLAG
};

//[720p--30fps]
static const T_U16 RECORD_720P_TAB[] = 
{

	
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

/****************   Camera anti-flicker mode   ****************/
static const T_U16 ANTI_FLICKER_DISABLE_TAB[] = 
{
	
    END_FLAG, END_FLAG
};

static const T_U16 ANTI_FLICKER_50HZ_TAB[] = 
{
	
    END_FLAG, END_FLAG
};

static const T_U16 ANTI_FLICKER_60HZ_TAB[] = 
{
	
    END_FLAG, END_FLAG
};

static const T_U16 ANTI_FLICKER_AUTO_TAB[] = 
{
	
    END_FLAG, END_FLAG
};

#endif
#endif
