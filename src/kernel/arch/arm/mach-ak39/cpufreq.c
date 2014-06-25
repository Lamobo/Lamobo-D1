/* arch/arm/mach-ak39/cpufreq.c
 *
 * AK98 CPUfreq Support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/anyka_cpufreq.h>
#include <linux/dma-mapping.h>
#include <linux/freezer.h>
#include <linux/syscalls.h>

#if 0
#include <mach/cpufreq.h>
#include <mach/clock.h>

#define SIZE_1K 0x00000400      /* 1K */
extern atomic_t suspend_flags;

static struct cpufreq_freqs freqs;
static T_OPERATION_MODE current_mode;
static int previous_mode_flag;
static int cpufreq_in_ddr2 = 1;
static unsigned int clkdiv = 1000000;

typedef enum {
	CPU_LOW_MODE = 0,
	CPU_NORMAL_MODE,
	CPU_NORMAL_2XMODE,
} T_cpufreq_mode;

#if 1
struct cpufreq_mode_clkdiv cpufreq_divs[] = {
	//mode_name  pll_sel  clk168_div  cpu_div  mem_div  asic_div  low_clock is_3x  (cpu,mem,asic[MHz])
	{LOW_MODE_CLOCK_0, 	0x2d,	0,	0,	2,	4,	1,	0}, /* (90,180,90) */
	{LOW_MODE_CLOCK_1, 	0x2d,	0,	0,	2,	4,	1,	0}, /* (90,180,90) */
	{LOW_MODE_CLOCK_2, 	0x2d,	0,	0,	2,	4,	1,	0}, /* (90,180,90) */
	{LOW_MODE_CLOCK_3, 	0x2d,	0,	0,	2,	4,	1,	0}, /* (90,180,90) */
	{LOW_MODE_CLOCK_4, 	0x2d,	0,	0,	2,	4,	1,	0}, /* (90,180,90) */
	{LOW_MODE_CLOCK_5, 	0x2d,	0,	0,	2,	4,	1,	0}, /* (90,180,90) */
	{LOW_MODE_CLOCK_6, 	0x2d,	0,	0,	2,	4,	1,	0}, /* (90,180,90) */
	{LOW_MODE_CLOCK_7, 	0x2d,	0,	0,	2,	4,	1,	0}, /* (90,180,90) */ 
	
	{NORMAL_MODE_CLOCK_0, 0x2d,	 0,	0,	2,	4,	0,	0}, /* (180,180,90) */
	{NORMAL_MODE_CLOCK_1, 0x2d,	 0,	0,	2,	4,	0,	0}, /* (180,180,90) */
	{NORMAL_MODE_CLOCK_2, 0x2d,  0,	0,	2,	4, 	0,	0}, /* (180,180,90) */
	{NORMAL_MODE_CLOCK_3, 0x2d,  0,	0,	2,	4,	0,	0}, /* (180,180,90) */
	{NORMAL_MODE_CLOCK_4, 0x2d,  0,	0,	2,	4, 	0,	0}, /* (180,180,90) */
	{NORMAL_MODE_CLOCK_5, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{NORMAL_MODE_CLOCK_6, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{NORMAL_MODE_CLOCK_7, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */    

	{VIDEO_MODE_CLOCK_0, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{VIDEO_MODE_CLOCK_1, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{VIDEO_MODE_CLOCK_2, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{VIDEO_MODE_CLOCK_3, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{VIDEO_MODE_CLOCK_4, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{VIDEO_MODE_CLOCK_5, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{VIDEO_MODE_CLOCK_6, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{VIDEO_MODE_CLOCK_7, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	{LOWBATTERY_MODE_CLOCK, 0x2d,	 0,	0,	2,	4,	0,	0}, /* (180,180,90) */
	{USBPLUG_MODE_CLOCK, 0x2d,  0,	1,	2,	4, 	0,	0}, /* (360,180,90) */
	
};
#else
struct cpufreq_mode_clkdiv cpufreq_divs[] = {
    //mode_name  pll_sel  clk168_div  cpu_div  mem_div  asic_div  low_clock is_3x  (cpu,mem,asic[MHz])
    {LOW_MODE_CLOCK_0,  0x14,	0,  0,  2,  4,  1,  0}, /* (65,130,65) */
    {LOW_MODE_CLOCK_1,  0x14,	0,  0,  2,  4,  0,  0}, /* (65,130,65) */
    {LOW_MODE_CLOCK_2,  0x14,	0,  1,  2,  4,  0,  0}, /* (130,130,65) */
    {LOW_MODE_CLOCK_3,  0x14,	0,	0,  2,  2,  0,  0}, /* (130,130,65) */
    {LOW_MODE_CLOCK_4,  0x28,	0,  0,  2,  4,  0,  0}, /* (170,170,85) */
    {LOW_MODE_CLOCK_5,  0x2D,	0,  0,  2,  4,  0,  0}, /* (180,180,90) */
    {LOW_MODE_CLOCK_6,  0x37,	0,  0,  2,  4,  0,  0}, /* (200,200,100) */
    {LOW_MODE_CLOCK_7,  0x37,	0,  1,  2,  4,  0,  0}, /* (400,200,100) */

    {NORMAL_MODE_CLOCK_0, 0x37,  0, 0,  2,  4,  0,  0}, /* (200,200,100) */
    {NORMAL_MODE_CLOCK_1, 0x37,  0, 0,  2,  4,  0,  0}, /* (200,200,100) */
    {NORMAL_MODE_CLOCK_2, 0x37,  0, 0,  2,  4,  0,  0}, /* (200,200,100) */
    {NORMAL_MODE_CLOCK_3, 0x37,  0, 0,  2,  4,  0,  0}, /* (200,200,100) */
    {NORMAL_MODE_CLOCK_4, 0x37,  0, 0,  2,  4,  0,  0}, /* (200,200,100) */
    {NORMAL_MODE_CLOCK_5, 0x37,  0, 1,  2,  4,  0,  0}, /* (400,200,100) */
    {NORMAL_MODE_CLOCK_6, 0x37,  0, 1,  2,  4,  0,  0}, /* (400,200,100) */
    {NORMAL_MODE_CLOCK_7, 0x37,  0, 1,  2,  4,  0,  0}, /* (400,200,100) */

    {VIDEO_MODE_CLOCK_0, 0x36,  0,  1,  0,  0,  0,  1}, /* (396,132,132) */
    {VIDEO_MODE_CLOCK_1, 0x36,  0,  1,  0,  0,  0,  1}, /* (396,132,132) */
    {VIDEO_MODE_CLOCK_2, 0x36,  0,  1,  0,  0,  0,  1}, /* (396,132,132) */
    {VIDEO_MODE_CLOCK_3, 0x36,  0,  1,  0,  0,  0,  1}, /* (396,132,132) */
    {VIDEO_MODE_CLOCK_4, 0x36,  0,  1,  0,  0,  0,  1}, /* (396,132,132) */
    {VIDEO_MODE_CLOCK_5, 0x36,  0,  1,  0,  0,  0,  1}, /* (396,132,132) */
    {VIDEO_MODE_CLOCK_6, 0x36,  0,  1,  0,  0,  0,  1}, /* (396,132,132) */
    {VIDEO_MODE_CLOCK_7, 0x36,  0,  1,  0,  0,  0,  1}, /* (396,132,132) */
};
#endif

static T_OPERATION_MODE prev_suspend_mode;
#define SUSPEND_NORMAL_MODE		NORMAL_MODE_CLOCK_2

static int get_cpufreq_mode_clkdiv(T_OPERATION_MODE mode, struct cpufreq_mode_clkdiv *clkdiv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cpufreq_divs); i++) {
		if (cpufreq_divs[i].mode_name == mode) {
			*clkdiv = cpufreq_divs[i];
			return 0;
		}
	}
	return -1;
}

static unsigned int calcue_power(unsigned int num)
{
    unsigned int i;

    if(num < 0)
        return -1;
    if((num == 2)||(num == 0))
        return 0;

    for(i = 0; num % 2 == 0; i++){
        num /= 2;
    }
    if(num > 2)
        return -1;

    return i;
}

static int current_is_lowmode(struct cpufreq_mode_clkdiv *cpufreq)
{
    if (freqs.old_cpufreq.low_clock == 1)
        return 1;
    else
        return 0;
}

static int new_is_lowmode(struct cpufreq_mode_clkdiv *cpufreq)
{
    if ((cpufreq->low_clock == 1))
        return 1;
    else
        return 0;
}

static int new_is_2x_mode(struct cpufreq_mode_clkdiv *cpufreq)
{
    if ((cpufreq->cpu_div == 1))
        return 1;
    else
        return 0;
}

static void config_asicclk_parameter(struct cpufreq_mode_clkdiv *cpufreq,
	unsigned long *ratio)
{
	unsigned long clk_ratio = *ratio;
	unsigned int asicdiv;
		
	asicdiv = calcue_power(cpufreq->asic_div);
	if( asicdiv < 0)
		printk("Error, calcue asic_div.");

	clk_ratio &= ~(0x7 << 6);
	clk_ratio |= ((asicdiv << 6) | CLOCK_ASIC_MEM_ENA);

	*ratio = clk_ratio;
}

static void config_clock_parameter(struct cpufreq_mode_clkdiv *cpufreq,
	unsigned long *ratio)
{
	unsigned long clk_ratio = *ratio;
	unsigned int memdiv, asicdiv;

	memdiv = calcue_power(cpufreq->mem_div);
	if( memdiv < 0)
		printk("Error, calcue mem_div.");
	asicdiv = calcue_power(cpufreq->asic_div);
	if( asicdiv < 0)
		printk("Error, calcue asic_div.");
	
	clk_ratio &= ~((1 << 28)|(1 << 22)|(0xF << 17)|(1 << 15)|(0x7 << 9)|(0x7 << 6)|(0x3F << 0));

	clk_ratio |= (cpufreq->is_3x << 28);		//is 3x ?
	clk_ratio |= (cpufreq->clk168_div << 17);	//clk168 div
	clk_ratio |= (cpufreq->cpu_div << 15);		//cpu clk = mem clk or cpu clk = asic clk
	clk_ratio |= (cpufreq->low_clock << 22);	//cpu clk = asic clk
	clk_ratio |= (memdiv << 9)|(asicdiv << 6);	//mem div and asic div
	clk_ratio |= (cpufreq->pll_sel << 0);		//pll_sel
	clk_ratio |= PLL_CHANGE_ENA;
	
	*ratio = clk_ratio;
}

/*
 *  *function: enter L2 modify register parameters of clock for change sys clcok
 *   */
void L2_LINK(freqchange) L2FUNC_NAME(freqchange)(unsigned long param1,
	unsigned long param2,unsigned long param3,unsigned long param4)
{
	DISABLE_CACHE_MMU();

    // check this bit and unitil both are empty
    while(!((REG32(PHY_RAM_CFG_REG4) & (FIFO_R_EMPTY | FIFO_CMD_EMPTY)) == (FIFO_R_EMPTY | FIFO_CMD_EMPTY)))
        ;

    // setup periodic of refresh interval and disable auto-refresh
    REG32(PHY_RAM_CFG_REG4) = REFRESH_PERIOD_INTERVAL;

    DDR2_ENTER_POWERDOWN();
    // after send enter self - refresh, delay stable clock at least more than 2 tck
    PM_DELAY(0x4);

    //disable ram clock
    REG32(PHY_CLOCK_CTRL_REG) |= (1<<10);

    // other mode change to normal mode
    //cpu clock from other mode to normal
	REG32(PHY_CLOCK_DIV_REG) &= ~((1 << 28)|(1 << 22));
	PM_DELAY(0x100);

    //set clock div and check pll[12]
    REG32(PHY_CLOCK_DIV_REG) = param1;
	while (REG32(PHY_CLOCK_DIV_REG) & PLL_CHANGE_ENA);
	PM_DELAY(0x10);
	
    //enable ram clock
    REG32(PHY_CLOCK_CTRL_REG) &= ~(1<<10);
    // new clock stable at least more than 1tck,here is ignore because follow has few instruction.

    // softreset ddr2 memory controller
    REG32(0x0800000c) |= (0x1 << 26);
    PM_DELAY(0x10);
    REG32(0x0800000c) &= ~(0x1 << 26);

    // re-init ram controller
    REG32(0x2000e05c) = 0x00000200; // bypass DCC
    REG32(0x2000e078) = 0x43020100; // initial sstl = 00(12ma), tsel = 10(150ohm)
    REG32(0x2000e000) = 0x00004e90; // 32 bit bus width

    // exit precharge power-down mode before delay at least 1 tck
    PM_DELAY(10);
    DDR2_EXIT_POWERDOWN();

    // load mr, reset dll and delay for 200 tck, and set odt high
    REG32(PHY_RAM_CPU_CMD) = 0x02800532;
    // send nop, delay for 200 tck for dll reset,
    PM_DELAY(0x10);

    // load mr, clean reset dll and remain odt low in ddr2 memory
    REG32(PHY_RAM_CPU_CMD) = 0x1a800432;

    //enable dll and wate for dll stable in ram controller
    REG32(0x2000e020) = 0x00000003;
    while (!(REG32(0x2000e020) & (1 << 2)));

    // open auto-referesh
    // default as mclk = 120mhz for calc tck=8.3ns, trefi=7.7us
    REG32(0x2000e00c) = (0x39f<<1)|0x1;

    //calibration sart and wait for finish
    REG32(0x2000e024) = ((param2>>0x7)<<10)|0x1;
    while (!(REG32(0x2000e024) & (1 << 1)));

    ENABLE_CACHE_MMU();
}

/*
 *function: change pll clock, include mem clock,asic clock and cpu clock.
 */
static void cpufreq_change_clocks(struct cpufreq_mode_clkdiv *cpufreq)
{
    unsigned long ratio;
    void *addr;
    unsigned long phy_addr;
	
	addr = kzalloc(512, GFP_KERNEL | GFP_ATOMIC);
    if (addr == NULL)
        return ;
    phy_addr = virt_to_phys(addr);
	
    ratio = REG32(CLOCK_DIV_REG);
    config_clock_parameter(cpufreq, &ratio);
	
    SPECIFIC_L2BUF_EXEC(freqchange, ratio, phy_addr,0,0);
	
    kfree(addr);
}

/*
 *function: according to needed new clocks and determine branch of cpufreq
 * @cpufreq: structure include mode name and clock div
 * note: the mode enter L2 cpufreq
 */
static int l2_cpu_freq_change(struct cpufreq_mode_clkdiv *cpufreq)
{
	int error;
	
	error = usermodehelper_disable();
	if (error)
		goto Finish;

	// freeze process and kernel task.
	error = cpufreq_freeze_processes();
	if (error)
		goto freeze;

	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);	
	cpufreq_change_clocks(cpufreq);
	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

	thaw_processes();
	usermodehelper_enable();
	
	return 0;
	
freeze:
	thaw_processes();
Finish:
	usermodehelper_enable();
	return error;
}

static void ddr2_transfrom_clock_mode(T_cpufreq_mode state)
{
	unsigned long ratio;
	
	ratio = REG32(CLOCK_DIV_REG);
	ratio &= ~(1 << 22);
	switch (state) {
		case CPU_LOW_MODE:
			ratio |= (1 << 22); //|(1 << 14);
			ratio &= ~(1 << 15);
			break;
		case CPU_NORMAL_MODE:
			ratio &= ~(1 << 15);
			break;
		case CPU_NORMAL_2XMODE:
			ratio |= (1 << 15);
			break;
		default:
			printk("CPUFREQ: need mode is error.\n");
			break;
	}
	REG32(CLOCK_DIV_REG) = ratio;
}

/*
 *function: according to needed new clocks and determine branch of cpufreq
 * @cpufreq: structure include mode name and clock div
 * note: the mode cpufreq in ddr2
 */
static int ddr2_cpu_freq_change(struct cpufreq_mode_clkdiv *cpufreq)
{
	unsigned long ratio, flags;
	unsigned long cpuid = 0;
	
    cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
	local_irq_save(flags);
	
    // check if cpu clock from other mode to normal and cutover to normal
    if (current_is_lowmode(cpufreq)) {
		ddr2_transfrom_clock_mode(CPU_NORMAL_MODE);
        //cpuid = REG32(CPU_CHIP_ID);  // used as delay
        udelay(10);
    }
	
	// change asic clock 
	if (freqs.old_cpufreq.asic_clk != freqs.new_cpufreq.asic_clk) {
		ratio =  REG32(CLOCK_DIV_REG);
		config_asicclk_parameter(cpufreq, &ratio);
		
		REG32(CLOCK_DIV_REG) = ratio;
		while (REG32(CLOCK_DIV_REG) & CLOCK_ASIC_MEM_ENA);
	}
	
	// change cpu clock
    if (freqs.old_cpufreq.cpu_clk != freqs.new_cpufreq.cpu_clk) {
        if (new_is_2x_mode(cpufreq))
			ddr2_transfrom_clock_mode(CPU_NORMAL_2XMODE);
        else if (!new_is_2x_mode(cpufreq)) {
            if (new_is_lowmode(cpufreq)) 
				ddr2_transfrom_clock_mode(CPU_LOW_MODE);
			else if (!current_is_lowmode(cpufreq))
            	ddr2_transfrom_clock_mode(CPU_NORMAL_MODE);
        }
		udelay(10);
    }
	
	local_irq_restore(flags);
	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	
	return 0;
}

static int cpu_freq_change(struct cpufreq_mode_clkdiv *cpufreq)
{
	int error;
	static int l2_cpufreq_flag = 0;
	
	if (freqs.old_cpufreq.pll_sel == freqs.new_cpufreq.pll_sel) {	
		if (current_is_lowmode(cpufreq) && l2_cpufreq_flag) {
			error = l2_cpu_freq_change(cpufreq);
			if (error)
				goto exit_cpufreq;
			
			cpufreq_in_ddr2 = 0;
		} else {
			error = ddr2_cpu_freq_change(cpufreq);
			if (error < 0)
				goto exit_cpufreq;
			
			cpufreq_in_ddr2 = 1;
			l2_cpufreq_flag = 0;
		}
	} else {
		error = l2_cpu_freq_change(cpufreq);
		if (error) 
			goto exit_cpufreq;
		
		cpufreq_in_ddr2 = 0;
		l2_cpufreq_flag = 1;
	}
	
	return 0;
	
exit_cpufreq:
	return error;
}

/*
 * change from low mode to normal mode for suspend (low mode --> normal mode)
 */
void cpu_freq_suspend_check(void)
{
	struct cpufreq_mode_clkdiv cpufreq;
	unsigned long ratio;
	
	// change to normal mode
	if (freqs.old_cpufreq.low_clock == 1) {
		if (cpufreq_in_ddr2) {
			ddr2_transfrom_clock_mode(CPU_NORMAL_MODE);
			udelay(10);
		} else {
			// save low mode before suspend change normal
			prev_suspend_mode = current_mode;
			
			if(!get_cpufreq_mode_clkdiv(SUSPEND_NORMAL_MODE, &cpufreq)){
				cpufreq_change_clocks(&cpufreq);
			}
		}
	}
}
EXPORT_SYMBOL(cpu_freq_suspend_check);

/*
 * restore low mode after resume (normal mode --> low mode)
 */
void cpu_freq_resume_check(void)
{
	struct cpufreq_mode_clkdiv cpufreq;
	unsigned long ratio;

	// chang to low mode 
	if (freqs.old_cpufreq.low_clock == 1) {
		if (cpufreq_in_ddr2) {
			ddr2_transfrom_clock_mode(CPU_LOW_MODE);
			udelay(10);
		} else {
			if(!get_cpufreq_mode_clkdiv(prev_suspend_mode, &cpufreq)){
				cpufreq_change_clocks(&cpufreq);
			}
		}
	}
}
EXPORT_SYMBOL(cpu_freq_resume_check);

static void info_clock_value(void)
{
	printk("Cpufreq: system clocks(unit:MHz)[cpu,mem,asic] from (%d,%d,%d) to (%d,%d,%d)\n",
		freqs.old_cpufreq.cpu_clk/clkdiv, freqs.old_cpufreq.mem_clk/clkdiv,
		freqs.old_cpufreq.asic_clk/clkdiv, ak98_get_cpu_clk()/clkdiv, 
		ak98_get_mem_clk()/clkdiv, ak98_get_asic_clk()/clkdiv);
}

static int clock_need_changing(void)
{
	if((freqs.new_cpufreq.cpu_clk == freqs.old_cpufreq.cpu_clk)&&
	  (freqs.new_cpufreq.mem_clk == freqs.old_cpufreq.mem_clk)&&
	  (freqs.new_cpufreq.asic_clk == freqs.old_cpufreq.asic_clk))
		return 0;
	else
		return 1;
}

static unsigned int get_asic_clk(struct cpufreq_mode_clkdiv *cpufreq)
{
    unsigned int asicclk;

    if(cpufreq->is_3x)
        asicclk = ((PLL_CLK_MIN+(cpufreq->pll_sel*4))/(cpufreq->clk168_div+1))/3;
    else
        asicclk = ((PLL_CLK_MIN+(cpufreq->pll_sel*4))/(cpufreq->clk168_div+1))/cpufreq->asic_div;

    return asicclk;
}

static unsigned int get_mem_clk(struct cpufreq_mode_clkdiv *cpufreq)
{
	unsigned int memclk;
	
    if(cpufreq->is_3x)
        memclk = ((PLL_CLK_MIN+(cpufreq->pll_sel*4))/(cpufreq->clk168_div+1))/3;
    else
        memclk = ((PLL_CLK_MIN+(cpufreq->pll_sel*4))/(cpufreq->clk168_div+1))/cpufreq->mem_div;

    return memclk;
}

static unsigned int get_cpu_clk(struct cpufreq_mode_clkdiv *cpufreq)
{
    unsigned int cpuclk;

    if(cpufreq->is_3x) {
        cpuclk = (PLL_CLK_MIN+(cpufreq->pll_sel*4))/(cpufreq->clk168_div+1);
    } else {
        if(cpufreq->cpu_div) {
            cpuclk = (PLL_CLK_MIN+(cpufreq->pll_sel*4))/(cpufreq->clk168_div+1);
        } else {
            if(cpufreq->low_clock)
                cpuclk = get_asic_clk(cpufreq);
            else
                cpuclk = get_mem_clk(cpufreq);
        }
    }
	
    return cpuclk;
}

static void update_current_clock(void)
{
    freqs.old_cpufreq.cpu_clk = freqs.new_cpufreq.cpu_clk;
    freqs.old_cpufreq.mem_clk = freqs.new_cpufreq.mem_clk;
    freqs.old_cpufreq.asic_clk = freqs.new_cpufreq.asic_clk;
	freqs.old = freqs.new;
    freqs.old_cpufreq.pll_sel = freqs.new_cpufreq.pll_sel;
	freqs.old_cpufreq.low_clock = freqs.new_cpufreq.low_clock;
}

static void get_newmode_clock(struct cpufreq_mode_clkdiv *cpufreq)
{	
	freqs.new_cpufreq.cpu_clk = get_cpu_clk(cpufreq)*clkdiv;
	freqs.new_cpufreq.mem_clk = get_mem_clk(cpufreq)*clkdiv;
	freqs.new_cpufreq.asic_clk = get_asic_clk(cpufreq)*clkdiv;
	freqs.new = freqs.new_cpufreq.cpu_clk;	
	freqs.new_cpufreq.pll_sel = cpufreq->pll_sel;
	freqs.new_cpufreq.low_clock = cpufreq->low_clock;
}

void update_pre_mode(void)
{
	// assign to save old mode
	previous_mode_flag = freqs.old_cpufreq.low_clock;
}

unsigned int get_pll_sel(T_OPERATION_MODE state)
{
    int i, len;

    len = ARRAY_SIZE(cpufreq_divs);
    for (i = 0; i < len; i++) {
        if (state == cpufreq_divs[i].mode_name)
            break;
    }
    if (likely(i < len))
        return cpufreq_divs[i].pll_sel;

    return -1;
}
EXPORT_SYMBOL(get_pll_sel);

int current_mode_is_low_mode(void)
{
	return freqs.old_cpufreq.low_clock;
}
EXPORT_SYMBOL(current_mode_is_low_mode);

/*
 *function: get system boot's mode
 */
T_OPERATION_MODE get_current_mode(void)
{
	return current_mode;
}
EXPORT_SYMBOL(get_current_mode);

/* function: enter change cpufreq
 * @state: requested mode name
 * return:
 * 	-1: if system init mode is not surpport.
 * 	  0: if cpufreq change successful.
 */
int request_cpufreq_enter(T_OPERATION_MODE state)
{
	int i, len;
	int error;

	// check if request suspending, prevent cpufreq when suspending
	if (atomic_read(&suspend_flags))
		return 0;

	if (state == current_mode) {
		//printk("requset new mode equal to current mode.\n");
		return 0;
	}

	len = ARRAY_SIZE(cpufreq_divs);
	for (i = 0; i < len; i++) {
		if (state == cpufreq_divs[i].mode_name)
			break;
	}
	if (likely(i < len)) {
		get_newmode_clock(&cpufreq_divs[i]);
		update_pre_mode();
		
		if (!clock_need_changing()) {
			//printk("Cpufreq: new mode clocks equal to old mode clocks, exit changing.\n");
			return 0;
		}
		
		error = cpu_freq_change(&cpufreq_divs[i]);
		if (error) {
			printk("CPUFREQ: request new mode changed fail. working mode is current mode.\n\n");
			goto cpufreq_err;
		}
	} else {
		printk("CPUFREQ: requset new mode is not surpport.\n");
		goto cpufreq_err;
	}
	
	info_clock_value();
	update_current_clock();
	current_mode = state;

	return 0;

cpufreq_err:
	return -1;
}
EXPORT_SYMBOL(request_cpufreq_enter);

/*
 * function: get system boot's mode
 */
static int get_init_mode(void)
{
    int i;
    unsigned int pllclk, clk168, cpuclk, memclk, asicclk;
    unsigned int pllsel, clk168div, cpudiv, memdiv, asicdiv, lowclock;

    pllclk = ak98_get_pll_clk()/clkdiv;
    clk168 = ak98_get_clk168m_clk()/clkdiv;
    cpuclk = freqs.old_cpufreq.cpu_clk/clkdiv;
    memclk = freqs.old_cpufreq.mem_clk/clkdiv;
    asicclk = freqs.old_cpufreq.asic_clk/clkdiv;
    pllsel = ((pllclk - PLL_CLK_MIN)/4) & 0x3f;

    if(pllclk == clk168)
        clk168div = 0;
    else
        clk168div = pllclk/clk168;
	
	//system is 3x mode
    if((cpuclk / memclk == 3)&&(cpuclk / asicclk == 3)&&
       (cpuclk % memclk == 0)&&(cpuclk % asicclk == 0)) {
        for(i = 0; i < ARRAY_SIZE(cpufreq_divs); i++) {
            if((cpufreq_divs[i].is_3x == 1) &&
              (cpufreq_divs[i].clk168_div == clk168div) &&
              (cpufreq_divs[i].pll_sel == pllsel)){

                current_mode = cpufreq_divs[i].mode_name;
				freqs.old_cpufreq.low_clock = cpufreq_divs[i].low_clock;
                return 0;
            }
        }
        return -1;
    }

	//system is normal mode
	if(cpuclk == clk168) {
		lowclock = 0;
		cpudiv = 1;
	} else if(cpuclk == memclk) {
		lowclock = 0;
		cpudiv = 0;
	} else if(cpuclk == asicclk) {
		lowclock = 1;
		cpudiv = 0;
	}
	memdiv = clk168/memclk;
	asicdiv = clk168/asicclk;

	for(i = 0;	i < ARRAY_SIZE(cpufreq_divs); i++){
		if((cpufreq_divs[i].cpu_div == cpudiv)	&&
		  (cpufreq_divs[i].low_clock == lowclock)&&
		  (cpufreq_divs[i].mem_div == memdiv)	&&
		  (cpufreq_divs[i].asic_div == asicdiv) &&
		  (cpufreq_divs[i].pll_sel == pllsel)	&&
		  (cpufreq_divs[i].clk168_div == clk168div)){

			current_mode = cpufreq_divs[i].mode_name;
			freqs.old_cpufreq.low_clock = cpufreq_divs[i].low_clock;
			return 0;
		}
	}
	return -1;
}

static void cpufreq_operation_init(void)
{
    int i, error;
    unsigned int tmp;

    freqs.old_cpufreq.cpu_clk = ak98_get_cpu_clk();
    freqs.old_cpufreq.mem_clk = ak98_get_mem_clk();
    freqs.old_cpufreq.asic_clk = ak98_get_asic_clk();
    freqs.old = freqs.old_cpufreq.cpu_clk;

    freqs.flags = 0;

    tmp = cpufreq_divs[0].pll_sel;
    for(i = 1; i < ARRAY_SIZE(cpufreq_divs); i++){
        if(cpufreq_divs[i].pll_sel > tmp)
            tmp = cpufreq_divs[i].pll_sel;
    }
    freqs.old_cpufreq.pll_sel = tmp;
	
	error = get_init_mode();
	if(error < 0)
		current_mode = error;

    return;
}

/* ak98_cpufreq_init
 *
 * Attach the cpu frequence  scaling functions. This should be called
 * from the board specific initialisation if the board supports
 * it.
*/
int __init ak98_cpufreq_init(void)
{
	printk("AK98 cpu frequence change support, (c) 2011 ANYAK\n");	
	cpufreq_operation_init();

	return 0;
}
module_init(ak98_cpufreq_init);

#endif

