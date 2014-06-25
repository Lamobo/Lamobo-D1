#ifndef __AK_MOTOR_H__
#define __AK_MOTOR_H__

#ifdef __KERNEL__
enum ak_motor_phase {
	AK_MOTOR_PHASE_A = 0,
	AK_MOTOR_PHASE_B,
	AK_MOTOR_PHASE_C,
	AK_MOTOR_PHASE_D,
	AK_MOTOR_PHASE_NUM,
};

enum ak_motor_hit {
	AK_MOTOR_HIT_LEFT = 0,
	AK_MOTOR_HIT_RIGHT,
	AK_MOTOR_HIT_NUM,
};


struct ak_motor_plat_data 
{
    struct gpio_info gpio_phase[AK_MOTOR_PHASE_NUM];
	struct gpio_info gpio_hit[AK_MOTOR_HIT_NUM];
	unsigned int irq_hit_type[AK_MOTOR_HIT_NUM];
	void (* gpio_init) (const struct gpio_info *);

	unsigned int angular_speed;
};
#endif

#define AK_MOTOR_IOC_MAGIC 		'm'
#define AK_MOTOR_SET_ANG_SPEED 		_IOW(AK_MOTOR_IOC_MAGIC, 11, int)
#define AK_MOTOR_GET_ANG_SPEED 		_IOR(AK_MOTOR_IOC_MAGIC, 12, int)
#define AK_MOTOR_TURN_CLKWISE 		_IOW(AK_MOTOR_IOC_MAGIC, 13, int)
#define AK_MOTOR_TURN_ANTICLKWISE 	_IOW(AK_MOTOR_IOC_MAGIC, 14, int)
#define AK_MOTOR_GET_HIT_STATUS 	_IOW(AK_MOTOR_IOC_MAGIC, 15, int)
#define AK_MOTOR_TURN_STOP 			_IOW(AK_MOTOR_IOC_MAGIC, 16, int)

#define AK_MOTOR_EVENT_HIT 		(1)
#define AK_MOTOR_EVENT_UNHIT 	(2)
#define AK_MOTOR_EVENT_STOP 	(3)

#define AK_MOTOR_HITTING_LEFT 	(1<<0)
#define AK_MOTOR_HITTING_RIGHT 	(1<<1)

#define AK_MOTOR_MIN_SPEED 		(1)
#define AK_MOTOR_MAX_SPEED 		(16)
struct notify_data
{
	int hit_num;
	int event;
	int remain_angle;
};

#endif
