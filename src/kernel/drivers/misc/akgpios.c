/*
 * drivers/gpio/akgpios.c
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <plat-anyka/akgpios.h>
#include <mach/gpio.h>


//#define AKGPIO_DEDUB
#define DBG(fmt...)	//printk(fmt);

#define AKGPIO_DEV_NAME "akgpio"

#define init_MUTEX(sem)		sema_init(sem, 1)

struct akgpio_custom_device {
	struct gpio_info gpioinfo;
	int gpio;
	int used;
	int irq_flag;
	struct semaphore sem;
	int locked;
};

static struct custom_gpio_data *pdata = NULL;
static struct akgpio_custom_device *akgpios = NULL;
static int gpio_num;


#ifdef AKGPIO_DEDUB
static void print_gpio_set_info(struct gpio_info *gpio)
{	
	printk("GPIO[%d], dir[%s] out[%s] pulldown[%s] pullup[%s] int_pol[%s]\n",
		gpio->pin, 
		(gpio->dir == AK_GPIO_DIR_OUTPUT) ? "out" : "in",
		(gpio->value == AK_GPIO_OUT_HIGH) ? "high" : 
		(gpio->value == AK_GPIO_OUT_LOW) ? "low" : "no set", 
		(gpio->pulldown == AK_PULLDOWN_ENABLE) ? "enable" : "disable",
		(gpio->pullup == AK_PULLUP_ENABLE) ? "enable" : "disable",	
		(gpio->int_pol == AK_GPIO_INT_HIGHLEVEL) ? "high" :
		(gpio->int_pol == AK_GPIO_INT_LOWLEVEL) ? "low" : "no set");
}

static void print_gpios_info(struct akgpio_custom_device *gpioirq, int ngpio)
{
	int i;
	for (i = 0; i < ngpio; i++) {
		printk("akgpios[%d].gpio=%d, akgpios[%d].used=%d\n,"
			"	akgpios[%d].irq_flag=%d, akgpios[%d].locked=%d\n,"
			"	akgpios[%d].gpioinfo=%p, akgpios[%d].sem=%p\n\n",
			i, gpioirq[i].gpio, i, gpioirq[i].used, i, gpioirq[i].irq_flag,
			i, gpioirq[i].locked, i, &gpioirq[i].gpioinfo, i, &gpioirq[i].sem);
	}
}

#else
static void print_gpio_set_info(struct gpio_info *gpio) {}
static void print_gpios_info(struct akgpio_custom_device *gpioirq, int ngpio){}
#endif

static irqreturn_t akgpio_custom_irq(int irq, void *dev_id)
{
	struct akgpio_custom_device *gpioirq = dev_id;
	struct gpio_info *gpio = &gpioirq->gpioinfo;
	unsigned int gpio_level;

	DBG("akgpio irq: irq=%d, gpioirq->gpio=%d\n", irq, gpioirq->gpio);
	
	BUG_ON(irq != ak_gpio_to_irq(gpio->pin));

	gpio_level = ak_gpio_getpin(gpio->pin);
	//disalbe gpio_irq when the button down
	if ((gpio_level && gpio->int_pol) || (!gpio_level && !gpio->int_pol))
		ak_gpio_intpol(gpio->pin, !gpio->int_pol);
	
	//enable gpiot_irq when the button up
	if ((!gpio_level && gpio->int_pol) || (gpio_level && !gpio->int_pol)) {
		ak_gpio_intpol(gpio->pin, gpio->int_pol);

		/* release semaphore lock */
		if (gpioirq->locked) {
			up(&gpioirq->sem);
			gpioirq->locked = 0;
		}
	}
	
	return IRQ_HANDLED;
}

static long akgpio_custom_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int *gpio_array = pdata->gpiopin;
	unsigned int ngpio = pdata->ngpiopin;
	struct gpio_info gpio;
	struct gpio_group_info gpios_info;
	unsigned int irq, gpiopin;
	int i, ret;
	
	switch(cmd) {
		case GET_GPIO_NUM:
			/* get  gpio num */
			if (copy_to_user((void __user *)arg, &ngpio, sizeof(unsigned int)))
				return -EINVAL;			
			break;
		case GET_AVAILABLE_GPIO:
			/* get unused gpio num, and report user space */
			if (copy_to_user((void __user *)arg, gpio_array, ngpio*sizeof(unsigned int)))
				return -EINVAL;			
			break;
			
		case SET_GPIO_FUNC:
			/* set gpio attribute, include dir(in/out), out(low/high), pull(up/down) */
			if (copy_from_user(&gpio, (void __user *)arg, sizeof(struct gpio_info))) 
				return -EINVAL;
			
			ak_gpio_set(&gpio);
			break;
			
		case SET_GPIO_LEVEL:
			/* set gpio level when the pin as output */
			if (copy_from_user(&gpio, (void __user *)arg, sizeof(struct gpio_info))) 
				return -EINVAL;
			
			ak_gpio_setpin(gpio.pin, gpio.value);
			break;
			
		case SET_GROUP_GPIO_LEVEL:
			/* set gpio level when the pin as output */
			if (copy_from_user(&gpios_info, (void __user *)arg, sizeof(struct gpio_group_info))) 
				return -EINVAL;
			
			for(i = 0; i < gpios_info.gpio_num; i++) {
				ak_gpio_setpin(gpios_info.gpio[i].pin, gpios_info.gpio[i].value);
			}
			break;
			
		case GET_GPIO_VALUE:
			/* read gpio value when the pin is input */
			if (copy_from_user(&gpio, (void __user *)arg, sizeof(struct gpio_info))) 
				return -EINVAL;
			
			gpio.value = ak_gpio_getpin(gpio.pin);
			if(copy_to_user((void __user *)arg, &gpio, sizeof(struct gpio_info)))
				return -EINVAL;
			break;
			
		case SET_GPIO_IRQ:
			{
			/* set gpio irq func */
			struct akgpio_custom_device *gpioirq = NULL;
			if (copy_from_user(&gpio, (void __user *)arg, sizeof(struct gpio_info))) 
				return -EINVAL;
			
			for (i = 0; i < ngpio; i++) {
				gpioirq = &akgpios[i];
				if ((gpioirq->used)&&(gpioirq->gpio == gpio.pin)) {
					/* gpio irq had set, return for */
					break;
				} else if (!gpioirq->used) {
					gpioirq->used = 1;
					gpioirq->gpio = gpio.pin;
					down(&gpioirq->sem);
					break;
				}
			}

			for (i = 0; i < ngpio; i++) {
				gpioirq = &akgpios[i];
				if ((gpioirq->used)&&(gpioirq->gpio == gpio.pin)) {
					memcpy(&gpioirq->gpioinfo, &gpio, sizeof(struct gpio_info));
					
					print_gpio_set_info(&gpio);
					ak_gpio_set(&gpio);
					
					if (!gpioirq->irq_flag) {
						irq = ak_gpio_to_irq(gpio.pin);
						DBG("akgpio: gpio[%d], irq[%d]\n", gpio.pin, irq);
						ret = request_irq(irq, akgpio_custom_irq, 
							((gpio.int_pol == AK_GPIO_INT_HIGHLEVEL)?(IRQF_TRIGGER_HIGH):(IRQF_TRIGGER_LOW)), 
							"akgpio", gpioirq);
						if (ret) {
							printk("Request irq failed. ret=%d\n", ret);
							return -EINVAL;
						}
						gpioirq->irq_flag = 1;
					}
					break;
				}
			}
			}
			break;			
		
		case LISTEN_GPIO_IRQ:
			/* wait irq occurs */
			gpiopin = (unsigned int)arg;
			for (i = 0; i < ngpio; i++) {
				struct akgpio_custom_device *gpioirq = &akgpios[i];
				if ((gpioirq->used)&&(gpioirq->gpio == gpiopin)) {
					gpioirq->locked = 1;
					down(&gpioirq->sem);
					/* blocking, wait irq occurs, then continue */
					break;
				}
				
				if (i == ngpio) {
					printk("listen gpio irq: first request irq, then do this.\n");
					return -EINVAL;
				}
			}			
			break;
		
		case DELETE_GPIO_IRQ:
			/* delete gpio irq that specific */
			gpiopin = (unsigned int)arg;
			for (i = 0; i < ngpio; i++) {
				struct akgpio_custom_device *gpioirq = &akgpios[i];
				if ((gpioirq->used)&&(gpioirq->gpio == gpiopin)) {
					irq = ak_gpio_to_irq(gpiopin);
					free_irq(irq, gpioirq);
					gpioirq->irq_flag = 0;
					break;
				}

				if (i == ngpio) {
					printk("delete gpio irq: first request irq, then do this.\n");
					return -EINVAL;
				}
			}
			break;
			
		default:
			printk("akgpio: the ioctl is unknow.\n");
			break;
	}
	
	//print_gpios_info(akgpios, ngpio);
	return 0;
}

static int akgpio_custom_open(struct inode *node, struct file *file)
{
	/* do nothing, return correct */
	printk(KERN_INFO "open akgpio device success.\n");
	return 0;
}

static int akgpio_custom_release(struct inode *node, struct file *file)
{
	/* do nothing */
	return 0;
}

static const struct file_operations akgpio_ops = {
	.owner	= THIS_MODULE,
	.open	= akgpio_custom_open,
	.release = akgpio_custom_release,
	.unlocked_ioctl	= akgpio_custom_ioctl,
};

static struct miscdevice akgpio_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = AKGPIO_DEV_NAME,
	.fops = &akgpio_ops,
	.mode	= S_IRWXO,
};

static int akgpio_custom_probe(struct platform_device *pdev)
{
	int i;
	struct akgpio_custom_device *gpioirq = NULL;

	/* get platform data of akgpio */
	pdata = pdev->dev.platform_data;
	if ((pdata == NULL)||((pdata)&&(pdata->ngpiopin == 0)))
		return -ENODEV;
		
	/* register misc device */
	if (misc_register(&akgpio_dev)) {
		printk(KERN_ERR "akgpio: Unable register misc device.\n");
		return -ENODEV;
	}
	
	gpio_num = pdata->ngpiopin;

	akgpios = kzalloc(gpio_num * sizeof(struct akgpio_custom_device), GFP_KERNEL);
	if (akgpios == NULL)
		return -ENOMEM;
	gpioirq = akgpios;
	
	for (i = 0; i < gpio_num; i++) {
		gpioirq[i].gpio = -1;
		gpioirq[i].used = 0;
		gpioirq[i].irq_flag = 0;
		init_MUTEX(&gpioirq[i].sem);
		gpioirq[i].locked = 0;
	}

	printk("akgpio driver initialize.\n");
	print_gpios_info(gpioirq, gpio_num);
	return 0;
}

static int akgpio_custom_remove(struct platform_device *pdev)
{
	/* release platform data for akgpio */
	int i;
	struct akgpio_custom_device *gpioirq = akgpios;

	pdata = NULL;
	misc_deregister(&akgpio_dev);
	
	for (i = 0; i < gpio_num; i++) {
		gpioirq[i].gpio = -1;
		gpioirq[i].used = 0;
		gpioirq[i].irq_flag = 0;
		gpioirq[i].locked = 0;
	}
		
	return 0;
}

static struct platform_driver akgpio_custom_driver = {
	.probe = akgpio_custom_probe,
	.remove = __devexit_p(akgpio_custom_remove),
	.driver = {
		.owner = THIS_MODULE,
		.name = AKGPIO_DEV_NAME,
	},
};

static int __init akgpio_custom_init(void)
{
	return platform_driver_register(&akgpio_custom_driver);
}

static void __exit akgpio_custom_exit(void)
{
	platform_driver_unregister(&akgpio_custom_driver);
}

module_init(akgpio_custom_init);
module_exit(akgpio_custom_exit);

MODULE_AUTHOR("Anyka Microelectronic Ltd.");
MODULE_DESCRIPTION("Anyka gpio apply for user space control");
MODULE_ALIAS("Anyka GPIO Apply");
MODULE_LICENSE("GPL");

