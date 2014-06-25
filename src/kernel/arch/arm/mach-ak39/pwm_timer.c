#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <mach/gpio.h>
#include <asm/io.h>
#include <linux/module.h>
#include <mach/pwm_timer.h>
#include <mach/irqs.h>

struct ak_pwm_timer_info 
{
	int id;
	int gpio;
	int timerirq;
	u8 __iomem *base;
	T_GPIO_SHAREPIN_CFG sharepin; 
	int is_reserved;
};


static struct ak_pwm_timer_info pt_init_info[] = {
	{AK_PWM_TIMER1, AK_GPIO_4, IRQ_TIMER1, AK_PWM1_CTRL, ePIN_AS_PWM1, 1},
	{AK_PWM_TIMER2, AK_GPIO_5, IRQ_TIMER2, AK_PWM2_CTRL, ePIN_AS_PWM2, 0},
	{AK_PWM_TIMER3, AK_GPIO_6, IRQ_TIMER3, AK_PWM3_CTRL, ePIN_AS_PWM3, 0},
	{AK_PWM_TIMER4, AK_GPIO_7, IRQ_TIMER4, AK_PWM4_CTRL, ePIN_AS_PWM4, 0},
	{AK_PWM_TIMER5, AK_GPIO_8, IRQ_TIMER5, AK_PWM5_CTRL, ePIN_AS_PWM5, 0},
};


static DEFINE_MUTEX(timer_lock);
static struct ak_pwm_timer pwm_timer[AK_PWM_TIMER_CNT];
static int is_initilize = 0;

#define for_each_pwm_timer(i, timer) \
	for((i)=0, timer=pwm_timer; i<AK_PWM_TIMER_CNT; i++, timer++) 


static inline int pwm_freq_is_vaild(u32 freq)
{
	return ((freq >= PWM_MIN_FREQ) && (freq < PWM_MAX_FREQ));
}


static inline int id_is_vaild(int id)
{
	return ((id >= AK_PWM_TIMER1) && (id < AK_PWM_TIMER_NR));
}

static inline int timer_is_busy(int id) 
{
	return (pwm_timer[id].status == PWM_TIMER_BUSY);
}

static inline int timer_is_reserved(int id) 
{
	return (pwm_timer[id].status == PWM_TIMER_RESERVED);
}

/**
*  @brief       set ducy cycle 
*    write the value into corresponding regester
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   *pwm
*  @param[in]   high
*  @param[in]   low
*  @return      int
*/
static int ak_pwm_set_duty_cycle(struct ak_pwm_timer *pwm, u16 high, u16 low, u32 pre_div)
{	
	u32 regval;
	REG32(pwm->base + AK_PWM_TIMER_CTRL1) = high << 16 | low;

	regval = REG32(pwm->base + AK_PWM_TIMER_CTRL2);
	regval &= ~AK_PWM_TIMER_PRE_DIV_MASK;  	
	regval |= AK_PWM_TIMER_PRE_DIV(pre_div) | AK_TIMER_WORK_MODE(pwm->mode);
	REG32(pwm->base + AK_PWM_TIMER_CTRL2) = regval;

	printk("set high %hu, low %hu, div %d\n", high, low ,pre_div);
	return 0;
}


/**
*  @brief       get current ducy cycle 
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   *pwm
*  @param[out]  *high
*  @param[out]  *low
*  @return      int
*/
int ak_pwm_get_duty_cycle(struct ak_pwm_timer *pwm, unsigned short *high, unsigned short *low)
{
	unsigned long regval;

	regval = __raw_readl(pwm->base + AK_PWM_TIMER_CTRL1);

	*high = regval >> 16;
	*low = regval & 0xFFFF;

	return 0;
}
EXPORT_SYMBOL(ak_pwm_get_duty_cycle);

/**
*  @brief       ak_pwm_config
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *pwm
*  @param[in]   high
*  @param[in]   low
*  @return      int
*/
int ak_pwm_config(struct ak_pwm_timer *pwm, unsigned short high, unsigned short low, unsigned int freq)
{
	u32 pre_div;
	//BUG_ON(!pwm_freq_is_vaild(freq));

	pre_div = (REAL_CRYSTAL_FREQ / ((high + 1)+(low + 1)) )/freq - 1;

	pwm->pre_div = pre_div;
	pwm->pwm_hlimit = high;
	pwm->pwm_llimit = low;

	return ak_pwm_set_duty_cycle(pwm, high, low, pre_div);
}
EXPORT_SYMBOL(ak_pwm_config);

/**
*  @brief       enable corresponding pwm
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   *pwm
*  @return      int
*/
int ak_pwm_enable(struct ak_pwm_timer *pwm)
{
	T_GPIO_SHAREPIN_CFG sharepin;

	sharepin = pt_init_info[pwm->id].sharepin;
	ak_group_config(sharepin);			

	REG32(pwm->base + AK_PWM_TIMER_CTRL2) |= AK_PWM_TIMER_EN;
	return 0;
}
EXPORT_SYMBOL(ak_pwm_enable);


/**
*  @brief       disable corresponding pwm
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *pwm
*  @return      void
*/
void ak_pwm_disable(struct ak_pwm_timer *pwm)
{
	u32 regval;
	regval = REG32(pwm->base + AK_PWM_TIMER_CTRL2);
	regval &= ~AK_PWM_TIMER_EN;
	REG32(pwm->base + AK_PWM_TIMER_CTRL2) = regval;

	ak_setpin_as_gpio(pwm->pwm_pin);
}
EXPORT_SYMBOL(ak_pwm_disable);

/**
*  @brief   initilize the pwm/timer module.
*  @author  lixinhai
*  @date        2013-06-9
*  @return  return 0 when success. 
*/
static int __init ak_pwm_timer_init(void) 
{
	int i;
	struct ak_pwm_timer *timer;

	if(is_initilize == 1)
		return 0;

	for_each_pwm_timer(i, timer) {
		timer->id = i;	
		timer->base = pt_init_info[i].base;
		if(pt_init_info[i].is_reserved == 0)
			timer->status = PWM_TIMER_UNUSED;
		else
			timer->status = PWM_TIMER_RESERVED;
	}
	is_initilize = 1;
	return 0;
}
late_initcall(ak_pwm_timer_init);


/**
*  @brief   request a timer.
*  @author  lixinhai
*  @param[in]   the pwm/timer id
*  @param[in]   mode
*  @date        2013-06-9
*  @return  return the ak_pwm_timer handle. 
*/
static struct ak_pwm_timer *__pwm_timer_request(int id, u8 mode)
{
	struct ak_pwm_timer *pt;
	BUG_ON(!id_is_vaild(id));

	if(is_initilize == 0)
		ak_pwm_timer_init();

	/*if timer is reserved or busy, request fail.*/
	if(timer_is_reserved(id) || timer_is_busy(id)) {
		printk("pwm[%d] is working now. request fail.\n", id);
		return NULL;
	}

	pt = &pwm_timer[id];
	pt->mode = mode;
	pt->status = PWM_TIMER_BUSY;

	return pt;
}

/**
*  @brief   release the timer.
*  @author  lixinhai
*  @param[in]   ak_pwm_timer pointer.
*  @date        2013-06-9
*  @return  void 
*/
static void  __pwm_timer_release(struct ak_pwm_timer *pt)
{
	pt->status = PWM_TIMER_UNUSED;
}


/**
*  @brief   config the timer.
*  @author  lixinhai
*  @param[in]   ak_pwm_timer pointer
*  @param[in]   timer count
*  @param[in]   pre div
*  @date        2013-06-9
*  @return  success return 0, otherwise return negative value 
*/
int ak_timer_config(struct ak_pwm_timer *timer, u32 cnt, u8 pre_div)
{
	u32 regval;

	timer->timer_cnt = cnt;
	timer->pre_div = pre_div;


	REG32(timer->base + AK_PWM_TIMER_CTRL1) = timer->timer_cnt;

	REG32(timer->base + AK_PWM_TIMER_CTRL2) = AK_TIMER_TIMEOUT_CLR;

	regval = AK_PWM_TIMER_PRE_DIV(timer->pre_div) | 
				AK_TIMER_WORK_MODE(timer->mode);

	REG32(timer->base + AK_PWM_TIMER_CTRL2) |= regval;
	return 0;
}
EXPORT_SYMBOL(ak_timer_config);


/**
*  @brief   enable the timer.
*  @author  lixinhai
*  @param[in]   ak_pwm_timer pointer
*  @date        2013-06-9
*  @return  success return 0, otherwise return negative value 
*/
int ak_timer_enable(struct ak_pwm_timer *timer)
{
	u32 regval;

	regval = AK_TIMER_FEED_TIMER | AK_PWM_TIMER_EN;
	REG32(timer->base + AK_PWM_TIMER_CTRL2) |= regval;

	return 0;
}
EXPORT_SYMBOL(ak_timer_enable);


/**
*  @brief   enable the timer and wait for timeout.
*  @author  lixinhai
*  @param[in]   ak_pwm_timer pointer
*  @date        2013-06-9
*  @return  success return 0, otherwise return negative value 
*/
int ak_timer_enable_sync(struct ak_pwm_timer *timer)
{
	ak_timer_enable(timer);

	while(!(REG32(timer->base + AK_PWM_TIMER_CTRL2)&(1<<27)))
		;
	return 0;
}
EXPORT_SYMBOL(ak_timer_enable_sync);


/**
*  @brief   disable the timer.
*  @param[in]   ak_pwm_timer pointer
*  @author  lixinhai
*  @date        2013-06-9
*  @return  void 
*/
void ak_timer_disable(struct ak_pwm_timer *timer)
{
	u32 regval;

	regval = REG32(timer->base + AK_PWM_TIMER_CTRL2);
	regval &= ~AK_PWM_TIMER_EN;
	REG32(timer->base + AK_PWM_TIMER_CTRL2) = regval;
}
EXPORT_SYMBOL(ak_timer_disable);


/**
*  @brief   request a pwm by pwm_id.
*  @param[in]   pwm id
*  @author  lixinhai
*  @date        2013-06-9
*  @return  ak_pwm_timer handle. 
*/
struct ak_pwm_timer *ak_pwm_request(int pwm_id)
{
	struct ak_pwm_timer *pwm;

	mutex_lock(&timer_lock);
	pwm = __pwm_timer_request(pwm_id, AK_PT_MODE_PWM);
	mutex_unlock(&timer_lock);
	return pwm;
}
EXPORT_SYMBOL(ak_pwm_request);


/**
*  @brief   the timer interrupt handler.
*  @author  lixinhai
*  @date        2013-06-9
*  @return  irqreturn_t 
*/
static irqreturn_t timer_timeout_irq(int irq, void *dev_id)
{
	struct ak_pwm_timer *timer = dev_id;
	
	REG32(timer->base + AK_PWM_TIMER_CTRL2) |= AK_TIMER_TIMEOUT_CLR;
	timer->timer_cb(timer->timer_priv);	

	return IRQ_HANDLED;
}


/**
*  @brief   request a timer by timer_id.
*  @param[in]   timer id
*  @author  lixinhai
*  @date        2013-06-9
*  @return  success return 0, otherwise return negative value 
*/
struct ak_pwm_timer *ak_timer_request(int timer_id, bool auto_reload, int (*cb)(void* data))
{
	int ret;
	u8 mode;
   
	struct ak_pwm_timer *timer;
	mode = auto_reload ?AK_PT_MODE_TIMER_AUTO_LOAD:
			AK_PT_MODE_TIMER_ONE_SHOT;

	mutex_lock(&timer_lock);
	timer = __pwm_timer_request(timer_id, mode);

	if(cb != NULL) {
		timer->timer_cb = cb;

		timer->timer_irq = pt_init_info[timer_id].timerirq;

		/*if callback function not NULL, request timer irq.*/
		ret = request_irq(timer->timer_irq, timer_timeout_irq, IRQF_DISABLED, "timer", timer);
		disable_irq(timer->timer_irq);
		if(ret) {
			printk("request timer irq fail.\n");
			__pwm_timer_release(timer);
			return NULL;
		}
	}

	mutex_unlock(&timer_lock);
	return timer;
}
EXPORT_SYMBOL(ak_timer_request);


/**
*  @brief   release the pwm.
*  @param[in]   ak_pwm_timer pointer
*  @author  lixinhai
*  @date        2013-06-9
*  @return  success return 0, otherwise return negative value 
*/
void ak_pwm_release(struct ak_pwm_timer *pwm)
{

	mutex_lock(&timer_lock);
	__pwm_timer_release(pwm);
	mutex_unlock(&timer_lock);
}
EXPORT_SYMBOL(ak_pwm_release);


/**
*  @brief   release the timer.
*  @param[in]   ak_pwm_timer pointer
*  @author  lixinhai
*  @date        2013-06-9
*  @return  success return 0, otherwise return negative value 
*/
void ak_timer_release(struct ak_pwm_timer *timer)
{
	mutex_lock(&timer_lock);
	free_irq(timer->timer_irq, timer);
	__pwm_timer_release(timer);
	mutex_unlock(&timer_lock);
}
EXPORT_SYMBOL(ak_timer_release);


