#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <string.h>
#include <sys/select.h>

#include "PTZControl.h"
#define AK_MOTOR_DEV0 "/dev/ak-motor0"//左右
#define AK_MOTOR_DEV1 "/dev/ak-motor1"//上下

#define MOTOR_MAX_BUF 	(64)

typedef enum
{
	MT_STOP,
	MT_STEP,
	MT_RUN,
	MT_RUNTO,
	MT_CAL,
}MT_STATUS;

struct ak_motor
{
	
	int running;
	
	pthread_mutex_t lock;
	struct notify_data data;
	int fd;
	MT_STATUS mt_status;
	int step_degree;
	int cw;

	int default_speed;
	int run_speed;
	int runto_speed;
	int hit_speed;
	int step_speed;
	int direction;//0左右   1上下
	
	int nMax_hit;//撞击角度
	int nMax_unhit;//最大不撞击角度
	
	int dg_max;//最大角度
	int dg_min;//最小角度
	int dg_save;//保存设置位置的角度
	int dg_cur;//当前位置角度
	int dg_center;//中间位置角度
};

//角度的范围为[-x,x]，垂直方向上中间角度为-32，水平方向为0
#define MAX_SPEED 16
#define MAX_DG 360 * 2 + 1//最大转动角度，这里定义足够大

static struct ak_motor akmotor[] = 
{
	[0] = {
		.fd = -1,
		.mt_status = MT_STOP,
		.step_degree = 8,//这个值或者它的倍数的误差最小
		
		.default_speed = MAX_SPEED,//5,
		.run_speed = 10,//5,
		.runto_speed = MAX_SPEED,//15,
		.hit_speed = MAX_SPEED,//15,
		.step_speed = MAX_SPEED,//15,//8,
		
		.direction = 0,
		.dg_center = 0,
	},
	[1] = {
		.fd = -1,
		.mt_status = MT_STOP,
		.step_degree = 8,
		
		.default_speed = MAX_SPEED,//3,
		.run_speed = 10,//3,
		.runto_speed = MAX_SPEED,//15,
		.hit_speed = MAX_SPEED,//15,
		.step_speed = MAX_SPEED,//15,//8,
		
		.direction = 1,
		.dg_center = -32,
	},
};


static int motor_turn(struct ak_motor *motor, int angle, int is_cw)
{
	if(motor->fd < 0)
		return -1;
	int ret;
	int cmd = is_cw ? AK_MOTOR_TURN_CLKWISE:AK_MOTOR_TURN_ANTICLKWISE;
	
	ret = ioctl(motor->fd, cmd, &angle);	
	if(ret) {
		printf("%s fail. is_cw:%d, angle:%d, ret:%d.\n", __func__, angle, is_cw, ret);
	}
	return ret;
}

static int motor_change_speed(struct ak_motor *motor, int ang_speed)
{
	int ret;	
	int val = ang_speed;
	if(motor->fd < 0)
		return -1;
	ret = ioctl(motor->fd, AK_MOTOR_SET_ANG_SPEED, &val);	
	if(ret) {
		printf("%s fail.  ang_speed:%d, ret:%d.\n", __func__, ang_speed, ret);
	}
	return ret;
}


static int ak_motor_wait_stop(struct ak_motor *motor, int* remain_angle)
{
	fd_set fds;
	int ret;
	if(motor->fd < 0)
		return -1;
start:
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, NULL);
	if(ret <= 0) {
		printf("wait, ret:%d,\n", ret);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
		
	if(motor->data.event != AK_MOTOR_EVENT_STOP)
	{
		printf("not stop, %d, %d\n", motor->data.event, motor->data.remain_angle);
		if(motor->data.remain_angle > 0)
		{
			//可能会由于抖动的原因误报，
			//如果出现这种情况就按原来的方向转动剩余角度
			motor_turn(motor, motor->data.remain_angle, motor->cw);
			goto start;
		}	
	}

	*remain_angle = motor->data.remain_angle;
	return 0;
}

static int ak_motor_wait_hit(struct ak_motor *motor, int* remain_angle)
{
	fd_set fds;
	int ret;
	if(motor->fd < 0)
		return -1;
start:
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, NULL);
	if(ret <= 0) {
		printf("wait, ret:%d,\n", ret);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
	
	if(motor->data.event != AK_MOTOR_EVENT_HIT)
	{
		printf("not hit,motor->data.event = %d, %d\n", motor->data.event, motor->data.remain_angle);
		goto start;
	}
	
	*remain_angle = motor->data.remain_angle;
	return 0;
}

static int ak_motor_wait_unhit(struct ak_motor *motor, int* remain_angle)
{
	fd_set fds;
	int ret;
	if(motor->fd < 0)
		return -1;
start:
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, NULL);
	if(ret <= 0) {
		printf("wait, ret:%d,\n", ret);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
		
	if(motor->data.event != AK_MOTOR_EVENT_UNHIT)
	{
		printf("not unhit,motor->data.event = %d,,%d\n", motor->data.event, motor->data.remain_angle);
		goto start;
	}
	
	*remain_angle = motor->data.remain_angle;
	return 0;
}

static int ak_motor_turn_clkwise(struct ak_motor *motor)
{
	int rg;
	if(motor->dg_max - motor->dg_cur <= motor->step_degree)
	{
		int xrg = motor->dg_max - motor->dg_cur;
		printf("dg_max=%d, dg_cur=%d, %d\n", motor->dg_max, motor->dg_cur, motor->dg_max - motor->dg_cur);
		motor_turn(motor, MAX_DG, motor->cw);
		ak_motor_wait_hit(motor, &rg);
		motor->cw = 0;
		motor_turn(motor, xrg + 1, motor->cw);
		ak_motor_wait_stop(motor, &rg);
	}
	else
	{
		motor_turn(motor, motor->step_degree, motor->cw);
		ak_motor_wait_stop(motor, &rg);
		motor->dg_cur += motor->step_degree;
	}

	return 0;
}
static int ak_motor_turn_anticlkwise(struct ak_motor *motor)
{
	int rg;
	if(motor->dg_cur - motor->dg_min <= motor->step_degree)
	{
		int xrg = motor->dg_cur - motor->dg_min;
		printf("anti:dg_min=%d, dg_cur=%d, %d\n", motor->dg_min, motor->dg_cur, motor->dg_cur - motor->dg_min);
		motor_turn(motor, MAX_DG, motor->cw);
		ak_motor_wait_hit(motor, &rg);
		motor->cw = 1;
		motor_turn(motor, xrg + 1, motor->cw);
		ak_motor_wait_stop(motor, &rg);
	}
	else
	{
		motor_turn(motor, motor->step_degree, motor->cw);
		ak_motor_wait_stop(motor, &rg);
		motor->dg_cur -= motor->step_degree;
		
	}
	
	return 0;
}

static void* ak_motor_cal_thread(void* data)
{
	printf("%s\n", __func__);
	//巡航，校准
	struct ak_motor *motor = NULL;
	
	int rg;
	int a,b,c,d;
	//先水平，后垂直
	for(int i = 0; i < 2; i ++)
	{
		motor = &akmotor[i];
		if(motor->mt_status != MT_STOP)
			continue;
		if(motor->fd < 0)
			continue;
		motor_change_speed(motor, motor->runto_speed);
		motor->mt_status = MT_CAL;
		
		//顺时针转到底
		motor->cw = 1;
		motor_turn(motor, MAX_DG, motor->cw);
		ak_motor_wait_hit(motor, &rg);
		a = rg;
		motor->cw = 0;
		//逆时针转到底
		motor_turn(motor, rg, motor->cw);
		ak_motor_wait_unhit(motor, &rg);
		b = rg;
	
		motor_turn(motor, rg, motor->cw);
		ak_motor_wait_hit(motor, &rg);
		c = rg;
		//计算出各种角度后顺时针转到中间
		motor->cw = 1;
		motor_turn(motor, rg, motor->cw);
		ak_motor_wait_unhit(motor, &rg);
		d = rg;
		printf("a = %d, b = %d, c = %d, d = %d, ac = %d, bd = %d\n", a,b,c,d, (a - c), (b + d - 2 * c));
	
		motor->nMax_hit = a - c;
		motor->nMax_unhit = b + d - 2 * c;
	
		motor->dg_max = motor->nMax_unhit / 2;
		motor->dg_min =  - (motor->nMax_unhit) / 2;
		motor->dg_save = motor->dg_center;
		motor->dg_cur = motor->dg_min;
	
		rg = motor->dg_save - motor->dg_cur;
		motor_turn(motor, rg, motor->cw);
		ak_motor_wait_stop(motor, &rg);
		motor->dg_cur = motor->dg_save;
	
		motor->mt_status = MT_STOP;
		motor_change_speed(motor, motor->step_speed);
	}
	printf("cal ok\n");
	return NULL;
}


static int ak_motor_cal(struct ak_motor* motor)
{
	pthread_t th;
	pthread_create(&th, NULL, ak_motor_cal_thread, motor);
	return 0;
}

static void* ak_motor_run_thread(void* data)
{
	struct ak_motor *motor = data;
	if(motor->mt_status != MT_STOP)
		return 0;

	motor->mt_status = MT_RUN;
	motor_change_speed(motor, motor->run_speed);
	
	motor->cw = 1;//默认从顺时针转
	
	while(motor->mt_status == MT_RUN)
	{
		if(motor->cw == 1)
		{
			ak_motor_turn_clkwise(motor);
		}
		else
		{
			ak_motor_turn_anticlkwise(motor);
		}
	}
	
	motor_change_speed(motor, motor->step_speed);
	return NULL;
}

static int ak_motor_run(struct ak_motor* motor)
{
	if(motor->fd < 0)
		return -1;
	pthread_t th;
	pthread_create(&th, NULL, ak_motor_run_thread, motor);
	return 0;
}

static void* ak_motor_runto_thread(void* data)
{
	struct ak_motor *motor = data;
	if(motor->mt_status != MT_STOP)
		return 0;

	int rg;
	motor->mt_status = MT_RUNTO;
	
	motor_change_speed(motor, motor->runto_speed);
	int dg = motor->dg_save - motor->dg_cur;//计算当前位置和保存位置的角度
	
	if(dg > 0)//判断转动方向
	{
		motor->cw = 1;	
	}
	else if(dg < 0)
	{
		motor->cw = 0;
		dg = -dg;
	}
	printf("cw = %d,angle = %d\n", motor->cw, dg);
	motor_turn(motor, dg, motor->cw);
	ak_motor_wait_stop(motor, &rg);
	motor->dg_cur = motor->dg_save;
	
	motor_change_speed(motor, motor->step_speed);
	motor->mt_status = MT_STOP;
	return NULL;
}

static int ak_motor_runto(struct ak_motor* motor)
{
	if(motor->fd < 0)
		return -1;
	pthread_t th;
	pthread_create(&th, NULL, ak_motor_runto_thread, motor);
	return 0;
}


static int ak_motor_init(struct ak_motor *motor, char *name)
{
	if(!motor)
		return -1;

	motor->fd = open(name, O_RDWR);
	if(motor->fd < 0)
		return -2;
	return 0;
}

int PTZControlInit()
{
	ak_motor_init(&akmotor[0], AK_MOTOR_DEV0);
	ak_motor_init(&akmotor[1], AK_MOTOR_DEV1);
	ak_motor_cal(NULL);
	return 0;
}

int PTZControlStepUp()
{
	struct ak_motor* motor = &akmotor[1];
	if(motor->mt_status != MT_STOP)
		return -1;
	motor->mt_status = MT_STEP;
	motor->cw = 1;
	
	ak_motor_turn_clkwise(motor);
	
	motor->mt_status = MT_STOP;
	//printf("up:curpos=%d\n", motor->dg_cur);
	return 0;
}
int PTZControlStepDown()
{
	struct ak_motor* motor = &akmotor[1];
	
	if(motor->mt_status != MT_STOP)
		return -1;
	motor->mt_status = MT_STEP;
	motor->cw = 0;
	
	ak_motor_turn_anticlkwise(motor);
	
	motor->mt_status = MT_STOP;
	//printf("down:curpos=%d\n", motor->dg_cur);
	return 0;
}
int PTZControlStepLeft()
{
	struct ak_motor* motor = &akmotor[0];
	if(motor->mt_status != MT_STOP)
		return -1;
	motor->mt_status = MT_STEP;
	motor->cw = 1;
	
	ak_motor_turn_clkwise(motor);
	
	motor->mt_status = MT_STOP;
	//printf("left:curpos=%d\n", motor->dg_cur);
	return 0;
}
int PTZControlStepRight()
{
	struct ak_motor* motor = &akmotor[0];
	
	if(motor->mt_status != MT_STOP)
		return -1;
	motor->mt_status = MT_STEP;
	motor->cw = 0;
	
	ak_motor_turn_anticlkwise(motor);
	
	motor->mt_status = MT_STOP;
	//printf("right:curpos=%d\n", motor->dg_cur);
	return 0;
}
int PTZControlUpDown()
{
	struct ak_motor* pmotor = &akmotor[1];
	printf("up and down:");
	if(pmotor->mt_status == MT_RUN)
	{
		printf("run-->stop\n");
		pmotor->mt_status = MT_STOP;	
	}
	else if(pmotor->mt_status == MT_STOP)
	{
		printf("stop-->run\n");
		ak_motor_run(pmotor);
	}
	
	return 0;
}
int PTZControlLeftRight()
{
	struct ak_motor* pmotor = &akmotor[0];
	printf("left and right:");
	if(pmotor->mt_status == MT_RUN)
	{
		printf("run-->stop\n");
		pmotor->mt_status = MT_STOP;	
	}
	else if(pmotor->mt_status == MT_STOP)
	{
		printf("stop-->run\n");
		ak_motor_run(pmotor);
	}
	return 0;
}
int PTZControlSetPosition()
{
	akmotor[0].dg_save = akmotor[0].dg_cur;
	akmotor[1].dg_save = akmotor[1].dg_cur;
	
	printf("savepos= %d,,%d\n", akmotor[0].dg_save, akmotor[1].dg_cur);
	return 0;
}
int PTZControlRunPosition()
{
	akmotor[0].mt_status = MT_STOP;
	akmotor[1].mt_status = MT_STOP;
	
	ak_motor_runto(&akmotor[0]);
	ak_motor_runto(&akmotor[1]);
	
	return 0;
}

int PTZControlDeinit()
{
	akmotor[0].mt_status = MT_STOP;
	akmotor[1].mt_status = MT_STOP;
	
	if(akmotor[0].fd > 0)
		close(akmotor[0].fd);
	if(akmotor[1].fd > 0)
		close(akmotor[1].fd);
	
	akmotor[0].fd = -1;
	akmotor[1].fd = -1;
	return 0;
}
