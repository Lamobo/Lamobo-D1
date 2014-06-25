#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/hrtimer.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/completion.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <asm/bitops.h>
#include <sound/control.h>
#include <sound/info.h>
#include <linux/cpufreq.h>
#include <linux/interrupt.h>
#include <sound/ak_pcm.h>


#include <plat/l2.h>
#include <mach/ak_codec.h>
#include <mach/gpio.h>
#include <mach/clock.h>
#include <linux/vmalloc.h>


//#define SND_CODEC_DBG

#ifdef SND_CODEC_DBG
#ifdef __KERNEL__
#define PDEBUG(fmt, args...) 	printk(KERN_INFO "ak39 codec:" fmt, ##args)
#else
#define PDEBUG(fmt, args...) 	fprintf(stderr, "%s %d:" fmt,__FILE__, __LINE__, ## args)
#endif
#else
#define PDEBUG(fmt, args...) 
#endif


enum mixer_vol_port {
	MIXER_PORT_HP_VOL = 0,
	MIXER_PORT_LINEIN_VOL,
	MIXER_PORT_MIC_VOL,
	MIXER_PORT_VOL_COUNT,
};


enum mixer_dst_port {
	MIXER_ADDR_DST_HP = 0,
	MIXER_ADDR_DST_ADC2,
	MIXER_DST_COUNT,
};

enum mixer_detect_port {
	MIXER_ADDR_HPDET = 0,
	MIXER_ADDR_MICDET,
	MIXER_ADDR_LINEINDET,
	MIXER_SWITCH_COUNT,
};


enum mixer_outmode {
	MIXER_ADDR_OUTMODE_DAC = 0, 
	MIXER_OUTMODE_COUNT,
};

enum mixer_chnl_duration {
	MIXER_ADDR_PLAY_DURATION = 0,
	MIXER_DURATION_COUNT,
};

#define MAX_DACCLK 24000000

#define HEADPHONE_DEFAULT_VOL           (HEADPHONE_GAIN_MAX - 2)
#define LINEIN_DEFAULT_VOL               10
#define MIC_DEFAULT_VOL                  6

#define PLAYMODE_STATUS_HP 			(1)
#define PLAYMODE_STATUS_SPEAKER 	(2)

#define OUTMODE_AUTO 			(0)
#define OUTMODE_HP 				(1)
#define OUTMODE_LINEOUT 		(2)
#define OUTMODE_MIN             OUTMODE_AUTO
#define OUTMODE_MAX             OUTMODE_HP

#define CHNLDURATION_CONSTANT        0
#define CHNLDURATION_EVEROPEN        1
#define CHNLDURATION_MIN             CHNLDURATION_CONSTANT
#define CHNLDURATION_MAX             CHNLDURATION_EVEROPEN

struct ak39_codec {
	struct ak_codec_dai dai;
	void __iomem  *analog_ctrl_base;
	void __iomem  *adda_cfg_base;
	struct delayed_work d_work;

	int mixer_volume[MIXER_PORT_VOL_COUNT];
	int mixer_route[MIXER_DST_COUNT];
	int mixer_switch[MIXER_SWITCH_COUNT];
	int mixer_outmode[MIXER_OUTMODE_COUNT];
	int mixer_ch_duration[MIXER_DURATION_COUNT];

	struct gpio_info hpdet_gpio;
	struct gpio_info spkrshdn_gpio;
	struct gpio_info hpmute_gpio;

	int hp_det_irq;
	int irq_hp_on_type;
	int hp_on_value;
	int playmode;
	int hpmute_en_val;

	unsigned 	used_hp_mute:1; // whether to use hardware de-pipa or not
	unsigned 	outputing:1;
	unsigned 	dac_state:1;
	unsigned 	adc2_state:1;
};

struct ak39_volume_info
{
	int vol_default;
	int vol_min;
	int vol_max;
};


#define VOL_INFO_DEFAULT 	0
#define VOL_INFO_MIN		1
#define VOL_INFO_MAX 		2

static struct ak39_volume_info vol_info[MIXER_PORT_VOL_COUNT] = {
	[MIXER_PORT_HP_VOL] = {
		.vol_default = HEADPHONE_DEFAULT_VOL,
		.vol_min = HEADPHONE_GAIN_MIN,
		.vol_max = HEADPHONE_GAIN_MAX,
	},
	[MIXER_PORT_LINEIN_VOL] = {
		.vol_default = LINEIN_DEFAULT_VOL,
		.vol_min = LINEIN_GAIN_MIN,
		.vol_max = LINEIN_GAIN_MAX,
	},
	[MIXER_PORT_MIC_VOL] = {
		.vol_default = MIC_DEFAULT_VOL,
		.vol_min = MIC_GAIN_MIN,
		.vol_max = MIC_GAIN_MAX,
	}

};

static int default_route[MIXER_DST_COUNT] = {
	[MIXER_ADDR_DST_HP] = SOURCE_DAC,
   	[MIXER_ADDR_DST_ADC2] = SOURCE_MIC,	
};

extern unsigned long playback_statu;

#if 0
static void dump_codec_reg(struct ak39_codec *codec) 
{
	u32 reg_val;

	reg_val = REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG);
	printk("\nCLOCK_CTRL_REG(0x0800%04x):%08x.\n", CLOCK_CTRL_REG, reg_val);

	reg_val = REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG);
	printk("HIGHSPEED_CLOCK_CTRL_REG(0x0800%04x):%08x.\n", HIGHSPEED_CLOCK_CTRL_REG, reg_val);

	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	printk("\nANALOG_CTRL_REG1(0x0800%04x):%08x.\n", ANALOG_CTRL_REG1, reg_val);

	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
	printk("ANALOG_CTRL_REG2(0x0800%04x):%08x.\n", ANALOG_CTRL_REG2, reg_val);
	
	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG3);
	printk("ANALOG_CTRL_REG3(0x0800%04x):%08x.\n", ANALOG_CTRL_REG3, reg_val);

	reg_val = REG32(codec->adda_cfg_base + ADC2_CONFIG_REG);
	printk("ADC2_CONFIG_REG(0x2011%04x):%08x.\n", ADC2_CONFIG_REG, reg_val);

	reg_val = REG32(codec->adda_cfg_base + ADC2_DATA_REG);
	printk("ADC2_DATA_REG(0x2011%04x):%08x.\n", ADC2_DATA_REG, reg_val);

	reg_val = REG32(codec->adda_cfg_base + DAC_CONFIG_REG);
	printk("DAC_CONFIG_REG(0x2011%04x):%08x.\n", DAC_CONFIG_REG, reg_val);

	reg_val = REG32(codec->analog_ctrl_base + ADC1_CONF_REG1);
	printk("ADC1_CONF_REG1:(0x0800%04x):%08x.\n", ADC1_CONF_REG1, reg_val);
}
#endif
static inline struct ak39_codec *to_ak39_codec(struct ak_codec_dai *dai)
{
	return dai ? container_of(dai, struct ak39_codec, dai): NULL;
}


static inline void pd_ref_enable(struct ak39_codec *codec) 
{
	u32 reg_val;

	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	reg_val &= ~PD_REF;     //power on codec
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
}

static inline void pd_ref_disable(struct ak39_codec *codec) 
{
	u32 reg_val;

	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	reg_val |= PD_REF;     //power off codec
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
}

#if 0
static void SetHpDisChgCur(unsigned long value)
{
	int reg_val;
	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	reg_val &= ~(0x7 << 29);
	reg_val |= (value << 29);
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
}


static void set_cur_pmos(struct ak39_codec *codec, unsigned long value)
{
	int reg_val;
	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	reg_val &= ~(0xf << 25);
	reg_val |= (value << 25);
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
}
#endif

static void set_cur_vcm2_dischg(struct ak39_codec *codec, unsigned long value)
{
	int reg_val;
	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	reg_val &= ~(0x1f << 4);
	reg_val |= (value << 4);
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
}

static void set_cur_vcm2(struct ak39_codec *codec, unsigned long value)
{
	int reg_val;
	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
	reg_val &= ~(0x1F << 16);
	reg_val |= (value << 16);
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;
}


/**
 * @brief  select HP in signal
 * @author 
 * @date   
 * @param[in]  (HP_In_Signal)signal: signal desired
 * @return  void
 */
void ak39_set_hp_in(struct ak39_codec *codec, unsigned long signal)
{
	unsigned long reg_val;
	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
    reg_val &= ~(0x7 << 12);
    reg_val |= ((signal&0x7) << 12);
    REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
}


/**
 * @brief      set ADC2 source
 * @author 
 * @date   
 * @param[in]    signal:  DAC|LINEIN|MIC
 * @return  void
 */
void ak39_set_adc2_in(struct ak39_codec *codec, unsigned long signal)
{
	unsigned long reg_val;
	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
	reg_val &= ~(0x7 << 2);
	reg_val |= ((signal&0x7) << 2);
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;
}


/**
 * @brief  set HP gain
 * @author 
 * @date   
 * @param[in]  x: (0.x)times
 * @return  void
 */
void ak39_set_hp_gain(struct ak39_codec *codec, unsigned long gain)
{
    unsigned long reg_value;
    unsigned long gain_table[6] = {0x0, 0x18, 0x14, 0x12, 0x11, 0x10};

    reg_value = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
    reg_value &= ~MASK_HP_GAIN;
    reg_value |= HP_GAIN(gain_table[gain]);
    REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_value;
}

/**
 * @brief      set mic gain
 * @author 
 * @date   
 * @param[in]    gain: 0~7
 * @return  void
 */
void ak39_set_mic_gain(struct ak39_codec *codec, unsigned long gain)
{
	unsigned long reg_val = 0;

	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
	reg_val &= ~(0x7<<10);
	reg_val |= ((gain&0x7)<<10);
	reg_val |= (1<<15); //double Mic gain
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;
}

/**
 * @brief      set linein gain
 * @author 
 * @date   
 * @param[in]    gain: 0~15
 * @return  void
 */
void ak39_set_linein_gain(struct ak39_codec *codec, unsigned long gain)
{
	unsigned long reg_val = 0;

	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
	reg_val &= ~(0xF<<6);
	reg_val |= ((gain&0xF)<<6);
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;
}

enum depipa_noise_ctrl {
	DEPIPA_NOISE_NOT_USE,
	DEPIPA_NOISE_USE_3KOHM_RESISTOR,
	DEPIPA_NOISE_USE_1KOHM_RESISTOR,
	DEPIPA_NOISE_USE_MAX_RESISTOR,
};

static void set_bit_depipa_noise_ctrl(struct ak39_codec *codec, int mode)
{
	unsigned long reg_val = 0;

	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);

	if(mode == DEPIPA_NOISE_USE_MAX_RESISTOR) {
		reg_val |= (PRE_EN1);
		reg_val |= (PRE_EN2);
	} else if(mode == DEPIPA_NOISE_USE_3KOHM_RESISTOR) {
		reg_val |= (PRE_EN1);
		reg_val &= ~(PRE_EN2);
	} else if(mode == DEPIPA_NOISE_USE_1KOHM_RESISTOR) {
		reg_val &= ~(PRE_EN1);
		reg_val |= (PRE_EN2);
	} else {
		reg_val &= ~(PRE_EN1);
		reg_val &= ~(PRE_EN2);
	}

	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
}


static void ak39_set_vcm_ref_power(struct ak39_codec *codec, bool bOn)
{
	unsigned long reg_val;
	if(bOn)
	{
		//add power control here for MIC-to-Lineout channel
		//power on REF
		reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
		reg_val &= ~(PD_REF | PL_VCM2 );
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;

		//power on vcm2/vcm3
	    reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	    reg_val &= ~(PD_VCM2 | PD_VCM3);
	    REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
	}
	else
	{
		//power off vcm2/vcm3
	    reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	    reg_val |= (PD_VCM2);
	    reg_val |= (PD_VCM3);           
	    REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;

	    //power off codec
	    reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	    reg_val |= (PD_REF | PL_VCM2);
	    REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
	}
}


/**
 * @brief   power on HP
 * @author 
 * @date   
 * @param[in] bool poweron or poweroff
 * @         [in] bool use delay to de-pipa or not
 * @return  void
 */
void ak39_set_hp_power(struct ak39_codec *codec, bool bOn, bool soft_de_pipa)
{
	int reg_value;

	printk("ak39_set_hp_power %d,soft_de_pipa=%d\n",bOn,soft_de_pipa);
	if(bOn)
	{
//		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) |= (PL_VCM2);
		set_bit_depipa_noise_ctrl(codec, DEPIPA_NOISE_USE_MAX_RESISTOR);
					
		mdelay(10);

		pd_ref_enable(codec);

		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) |= (0x7UL << 29);

		mdelay(20);
		reg_value = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
		reg_value &= ~(PD1_HP);
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_value;
		
		mdelay(10);
		
		reg_value = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
		reg_value &= ~(PD2_HP);
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_value;

		ak39_set_vcm_ref_power(codec, 1);
		
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) &= ~(0x1f << 4);
		set_cur_vcm2(codec, 0x1e);
		mdelay(100);
		
		set_bit_depipa_noise_ctrl(codec, DEPIPA_NOISE_NOT_USE);
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) &= ~(RST_DAC);
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) &= ~(0x7UL << 29);
	}
	else
	{
		set_bit_depipa_noise_ctrl(codec, DEPIPA_NOISE_USE_3KOHM_RESISTOR);

		if (!codec->adc2_state) {
			//off mute
			REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) &= ~(0x7 << 12);
		//	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) |= (0x1f << 4);

			ak39_set_vcm_ref_power(codec, 0);
			set_cur_vcm2_dischg(codec, 0x1f);
			mdelay(50);
		}

		//power off hp
		reg_value = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
		reg_value |= ((PD2_HP));
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_value;

		//power off hp
		reg_value = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
		reg_value |= (PD1_HP);
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_value;
	}
}


/**
 * @brief      set linein interface power
 * @author 
 * @date   
 * @param[in]    bTrue: 1-power on; 0-power off 
 * @return  void
 */
static void ak39_set_linein_power(struct ak39_codec *codec, bool bOn)
{
	unsigned long reg_val = 0;
	if(bOn)
	{
		reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
	    reg_val &= ~(1 << 14); //power on the channel
	    REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;
	}
	else
	{
		reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
		reg_val |= (1 << 14); //power off the channel
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;
	}
}

/**
 * @brief      set DAC to lineout
 * @author 
 * @date   
 * @param[in]    bTrue: 1-connet DACto lineout; 0-disconnect 
 * @return  void
 */
static void ak39_set_mic_power(struct ak39_codec *codec, bool bOn)
{
 	unsigned long reg_val = 0;
	if(bOn)
	{
		reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
		reg_val |= VDD_MIC_SEL;
		REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;

		//power on mic interface
		reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
        reg_val &= ~(0x1 << 23);  //power on differential mic
        REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;
	}
	else
	{
		reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2);
        reg_val |= (0x1 << 23);  //power off differential mic
        REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) = reg_val;
	}
}

static void ak39_set_sel_vref(struct ak39_codec *codec, bool bOn) 
{
	unsigned long reg_val;
	reg_val = REG32(codec->analog_ctrl_base + ADC1_CONF_REG1);
	if(bOn)
		reg_val |= SEL_VREF;
	else	
		reg_val &= ~SEL_VREF;

	REG32(codec->analog_ctrl_base + ADC1_CONF_REG1) = reg_val;
}


static void set_dac_highspeed(struct ak39_codec *codec,
	   	unsigned long sample_rate)
{
	static unsigned long rate_list[] =
   				{8000, 16000, 24000, 48000, 96000};
	static unsigned long dac_hclk[sizeof(rate_list)/sizeof(rate_list[0])] = 
	{6, 12, 18, 36, 72};

	unsigned long        reg_value;
	unsigned long        pllclk;
	unsigned long        hclk_div;
	unsigned long        hclk;
	unsigned long        i;

	pllclk = ak_get_asic_pll_clk();
	pllclk /= 1000000;

	printk("seths sr %lu,pll %lu\n",sample_rate,pllclk);
	for(i=0; i < sizeof(rate_list)/sizeof(rate_list[0]); ++i)
	{
		if(sample_rate <= rate_list[i])
		{
			break;
		}
	} 

	if(sizeof(rate_list)/sizeof(rate_list[0]) == i)
	{
		hclk = dac_hclk[i-1];
	}
	else
	{
		hclk = dac_hclk[i];
	}

	hclk_div = pllclk / hclk;  
	//04V 3771 chip: when the divider of the DAC CLK is even number,
	//the DAC MODULE is more steady
	hclk_div &= ~1;

	if(0 != hclk_div)
	{
		hclk_div = hclk_div - 1;
	}   

	reg_value = REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG);
	reg_value &= ~(MASK_DAC_HCLK_DIV);
	reg_value |= DAC_HSDIV_VLD | DAC_HCLK_DIV(hclk_div);
	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) = reg_value;

	i = 0;
	while(REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) & (DAC_HSDIV_VLD))
	{
		++i;
		if(i > 100000)
		{
			printk("set high speed clk reg fail\n");
			return ;
		}
	}    
}

static void set_adc_highspeed(struct ak39_codec *codec,
	   	unsigned long sample_rate)
{
	unsigned long        reg_value;
	unsigned long        pllclk;
	unsigned long        hclk_div;
	unsigned long        hclk;
	unsigned long        i;

	pllclk = ak_get_asic_pll_clk();
	for (i=1; i<=0x40; i++)
	{
		hclk = pllclk/i;
		if ((hclk<=62000000)&&(hclk>=28000000))
		{
			break;
		}
	}
	hclk_div = i-1;

	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) &= ~(1<<29);

	i = 0;
	while(REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) & (ADC2_HSDIV_VLD))
	{
		++i;
		if(i > 100000)
		{
			printk("adc set high speed clk reg fail\n");
			return ;
		}
	}  	

	reg_value = REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG);
	reg_value &= ~(MASK_ADC2_HCLK_DIV);
	reg_value |= ADC2_HSDIV_VLD | ADC2_HCLK_DIV(hclk_div);
	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) = reg_value;

	i = 0;
	while(REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) & (ADC2_HSDIV_VLD))
	{
		++i;
		if(i > 100000)
		{
			printk("adc set high speed clk reg fail\n");
			return ;
		}
	}   

	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) |= (1<<29);
}

/**
 * @brief      set ADC2 mode and clk_div
 * @author 
 * @date   
 * @param[in]    des_sr: desired rate to be set
 * @param[out]  mode_sel: reg(0x08000064) bit[12]
 * @param[out]  mclkdiv: reg(0x08000008) bit[11:4]
 * @return  void
 */
static unsigned long get_adc2_osr_div(struct ak39_codec *codec, unsigned char *mode_sel,
	   	unsigned char *mclkdiv, unsigned long des_sr)
{
	unsigned short k, max_div;
    unsigned short OSR_value=256;
    unsigned long SR_save, out_sr=0;
    signed long a, b;
	unsigned long clk168m;
	unsigned long perfect_pll;
	
	clk168m = ak_get_asic_pll_clk();
    max_div = 0x40;
    SR_save = 0;
    *mode_sel = 0;
    *mclkdiv = 0;

    if (des_sr > 24000)
    {
        OSR_value = 256;
        *mode_sel = 1; //48k mode
    }
    else
    {
        OSR_value = 512;
        *mode_sel = 0; //16k mode
    }
    
    for(k=0; k<max_div; k++) //DIV
    {
        out_sr = clk168m/(k+1)/OSR_value;
        a = out_sr - des_sr;
        a = (a>0)? a : (-a);
        b = SR_save - des_sr;
        b = (b>0)? b : (-b);
        if (a<b)
        {
            SR_save = out_sr;
            *mclkdiv = k;

			perfect_pll = OSR_value * (k+1) *des_sr;
        }
    }

	printk("adc: perfect asic pll clk is:%lu, actual sample rate is %lu.\n", perfect_pll, SR_save);

	printk( "---ADC clk168m = %lu, SR_save = %lu. des_sr = %lu, div = %d\n",clk168m, SR_save, des_sr, *mclkdiv);
	return SR_save;
}

/**
 * @brief   Get OSR and DACDIV refer to appointed PLL and sample rate
 * @author  
 * @date   
 * @input   des_sr: destination sample rate
 * @output  osrindex: OSR index
 * @output  mclkdiv: mclk div
 * @return  void
 */
static void get_dac_osv_div(struct ak39_codec *codec, unsigned char *osrindex,
	   	unsigned char *mclkdiv, unsigned long des_sr)
{
    const unsigned short OSR_table[8] = 
        {256, 272, 264, 248, 240, 136, 128, 120};

    unsigned long j;
    unsigned long max_div;
    long k;
    unsigned long SR_save, out_sr=0;
    long a;
    long b;
    unsigned long clk168m;

	unsigned long perfect_pll;

	clk168m = ak_get_asic_pll_clk(); 

    max_div = 0x100;
    SR_save = 0;
    *osrindex = 0;
    *mclkdiv = 0xff;
    for(j=0; j<8; j++) //OSR index
    {
        for(k=max_div-1; k>=0; k--) //DAC_DIV value
        {
            out_sr = clk168m/(k+1);
            if (out_sr > MAX_DACCLK)
                break;
            out_sr = out_sr/OSR_table[j];
            a = out_sr-des_sr;
            a = (a>0)? a : (-a);
            b = SR_save-des_sr;
            b = (b>0)? b : (-b);
            if (a<b)
            {
                SR_save = out_sr;
                *mclkdiv = k;
                *osrindex = j;
				perfect_pll = OSR_table[j]*des_sr *(k + 1);
			}
		}
	}
	//WARN(1, "111");
	printk("perfect asic pll clk is:%lu, actual sample rate is %lu.\n", perfect_pll, SR_save);

}


/**
 *repair the volume range.
 *
 * */
static inline int volume_repair_range(int port, int vol)
{
	if(vol < vol_info[port].vol_min)
		vol = vol_info[port].vol_min;
	else if(vol > vol_info[port].vol_max)
		vol = vol_info[port].vol_max;

	return vol;
}

static inline int get_volume_info(int port, int type) 
{
	int vol;

	switch(type){
		case VOL_INFO_DEFAULT:
			vol = vol_info[port].vol_default;
			break;
		case VOL_INFO_MIN:
			vol = vol_info[port].vol_min;
			break;
		case VOL_INFO_MAX:
			vol = vol_info[port].vol_max;
			break;
		default:
			vol = 0;
			break;
	}
	return vol;
}


/*
 *set mixer volume by port,
 *return old volume;
 *
 * */
static inline int set_mixer_volume(struct ak39_codec *codec,
		int port, int volume)
{
	int old_vol;

	old_vol = codec->mixer_volume[port];
	codec->mixer_volume[port] = volume;

	return old_vol;
}

static inline int get_mixer_volume(struct ak39_codec *codec,
		int port)
{
	return codec->mixer_volume[port];
}

/**
 * @brief   set hp/linein/mic gain
 * @author lixinhai 
 * @date  2013-08-02 
 * @input  codec: handle. 
 * @input  port: control port:hp, linein or mic.
 * @input  gain: gain value. 
 * @return  void
 */
static void ak39_set_gain(struct ak39_codec *codec, int port, int gain)
{
	switch(port) {
		case MIXER_PORT_HP_VOL:
			ak39_set_hp_gain(codec, gain);
			break;
		case MIXER_PORT_LINEIN_VOL:
			ak39_set_linein_gain(codec, gain);
			break;
		case MIXER_PORT_MIC_VOL:
			ak39_set_mic_gain(codec, gain);
			break;
	}
}

/**
 * @brief   set playback mode: headphone or speaker
 * @author lixinhai 
 * @date   2013-08-02 
 * @input  codec: handle. 
 * @input  plug_in: headphone is plug in.
 * @return  void
 */
static inline void set_playmode_switch(struct ak39_codec *codec,
		int plug_in)
{
	if(codec->playmode != PLAYMODE_AUTO_SWITCH)
		return;

	if(plug_in)
		codec->mixer_switch[MIXER_ADDR_HPDET] = PLAYMODE_STATUS_HP;
	else
		codec->mixer_switch[MIXER_ADDR_HPDET] = PLAYMODE_STATUS_SPEAKER;
}

/**
 * @brief   get playback mode: headphone or speaker
 * @author lixinhai 
 * @date   2013-08-02 
 * @input  codec: handle. 
 * @return  void
 */
static int ak39_codec_get_playmode(struct ak39_codec *codec)
{
	if(codec->playmode == PLAYMODE_HP) {
		return PLAYMODE_STATUS_HP;
	} else if(codec->playmode == PLAYMODE_SPEAKER)
		return PLAYMODE_STATUS_SPEAKER;	
	else
		return codec->mixer_switch[MIXER_ADDR_HPDET];
}

/**
 * @brief  set route source on destation. 
 * @author lixinhai 
 * @date   2013-08-02 
 * @input  codec: handle. 
 * @input  dst: destation
 * @input  src: source
 * @input  src_mask: source mask.
 * @return new destation of route.
 */
static inline int set_route_src_on_dst(struct ak39_codec *codec, 
		int dst, int src, int src_mask)
{
	codec->mixer_route[dst] &= ~src_mask;
	codec->mixer_route[dst] |= src;

	PDEBUG("%s: destion:%d, src:%d, src_mask:%d, mixer source:%d.\n", 
			__func__, dst, src, src_mask, codec->mixer_route[dst]);
	return codec->mixer_route[dst];
}

static inline int get_route_src_on_dst(struct ak39_codec *codec, 
		int dst, int src_mask)
{
	return codec->mixer_route[dst] & src_mask;
}


static inline int check_route_src_on_dst(struct ak39_codec *codec, 
		int dst, int src)
{
	return !!(codec->mixer_route[dst] & src);
}

static int calc_route_src_status(struct ak39_codec *codec, int dst, int src, int src_mask)
{
	int i;
	int status = 0;

	for(i=0; i<MIXER_DST_COUNT; i++) {
		if(i == dst)
			status |= codec->mixer_route[i] & (src & src_mask);
		else
			status |= codec->mixer_route[i] & src_mask;
	}	

	return !!status;
}


/**
 * @brief      set input device power
 * @author 
 * @date   
 * @param[in]      src: DAC|LINEIN|MIC  
 *                       addr:route No.  
 *                       CurSrc:all three route's current src
 * @return  void
 */
static void ak39_set_src_power(struct ak39_codec *codec, int dst, int src)
{
	int s_dac, s_linein, s_mic, s_src;

	s_dac = calc_route_src_status(codec, dst, src, SOURCE_DAC);
	s_linein = calc_route_src_status(codec, dst, src, SOURCE_LINEIN);
	s_mic = calc_route_src_status(codec, dst, src, SOURCE_MIC);


	s_src = s_dac|s_linein|s_mic;

	PDEBUG("%s: src:%d, s_dac:%d, s_linein:%d, s_mic:%d.\n",
		   	__func__, src, s_dac, s_linein, s_mic);

	ak39_set_sel_vref(codec, s_src);
	ak39_set_vcm_ref_power(codec, s_src);
	
	//set DAC power in PCM interface function: codec_playback_prepare()
	ak39_set_linein_power(codec, s_linein);
	ak39_set_mic_power(codec, s_mic);
}


static int ak39_codec_stream_control(struct ak39_codec *codec,
		int dst, int running)
{
	int src;

	src = get_route_src_on_dst(codec, dst, SOURCE_MIXED_ALL_MASK);       
	PDEBUG("%s: dst:%d, src:%d, status:%s.\n",
		   	__func__, dst, src, running ? "running":"stop");

	if(!running)
		src = 0;
	
	switch(dst) {
		case MIXER_ADDR_DST_HP:
			if(codec->used_hp_mute)
				ak_gpio_setpin(codec->hpmute_gpio.pin, codec->hpmute_en_val);

			ak39_set_hp_power(codec, !!src, !codec->used_hp_mute);

			if(codec->used_hp_mute) {
				int dis_val;

				dis_val = (codec->hpmute_en_val == AK_GPIO_OUT_HIGH)? 
						AK_GPIO_OUT_LOW:AK_GPIO_OUT_HIGH;
				ak_gpio_setpin(codec->hpmute_gpio.pin, dis_val);
			}
			ak39_set_hp_in(codec, src);
			ak39_set_src_power(codec, dst, src);

			break;
		case MIXER_ADDR_DST_ADC2:
			ak39_set_adc2_in(codec, src);
			ak39_set_src_power(codec, dst, src);
			//dump_codec_reg(codec);
			break;
		default:
			break;
	}

	return 0;
}


/**
 * @brief  open a dac device 
 * @author 
 * @date   
 * @return void
 */
void ak39_codec_dac_open(struct ak_codec_dai *dai)
{
	int reg_value;
	int i;
	struct ak39_codec *codec = to_ak39_codec(dai);

	if(codec->dac_state)
		return;

	codec->dac_state = 1;

	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) &= ~(1<<4);
	REG32(codec->analog_ctrl_base + CLOCK_GATE_CTRL_REG) &= ~(1<<4);

	REG32(codec->adda_cfg_base + DAC_CONFIG_REG) |= MUTE;

	REG32(codec->adda_cfg_base + DAC_CONFIG_REG) &= ~DAC_CTRL_EN;
	REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) &= ~DAC_CLK_EN;

	REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) |= DAC_DIV_VLD;
	i = 0;
	while(REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) & (DAC_DIV_VLD))
	{
		++i;
		if(i > 100000)
		{
			printk("set da clk reg fail\n");
			return ;
		}
	}   

	// to enable DAC CLK
	REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) |= DAC_CLK_EN;

	//soft reset DAC
	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) &= ~(DAC_SOFT_RST);
	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) |= DAC_SOFT_RST;

	//to enable internal dac/adc via i2s
	REG32(codec->analog_ctrl_base + MULTIPLE_FUN_CTRL_REG1) |= IN_DAAD_EN;

	//set default dac div and osr, for de pipa noise

	//I2S config reg
	REG32(codec->adda_cfg_base + I2S_CONFIG_REG) &= (~I2S_CONFIG_WORDLENGTH_MASK);
	REG32(codec->adda_cfg_base + I2S_CONFIG_REG) |= (16<<0);

	//config I2S interface DAC
	REG32(codec->adda_cfg_base + DAC_CONFIG_REG) |= DAC_CTRL_EN;

	//enable accept data from l2
	REG32(codec->adda_cfg_base + DAC_CONFIG_REG) |= L2_EN;
	REG32(codec->adda_cfg_base + DAC_CONFIG_REG) |= FORMAT;
	REG32(codec->adda_cfg_base + DAC_CONFIG_REG) &= ~ARM_INT;
	REG32(codec->adda_cfg_base + I2S_CONFIG_REG) &= ~POLARITY_SEL; 

	REG32(codec->analog_ctrl_base + FADEOUT_CTRL_REG) |= DAC_FILTER_EN;

	REG32(codec->adda_cfg_base + DAC_CONFIG_REG) &= ~MUTE;


	reg_value = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
	reg_value &= ~( PD_OP | PD_CK);
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_value;
}


/**
 * @brief   Close a dac device
 * @author  
 * @date   
 * @return  void
 */
void ak39_codec_dac_close(struct ak_codec_dai *dai)
{
	int reg_val;
	struct ak39_codec *codec = to_ak39_codec(dai);

	if(!codec->dac_state)
		return;

	codec->dac_state = 0;
	REG32(codec->adda_cfg_base + DAC_CONFIG_REG) &= (~L2_EN);

	// to power off DACs/DAC clock
	reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);   
	reg_val |= (PD_OP);
	reg_val |= (PD_CK);
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;

	mdelay(5);

	REG32(codec->analog_ctrl_base + MULTIPLE_FUN_CTRL_REG1) &= (~(IN_DAAD_EN));// tdisable internal DAC/ADC
	REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) &= (~(DAC_CLK_EN|DAC_DIV_VLD));// disable DAC CLK
	REG32(codec->analog_ctrl_base + DAC_CONFIG_REG) &= (~DAC_CTRL_EN);// to disable DACs
}

/**
 * @brief   Set sample rate
 * @author 
 * @date   
 * @param[in]  samplerate: desired sample rate
 * @return  void
 */
void ak39_codec_set_dac_samplerate(struct ak_codec_dai *dai, unsigned int samplerate)
{
	unsigned char osr, mclkdiv;
	struct ak39_codec *codec = to_ak39_codec(dai);

	get_dac_osv_div(codec, &osr, &mclkdiv, samplerate);

	//disable HCLK, and disable DAC interface
	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) &= ~(DAC_HCLK_EN);
	REG32(codec->analog_ctrl_base + DAC_CONFIG_REG) &= ~(DAC_CTRL_EN);

	set_dac_highspeed(codec, samplerate);
	
	 // NOTE: should reset DAC!!!
    REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) &= (~DAC_SOFT_RST);
	mdelay(5);

    //set OSR
    REG32(codec->analog_ctrl_base + FADEOUT_CTRL_REG) &= (~OSR_MASK);
    REG32(codec->analog_ctrl_base + FADEOUT_CTRL_REG) |= (OSR(osr));
	mdelay(2);

    //set DIV
    REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) &= (~(MASK_DAC_DIV_INT));
    REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) |= (DAC_DIV_INT(mclkdiv));

	REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) &= (~(MASK_DAC_DIV_FRAC));
	REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) |= (DAC_DIV_VLD);
	mdelay(2);	

    //to reset dac
    REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) |= DAC_SOFT_RST;

	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) &= (~DAC_SOFT_RST);
	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) |= DAC_SOFT_RST;

	//enable HCLK, and enable DAC interface
    REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) |= (DAC_HCLK_EN);
	REG32(codec->analog_ctrl_base + DAC_CONFIG_REG) |= (DAC_CTRL_EN);
}

/**
 * @brief   Set DAC channels: mono,stereo
 * @author 
 * @date   
 * @param[in]  bool mono: true-mono, false-stereo
 * @return  void
 */
void ak39_codec_set_dac_channels(struct ak_codec_dai *dai, unsigned int chnl)
{

}


/**
 * @brief  open ADC2
 * @author 
 * @date   
 * @param[in]  void
 * @return  void
 */
void ak39_codec_adc2_open(struct ak_codec_dai *dai)
{
	struct ak39_codec *codec = to_ak39_codec(dai);

	//unsigned long i;
	if(codec->adc2_state)
		return;

	codec->adc2_state = 1;

	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) &= ~(1<<3);
	REG32(codec->analog_ctrl_base + CLOCK_GATE_CTRL_REG) &= ~(1<<3);

	// soft reset ADC2 controller
	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) |= (ADC2_SOFT_RST);
	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) &= ~(ADC2_SOFT_RST);

//	set_cur_pmos(codec, 0x0);  //must be set to 0 when ADC2 is working

	REG32(codec->analog_ctrl_base + MULTIPLE_FUN_CTRL_REG1) |= IN_DAAD_EN; //enable internal

	//disable vcm2 discharge
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) &= ~(0x1f << 4);

	//SelVcm3
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) |= VCM3_SEL;

	//PowerOn Vcm2/3
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) &= ~PD_VCM3;
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) &= ~PD_VCM2;

	//SetVcmNormal
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) &= ~PL_VCM2;
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) &= ~PL_VCM3;

	//EnableAdc2Limit
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) |= ADC_LIM;

	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) &= ~HOST_RD_INT_EN;
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) |= CH_POLARITY_SEL;//Receive the left channel data when the lrclk is high
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) &= ~I2S_EN;        //Internal ADC MODE
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) &= ~WORD_LENGTH_MASK;
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) |= (0xF << 8);     //WORD LENGTH IS 16 BIT

	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) |= (ADC2_SOFT_RST);

	//Enable adc controller
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) |= ADC2_CTRL_EN;

	//Enable l2
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) |= ADC2MODE_L2_EN;

	//Power on adc2 conversion
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) &= ~PD_S2D;

	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) &= ~PD_ADC2;

	mdelay(1);
	pd_ref_enable(codec);
}

/**
 * @brief      close ADC2
 * @author 
 * @date   
 * @param[in]    void 
 * @return  void
 */
void ak39_codec_adc2_close(struct ak_codec_dai *dai)
{
	unsigned long reg_val;
	struct ak39_codec *codec = to_ak39_codec(dai);

	if(!codec->adc2_state)
		return;

	codec->adc2_state = 0;

	//disable l2
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) &= ~ADC2MODE_L2_EN;

	//disable adc2 clk
	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) &= ~ADC2_CLK_EN;

	//disable ADC2 interface
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) &= ~ADC2_CTRL_EN;

	//Power off adc2
	REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG2) |= PD_ADC2;

	if((2 << 12) != (REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) & (2 << 12)))
	{
		if(!(REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) & DAC_CLK_EN))
		{
			reg_val = REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1);
			reg_val |= PD_VCM3;
			reg_val |= PD_VCM2;
			reg_val |= PL_VCM2;
			//reg_val |= PD_REF;
			REG32(codec->analog_ctrl_base + ANALOG_CTRL_REG1) = reg_val;
			pd_ref_disable(codec);

		}
	}
	REG32(codec->analog_ctrl_base + CLOCK_CTRL_REG) |= (DAC_DIV_VLD);     //inhabit adc2 clock

}

/**
 * @brief      set ADC2 sample rate
 * @author 
 * @date   
 * @param[in]    samplerate:  desired rate to be set
 * @return  void
 */
unsigned long ak39_codec_set_adc2_samplerate(struct ak_codec_dai *dai,
	   		unsigned int  samplerate)
{
	unsigned char mode_sel = 0;
    unsigned char save_div = 0;
	unsigned long out_sr = 0;
	unsigned long i = 0;
	struct ak39_codec *codec = to_ak39_codec(dai);

	//disable HCLK, and disable ADC interface
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) &= ~(ADC2_CTRL_EN);
	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) &= ~(ADC2_HCLK_EN);
	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) &= ~(ADC2_CLK_EN);

	set_adc_highspeed(codec, samplerate);		
	
	out_sr = get_adc2_osr_div(codec, &mode_sel, &save_div, samplerate);

	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) &= ~(1<<27); //reset adc from adc clk
	i = 0;
    while(REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) & (ADC2_DIV_VLD))
    {
        ++i;
        if(i > 100000)
        {
            printk("adc set sr fail\n");
            return 0;
        }
    } 
    REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) &= ~(MASK_ADC2_DIV);
    REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) |= ADC2_DIV(save_div);

    REG32(codec->analog_ctrl_base + FADEOUT_CTRL_REG) &= ~(1<<ADC2_OSR_BIT);
    REG32(codec->analog_ctrl_base + FADEOUT_CTRL_REG) |= (mode_sel << ADC2_OSR_BIT);

	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) |= (ADC2_DIV_VLD);
	i = 0;
    while(REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) & (ADC2_DIV_VLD))
    {
        ++i;
        if(i > 100000)
        {
            printk("adc set clk reg fail\n");
            return 0;
        }
    } 
	REG32(codec->analog_ctrl_base + SOFT_RST_CTRL_REG) |= (1<<27); //release the reset adc from adc clk

	
	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) |= (ADC2_CLK_EN);//enable ADC2 Clock 
	REG32(codec->analog_ctrl_base + HIGHSPEED_CLOCK_CTRL_REG) |= (ADC2_HCLK_EN);//enable ADC2 Clock 
	REG32(codec->adda_cfg_base + ADC2_CONFIG_REG) |= (ADC2_CTRL_EN);
	return out_sr;
}


/**
 * @brief      set ADC23 channel
 * @author 
 * @date   
 * @param[in]    chnl: 1-mono; 2-stereo
 * @return  void
 */
void ak39_codec_set_adc2_channels(struct ak_codec_dai *dai, unsigned int chnl)
{

}


/**
 * @brief   cfg shutdown speaker GPIO
 * @author 
 * @date   
 * @param[in]  bOn: 1-power on; 0-power off
 * @return  void
 */
void ak39_codec_speak_on(struct ak39_codec *codec, bool bOn)
{
	unsigned int pin = codec->spkrshdn_gpio.pin; 
	ak_setpin_as_gpio(pin);
	ak_gpio_cfgpin(pin, AK_GPIO_DIR_OUTPUT);
	ak_gpio_setpin(pin, bOn);
}


/**
 * @brief  when playback start, ak39_codec_playback_start must be called to start output channel
 * @author  Cheng JunYi
 * @revisor	Wu Daochao(2012-08-23)
 * @date   
 * @return void
 */
static void ak39_codec_playback_start(struct ak_codec_dai *dai)
{
	struct ak39_codec *codec = to_ak39_codec(dai);

	if(codec->mixer_ch_duration[MIXER_ADDR_PLAY_DURATION] == CHNLDURATION_EVEROPEN)
		return;

	ak39_codec_get_playmode(codec);
	ak39_codec_stream_control(codec, MIXER_ADDR_DST_HP, 1);

	codec->outputing = true;
	
}

void ak39_start_to_play(struct ak_codec_dai *dai, 
		unsigned int channels, unsigned int samplerate)
{
	ak39_codec_set_dac_samplerate(dai, samplerate);
	ak39_codec_set_dac_channels(dai, channels);
	ak39_codec_dac_open(dai);
	ak39_codec_playback_start(dai);
}


static void ak39_codec_playback_stop(struct ak_codec_dai *dai)
{
	struct ak39_codec *codec = to_ak39_codec(dai);
	
	//if we want to open some channel for ever, return
	if(codec->mixer_ch_duration[MIXER_ADDR_PLAY_DURATION] == CHNLDURATION_EVEROPEN)
		return;

	/*close the dac output*/
	ak39_codec_stream_control(codec, MIXER_ADDR_DST_HP, 0);
	codec->outputing = false;
}


static void ak39_codec_capture_start(struct ak_codec_dai *dai)
{
	struct ak39_codec *codec = to_ak39_codec(dai);

	ak39_codec_stream_control(codec, MIXER_ADDR_DST_ADC2, 1);
}

static void ak39_codec_capture_stop(struct ak_codec_dai *dai)
{
	struct ak39_codec *codec = to_ak39_codec(dai);

	ak39_codec_stream_control(codec, MIXER_ADDR_DST_ADC2, 0);
}

/**********************HPDet switch**************************/

static int set_hpdet_status(struct ak39_codec *codec)
{
	int plug_in = 0;
	int irq_type;

	if(ak_gpio_getpin(codec->hpdet_gpio.pin) == codec->hp_on_value)
	{
		//hp is plugged in
		plug_in = 1;
		irq_type = (codec->irq_hp_on_type == IRQ_TYPE_LEVEL_LOW) ?
					IRQ_TYPE_LEVEL_HIGH:IRQ_TYPE_LEVEL_LOW;
	}
	else
	{
		//hp is pulled out
		plug_in = 0;
		irq_type = codec->irq_hp_on_type;
	}

	printk("detect the headphone plug %s.\n", plug_in ? "on":"out");
	irq_set_irq_type(codec->hp_det_irq, irq_type);

	set_playmode_switch(codec, plug_in);

	ak39_codec_speak_on(codec, !plug_in);
	return 0;
}


/**
 * @brief  when hp state is changed,schedule hp_det_worker to config output \
               channel to speaker or not
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static void hp_det_worker(struct work_struct *work)
{
	struct ak39_codec *codec = container_of(work, struct ak39_codec,d_work.work);

	set_hpdet_status(codec);

	ak_codec_ctl_event(SNDRV_CTL_ELEM_IFACE_HWDEP,
		   	SNDRV_CTL_EVENT_MASK_VALUE, "HPDet switch");

	enable_irq(codec->hp_det_irq);
}

/**
 * @brief  hp det
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static irqreturn_t ak39_codec_hpdet_irq(int irq, void *dev_id)
{
	struct ak39_codec *codec = dev_id;
	
	disable_irq_nosync(irq);

	schedule_delayed_work(&codec->d_work, msecs_to_jiffies(100));
	return IRQ_HANDLED;
}


/**************
 * mixer interface
 **************/


/***********************config channel duration*******************************/
#define AK39PCM_OUTPUTCHNL_DURATION(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .access = SNDRV_CTL_ELEM_ACCESS_READWRITE, \
  .name = xname, .index = xindex, \
  .info = codec_ChnlDuration_info, \
  .get = codec_ChnlDuration_get, \
  .put = codec_ChnlDuration_put, \
  .private_value = addr \
}

/**
 * @brief  info callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_ChnlDuration_info(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	int addr = kcontrol->private_value;
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	if(MIXER_ADDR_PLAY_DURATION == addr)
	{
		uinfo->value.integer.min = CHNLDURATION_MIN;
		uinfo->value.integer.max = CHNLDURATION_MAX;
	}
	else
	{
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief  get callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_ChnlDuration_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);

	int addr = kcontrol->private_value;
	if(addr != MIXER_ADDR_PLAY_DURATION)
	{
		return -EINVAL;
	}

	ucontrol->value.integer.value[0] = codec->mixer_ch_duration[addr];
	return 0;
}

/**
 * @brief  put callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_ChnlDuration_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);

	int addr = kcontrol->private_value;
	int duration = ucontrol->value.integer.value[0];

	printk("ak39 codec: ChnlDuration_put duration=%d\n",duration);
	if(duration == codec->mixer_ch_duration[MIXER_ADDR_PLAY_DURATION]){
		return 0;
	}
	if(addr != MIXER_ADDR_PLAY_DURATION)	{
		return -EINVAL;
	}
	if((duration > CHNLDURATION_MAX)||(duration < CHNLDURATION_MIN)) {
		return -EINVAL;
	}

	codec->mixer_ch_duration[MIXER_ADDR_PLAY_DURATION] = duration;
	if((duration == CHNLDURATION_CONSTANT) &&
	  		(!test_bit(0, &playback_statu))) {
		ak39_codec_playback_stop(dai);
	}

	return 0;
}

/*********************select fixed output channel**********************************/
#define AK39PCM_DAC_OUT_MODE(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .access = SNDRV_CTL_ELEM_ACCESS_READWRITE, \
  .name = xname, .index = xindex, \
  .info = codec_dac_outmode_info, \
  .get = codec_dac_outmode_get, \
  .put = codec_dac_outmode_put, \
  .private_value = addr \
}

/**
 * @brief  info callback. 
 * becsuse of arch-ak39 has dac output only,
 * this interface is not used.
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_dac_outmode_info(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	int addr = kcontrol->private_value;
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	if(MIXER_ADDR_OUTMODE_DAC == addr)
	{
		uinfo->value.integer.min = OUTMODE_MIN;
		uinfo->value.integer.max = OUTMODE_MAX;
	}
	else
	{
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief  get callback
 * becsuse of arch-ak39 has dac output only,
 * this interface is not used.
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_dac_outmode_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int addr = kcontrol->private_value;

	if(addr != MIXER_ADDR_OUTMODE_DAC)
	{
		return -EINVAL;
	}

	ucontrol->value.integer.value[0] = OUTMODE_HP;
	return 0;
}

/**
 * @brief  put callback
 * becsuse of arch-ak39 has dac output only,
 * this interface is not used.
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_dac_outmode_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	printk("ak39 platform not support out mode setting.\n");
	return 0;
}

/**********************hp detect (read-only)****************************/
#define AK39PCM_SWITCH(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_HWDEP, \
  .access = SNDRV_CTL_ELEM_ACCESS_READ, \
  .name = xname, .index = xindex, \
  .info = codec_switch_info, \
  .get = codec_switch_get, \
  .private_value = addr \
}

/**
 * @brief  info callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_switch_info(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	int addr = kcontrol->private_value;
	if(MIXER_ADDR_HPDET != addr)
	{
		return -EINVAL;
	}

	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;		
	return 0;
}

/**
 * @brief  get callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_switch_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);

	int addr = kcontrol->private_value;
	if(MIXER_ADDR_HPDET != addr)
	{
		return -EINVAL;
	}

	ucontrol->value.integer.value[0] = codec->mixer_switch[addr];
	PDEBUG("%s enter, value:%lu.\n", __func__, ucontrol->value.integer.value[0]);
	return 0;
}

/***************************VOLUME*********************/
#define AK39PCM_VOLUME(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .access = SNDRV_CTL_ELEM_ACCESS_READWRITE, \
  .name = xname, .index = xindex, \
  .info = codec_volume_info, \
  .get = codec_volume_get, \
  .put = codec_volume_put, \
  .private_value = addr \
}

/**
 * @brief  info callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_volume_info(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	int port = kcontrol->private_value;
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;


	if(port < 0 || port >= MIXER_PORT_VOL_COUNT) {
		printk(KERN_ERR "%s port %d is invaild.\n", __func__, port);
		return -EINVAL;
	}
	uinfo->value.integer.min = get_volume_info(port, VOL_INFO_MIN);
	uinfo->value.integer.max = get_volume_info(port, VOL_INFO_MAX);

	return 0;
}

/**
 * @brief  get callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_volume_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);

	int port = kcontrol->private_value;
	if(port < 0 || port >= MIXER_PORT_VOL_COUNT) {
		printk(KERN_ERR "%s port %d is invaild.\n", __func__, port);
		return -EINVAL;
	}

	ucontrol->value.integer.value[0] = get_mixer_volume(codec, port);
	PDEBUG("%s enter, port:%d, volume:%lu.\n",
		   	__func__, port, ucontrol->value.integer.value[0]);
	return 0;
}

/**
 * @brief  put callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_volume_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);

	int port = kcontrol->private_value;
	int vol, old_vol;
	int change = 0;

	vol = ucontrol->value.integer.value[0];

	PDEBUG("%s enter, port:%d, volume:%d.\n", __func__, port, vol);
	vol = volume_repair_range(port, vol);

	old_vol = set_mixer_volume(codec, port, vol);
	if(vol != old_vol) {
		change = 1;
		ak39_set_gain(codec, port, vol);
	}

	return change;
}

/***************************ROUTE**************************/
#define AK39PCM_ROUTE(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .access = SNDRV_CTL_ELEM_ACCESS_READWRITE, \
  .name = xname, \
  .index = xindex, \
  .info = codec_route_info, \
  .get = codec_route_get, \
  .put = codec_route_put, \
  .private_value = addr \
}

/**
 * @brief  info callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_route_info(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo)
{
	int addr = kcontrol->private_value;

	if(addr >= MIXER_DST_COUNT)
	{
		return -EINVAL;
	}
	
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = SIGNAL_SRC_MAX;

	PDEBUG("%s enter, min:0, max:%d.\n", __func__, SIGNAL_SRC_MAX);
	return 0;
}

/**
 * @brief  get callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_route_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);
	int dst = kcontrol->private_value;

	if(dst >= MIXER_DST_COUNT)
	{
		return -EINVAL;
	}

	ucontrol->value.integer.value[0] = get_route_src_on_dst(codec, dst, SOURCE_MIXED_ALL_MASK);
	PDEBUG("%s enter, value:%lu.\n", __func__, ucontrol->value.integer.value[0]);
	return 0;
}

/**
 * @brief  put callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
/*application has to call this callback with params value=0 to power down input&output devices*/
static int codec_route_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);
	int change = 1;
	unsigned long dst = kcontrol->private_value;
	int src = 0;

	src = ucontrol->value.integer.value[0];
	PDEBUG("%s:set route: dst:%lu, src:%d.\n", __func__, dst, src);

	if((src < SIGNAL_SRC_MUTE) || (src > SIGNAL_SRC_MAX))
		return -EINVAL;

	if(dst == MIXER_ADDR_DST_HP) {
		int plug_in = 0;

		if(codec->playmode == PLAYMODE_HP)
			plug_in = 1;
		else if(codec->playmode == PLAYMODE_SPEAKER)
			plug_in = 0;

		set_playmode_switch(codec, plug_in);

		/*power on the speaker when headphone plug out.*/
		ak39_codec_speak_on(codec, !plug_in); 

	}else if(dst == MIXER_ADDR_DST_ADC2) {
	}

	if(get_route_src_on_dst(codec, dst, SOURCE_MIXED_ALL_MASK) == src) {
		change = 0;
	}
	set_route_src_on_dst(codec, dst, src, SOURCE_MIXED_ALL_MASK);

	return change;
}

/***************************ROUTE**************************/
#define AK39PCM_AEC(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .access = SNDRV_CTL_ELEM_ACCESS_READWRITE, \
  .name = xname, \
  .index = xindex, \
  .info = codec_aec_info, \
  .get = codec_aec_get, \
  .put = codec_aec_put, \
  .private_value = addr \
}

/**
 * @brief  info callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_aec_info(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	//int port = kcontrol->private_value;
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;

	printk(  "get the aec info \n" );
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 10;

	return 0;
}

/**
 * @brief  get callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_aec_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	//struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);

	//int port = kcontrol->private_value;
	ucontrol->value.integer.value[0] = dai->aec_flag;
	//printk("%s enter, %d\n", __func__, ucontrol->value.integer.value[0]);
	return 0;
}

/**
 * @brief  put callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int codec_aec_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct ak_codec_dai *dai = snd_kcontrol_chip(kcontrol);
	//struct ak39_codec *codec = container_of(dai, struct ak39_codec, dai);

	int src = ucontrol->value.integer.value[0];
	int port = kcontrol->private_value;


	printk("%s enter, %d  %d \n", __func__, port, src);
	dai->aec_flag = src;

	return 1;
}




static struct ak_proc_entry codec_pentries[] = {
};

static struct snd_kcontrol_new codec_controls[] = {
	AK39PCM_VOLUME("Headphone Playback Volume", 0, MIXER_PORT_HP_VOL),
	AK39PCM_VOLUME("LineIn Capture Volume", 0, MIXER_PORT_LINEIN_VOL),
	AK39PCM_VOLUME("Mic Capture Volume", 0, MIXER_PORT_MIC_VOL),
	AK39PCM_ROUTE("Headphone Playback Route", 0, MIXER_ADDR_DST_HP),
	AK39PCM_ROUTE("ADC Capture Route", 0, MIXER_ADDR_DST_ADC2),
	/*AK39PCM_ROUTE("LineIn Capture Route", 0, MIXER_ADDR_LINEINSRC),*/
	AK39PCM_SWITCH("HPDet switch", 0, MIXER_ADDR_HPDET),
	AK39PCM_DAC_OUT_MODE("DAC out mode", 0, MIXER_ADDR_OUTMODE_DAC),
	AK39PCM_OUTPUTCHNL_DURATION("duration of output channel", 0, MIXER_ADDR_PLAY_DURATION),
	AK39PCM_AEC("Set the aec", 0, 0),
};

struct ak_codec_ops ak39_codec_ops = {
	.dac_init		= ak39_codec_dac_open,
	.dac_exit		= ak39_codec_dac_close,
	.adc_init		= ak39_codec_adc2_open,
	.adc_exit		= ak39_codec_adc2_close,
	.set_dac_samplerate	= ak39_codec_set_dac_samplerate,
	.set_adc_samplerate	= ak39_codec_set_adc2_samplerate,
	.set_dac_channels	= ak39_codec_set_dac_channels,
	.set_adc_channels	= ak39_codec_set_adc2_channels,
	.playback_start		= ak39_codec_playback_start,
	.playback_end		= ak39_codec_playback_stop,
	.capture_start		= ak39_codec_capture_start,
	.capture_end		= ak39_codec_capture_stop,
	.start_to_play		= ak39_start_to_play,
};


static int ak39_codec_probe(struct platform_device *pdev)
{
	int err = 0;
	int i;
	struct ak39_codec_platform_data *pdata = pdev->dev.platform_data;
	struct ak39_codec *codec;
	struct resource *res;

	printk("ak39_codec_probe enter.\n");

	codec = kzalloc(sizeof(*codec), GFP_KERNEL);
	if (!codec)
		return -ENOMEM;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "akpcm_AnalogCtrlRegs"); 
	if(!res)
	{
		printk(KERN_ERR "no memory resource for analog_ctrl_res\n");
		err = -ENXIO;
		goto out_free_codec;
	}

	codec->analog_ctrl_base = ioremap(res->start, res->end - res->start + 1);
	if (!codec->analog_ctrl_base) {
		printk(KERN_ERR "could not remap analog_ctrl_res memory");
		err = -ENXIO;
		goto out_free_codec;
	}


	//get ADC2 mode registers
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "akpcm_ADC2ModeCfgRegs");
	if(!res)
	{
		printk(KERN_ERR "no memory resource for adda_cfg_res\n");
		err = -ENXIO;
		goto out_unremap_analog;
	}
	codec->adda_cfg_base = ioremap(res->start, res->end - res->start + 1);
	if (!codec->adda_cfg_base) {
		printk(KERN_ERR "could not remap adda_cfg_res memory");
		err = -ENXIO;
		goto out_unremap_analog;
	}


	for(i=0; i<MIXER_PORT_VOL_COUNT; i++) {
		set_mixer_volume(codec, i, get_volume_info(i, VOL_INFO_DEFAULT));
		ak39_set_gain(codec, i, get_mixer_volume(codec, i));
	}

	for(i=0; i<MIXER_DST_COUNT; i++) {
		set_route_src_on_dst(codec, i, default_route[i], SOURCE_MIXED_ALL_MASK);
	}
	
	/*only a hp output at arch-ak39*/
	codec->mixer_outmode[MIXER_ADDR_OUTMODE_DAC] = OUTMODE_HP; 
	codec->mixer_ch_duration[MIXER_ADDR_PLAY_DURATION] = CHNLDURATION_CONSTANT;
	
	/*FIXME:can add the code for worker vaild when switch auto mode*/
	INIT_DELAYED_WORK(&codec->d_work, hp_det_worker);
	
	codec->playmode = pdata->boutput_only;

	//config hp det gpio
	codec->hpdet_gpio = pdata->hpdet_gpio;
	
	//config speaker shutdown gpio
	codec->spkrshdn_gpio = pdata->spk_down_gpio;
	ak_gpio_set(&(codec->spkrshdn_gpio));

	//config speaker shutdown gpio
	codec->spkrshdn_gpio = pdata->spk_down_gpio;
	ak_gpio_set(&(codec->spkrshdn_gpio));

	//set hp det pin irq
	if(codec->hpdet_gpio.pin >= 0) {
		ak_gpio_set(&(codec->hpdet_gpio));

		if(AK_GPIO_OUT_LOW == codec->hp_on_value)
			codec->irq_hp_on_type = IRQ_TYPE_LEVEL_LOW;
		else if(AK_GPIO_OUT_HIGH == codec->hp_on_value)
			codec->irq_hp_on_type = IRQ_TYPE_LEVEL_HIGH;

		codec->hp_det_irq = pdata->hpdet_irq;
		codec->hp_on_value = pdata->hp_on_value;

		set_hpdet_status(codec);
		err = request_irq(codec->hp_det_irq, ak39_codec_hpdet_irq,
			   	IRQF_DISABLED, pdev->name, codec);
		if(err)
		{
			printk(KERN_ERR "request irq error!");
			goto err_out;
		}
	}

	codec->dai.ops = &ak39_codec_ops;
	codec->dai.num_kcontrols = ARRAY_SIZE(codec_controls);
	codec->dai.kcontrols = codec_controls;
	codec->dai.num_pentries = ARRAY_SIZE(codec_pentries);
	codec->dai.pentries = codec_pentries;
	codec->dai.entries_private = codec;
	
	if(ak_codec_register(&codec->dai))
		goto err_out;

	codec->used_hp_mute = pdata->bIsHPmuteUsed;
	codec->hpmute_en_val = pdata->hp_mute_enable_value;

	if(codec->used_hp_mute){
		//config hp mute gpio
		codec->hpmute_gpio = pdata->hpmute_gpio;
		ak_gpio_set(&(codec->hpmute_gpio));
	}

	codec->outputing = false;

	platform_set_drvdata(pdev, codec);
	return 0;

err_out:
//out_unremap_adda:
	iounmap(codec->adda_cfg_base);
out_unremap_analog:
	iounmap(codec->analog_ctrl_base);
out_free_codec:
	kfree(codec);
	return err;
}

static int ak39_codec_remove(struct platform_device *pdev)
{
	struct ak39_codec *codec = platform_get_drvdata(pdev);

	free_irq(codec->hp_det_irq, codec);

	/*FIXME:can add the code for worker vaild when switch auto mode*/
	cancel_delayed_work_sync(&codec->d_work);

	iounmap(codec->analog_ctrl_base);
	iounmap(codec->adda_cfg_base);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver ak39_codec_driver = {
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "ak39-codec",
	},
	.probe	= ak39_codec_probe,
	.remove	= ak39_codec_remove,
};

static int __init ak39_codec_init(void)
{
	return platform_driver_register(&ak39_codec_driver);
}

static void __exit ak39_codec_exit(void)
{
	platform_driver_unregister(&ak39_codec_driver);
}
module_init(ak39_codec_init);
module_exit(ak39_codec_exit);

MODULE_AUTHOR("anyka");
MODULE_DESCRIPTION("ak39 codec Driver");
MODULE_LICENSE("GPL");

