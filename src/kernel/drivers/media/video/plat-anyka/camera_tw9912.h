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
     0x04 , 0x00  ,
     0x05 , 0x1A ,  // Progressive BT.656, HSY&VSY Deinterlacer, HSO no invert
     //0x06 , 0x00  ,  // C & V ADC power down
    
	 0x07, 0x22,	
	 0x08, 0x11,	
	 0x09, 0x40,
	 0x0A, 0x10,
	 0x0B, 0xD0,
     
     
     0x0C , 0xCC  ,
     0x0D , 0x15  ,
     /*
     0x10 , 0x00  ,
	 0x11 , 0x64  ,
     0x12 , 0x11 ,
     0x13 , 0x80 ,
     0x14 , 0x80  ,
     0x15 , 0x00  ,

     0x17 , 0x30  ,
     0x18 , 0x44  ,
     0x19 , 0x58  ,
    // 0x1A , 0x00  ,
   
     0x1C , 0x07  ,  
     0x1D , 0x7F  ,
     0x1E , 0x08  ,  
     0x1F , 0x00  ,
     0x20 , 0x50  ,
     0x21 , 0x42  ,
     0x22 , 0xF0  ,
     0x23 , 0xD8  ,
     0x24 , 0xBC  ,
     0x25 , 0xB8  ,
     0x26 , 0x44  ,
     0x27 , 0x38  ,
     0x28 , 0x00  ,
     0x29 , 0x00  ,
     0x2A , 0x78  ,
     0x2B , 0x44  ,
     0x2C , 0x30  ,
     0x2D , 0x14  ,
     0x2E , 0xA5  ,
     0x2F , 0xE0  ,
    */  
     0x33 , 0x05  ,
     0x34 , 0x9C  ,  //ID Detection Control
     0x35 , 0x00  ,
     0x36 , 0xE2  ,  //De-interlacer control
     
     0x37 , 0x2D ,  //De-interlacer delay control
     0x38 , 0x51  ,  //De-interlacer sync control
    
     0xC0 , 0x01  ,  //ADC clock select oscillator clock
 /*
     0xC2 , 0x01  ,
     0xC3 , 0x03  ,
     0xC4 , 0x5A  ,
     0xC5 , 0x00  ,
     0xC6 , 0x20  ,
     0xC7 , 0x04  ,
     0xC8 , 0x00  ,
     0xC9 , 0x06  ,
     0xCA , 0x06  ,
     0xCB , 0x30  ,
     0xCC , 0x00  ,
     0xCD , 0x54  ,
     0xD0 , 0x00  ,
     0xD1 , 0xF0  ,
     0xD2 , 0xF0  ,
     0xD3 , 0xF0  ,
     0xD4 , 0x00  ,
     0xD5 , 0x00  ,
     0xD6 , 0x10  ,
     0xD7 , 0x70  ,
     0xD8 , 0x00  ,
     0xD9 , 0x04  ,
     0xDA , 0x80  ,
     0xDB , 0x80  ,
     0xDC , 0x20  ,
     0xE0 , 0x00  ,
     * */
     0xE1 , 0x49  ,  //GPLL control
     0xE2 , 0xD9  ,
      
     0xE6 , 0x00  ,
     0xE7 , 0x2A  ,
     0xE8 , 0x0F  ,  // Y C V decoder mode
     0xE9 , 0x4F  ,  // clock control deinterlacing mode 0x61 default
    
	//DELAY_FLAG, 100,	//DELAY= 100




/*
0xFF, 0x00,//Page 0		
	//0x88, 0x02, //mark by paul's new sensor table
//0x01, 0x79,	
//	0x02, 0x48,//From YIN2 
	0x02, 0x40,//From YIN0 
	0x03, 0x24,//add	
0x04, 0x00,	
	0x05, 0x1E,
	0x06, 0x03,
	//D1 TRY
	0x07, 0x12,
	0x08, 0x14,	
	0x09, 0x20,
	0x0A, 0x26,
	0x0B, 0xFE,

//	0x07, 0x12,
//	0x08, 0x14,
//	0x09, 0x20,
//	0x0A, 0x0E,
//	0x0B, 0xD0,
	0x0C, 0xCC,
0x0D, 0x15,	
	0x11, 0x64,
0x12, 0x11,
0x13, 0x80,	
0x14, 0x80,	
0x15, 0x00,
0x17, 0x30,
0x18, 0x44,
0x1A, 0x10,
0x1B, 0x00,	
	0x1C, 0x1F,//0x1C, 0x0F,
0x1D, 0x7F,
	0x1E, 0x18,//0x1E, 0x08,
0x1F, 0x00,	
0x20, 0x50,
0x21, 0x42,
0x22, 0xF0,
0x23, 0xD8,
0x24, 0xBC,
0x25, 0xB8,
0x26, 0x44,
0x27, 0x38,
0x28, 0x00,
0x29, 0x00,
0x2A, 0x78,
0x2B, 0x44,
0x2C, 0x30,
0x2D, 0x14,
0x2E, 0xA5,
	0x2F, 0x26,
	0x30, 0x00,
0x31, 0x10,
0x32, 0x00,
0x33, 0x05,
0x34, 0x1A,
0x35, 0x00,	
	0x36, 0xe2,
	0x37, 0x2D,//VGA
//	0x37, 0x01,//D1
	0x38, 0x01,
0x40, 0x00,
0x41, 0x80,
0x42, 0x00,		
	0xC0, 0x01,
0xC1, 0x07,	
	0xC2, 0x01,
	0xC3, 0x03,
	0xC4, 0x5A,
	0xC5, 0x00,
	0xC6, 0x20,
0xC7, 0x04,
0xC8, 0x00,
0xC9, 0x06,
0xCA, 0x06,	
	0xCB, 0x30,
	0xCC, 0x00,
0xCD, 0x54,	
0xD0, 0x00,
0xD1, 0xF0,
0xD2, 0xF0,
0xD3, 0xF0,	
0xD4, 0x00,
0xD5, 0x00,
0xD6, 0x10,	
	0xD7, 0x70,
0xD8, 0x00,	
	0xD9, 0x04,
0xDA, 0x80,
0xDB, 0x80,	
0xDC, 0x20,
0xE0, 0x00,
	0xE1, 0x49,
	0xE2, 0xD9,
0xE3, 0x00,
0xE4, 0x00,
0xE5, 0x00,	
	0xE6, 0x00,
	0xE8, 0x0F,
	
	0xE9, 0x61,
	//{0xff, 0xff}
*/	
/*
	0xFF, 0x00,
    0x01, 0x78,
    0x02, 0x40,
    0x03, 0x20,
    0x04, 0x00,
    0x05, 0x12,
    0x06, 0x00,
    0x07, 0x22, //0x02
    0x08, 0x12,
    0x09, 0x40, //0xf0
    0x0A, 0x10, //0x14
    0x0B, 0xD0,
    0x0C, 0xCC,
    0x0D, 0x15,
    
    0x10, 0x00,
    0x11, 0x64,
    0x12, 0x11,
    0x13, 0x80,
    0x14, 0x80,
    0x15, 0x00,
    0x17, 0x30,
    0x18, 0x44,
    0x1A, 0x10,
    0x1B, 0x00,
    0x1C, 0x07,
    0x1D, 0x7F,
    0x1E, 0x08,
    0x1F, 0x00,
    
    0x20, 0x50,
    0x21, 0x42,
    0x22, 0xF0,
    0x23, 0xD8,
    0x24, 0xBC,
    0x25, 0xB8,
    0x26, 0x44,
    0x27, 0x38,
    0x28, 0x00,
    0x29, 0x00,
    0x2A, 0x78,
    0x2B, 0x44,
    0x2C, 0x30,
    0x2D, 0x14,
    0x2E, 0xA5,
    0x2F, 0x26,
    
    0x30, 0x00,
    0x31, 0x10,
    0x32, 0xFF,
    0x33, 0x05,
    0x34, 0x1A,
    0x35, 0x00,
    0x36, 0xE2,
    0x37, 0x2D,
    0x38, 0x01,
    
    0x40, 0x00,
    0x41, 0x80,
    0x42, 0x00,
    
    0xC0, 0x01,
    0xC1, 0x07,
    0xC2, 0x01,
    0xC3, 0x03,
    0xC4, 0x5A,
    0xC5, 0x00,
    0xC6, 0x20,
    0xC7, 0x04,
    0xC8, 0x00,
    0xC9, 0x06,
    0xCA, 0x06,
    0xCB, 0x30,
    0xCC, 0x00,
    0xCD, 0x54,
    
    0xD0, 0x00,
    0xD1, 0xF0,
    0xD2, 0xF0,
    0xD3, 0xF0,
    0xD4, 0x00,
    0xD5, 0x00,
    0xD6, 0x10,
    0xD7, 0x70,
    0xD8, 0x00,
    0xD9, 0x04,
    0xDA, 0x80,
    0xDB, 0x80,
    0xDC, 0x20,
    
    0xE0, 0x00,
    0xE1, 0x49,
    0xE2, 0xD9, //0x01
    0xE3, 0x00,
    0xE4, 0x00,
    0xE5, 0x00,
    
    0xE6, 0x00,
    0xE7, 0x2A,
    0xE8, 0x0F,
    0xE9, 0x61,
*/

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
static const T_U16 XGA_MODE_TAB[] = //1024x768
{
	 0xFF , 0x00,
 
	 0x07, 0x33,	
	 0x08, 0x12,	
	 0x09, 0x00,
	 0x0A, 0x10,
	 0x0B, 0xFF, 
	 
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
