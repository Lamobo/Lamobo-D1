/*
 *  arch/arm/mach-ak39/gpio.c
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */
#include <linux/module.h>
#include <asm/irq.h>
#include <mach/gpio.h>

//share pin config fore module in AK39xx
struct gpio_sharepin_cfg share_cfg_module[] = {
	{ePIN_AS_OPCLK,    SHARE_CFG1,	(0x3<<2), (1<<2), 0, 0, 0, 0},
    {ePIN_AS_JTAG,     SHARE_CFG1,	(0x3f<<16)|(1<<1), (0x15<<16)|(1<<1), 0, 0, 0, 0},
    {ePIN_AS_RTCK,	   SHARE_CFG1,	(0x3<<22), (1<<22), 0, 0, 0, 0},
    {ePIN_AS_I2S,      SHARE_CFG13,	((0xf<<8)|(1<<13)), ((0xf<<8)|(1<<13)), 0, 0, (0x3<<4), (0x3<<4)},
    {ePIN_AS_PWM1,     SHARE_CFG1,	((0x3<<16)|(1<<4)), ((0x3<<16)|(1<<4)), 0, 0, 0, 0},
    {ePIN_AS_PWM2,     SHARE_CFG1,	(0x3<<18), (0x3<<18), 0, 0, 0, 0},
    {ePIN_AS_PWM3,     SHARE_CFG1,	((0x3<<20)|(1<<6)), ((0x3<<20)|(1<<6)), 0, 0, 0, 0},
    {ePIN_AS_PWM4,     SHARE_CFG1,	((0x3<<22)|(1<<7)), ((0x3<<22)|(1<<7)), 0, 0, 0, 0},
#if defined(CONFIG_CPU_AK3910)
	{ePIN_AS_PWM5,     SHARE_CFG13,	(0x3<<2), (0x3<<2), 0, 0, (0x3<<4), (0x1<<4)},
    {ePIN_AS_SPI1,     SHARE_CFG3,	0, 0, 0, 0, ((0x3<<23)|(1<<21)|(1<<19)|(0x3)),((0x1<<23)|(1<<21)|(1<<19)|(0x3))},
    {ePIN_AS_SPI2,     SHARE_CFG3,	0, 0, 0, 0, ((0x3<<23)|(1<<21)|(1<<19)|(0xf<<2)),((0x3<<23)|(1<<21)|(1<<19)|(0xa<<2))},
#elif defined(CONFIG_CPU_AK3916) || defined(CONFIG_CPU_AK3918)
	{ePIN_AS_PWM5,	   SHARE_CFG13, ((1<<12)|(0x3<<2)), ((1<<12)|(0x3<<2)), 0, 0, (0x3<<4), (0x1<<4)},
	{ePIN_AS_SPI1,	   SHARE_CFG3,	0, 0, 0, 0, ((0x3<<23)|(1<<21)|(1<<19)|(0x3)),((0x1<<23)|(1<<21)|(1<<19)|(0x3))},
	{ePIN_AS_SPI2,	   SHARE_CFG3,	0, 0, 0, 0, ((0x3<<23)|(1<<21)|(1<<19)|(0xf<<2)),((0x3<<23)|(1<<21)|(1<<19)|(0xa<<2))},
#endif
	{ePIN_AS_UART1,    SHARE_CFG1,	(0x3<<14), (0x3<<14), 0, 0, 0, 0},
    {ePIN_AS_UART2,    SHARE_CFG1,	(0xff<<16), (0xaa<<16), 0, 0, 0, 0},
    {ePIN_AS_CAMERA,   SHARE_CFG2,	0, 0, (0xf), (0x0), 0, 0},
    {ePIN_AS_SDIO,     SHARE_CFG3,	0, 0, 0, 0, (0xff<<16), (0x57<<16)},
    {ePIN_AS_MCI,	   SHARE_CFG3,	0, 0, 0, 0, (0xf<<6), (0xf<<6)},
	{ePIN_AS_MCI_8LINE, SHARE_CFG3, 0, 0, 0, 0, (0x3ff<<6), (0x15f<<6)},
	{ePIN_AS_MAC,      SHARE_CFG12,	(0x3<<2), (1<<2), (0x1fff<<4), (0x1fff<<4), 0, 0},
	{ePIN_AS_I2C,      SHARE_CFG3,	0, 0, 0, 0, (0x3<<25), (0x3<<25)},
	{ePIN_AS_IRDA,     SHARE_CFG3,	0, 0, 0, 0, (0x3<<2), (0x1<<2)},
    {ePIN_AS_DUMMY,    EXIT_CFG,	0, 0, 0, 0, 0, 0}
};

struct gpio_pupd_cfg pupd_cfg_info[] = {
	//pin, index, register, up/down
	{AK_GPIO_0, 0, PUPD_CFG1, PULLUP},
	{AK_GPIO_1, 19, PUPD_CFG1, PULLUP},
	{AK_GPIO_2, 20, PUPD_CFG1, PULLUP},
	{AK_GPIO_3, 1, PUPD_CFG1, PULLDOWN},
	{AK_GPIO_4, 21, PUPD_CFG1, PULLUP},
	{AK_GPIO_5, 22, PUPD_CFG1, PULLUP},
	{AK_GPIO_6, 23, PUPD_CFG1, PULLUP},
	{AK_GPIO_7, 24, PUPD_CFG1, PULLUP},
	{AK_GPIO_8, 4, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_9, 5, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_10, 7, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_11, 8, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_12, 10, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_13, 11, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_14, 14, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_15, 15, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_16, 16, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_17, 17, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_18, 18, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_19, 20, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_20, 21, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_21, 22, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_22, 23, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_23, 25, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_24, 26, PUPD_CFG2, PULLDOWN},
	{AK_GPIO_25, 0, PUPD_CFG3, PULLUP},
	{AK_GPIO_26, 1, PUPD_CFG3, PULLUP},
	{AK_GPIO_27, 20, PUPD_CFG3, PULLUP},
	{AK_GPIO_28, 21, PUPD_CFG3, PULLUP},
	{AK_GPIO_29, 2, PUPD_CFG3, PULLUP},
	{AK_GPIO_30, 3, PUPD_CFG3, PULLUP},
	{AK_GPIO_31, 4, PUPD_CFG3, PULLUP},
	{AK_GPIO_32, 5, PUPD_CFG3, PULLUP},
	{AK_GPIO_33, 6, PUPD_CFG3, PULLUP},
	{AK_GPIO_34, 7, PUPD_CFG3, PULLUP},
	{AK_GPIO_35, 8, PUPD_CFG3, PULLUP},
	{AK_GPIO_36, 9, PUPD_CFG3, PULLUP},
	{AK_GPIO_37, 10, PUPD_CFG3, PULLUP},
	{AK_GPIO_38, 11, PUPD_CFG3, PULLUP},
	{AK_GPIO_39, 12, PUPD_CFG3, PULLUP},
	{AK_GPIO_40, 13, PUPD_CFG3, PULLUP},
	{AK_GPIO_41, 14, PUPD_CFG3, PULLUP},
	{AK_GPIO_42, 15, PUPD_CFG3, PULLUP},
	{AK_GPIO_43, 16, PUPD_CFG3, PULLUP},
	{AK_GPIO_44, 17, PUPD_CFG3, PULLUP},
	{AK_GPIO_45, 18, PUPD_CFG3, PULLUP},
	{AK_GPIO_46, 19, PUPD_CFG3, PULLUP},
	{AK_GPIO_47, 2, PUPD_CFG1, PULLDOWN},
	{AK_GPIO_48, 3, PUPD_CFG1, PULLDOWN},
	{AK_GPIO_50, 5, PUPD_CFG1, PULLUP},
	{AK_GPIO_51, 6, PUPD_CFG1, PULLUP},
	{AK_GPIO_52, 7, PUPD_CFG1, PULLUP},
	{AK_GPIO_53, 8, PUPD_CFG1, PULLUP},
	{AK_GPIO_54, 9, PUPD_CFG1, PULLDOWN},
	{AK_GPIO_55, 10, PUPD_CFG1, PULLDOWN},
	{AK_GPIO_56, 11, PUPD_CFG1, PULLUP},
	{AK_GPIO_57, 12, PUPD_CFG1, PULLUP},
	{AK_GPIO_58, 13, PUPD_CFG1, PULLUP},
	{AK_GPIO_59, 14, PUPD_CFG1, PULLDOWN},
	{AK_GPIO_60, 15, PUPD_CFG1, PULLDOWN},
	{AK_GPIO_61, 16, PUPD_CFG1, PULLDOWN},
	{AK_GPIO_62, 17, PUPD_CFG1, PULLUP},
	{AK_GPIO_63, 18, PUPD_CFG1, PULLUP},
};

//this used to clr in gpio chare pin cfg1
struct sharepin_as_gpio sharepin_cfg_gpio1[] = {
    {0,		0,		0,	AS_GPIO_CFG_BIT1},
    {3,		3,		1,	AS_GPIO_CFG_BIT1},
    {47,	47,		2,	AS_GPIO_CFG_BIT2},
    {48,	48,		4,	AS_GPIO_CFG_BIT1},
    {50,	50,		6,	AS_GPIO_CFG_BIT1},
    {51,	51,		7,	AS_GPIO_CFG_BIT1},
    {52,	52,		8,	AS_GPIO_CFG_BIT1},
    {53,	53,		9,	AS_GPIO_CFG_BIT1},
    {54,	54,		10,	AS_GPIO_CFG_BIT1},
    {55,	55,		11,	AS_GPIO_CFG_BIT1},
    {56,	56,		12,	AS_GPIO_CFG_BIT1},
    {57,	57,		13,	AS_GPIO_CFG_BIT1},
    {1,		1,		14,	AS_GPIO_CFG_BIT1},
    {2,		2,		15,	AS_GPIO_CFG_BIT1},
    {4,		4,		16,	AS_GPIO_CFG_BIT2},
    {5,		5,		18,	AS_GPIO_CFG_BIT2},
    {6,		6,		20,	AS_GPIO_CFG_BIT2},
    {7,		7,		22,	AS_GPIO_CFG_BIT2},
};

//this used to clr in gpio chare pin cfg2
struct sharepin_as_gpio sharepin_cfg_gpio3[] = {
    {25,	25,		0,	AS_GPIO_CFG_BIT1},
    {26,	26,		1,	AS_GPIO_CFG_BIT1},
    {29,	29,		2,	AS_GPIO_CFG_BIT2},
    {30,	30,		4,	AS_GPIO_CFG_BIT2},
    {31,	31,		6,	AS_GPIO_CFG_BIT1},
    {32,	32,		7,	AS_GPIO_CFG_BIT1},
    {33,	33,		8,	AS_GPIO_CFG_BIT1},
    {34,	36,		9,	AS_GPIO_CFG_BIT1},
    {37,	38,		10,	AS_GPIO_CFG_BIT2},
    {39,	39,		12,	AS_GPIO_CFG_BIT2},
    {40,	40,		14,	AS_GPIO_CFG_BIT2},
    {41,	41,		16, AS_GPIO_CFG_BIT1},
    {42,	42,		17, AS_GPIO_CFG_BIT1},
    {43,	43,		18, AS_GPIO_CFG_BIT2},
    {44,	44,		20, AS_GPIO_CFG_BIT2},
    {45,	46,		22, AS_GPIO_CFG_BIT2},
    {27,	27,		25, AS_GPIO_CFG_BIT1},
    {28,	28,		26, AS_GPIO_CFG_BIT1},
};

#define INVALID_WK_BIT 0xff
struct t_gpio_wakeup_cfg gpio_wakeup_cfg[] = {
	//gpio_start        gpio_end      start_bit
	{AK_GPIO_0,		AK_GPIO_7,   0},
	{AK_GPIO_12,	AK_GPIO_14,  8},
	{AK_GPIO_22,	AK_GPIO_30,  11},
	{AK_GPIO_39,	AK_GPIO_44,  16},
	{AK_GPIO_47,	AK_GPIO_55,  22},
	{AK_GPIO_57,	AK_GPIO_57,  31},
};


unsigned int ak3910_invalid_gpio[] = {
	AK_GPIO_37, AK_GPIO_38, AK_GPIO_39, AK_GPIO_40, AK_GPIO_56,
	AK_GPIO_58, AK_GPIO_59, AK_GPIO_60, AK_GPIO_61, AK_GPIO_62,
	AK_GPIO_63,
};

unsigned int ak3916_invalid_gpio[] = {
};

static unsigned char get_bit_by_pin_wk(unsigned char pin)
{
    int i, n;
    n = ARRAY_SIZE(gpio_wakeup_cfg);

    for (i=0; i<n; i++) {
        if (pin >= gpio_wakeup_cfg[i].gpio_start && pin <= gpio_wakeup_cfg[i].gpio_end)
            return gpio_wakeup_cfg[i].start_bit + (pin - gpio_wakeup_cfg[i].gpio_start);
    }

    return INVALID_WK_BIT;
}

/* when the specific bit is set to 0, the wake-up GPIO is rising triggered
 * when the specific bit is set to 1, the wake-up GPIO is falling triggered
 */
void ak_gpio_wakeup_pol(unsigned int pin, unsigned char pol)
{
    unsigned char bit = get_bit_by_pin_wk(pin);
    unsigned int val;

    if (bit == INVALID_WK_BIT) {
        panic("this pin %u doesn't support wakeup function\n", pin);
        return;
    }
    
    val = REG32(AK_WGPIO_POLARITY);
    val &= ~(1 << bit);
    val |= (pol << bit);
    REG32(AK_WGPIO_POLARITY) = val;
}
EXPORT_SYMBOL(ak_gpio_wakeup_pol);

int ak_gpio_wakeup(unsigned int pin, unsigned char enable)
{
    unsigned char bit = get_bit_by_pin_wk(pin);
    unsigned int val;

    if (bit == INVALID_WK_BIT) {
        panic("this pin %d doesn't support wakeup function\n", pin);
        return -1;
    }
    //clear wake gpio status
    val = REG32(AK_WGPIO_CLEAR);
    val |= (1 << bit);
    REG32(AK_WGPIO_CLEAR) = val;
    val &= ~(1 << bit);
    REG32(AK_WGPIO_CLEAR) = val;
    
    val = REG32(AK_WGPIO_ENABLE);
    if (enable == AK_WAKEUP_ENABLE) {
        val |= (1 << bit);
    } else if (enable == AK_WAKEUP_DISABLE) {
        val &= ~(1 << bit);
    } else
        panic("wrong enable value in ak_gpio_wakeup\n");
    REG32(AK_WGPIO_ENABLE) = val;
    
    return 0;
}
EXPORT_SYMBOL(ak_gpio_wakeup);

/*
 * @brief set gpio pin group as specified module used
 * @param[in] PinCfg enum data. the specified module
 */
void ak_group_config(T_GPIO_SHAREPIN_CFG mod_name)
{
    unsigned long i, flags, val = 0;
    
    if(ePIN_AS_GPIO == mod_name) {
        //set all pin as gpio except uart0
        local_irq_save(flags);
        __raw_writel(0xc000, AK_SHAREPIN_CON1);
		__raw_writel(0xf, AK_SHAREPIN_CON2);
		__raw_writel(0x0, AK_SHAREPIN_CON3);
        local_irq_restore(flags);
        return;
    }

    for(i = 0; ; i++) {
        if(ePIN_AS_DUMMY == share_cfg_module[i].func_module)
            break;

        if(mod_name == share_cfg_module[i].func_module) {    
            //set pull attribute for module
            g_ak39_setgroup_attribute(mod_name);
            
            local_irq_save(flags);
            switch(share_cfg_module[i].share_config) {
                case SHARE_CFG1: //set share pin cfg reg1
                    val = __raw_readl(AK_SHAREPIN_CON1);
                    val &= ~(share_cfg_module[i].reg1_bit_mask);
                    val |= (share_cfg_module[i].reg1_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON1);
                    break;
                    
                case SHARE_CFG2: //set share pin cfg reg2
                    val = __raw_readl(AK_SHAREPIN_CON2);
                    val &= ~(share_cfg_module[i].reg2_bit_mask);
                    val |= (share_cfg_module[i].reg2_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON2);
                    break;
					
                case SHARE_CFG3: //set share pin cfg reg3
                    val = __raw_readl(AK_SHAREPIN_CON3);
                    val &= ~(share_cfg_module[i].reg3_bit_mask);
                    val |= (share_cfg_module[i].reg3_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON3);
                    break; 
					
                case SHARE_CFG12: 
					val = __raw_readl(AK_SHAREPIN_CON1);
                    val &= ~(share_cfg_module[i].reg1_bit_mask);
                    val |= (share_cfg_module[i].reg1_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON1);
                    
                    val = __raw_readl(AK_SHAREPIN_CON2);
                    val &= ~(share_cfg_module[i].reg2_bit_mask);
                    val |= (share_cfg_module[i].reg2_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON2);
                    break;
				case SHARE_CFG13: 
                    val = __raw_readl(AK_SHAREPIN_CON1);
                    val &= ~(share_cfg_module[i].reg1_bit_mask);
                    val |= (share_cfg_module[i].reg1_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON1);
                    
                    val = __raw_readl(AK_SHAREPIN_CON3);
                    val &= ~(share_cfg_module[i].reg3_bit_mask);
                    val |= (share_cfg_module[i].reg3_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON3);
                    break;
					
				case SHARE_CFG23: 
                    val = __raw_readl(AK_SHAREPIN_CON2);
                    val &= ~(share_cfg_module[i].reg2_bit_mask);
                    val |= (share_cfg_module[i].reg2_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON2);
                    
                    val = __raw_readl(AK_SHAREPIN_CON3);
                    val &= ~(share_cfg_module[i].reg3_bit_mask);
                    val |= (share_cfg_module[i].reg3_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON3);
                    break;
					
				case SHARE_CFG123:
                    val = __raw_readl(AK_SHAREPIN_CON1);
                    val &= ~(share_cfg_module[i].reg1_bit_mask);
                    val |= (share_cfg_module[i].reg1_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON1);
                    
                    val = __raw_readl(AK_SHAREPIN_CON2);
                    val &= ~(share_cfg_module[i].reg2_bit_mask);
                    val |= (share_cfg_module[i].reg2_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON2);
					
					val = __raw_readl(AK_SHAREPIN_CON3);
                    val &= ~(share_cfg_module[i].reg3_bit_mask);
                    val |= (share_cfg_module[i].reg3_bit_value);
                    __raw_writel(val, AK_SHAREPIN_CON3);
                    break;
					
                default:
                    break;
            }
            local_irq_restore(flags);
            return ;
        }
    }
    return ;
}
EXPORT_SYMBOL(ak_group_config);

static unsigned char gpio_assert_legal(unsigned long pin)
{
    int i, len;
    unsigned int *gpio_legal;

    if ((pin < 0) || (pin > GPIO_UPLIMIT)) {
        return AK_FALSE;
    }
    
#if defined(CONFIG_CPU_AK3910)
    len = ARRAY_SIZE(ak3910_invalid_gpio);
    gpio_legal = ak3910_invalid_gpio;
#elif defined(CONFIG_CPU_AK3916) || defined(CONFIG_CPU_AK3918)
	len = ARRAY_SIZE(ak3916_invalid_gpio);
    gpio_legal = ak3916_invalid_gpio;
#endif

    for(i = 0; i < len; i++) {
        if(gpio_legal[i] == pin)
            return AK_FALSE;    
    }
    return AK_TRUE;
}


/**
 * @brief set gpio share pin as gpio 
 * @param pin [in]  gpio pin ID
 */
int g_ak39_setpin_as_gpio(unsigned int pin)
{
    int i, bit = 0;
    unsigned long flags;

    //check param
    if(!gpio_assert_legal(pin)) {
        panic("Error, Invalid gpio %u configuration!\n", pin);
        return -1;
    }

	/* reserved uart0 confige, but provide the error info  */
    if(AK_GPIO_1 == pin || AK_GPIO_2 == pin) {
        REG32(AK_SHAREPIN_CON1) |= (0x3 << 14);
		return -1;
	}	
    
    //loop to find the correct bits to clr in share ping cfg1
    for(i = 0; i < ARRAY_SIZE(sharepin_cfg_gpio1); i++){
        if((pin >= sharepin_cfg_gpio1[i].gpio_start) 
			&& (pin <= sharepin_cfg_gpio1[i].gpio_end))
        {
            local_irq_save(flags);
			
            bit = sharepin_cfg_gpio1[i].index;
			if (sharepin_cfg_gpio1[i].flag == AS_GPIO_CFG_BIT1)
	            REG32(AK_SHAREPIN_CON1) &= ~(1 << bit);
			else if (sharepin_cfg_gpio1[i].flag == AS_GPIO_CFG_BIT2)
	            REG32(AK_SHAREPIN_CON1) &= ~(0x3 << bit);

			local_irq_restore(flags);
            return 0;
        }
    }

    //find the correct bits to clr in share ping cfg2
    if ((pin >= AK_GPIO_8) && (pin <= AK_GPIO_24)) {
        local_irq_save(flags);
        if (pin <= AK_GPIO_11)
            REG32(AK_SHAREPIN_CON2) |= (1<<(pin-8));
        else
            REG32(AK_SHAREPIN_CON2) &= ~(1<<(pin-8));
        local_irq_restore(flags);
        return 0;
    }

    //loop to find the correct bits to set in share ping cfg3
    for(i = 0; i < ARRAY_SIZE(sharepin_cfg_gpio3); i++){
        if((pin >= sharepin_cfg_gpio3[i].gpio_start) 
			&& (pin <= sharepin_cfg_gpio3[i].gpio_end))
        {
			local_irq_save(flags);
			
            bit = sharepin_cfg_gpio3[i].index;
			if (sharepin_cfg_gpio3[i].flag == AS_GPIO_CFG_BIT1)
				REG32(AK_SHAREPIN_CON3) &= ~(1 << bit);
			else if (sharepin_cfg_gpio3[i].flag == AS_GPIO_CFG_BIT2)
	            REG32(AK_SHAREPIN_CON3) &= ~(0x3 << bit);
			
            local_irq_restore(flags);
            return 0;
        }
    }
    return 0;
}

/*
    enable: 1:enable pullup 0:disable pullup function
      if the pin is attached pullup and pulldown resistor, then writing 1 to enable
        pullup, 0 to enable pulldown, if you want to disable pullup/pulldown, then 
        disable the PE parameter
*/
int g_ak39_gpio_pullup(unsigned int pin, unsigned char enable)
{
    void __iomem *base = AK_PPU_PPD_BASE(pin);
    unsigned long flags;
	int i;
	
   if(!gpio_assert_legal(pin)) {
        panic("Error, Invalid gpio %u configuration!\n", pin);
        return -1;
    }

	for (i = 0; i < ARRAY_SIZE(pupd_cfg_info); i++) {
		if (pin == pupd_cfg_info[i].pin) {

			if (pupd_cfg_info[i].pupd_type == PULLDOWN)
				panic("Invalid GPIO[%d] pullup config.\n", pin);
			
			switch(pupd_cfg_info[i].pupd_cfg) {
				case PUPD_CFG1:
					base = AK_PPU_PPD1;
					break;
				case PUPD_CFG2:
					base = AK_PPU_PPD2;
					break;
				case PUPD_CFG3:
					base = AK_PPU_PPD3;
					break;
			}
			
			local_irq_save(flags);
			if (enable == AK_PULLUP_ENABLE)
				REG32(base)	&= ~(1 << pupd_cfg_info[i].index);
			else if (enable == AK_PULLUP_DISABLE)
				REG32(base)	|= (1 << pupd_cfg_info[i].index);
			local_irq_restore(flags);
			break;
		}
	}

	return 0;
}

//1.enable pulldown 0.disable pulldown
 int g_ak39_gpio_pulldown(unsigned int pin, unsigned char enable)
{
    void __iomem *base = AK_PPU_PPD_BASE(pin);
    unsigned long flags;
	int i;
    
    if(!gpio_assert_legal(pin)) {
        panic("Error, Invalid gpio %u configuration!\n", pin);
        return -1;
    }
    
	for (i = 0; i < ARRAY_SIZE(pupd_cfg_info); i++) {
		if (pin == pupd_cfg_info[i].pin) {

			if (pupd_cfg_info[i].pupd_type == PULLUP)
				panic("Invalid GPIO[%d] pulldown config.\n", pin);
			
			switch(pupd_cfg_info[i].pupd_cfg) {
				case PUPD_CFG1:
					base = AK_PPU_PPD1;
					break;
				case PUPD_CFG2:
					base = AK_PPU_PPD2;
					break;
				case PUPD_CFG3:
					base = AK_PPU_PPD3;
					break;
			}
			
			local_irq_save(flags);
			if (enable == AK_PULLDOWN_ENABLE) 
				REG32(base)	&= ~(1 << pupd_cfg_info[i].index);
			else if (enable == AK_PULLDOWN_DISABLE)
				REG32(base)	|= (1 << pupd_cfg_info[i].index);
			local_irq_restore(flags);
			break;
		}
	}

	return 0; 
}

void g_ak39_setgroup_attribute(T_GPIO_SHAREPIN_CFG mod_name)
{
    unsigned long pin, start_pin = 0, end_pin = 0;
    
    switch (mod_name) {
        case ePIN_AS_MCI:
            start_pin = 31, end_pin = 36;
            for (pin = start_pin; pin <= end_pin; pin++)
            {         
                g_ak39_gpio_pullup(pin, AK_TRUE);
            }
            break; 

		case ePIN_AS_MCI_8LINE:
            start_pin = 31, end_pin = 40;
            for (pin = start_pin; pin <= end_pin; pin++)
            {         
                g_ak39_gpio_pullup(pin, AK_TRUE);
            }
            break;
			
	    case ePIN_AS_SDIO:
            start_pin = 41, end_pin = 46;
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_TRUE);
            }
            break;
			
        case ePIN_AS_SPI1:
			/* spi_clk  #spi_cs */
            start_pin = 25, end_pin = 26;
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_TRUE);
            }

			start_pin = 43, end_pin = 46;
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_TRUE);
            }
			
            break;
			
        case ePIN_AS_SPI2:
			/* spi_clk  #spi_cs */
            start_pin = 29, end_pin = 30;
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_TRUE);
            }
			
            start_pin = 43, end_pin = 46;
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_TRUE);
            }
            break;  
			
        case ePIN_AS_I2S:
            start_pin = 52, end_pin = 53;
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_FALSE);   
            }
            g_ak39_gpio_pullup(AK_GPIO_57, AK_FALSE);
			g_ak39_gpio_pulldown(AK_GPIO_55, AK_FALSE);
			g_ak39_gpio_pullup(AK_GPIO_55, AK_FALSE);
            break;
            
        case ePIN_AS_UART1: 
            start_pin = 1, end_pin = 2;
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_TRUE);
            }
            break;
            
        case ePIN_AS_UART2: 
            start_pin = 4, end_pin = 7;           
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_TRUE);                
            }   
            break;
            
        case ePIN_AS_I2C:
            start_pin = 27, end_pin = 28;
            for (pin = start_pin; pin <= end_pin; pin++)
            {
                g_ak39_gpio_pullup(pin, AK_FALSE);
            }
            break;
        default:
            break;
    }
}

/* 
 * configuration gpio pin
 * 0: corresponding port is input mode
 * 1: corresponding port is output mode
 */
int g_ak39_gpio_cfgpin(unsigned int pin, unsigned int to)
{
    void __iomem *base = AK_GPIO_DIR_BASE(pin);
    unsigned int offset = ((pin) & 31);
    unsigned long flags;
    
     if(!gpio_assert_legal(pin)) {
        panic("Error, Invalid gpio %d configuration!\n", pin);
        return -1;
    }

	/* gpio[49] can't set output mode for ak39xx*/
	if ((to == AK_GPIO_DIR_OUTPUT)&&(pin == AK_GPIO_49)) {
		panic("Error, gpio %d isn't config output mode\n", pin);
		return -1;
	}

    local_irq_save(flags);
    if (AK_GPIO_DIR_INPUT == to)
        REG32(base) &= ~(1 << offset);
    else if (AK_GPIO_DIR_OUTPUT == to)
        REG32(base) |= (1 << offset);
    local_irq_restore(flags);
	
    return 0;
}

/* hold the real-time output value from GPIO[x] */ 
int g_ak39_gpio_setpin(unsigned int pin, unsigned int to)
{
    void __iomem *base = AK_GPIO_OUT_BASE(pin);
    unsigned int offset = ((pin) & 31);
    unsigned long flags;
        
    if(!gpio_assert_legal(pin)) {
        panic("Error, Invalid gpio %d configuration!\n", pin);
        return -1;
    }

	/* gpio[49] can't set output level for ak39xx*/
	if (pin == AK_GPIO_49) {
		panic("Error, gpio %d isn't config outpu level\n", pin);
		return -1;
	}

    local_irq_save(flags);
    if (AK_GPIO_OUT_LOW == to)
        REG32(base) &= ~(1 << offset);
    else if (AK_GPIO_OUT_HIGH == to)
        REG32(base) |= (1 << offset);
    local_irq_restore(flags);
	
    return 0;
}


/* hold the real-time input value of GPIO[x] */ 
 int g_ak39_gpio_getpin(unsigned int pin)
{
    void __iomem *base = AK_GPIO_IN_BASE(pin);
    unsigned int offset = ((pin) & 31);

    if(!gpio_assert_legal(pin)) {
        panic("Error, read invalid gpio %d status!\n", pin);
        return -1;
    }
    return ((__raw_readl(base) & (1 << offset)) == (1 << offset));
}


/* 
 * enalbe/disable the interrupt function of GPIO[X]
 * 1: interrupt function of corresponding port is enable
 * 0: interrupt function of corresponding port is disable
 */
int g_ak39_gpio_inten(unsigned int pin, unsigned int enable)
{
    void __iomem *base = AK_GPIO_INTEN_BASE(pin);
    unsigned int offset = ((pin) & 31);
    unsigned long flags;

    if(!gpio_assert_legal(pin)) {
        panic("Error, invalid gpio %d!\n", pin);
        return -1;
    }
    
    local_irq_save(flags);
    if (AK_GPIO_INT_ENABLE == enable)
        REG32(base) |= (1 << offset);
    else if (AK_GPIO_INT_DISABLE == enable)
        REG32(base) &= ~(1 << offset);
    local_irq_restore(flags);
	
    return 0;
}


/*
 * interrupt polarity selection
 * 0: the input interrupt polarity of GPIO[X] is active high
 * 1: the input interrupt polarity of GPIO[X] is active low
 */
int g_ak39_gpio_intpol(unsigned int pin, unsigned int level)
{
    void __iomem *base = AK_GPIO_INTPOL_BASE(pin);
    unsigned int offset = ((pin) & 31);
    unsigned long flags;

    if(!gpio_assert_legal(pin)) {
        panic("Error, invalid gpio %d!\n", pin);
        return -1;
    }
    
    local_irq_save(flags);
    if (AK_GPIO_INT_HIGHLEVEL == level)
        REG32(base) &= ~(1 << offset);
    else if (AK_GPIO_INT_LOWLEVEL == level)
        REG32(base) |= (1 << offset);
    local_irq_restore(flags);
	
    return 0;
}

int reg_set_mutli_bit(void __iomem *reg, unsigned int value, int bit, int index)
{
	unsigned long con, flags;

	if (bit <= 0)
		return -1;
	
	local_irq_save(flags);
	
	con = __raw_readl(reg);
	con &= ~(((1 << bit) - 1) << index);
	con |= (value << index);
	__raw_writel(con, reg);
	
	local_irq_restore(flags);
	return 0;
}
EXPORT_SYMBOL(reg_set_mutli_bit);

int g_ak39_gpio_to_irq(unsigned int pin)
{
    if(!gpio_assert_legal(pin)) {
        panic("Error, invalid gpio %d!\n", pin);
        return -1;
    }
    return (IRQ_GPIO_0 + (pin - AK_GPIO_0));
}

int g_ak39_irq_to_gpio(unsigned int irq)
{
    return (AK_GPIO_0 + (irq - IRQ_GPIO_0));
}


#if 0
static const char *ak39_gpio_list[AK_GPIO_MAX];

int ak_gpio_request(unsigned long gpio, const char *label)
{
    if (gpio > GPIO_UPLIMIT)
        return -EINVAL;
    
    if (ak39_gpio_list[gpio])
        return -EBUSY;
    
    if (label)
        ak39_gpio_list[gpio] = label;
    else
        ak39_gpio_list[gpio] = "busy";
    
    return 0;
}
EXPORT_SYMBOL(ak_gpio_request);

void ak_gpio_free(unsigned long gpio)
{
    BUG_ON(!ak39_gpio_list[gpio]);
    
    ak39_gpio_list[gpio] = NULL;
}
EXPORT_SYMBOL(ak_gpio_free);

#else

int ak_gpio_request(unsigned long gpio, const char *label)
{
    return 0;
}
EXPORT_SYMBOL(ak_gpio_request);

void ak_gpio_free(unsigned long gpio)
{
}
EXPORT_SYMBOL(ak_gpio_free);

#endif

