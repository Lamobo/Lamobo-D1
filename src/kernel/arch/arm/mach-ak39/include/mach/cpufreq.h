#ifndef __CPUFREQ_H__
#define __CPUFREQ_H__

#if 0
#include <linux/cpufreq.h>
#include <plat/l2_exebuf.h>

#define MEM_CLK_DIV     0x7
#define ASIC_CLK_DIV    0x7

#define PLL_CLK_MIN     180
#define PLL_CLK_MAX     (PLL_CLK_MIN + 4*(0x3F))

//#define CPUFREQ_DEBUG

/* clock divider register */
#define PLL_CHANGE_ENA          (1 << 12)
#define CLOCK_ASIC_MEM_ENA      (1 << 14)

struct cpufreq_mode_clkdiv {
    T_OPERATION_MODE mode_name;
    unsigned int pll_sel;
    unsigned int clk168_div;
    unsigned int cpu_div;
    unsigned int mem_div;
    unsigned int asic_div;
    unsigned int low_clock;
    unsigned int is_3x;
};

#endif

#endif	/* end __CPUFREQ_H__ */
