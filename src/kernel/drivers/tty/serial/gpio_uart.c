#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/circ_buf.h>
#include <mach/gpio.h>
#include <linux/delay.h>

#include <mach/pwm_timer.h>

#define TIMER_CLOCK		(12000000)	/* Hz */
#define GPIO_SERIAL_NAME	("ttySAK")
#define GPIO_SERIAL_NR_PORTS	(1)
#define FAKE_PORT_MEMBASE 	((unsigned char __iomem*)(0xffffffff))
#define FAKE_PORT_MAPBASE 	((resource_size_t)(0xffffffff))

struct gpio_uart {
	int baud;
	int parity;
	int bits;
	int flow;

	int timer_count;
	int correction;			/* correction coefficient */

	struct ak_pwm_timer *timer;
	struct completion comp;
	struct gpio_info tx_pin;
};

static struct gpio_uart guart = {
	/* UART configure: 115200/75, 38400/80, 19200/80, 9600/80 */
	.baud		= 115200,
	.parity		= 'n',
	.bits		= 8,
	.flow		= 'n',
	.correction	= 55,

	/* TX GPIO pin configure */
	.tx_pin		= {
		.pin		= AK_GPIO_2,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.pullup		= AK_PULLUP_DISABLE,
		.pulldown	= -1,
		.value		= AK_GPIO_OUT_HIGH,
		.int_pol	= -1,
	},
};

#ifdef CONFIG_SERIAL_CORE_CONSOLE
#define uart_console(port)	((port)->cons && (port)->cons->index == (port)->line)
#else
#define uart_console(port)	(0)
#endif

#ifdef CONFIG_SERIAL_GPIO_CONSOLE
static struct uart_port port;
static struct uart_driver gpio_uart_driver;
static void gpio_putchar(unsigned char value);


static void gpio_uart_console_write(struct console *co, const char *s, unsigned count)
{
	unsigned int i;
	const char *ts = s;

	for (i = 0; i < count; i++, ts++) {
		if (*ts == '\n')
			gpio_putchar('\r');
		gpio_putchar(*ts);
	}
}

static struct tty_driver *gpio_uart_console_device(struct console *co, int *index)
{
	struct uart_driver *p = co->data;

	*index = co->index;
	return p->tty_driver;
}

static int gpio_uart_console_setup(struct console *co, char *options)
{
	int baud;
	int bits;
	int parity;
	int flow;

	ak_gpio_set(&guart.tx_pin);
	guart.timer_count = TIMER_CLOCK / guart.baud - guart.correction;
	port.mapbase = FAKE_PORT_MAPBASE;
	port.membase = FAKE_PORT_MEMBASE;

	if (options) {
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	} else {
		baud = guart.baud;
		bits = guart.bits;
		parity = guart.parity;
		flow = guart.flow;
	}

	return uart_set_options(&port, co, baud, parity, bits, flow);
}

static struct console gpio_serial_console = {
	.name		= GPIO_SERIAL_NAME,
	.write		= gpio_uart_console_write,
	.device		= gpio_uart_console_device,
	.setup		= gpio_uart_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &gpio_uart_driver,
};

static int __init gpio_serial_console_init(void)
{
	guart.timer = ak_timer_request(AK_PWM_TIMER2, 0, NULL);

	register_console(&gpio_serial_console);
	return 0;
}
console_initcall(gpio_serial_console_init);

#define GPIO_SERIAL_CONSOLE	&gpio_serial_console
#else
#define GPIO_SERIAL_CONSOLE	NULL
#endif
#if 0
static void gpio_delay(int count)
{
#if 0
	int delay = 1000*1000/115200;
	udelay(delay);
	return;
#endif

	REG32(guart.membase) = (count << 0);

	REG32(guart.membase+0x04) = (1 << 30);
	REG32(guart.membase+0x04) |= (1 << 24);
	REG32(guart.membase+0x04) |= (1 << 29) | (1 << 28);

	while(!(REG32(guart.membase+0x04) & (1 << 27)))
		;
}
#else

static void gpio_delay(int count)
{
	init_completion(&guart.comp);

	ak_timer_config(guart.timer, count, 0);
	ak_timer_enable_sync(guart.timer);
}

#endif

static void gpio_putchar(unsigned char value)
{
	int count = 8;

	/* send start bit */
	ak_gpio_setpin(guart.tx_pin.pin, AK_GPIO_OUT_LOW);
	gpio_delay(guart.timer_count);

	/* send data, no parity */
	while(count--) {
		ak_gpio_setpin(guart.tx_pin.pin, (value & 0x1));
		gpio_delay(guart.timer_count);
		value >>= 1;
	}

	/* send stop bit */
	ak_gpio_setpin(guart.tx_pin.pin, AK_GPIO_OUT_HIGH);
	gpio_delay(guart.timer_count);
}

static void gpio_uart_pm(struct uart_port *port, unsigned int state, unsigned int oldstate)
{
	switch (state) {
	case 3:
		break;
	case 0:
		break;
	default:
		break;
	}
}

static unsigned int gpio_uart_tx_empty(struct uart_port *port)
{
	return 1;
}

static unsigned int gpio_uart_get_mctrl(struct uart_port *port)
{
	return 0;
}

static void gpio_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static void gpio_uart_stop_tx(struct uart_port *port)
{
}

static void gpio_uart_start_tx(struct uart_port *port)
{
	struct uart_state *state = port->state;
	struct circ_buf *circ = &state->xmit;
	int txcount = uart_circ_chars_pending(circ);
	unsigned char value = 0;
	int i;

	for (i = 0; i < txcount; i++) {
		value = circ->buf[circ->tail];
		gpio_putchar(value);
		circ->tail = (circ->tail + 1) & (UART_XMIT_SIZE - 1);
	}
}

static void gpio_uart_stop_rx(struct uart_port *port)
{
}

static void gpio_uart_enable_ms(struct uart_port *port)
{
}

static void gpio_uart_break_ctl(struct uart_port *port, int break_state)
{
}

static int gpio_uart_startup(struct uart_port *port)
{
	return 0;
}

static void gpio_uart_shutdown(struct uart_port *port)
{
}

static void gpio_uart_set_termios(struct uart_port *port, 
				struct ktermios *termios, struct ktermios *old)
{
}

static const char *gpio_uart_type(struct uart_port *port)
{
	if (port->type == PORT_GPIO)
		return "GPIO";
	else
		return NULL;
}

static void gpio_uart_release_port(struct uart_port *port)
{
}

static int gpio_uart_request_port(struct uart_port *port)
{
	return 0;
}

static void gpio_uart_config_port(struct uart_port *port, int flags)
{
	port->type = PORT_GPIO;
}

static int gpio_uart_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	return 0;
}

static struct uart_ops gpio_uart_ops = {
	.pm             = gpio_uart_pm,
	.tx_empty       = gpio_uart_tx_empty,
	.get_mctrl      = gpio_uart_get_mctrl,
	.set_mctrl      = gpio_uart_set_mctrl,
	.stop_tx        = gpio_uart_stop_tx,
	.start_tx       = gpio_uart_start_tx,
	.stop_rx        = gpio_uart_stop_rx,
	.enable_ms      = gpio_uart_enable_ms,
	.break_ctl      = gpio_uart_break_ctl,
	.startup        = gpio_uart_startup,
	.shutdown       = gpio_uart_shutdown,
	.set_termios    = gpio_uart_set_termios,
	.type           = gpio_uart_type,
	.release_port   = gpio_uart_release_port,
	.request_port   = gpio_uart_request_port,
	.config_port    = gpio_uart_config_port,
	.verify_port    = gpio_uart_verify_port,
};

static struct uart_port port = {
	.iotype		= UPIO_MEM,
	.ops		= &gpio_uart_ops,
	.flags		= UPF_BOOT_AUTOCONF,
	.line		= 0,
	.cons 		= GPIO_SERIAL_CONSOLE, 
};

static struct uart_driver gpio_uart_driver = {
	.owner		= THIS_MODULE,
	.driver_name	= "gpio-uart",
	.dev_name	= GPIO_SERIAL_NAME,
	.major		= 0,
	.minor		= 0,
	.nr		= GPIO_SERIAL_NR_PORTS,
	.cons		= GPIO_SERIAL_CONSOLE,
};

static int gpio_uart_probe(struct platform_device *pdev)
{
	int ret;

	if (!uart_console(&port)) {
		port.membase = FAKE_PORT_MEMBASE;
		port.mapbase = FAKE_PORT_MAPBASE;
		guart.timer_count = TIMER_CLOCK / guart.baud - guart.correction;
		ak_gpio_set(&guart.tx_pin);
		guart.timer = ak_timer_request(AK_PWM_TIMER2, 0, NULL);
	}

	ret = uart_register_driver(&gpio_uart_driver);
	if (ret) {
		printk("UART register driver failed\n");
		return ret;
	}

	port.dev = &pdev->dev;
	ret = uart_add_one_port(&gpio_uart_driver, &port);
	if (ret) {
		printk("Add gpio uart port failed\n");
		return ret;
	}

	platform_set_drvdata(pdev, &port);

	printk("ak gpio uart initialize success.\n");
	return 0;
}

static int gpio_uart_remove(struct platform_device *pdev)
{
	platform_set_drvdata(pdev, NULL);
	uart_remove_one_port(&gpio_uart_driver, &port);
	uart_unregister_driver(&gpio_uart_driver);
	return 0;
}

static int gpio_uart_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int gpio_uart_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver gpio_uart_pdrv = {
	.probe		= gpio_uart_probe,
	.remove		= gpio_uart_remove,
	.suspend	= gpio_uart_suspend,
	.resume		= gpio_uart_resume,
	.driver		= {
		.name	= "gpio-uart",
		.owner	= THIS_MODULE,
	},
};

static int __init gpio_uart_init(void)
{
	printk("AK GPIO UART Driver (c) 2013 ANYKA.\n");
	return platform_driver_register(&gpio_uart_pdrv);
}

static void __exit gpio_uart_exit(void)
{
	platform_driver_unregister(&gpio_uart_pdrv);
}
module_init(gpio_uart_init);
module_exit(gpio_uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("J.C.Zhong <zhong_junchao@anyka.com>");
MODULE_DESCRIPTION("GPIO simulate UART driver");
