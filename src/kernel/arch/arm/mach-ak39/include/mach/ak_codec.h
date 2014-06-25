#ifndef AK39_CODEC_H
#define AK39_CODEC_H

#include <mach/gpio.h>
#include <linux/delay.h>
#include <sound/asound.h>


//pAddress0800   0x08000000-0x080000FF
#define CPUPLLCHN_CLOCK_CTRL_REG	0x0004

#define CLOCK_CTRL_REG				0x000C
#define HIGHSPEED_CLOCK_CTRL_REG	0x0010
#define CLOCK_GATE_CTRL_REG			0x001c
#define SOFT_RST_CTRL_REG			0x0020
#define MULTIPLE_FUN_CTRL_REG1		0x0058
#define FADEOUT_CTRL_REG			0x0070

#define ADC1_CONF_REG1				0x0098

#define ANALOG_CTRL_REG1			0x009c
#define ANALOG_CTRL_REG2			0x00A0
#define ANALOG_CTRL_REG3			0x00A4

//pAddress2011 0x2011000-0x20118008
#define DAC_CONFIG_REG              0x0000
#define I2S_CONFIG_REG              0x0004
#define CPU_DATA_REG                0x0008

#define ADC2_CONFIG_REG             0x8000
#define ADC2_DATA_REG				0x8004

//ADC2_CONFIG_REG(0x20118000)
#define WORD_LENGTH_MASK (0XF << 8)
#define I2S_EN           (1 << 4)
#define CH_POLARITY_SEL  (1 << 3)
#define HOST_RD_INT_EN   (1 << 2)
#define ADC2MODE_L2_EN   (1 << 1)
#define ADC2_CTRL_EN     (1 <<0)

//CLOCK_CTRL_REG(0x08000004)
#define ASIC_FREQ_ADJ      		(1 << 28)

//CLOCK_CTRL_REG(0x0800000C)
#define DAC_DIV_VLD				(1 << 29)
#define DAC_CLK_EN      		(1 << 28)
#define MASK_DAC_DIV_FRAC		(0xFFF << 12)
#define DAC_DIV_FRAC(val)		(((val)&0xFFF) << 12)
#define MASK_DAC_DIV_INT		(0xFF << 4)
#define DAC_DIV_INT(val)		(((val)&0xFF) << 4)

//HIGHSPEED_CLOCK_CTRL_REG(0x0800 0010)
#define ADC2_HSDIV_VLD		(1 << 29)
#define ADC2_HCLK_EN		(1 << 28)
#define MASK_ADC2_HCLK_DIV	(0x3F << 20)
#define ADC2_HCLK_DIV(val)	(((val)&0x3F)<<20)
#define DAC_HSDIV_VLD		(1 << 19)
#define DAC_HCLK_EN			(1 << 18)
#define MASK_DAC_HCLK_DIV	(0xFF << 10)
#define DAC_HCLK_DIV(val)	(((val)&0xFF)<<10)
#define ADC2_DIV_VLD	(1 << 9)
#define ADC2_CLK_EN		(1 << 8)
#define MASK_ADC2_DIV	(0x3F << 0)
#define ADC2_DIV(val)	((val)&0x3F)


//SOFT_RST_CTRL_REG(0x08000020)
#define DAC_SOFT_RST	((1 << 26)|(1 << 28))
#define ADC2_SOFT_RST	((1 << 27)|(1 << 29))


//MULTIPLE_FUN_CTRL_REG1(0x08000058)
#define IN_DAAD_EN      (1 << 25) //ENABLE INTERNAL DAC ADC via i2s

//FADEOUT_CTRL_REG(0x0080 0070)
#define OSR_MASK       (0x7 << 0)
#define OSR(value)     (((value)&0x7) << 0)
#define ADC2_OSR_BIT		31
#define DAC_FILTER_EN	(1<<3)

//ADC_CONF_REG1(0x0080 0098)
#define SEL_VREF 		(1<< 23)

//ANALOG_CTRL_REG1(0x0080 009c)
#define PD2_HP         	(1 << 24)
#define VCM3_SEL		(1 << 23)
#define MASK_HP_GAIN	(0x1F << 18)
#define HP_GAIN(val)	(((val)&0x1F)<<18)
#define PRE_EN1        (1 << 17)
#define PRE_EN2        (1 << 16)
#define PD1_HP         (1 << 15)
#define RST_DAC        (1 << 11)
#define PD_OP          (1 << 10)
#define PD_CK          (1 << 9)
#define PD_VCM3        (1 << 3)
#define PL_VCM2        (1 << 2)  //pull down to ground.
#define PD_VCM2        (1 << 1)  // power off 
#define PD_REF			(1 << 0)

//ANALOG_CTRL_REG2(0x0080 00A0)
#define VDD_MIC_SEL		(1 << 25)
#define PD_S2D			(1 << 22)
#define ADC_LIM        (1 << 21)
#define PD_MICP        (1 << 23)
#define PD_MICN        (1 << 24)
#define PD_ADC2          (1 << 1)
#define PL_VCM3        (1 << 0)

//ANALOG_CTRL_REG3(0x0080 00A4)

//DAC_CONFIG_REG(0x2011000)
#define ARM_INT        (1 << 3)  //ARM interrupt enable
#define MUTE           (1 << 2)  // repeat to sent the Last data to DAC
#define FORMAT         (1 << 4)    //  1 is used memeory saving format.
#define L2_EN          (1 << 1)
#define DAC_CTRL_EN    (1 << 0)

//I2S_CONFIG_REG(0x20110004)
#define LR_CLK          (1 << 6)
#define POLARITY_SEL    (1 << 5)  
#define I2S_CONFIG_WORDLENGTH_MASK              (0x1F << 0)

/////////////HP_IN  ADC23_IN
#define SOURCE_DAC           (0b001)
#define SOURCE_LINEIN        (0b010)
#define SOURCE_MIC           (0b100)
#define SIGNAL_SRC_MUTE      0
#define SIGNAL_SRC_MAX       (SOURCE_DAC|SOURCE_LINEIN|SOURCE_MIC)

#define SOURCE_DAC_MASK           (0b001)
#define SOURCE_LINEIN_MASK        (0b010)
#define SOURCE_MIC_MASK           (0b100)
#define SOURCE_MIXED_ALL_MASK 	   (SOURCE_DAC_MASK|SOURCE_LINEIN_MASK|SOURCE_MIC_MASK)


#define HEADPHONE_GAIN_MIN    0
#define HEADPHONE_GAIN_MAX    5
#define LINEIN_GAIN_MIN       0
#define LINEIN_GAIN_MAX       15
#define MIC_GAIN_MIN          0
#define MIC_GAIN_MAX          7


#define PLAYMODE_AUTO_SWITCH 	(0) /*hp & sp switch*/
#define PLAYMODE_HP 		  		(1)
#define PLAYMODE_SPEAKER 		(2)

/**
 * platform data of the ak39 pcm driver
 */
struct ak39_codec_platform_data
{
	struct gpio_info hpdet_gpio; /* gpio for headphone detecting */
	struct gpio_info spk_down_gpio; /* gpio for shutdown speaker */
	struct gpio_info hpmute_gpio;	/* gpio for headphone de-pipa */
	int hp_on_value;		/* the gpio value when headphone is pulg */
	int hpdet_irq;			/* the irq of heaphone detecting */
	int bIsHPmuteUsed;		/* does this board has headphone de-pipa hardware or not */
	int hp_mute_enable_value;	/* the gpio value when enable headphone de-pipa hardware */
	int bIsMetalfixed;		/* the ak37 SoC is metal fixed version or not  */
	int boutput_only;	   /* use only 0: hp & sp, 1: hp only, 2:sp only*/
};

#endif
