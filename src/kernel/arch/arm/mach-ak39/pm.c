/*
 * linux/arch/arm/mach-ak39/pm.c
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
//#include <linux/anyka_cpufreq.h>
#include <asm/cacheflush.h>
#include <plat/l2.h>
#include <plat/l2_exebuf.h>

void check_poweroff(void);

static suspend_state_t target_state;

#define ak39_pm_debug_init() do { } while(0)

static int ak39_pm_valid_state(suspend_state_t state)
{
	switch (state) {
		case PM_SUSPEND_ON:
		case PM_SUSPEND_STANDBY:
		case PM_SUSPEND_MEM:
		    return 1;
		default:
		    return 0;
	}
}

/*
 * Called after processes are frozen, but before we shutdown devices.
 */
static int ak39_pm_begin(suspend_state_t state)
{
    	target_state = state;
    	return 0;
}

void L2_LINK(standby) L2FUNC_NAME(standby)(unsigned long param1,
    unsigned long param2,unsigned long param3, unsigned long param4)
{
	unsigned long val;
	
	// invalidate and disable mmu
    DISABLE_CACHE_MMU();
	
	// check this bit and unitil both are empty 
	while(!((REG32(PHY_RAM_CFG_REG4) & (FIFO_R_EMPTY | FIFO_CMD_EMPTY)) == (FIFO_R_EMPTY | FIFO_CMD_EMPTY)))
		;
	
	// setup periodic of refresh interval and disable auto-refresh
    REG32(PHY_RAM_CFG_REG4) &= ~(AUTO_REFRESH_EN);

    // send all bank precharge
    DDR2_ENTER_SELFREFRESH();
    PM_DELAY(0x10);//at least more than 1 tck
    
	// set sdram mode before enter standby
	val = REG32(0x2000e000);
	REG32(0x2000e000) |= (0x3 << 0);
	
	// disable ram clock
    REG32(PHY_CLOCK_CTRL_REG) |= RAM_CLOCK_DISABLE;

	// enter standby
    REG32(PHY_CLOCK_DIV_REG) |= ENTER_STANDBY;
    PM_DELAY(0x2000);  //at least more than 3 tck only for selfresh

	// the system is standby ......
	
    // enable ram clock
    REG32(PHY_CLOCK_CTRL_REG) &= ~RAM_CLOCK_DISABLE;

	// restore from sdram mode after exit standby
	REG32(0x2000e000) = val;
	
	/* instruction:
	 *	PM_DELAY(0x1) = 128.8ns  when mem clk = 200M 
	 */
    PM_DELAY(0x10); // at least more than 1 tck prior exit self  refresh
    
    // exit DDR2 self-refresch
    DDR2_EXIT_SELFREFRESH();
	
	// send auto refresh and open odt high
    DDR2_ENTER_AUTOREFRESH();

	// enable auto-refresh
    REG32(PHY_RAM_CFG_REG4) |= AUTO_REFRESH_EN;
    PM_DELAY(0x100);

    // enable ICache & DCache, mmu
	ENABLE_CACHE_MMU();
}

static int ak39_pm_enter(suspend_state_t state)
{
	unsigned long flags;
	
	local_irq_save(flags);
	ak39_pm_debug_init();
	flush_cache_all();

	// change from low to normal mode  before enter standby if current is low mode 
	//cpu_freq_suspend_check();
	
	SPECIFIC_L2BUF_EXEC(standby, 0,0,0,0);

	// check power off
	check_poweroff();	

	// restore low mode
	//cpu_freq_resume_check();
	
	local_irq_restore(flags);
	return 0;
}

/*
 * Called right prior to thawing processes.
 */
static void ak39_pm_end(void)
{
    target_state = PM_SUSPEND_ON;
}

static struct platform_suspend_ops ak39_pm_ops = {
	.valid	= ak39_pm_valid_state,
	.begin	= ak39_pm_begin,
	.enter	= ak39_pm_enter,
	.end	= ak39_pm_end,
};

/* ak39_pm_init
 *
 * Attach the power management functions. This should be called
 * from the board specific initialisation if the board supports
 * it.
*/
int __init ak39_pm_init(void)
{
	printk("AK39 Power Management, (c) 2010 ANYKA\n");
	suspend_set_ops(&ak39_pm_ops);

	return 0;
}
arch_initcall(ak39_pm_init);

