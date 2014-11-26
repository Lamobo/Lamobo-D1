/* 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/mach/time.h>

#include <mach/map.h>


#define AK39_TIMER1_CTRL1		(AK_VA_SYSCTRL + 0xB4)
#define AK39_TIMER1_CTRL2		(AK_VA_SYSCTRL + 0xB8)
#define AK39_TIMER2_CTRL1		(AK_VA_SYSCTRL + 0xBC)
#define AK39_TIMER2_CTRL2		(AK_VA_SYSCTRL + 0xC0)
#define AK39_TIMER3_CTRL1		(AK_VA_SYSCTRL + 0xC4)
#define AK39_TIMER3_CTRL2		(AK_VA_SYSCTRL + 0xC8)
#define AK39_TIMER4_CTRL1		(AK_VA_SYSCTRL + 0xCC)
#define AK39_TIMER4_CTRL2		(AK_VA_SYSCTRL + 0xD0)
#define AK39_TIMER5_CTRL1		(AK_VA_SYSCTRL + 0xD4)
#define AK39_TIMER5_CTRL2		(AK_VA_SYSCTRL + 0xD8)

#define AK39_TIMER_CTRL1		AK39_TIMER1_CTRL1
#define AK39_TIMER_CTRL2		AK39_TIMER1_CTRL2
#define IRQ_TIMER				IRQ_TIMER1

#define TIMER_CNT				(12000000/HZ)
#define TIMER_USEC_SHIFT		16
#define TIMER_CNT_MASK			(0x3F<<26)

//define timer register bits
#define TIMER_CLEAR_BIT			(1<<30)
#define TIMER_FEED_BIT			(1<<29)
#define TIMER_ENABLE_BIT		(1<<28)
#define TIMER_STATUS_BIT		(1<<27)
#define TIMER_READ_SEL_BIT		(1<<26)

//define pwm/pwm mode
#define MODE_AUTO_RELOAD_TIMER	0x0
#define MODE_ONE_SHOT_TIMER		0x1
#define MODE_PWM				0x2   


static u_int64_t ghrtick = 0;
static unsigned long usec_per_tick; /* usec per tick, left shift 16 */

/* copy from plat-s3c/time.c
 *
 *  timer_mask_usec_ticks
 *
 * given a clock and divisor, make the value to pass into timer_ticks_to_usec
 * to scale the ticks into usecs
*/
static inline unsigned long
timer_mask_usec_ticks(unsigned long scaler, unsigned long pclk)
{
	unsigned long den = pclk / 1000;

	return ((1000 << TIMER_USEC_SHIFT) * scaler + (den >> 1)) / den;
}

static inline unsigned long timer_ticks_to_usec(unsigned long ticks)
{
    unsigned long ret;

    ret = ticks * usec_per_tick;
    ret += 1 << (TIMER_USEC_SHIFT - 4);

	return ret >> TIMER_USEC_SHIFT;
}

/*
 * Returns microsecond  since last clock interrupt.  Note that interrupts
 * will have been disabled by do_gettimeoffset()
 * IRQs are disabled before entering here from do_gettimeofday()
 *
 * FIXME: this need be checked 
 */
static unsigned long ak39_gettimeoffset(void)
{
	unsigned long tdone;
	unsigned long tcnt;

	/* work out how many ticks have gone since last timer interrupt */

	//select read current count mode
	tdone = __raw_readl(AK39_TIMER_CTRL2);
	__raw_writel(tdone | TIMER_READ_SEL_BIT, AK39_TIMER_CTRL2);

	tcnt = __raw_readl(AK39_TIMER_CTRL1);

	//recover read mode
	tdone = __raw_readl(AK39_TIMER_CTRL2);
	__raw_writel(tdone & (~TIMER_READ_SEL_BIT), AK39_TIMER_CTRL2);

	tdone = TIMER_CNT - tcnt;

	if (__raw_readl(AK39_TIMER_CTRL2) & TIMER_STATUS_BIT) {	/* Timer1 has generated interrupt, and not clear */

		/* Reread timer counter */
		//select read current count mode
		tdone = __raw_readl(AK39_TIMER_CTRL2);
		__raw_writel(tdone | TIMER_READ_SEL_BIT, AK39_TIMER_CTRL2);

		tcnt = __raw_readl(AK39_TIMER_CTRL1);

		//recover read mode
		tdone = __raw_readl(AK39_TIMER_CTRL2);
		__raw_writel(tdone & (~TIMER_READ_SEL_BIT), AK39_TIMER_CTRL2);

		tdone = TIMER_CNT - tcnt;

		if (tcnt != 0)
		tdone += TIMER_CNT;
	}

	return timer_ticks_to_usec(tdone);
}

static inline void ak39_timer_setup(void)
{
	unsigned long regval;

	/* clear timeout puls, reload */
    regval = __raw_readl(AK39_TIMER_CTRL2);
    __raw_writel(regval | TIMER_CLEAR_BIT, AK39_TIMER_CTRL2);
}

/*
 * IRQ handler for the timer
 */
static irqreturn_t ak39_timer_interrupt(int irq, void *dev_id)
{
	if (__raw_readl(AK39_TIMER_CTRL2) & TIMER_STATUS_BIT) {

        ghrtick += TIMER_CNT;

		timer_tick();

		ak39_timer_setup();
	}

	return IRQ_HANDLED;
}

#if 0
u_int64_t ak39_gethrtick(void)
{
	unsigned long timecnt = 0;
	unsigned long tdone;

	//select read current count mode
	tdone = __raw_readl(AK39_TIMER_CTRL2);
	__raw_writel(tdone | TIMER_READ_SEL_BIT, AK39_TIMER_CTRL2);

	timecnt = __raw_readl(AK39_TIMER_CTRL1);

	//recover read mode
	tdone = __raw_readl(AK39_TIMER_CTRL2);
	__raw_writel(tdone & (~TIMER_READ_SEL_BIT), AK39_TIMER_CTRL2);

	timecnt &= (~TIMER_CNT_MASK);

	return (ghrtick + (u_int64_t)(TIMER_CNT-timecnt));
}
#endif

static struct irqaction ak39_timer_irq = {
	.name = "timer tick",
	.flags = IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler = ak39_timer_interrupt,
};

static void __init ak39_timer_init(void)
{
	unsigned long timecnt = TIMER_CNT - 1;

    usec_per_tick = timer_mask_usec_ticks(1, 12000000);

	__raw_writel(timecnt, AK39_TIMER_CTRL1);
	__raw_writel((TIMER_ENABLE_BIT | TIMER_FEED_BIT | (MODE_AUTO_RELOAD_TIMER << 24)), 
		AK39_TIMER_CTRL2);

	/* setup irq handler for IRQ_TIMER */
	setup_irq(IRQ_TIMER, &ak39_timer_irq);
    ghrtick = 0;
}


struct sys_timer ak39_timer = {
	.init		= ak39_timer_init,
	.offset		= ak39_gettimeoffset,
	//.resume	= ak39_timer_setup
};

