#ifndef __PWM_TIMER_H__
#define __PWM_TIMER_H__


#define timer_cnt 		pt_u.timer.cnt
#define timer_irq 		pt_u.timer.irq
#define timer_cb  		pt_u.timer.callback
#define timer_reload 	pt_u.timer.auto_reload
#define timer_priv 		pt_u.timer.priv_data

#define pwm_hlimit 	pt_u.pwm.high_limit
#define pwm_llimit 	pt_u.pwm.low_limit
#define pwm_pin 		pt_u.pwm.pin

/*pwm/timer object*/
struct ak_pwm_timer
{
	int id;
	u8 mode;
	u8 pre_div;
	u8 __iomem *base;
	bool status;

	union {
		struct {
			u32 cnt;
			bool auto_reload;
			void *priv_data;
			int irq;
			int (*callback)(void*);
		}timer; /*timer private element*/
		struct {
			u16 high_limit;
			u16 low_limit;
			int pin;
		}pwm; /*pwm private element*/
	}pt_u;
};


struct ak_platform_pwm_bl_data {
	int pwm_id;
	unsigned int max_brightness;
	unsigned int dft_brightness;
	unsigned int high_limit;
	unsigned int low_limit;
	unsigned int pwm_clk;
	int (*init)(struct ak_pwm_timer *dev);
	int (*notify)(int brightness);
	void (*exit)(struct ak_pwm_timer *dev);
};



#define AK_PWM1_CTRL	(AK_VA_SYSCTRL+0xB4)
#define AK_PWM2_CTRL	(AK_VA_SYSCTRL+0xBC)
#define AK_PWM3_CTRL	(AK_VA_SYSCTRL+0xC4)
#define AK_PWM4_CTRL	(AK_VA_SYSCTRL+0xCC)
#define AK_PWM5_CTRL	(AK_VA_SYSCTRL+0xD4)


#define AK_PWM_TIMER_CTRL1 		(0x00)
#define AK_PWM_TIMER_CTRL2 		(0x04)

#define AK_PWM_HIGH_LEVEL(x) 		((x) << 16)
#define AK_PWM_LOW_LEVEL(x) 		(x)

#define AK_TIMER_TIMEOUT_CLR 		(1<<30)
#define AK_TIMER_FEED_TIMER 		(1<<29)
#define AK_PWM_TIMER_EN 			(1<<28)
#define AK_TIMER_TIMEOUT_STA 		(1<<27)
#define AK_TIMER_READ_SEL 			(1<<26)

#define AK_TIMER_WORK_MODE(x) 		((x)<<24)
#define AK_PWM_TIMER_PRE_DIV(x) 	((x) << 16)
#define AK_PWM_TIMER_PRE_DIV_MASK  	((0xff) << 16)


#define REAL_CRYSTAL_FREQ 		(12*1000*1000)
#define PWM_MAX_FREQ 		(6*1000*1000)
#define PWM_MIN_FREQ 		(92*1000)

#define AK_PWM_TIMER_CNT 	(5)

/*the pwm/timer number.*/
enum ak_pwm_timer_nr {
	AK_PWM_TIMER1,
	AK_PWM_TIMER2,
	AK_PWM_TIMER3,
	AK_PWM_TIMER4,
	AK_PWM_TIMER5,
	AK_PWM_TIMER_NR,
};


enum ak_pwm_timer_status {
	PWM_TIMER_UNUSED,
	PWM_TIMER_BUSY,
	PWM_TIMER_RESERVED,
};


enum ak_pwm_timer_mode 
{
	AK_PT_MODE_TIMER_AUTO_LOAD = 0,
	AK_PT_MODE_TIMER_ONE_SHOT,
	AK_PT_MODE_PWM,
};


int ak_pwm_get_duty_cycle(struct ak_pwm_timer *pwm, unsigned short *high, unsigned short *low);
int ak_pwm_config(struct ak_pwm_timer *pwm, unsigned short high, unsigned short low, unsigned int freq);

int ak_pwm_enable(struct ak_pwm_timer *pwm);
void ak_pwm_disable(struct ak_pwm_timer *pwm);
int ak_timer_enable(struct ak_pwm_timer *timer);
int ak_timer_enable_sync(struct ak_pwm_timer *timer);
void ak_timer_disable(struct ak_pwm_timer *timer);

struct ak_pwm_timer *ak_pwm_request(int pwm_id);
struct ak_pwm_timer *ak_timer_request(int timer_id, bool auto_reload, int (*cb)(void* data));

void ak_pwm_release(struct ak_pwm_timer *pwm);
void ak_timer_release(struct ak_pwm_timer *timer);
	
int ak_timer_config(struct ak_pwm_timer *timer, u32 cnt, u8 pre_div);

#endif

