#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <plat-anyka/akgpios.h>

typedef unsigned char __u8;

#define AK_WAKEUP_ENABLE		1
#define AK_WAKEUP_DISABLE		0
#define AK_FALLING_TRIGGERED	1
#define AK_RISING_TRIGGERED		0

#define	AK_GPIO_DIR_OUTPUT		1
#define	AK_GPIO_DIR_INPUT		0
#define	AK_GPIO_INT_DISABLE		0
#define	AK_GPIO_INT_ENABLE		1
#define	AK_GPIO_INT_LOWLEVEL	0
#define	AK_GPIO_INT_HIGHLEVEL	1

#define	AK_GPIO_OUT_LOW			0
#define	AK_GPIO_OUT_HIGH		1

#define	AK_PULLUP_DISABLE		0
#define	AK_PULLUP_ENABLE		1
#define	AK_PULLDOWN_DISABLE		0
#define	AK_PULLDOWN_ENABLE		1

#define	GPIO_PIN_MODE_GPIO		0
#define	GPIO_PIN_MODE_INT		1

struct gpio_info {
    int pin;
    char pulldown;
    char pullup;
    char value;
    char dir;
    char int_pol;
};

int main(int argc, char *argv[]){
    int fd = open("/dev/akgpio", O_RDWR);
    int ret;

    if(fd < 0){
        perror("open failer");
        return -1;
    }



    struct gpio_info gpio;

    //int gpios[]={1, 2, 4, 5, 6, 7, 27, 28, 29, 30, 37, 38, 56, 53};
    int gpios[]={53};
    int gpiosize = sizeof(gpios)/sizeof(int);

    int i = 0;

    memset(&gpio,0, sizeof(gpio));
    gpio.pulldown = -1;
    gpio.pullup = -1;
    gpio.dir = AK_GPIO_DIR_OUTPUT;
    gpio.int_pol = AK_GPIO_INT_DISABLE;


    int cnt = 0;
    while(cnt++ < 3){
        for(i = 0; i < gpiosize; i++){
            gpio.pin = gpios[i];
            gpio.value = AK_GPIO_OUT_LOW;
            ret = ioctl(fd, SET_GPIO_FUNC, &gpio);
            if(ret >= 0)
                printf("gpio %d SET_GPIO_LEVEL %d returns %d\n",gpio.pin, gpio.value, ret);
        }
        sleep(2);
        for(i = 0; i < gpiosize; i++){
            gpio.pin = gpios[i];
            gpio.value = AK_GPIO_OUT_HIGH;
            ret = ioctl(fd, SET_GPIO_FUNC, &gpio);
            if(ret >= 0)
                printf("gpio %d SET_GPIO_LEVEL %d returns %d\n",gpio.pin, gpio.value, ret);
        }
        sleep(1);

    }

    close(fd);
    return 0;
}
