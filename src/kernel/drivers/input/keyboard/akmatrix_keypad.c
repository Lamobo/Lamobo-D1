/**
*  @file      ak_matrix_keypad.c
*  @brief     GPIO driven matrix keyboard driver
*   Copyright C 2011 Anyka CO.,LTD
*   Based on matrix_keypad.c
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*  @author    zhou wenyong
*  @date      2011-08-05
*  @note      2010-12-24 created by cao_lianming
*  @note      2011-06-25 integrate and optimize matrix keypad driver -- 
*              only one copy code exists now. we use the same code whether
*              or not keypad is connected aw9523, whether or not there is a
*              grounding line.
*  @note      2011-06-27 fix a bug imported from previous version, which is that
*              driver can not work properly if there is grounding line in 3X3 
*              or 4X4 matrix keypad
*  @note      2011-07-08 fix the issue of  incorrect responsing to long press 
*             of menu key
*
* Notes: if the comments look like inaesthetic, try to press 
*        Alt+F12(source insight)
*/

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/input/akmatrix_keypad.h>
#include <mach/gpio.h>
#include <linux/slab.h>

/*
    structure used to present the keypad
*/
struct matrix_keypad {
	struct matrix_keypad_platform_data *pdata;
	struct input_dev *input_dev;
	unsigned short *keycodes;
	unsigned int row_shift;

	uint32_t last_key_state[MATRIX_MAX_COLS];
	struct delayed_work work;
	bool scan_pending;
	bool stopped;
	spinlock_t lock;
	bool start_close_int;
};

/**
*  @brief       get the value of gpio (this function may sleep)
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   gpio
*  @return      int
*/
static inline int ak_gpio_get_value_cansleep(unsigned gpio)
{
	might_sleep();
	return ak_gpio_getpin(gpio);
}
/**
*  @brief       set the value of gpio (this function may sleep)
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   gpio
*  @param[in]   value
*  @return      void
*/
static inline void ak_gpio_set_value_cansleep(unsigned gpio, int value)
{
	might_sleep();
	ak_gpio_setpin(gpio, value);
}

/**
*  @brief       called by activate_other_col (refer to it)
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]  *pdata
*  @param[in]   col
*  @param[in]   on
*  @return      void
*/
static void __activate_col(const struct matrix_keypad_platform_data *pdata,
			   int col, bool on)
{
	bool level_on = pdata->active_low;
	int i;

	for (i=0; i<pdata->num_col_gpios;i++)
	{
		if (i == col)
			continue;
		if (on) 
		{
			ak_gpio_setpin(pdata->col_gpios[i], level_on);
		} 
		else 
		{
			ak_gpio_setpin(pdata->col_gpios[i], !level_on);
		}
	}
}

/**
*  @brief       activate other colums
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *pdata
*  @param[in]   col -- the column excluded
*  @param[in]   on
*  @return      void
*/
static void activate_other_col(const struct matrix_keypad_platform_data *pdata,
			 int col, bool on)
{
	__activate_col(pdata, col, on);

	if (on && pdata->col_scan_delay_us)
		udelay(pdata->col_scan_delay_us);
}

/**
*  @brief       get the value of gpio (can sleep)
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *pdata
*  @param[in]   row
*  @return      bool
*/
static bool row_asserted(const struct matrix_keypad_platform_data *pdata,
			 int row)
{
	return ak_gpio_get_value_cansleep(pdata->row_gpios[row]) ?
			!pdata->active_low : pdata->active_low;
}

/**
*  @brief       enable_row_irqs
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *keypad
*  @return      void
*/
static void enable_row_irqs(struct matrix_keypad *keypad)
{
	const struct matrix_keypad_platform_data *pdata = keypad->pdata;
	int i;


	for (i = 0; i < pdata->num_row_gpios; i++)
		enable_irq(ak_gpio_to_irq(pdata->row_gpios[i]));
}

/**
*  @brief       disable_row_irqs
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *keypad
*  @return      void
*/
static void disable_row_irqs(struct matrix_keypad *keypad)
{
	const struct matrix_keypad_platform_data *pdata = keypad->pdata;
	int i;


	for (i = 0; i < pdata->num_row_gpios; i++)
		disable_irq_nosync(ak_gpio_to_irq(pdata->row_gpios[i]));
}

/**
*  @brief       print the CODE of button pressed -- for debugging
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   code
*  @param[in]   state
*  @return      void
*/
static void print_code(int code, int state)	
{	
	switch(code)
	{
	case KEY_VOLUMEUP:
		printk("KEY_VOLUMEUP  "); break;
	case KEY_RIGHT:
		printk("KEY_RIGHT  "); break;
	case KEY_MENU:
		printk("KEY_MENU  "); break;
	case KEY_UP:
		printk("KEY_UP  "); break;
	case KEY_REPLY:
		printk("KEY_CENTER  "); break;
	case KEY_DOWN:
		printk("KEY_DOWN  "); break;
	case KEY_VOLUMEDOWN:
		printk("KEY_VOLUMEDOWN  "); break;
	case KEY_LEFT:
		printk("KEY_LEFT  "); break;
	case KEY_HOME:
		printk("KEY_HOME  "); break;
	case KEY_BACK:
		printk("KEY_BACK  "); break;
	}
	printk("%s \n", state?"Down":"Up");
	
	
}

EXPORT_SYMBOL(print_code);

/**
*  @brief       This gets the keys from keyboard and reports it to input 
*                subsystem
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[out]  *work
*  @return      void
*/
static void matrix_keypad_scan(struct work_struct *work)
{
	struct matrix_keypad *keypad =
		container_of(work, struct matrix_keypad, work.work);
	struct input_dev *input_dev = keypad->input_dev;
	const struct matrix_keypad_platform_data *pdata = keypad->pdata;
	uint32_t new_state[MATRIX_MAX_COLS];
	int row, col, code, num_cols;
	
	memset(new_state, 0, sizeof(new_state));


	num_cols = pdata->num_col_gpios;
	if (pdata->grounding)
		num_cols++;
	for (col = 0; col < num_cols; col++)
	{		
		activate_other_col(pdata, col, true);	
		for (row = 0; row < pdata->num_row_gpios; row++)
		{
			new_state[col] |=
				row_asserted(pdata, row) ? (1 << row) : 0;
			
		}	
		
		activate_other_col(pdata, col, false);
	}	

	/*
	 *  if the button pressed is connected to the grounding line, the state of 
	 *  the row input line connected to the button pressed will keep low level 
	 *  when we activate other lines
	*/
	if (pdata->grounding)
	{
		for (row=0; row < pdata->num_row_gpios; row++) 
		{
			for (col=0; col<num_cols; col++)
				if ( !(new_state[col]&(1<<row)))
					break;
			if (col == num_cols)
			{
				//grounding line at the end
				for (col=0; col<num_cols-1; col++)
				{
					new_state[col] = new_state[col] & (~(1<<row));
				}
			}
		}

	}
	/* update input status, needed if keypad is connected to AW9523 */
	ak_gpio_getpin(pdata->row_gpios[0]);

	num_cols = pdata->num_col_gpios;
	if (pdata->grounding)
		num_cols++;
	for (col = 0; col < num_cols; col++) {
		uint32_t bits_changed;

		bits_changed = (keypad->last_key_state[col] ^ new_state[col]);

		
		if (bits_changed == 0)
			continue;

		for (row = 0; row < pdata->num_row_gpios; row++) {
			if ((bits_changed & (1 << row)) == 0)
				continue;

			code = MATRIX_SCAN_CODE(row, col, keypad->row_shift);
			input_event(input_dev, EV_MSC, MSC_SCAN, code);
			input_report_key(input_dev,
					 keypad->keycodes[code],
					 new_state[col] & (1 << row));
			//print_code(keypad->keycodes[code], new_state[col] & (1 << row));
		}
	}
	input_sync(input_dev);

	memcpy(keypad->last_key_state, new_state, sizeof(new_state));


	/* Enable IRQs again */
	spin_lock_irq(&keypad->lock);
	keypad->scan_pending = false;
	enable_row_irqs(keypad);
	spin_unlock_irq(&keypad->lock);
}

/**
*  @brief       matrix keypad interrupt routine
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   irq
*  @param[out]  *id
*  @return      irqreturn_t
*/
static irqreturn_t matrix_keypad_interrupt(int irq, void *id)
{
	struct matrix_keypad *keypad = id;
	unsigned long flags;


	spin_lock_irqsave(&keypad->lock, flags);

	/*
	 * See if another IRQ beaten us to it and scheduled the
	 * scan already. In that case we should not try to
	 * disable IRQs again.
	 */
	if (unlikely(keypad->scan_pending || keypad->stopped))
		goto out;

	disable_row_irqs(keypad);
	keypad->scan_pending = true;
	schedule_delayed_work(&keypad->work,
		msecs_to_jiffies(keypad->pdata->debounce_ms));

out:
	/* 
	  *	if disable_row_irqs(keypad); in init_matrix_gpio is not reached, then
	  *	we need excuting it here. otherwise system can not start successfully if
	  *	one key is pressed on OS starts.
	*/
	if (!keypad->start_close_int)
	{
		disable_row_irqs(keypad);
		keypad->start_close_int = true;
	}
	spin_unlock_irqrestore(&keypad->lock, flags);
	return IRQ_HANDLED;
}

/**
*  @brief       start the keypad
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *dev
*  @return      int
*/
static int matrix_keypad_start(struct input_dev *dev)
{
	struct matrix_keypad *keypad = input_get_drvdata(dev);

	keypad->stopped = false;
	mb();

	/*
	 * Schedule an immediate key scan to capture current key state;
	 * columns will be activated and IRQs be enabled after the scan.
	 */
	schedule_delayed_work(&keypad->work, 0);

	return 0;
}

/**
*  @brief       stop the keypad
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[out]  *dev
*  @return      void
*/
static void matrix_keypad_stop(struct input_dev *dev)
{
	struct matrix_keypad *keypad = input_get_drvdata(dev);

	keypad->stopped = true;
	mb();
	flush_work(&keypad->work.work);
	/*
	 * matrix_keypad_scan() will leave IRQs enabled;
	 * we should disable them now.
	 */
	disable_row_irqs(keypad);
}

#ifdef CONFIG_PM
/**
*  @brief       matrix_keypad_suspend
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *pdev
*  @param[in]   state
*  @return      int
*/
static int matrix_keypad_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct matrix_keypad *keypad = platform_get_drvdata(pdev);
	const struct matrix_keypad_platform_data *pdata = keypad->pdata;
	int i;

	matrix_keypad_stop(keypad->input_dev);

	if (device_may_wakeup(&pdev->dev))
		for (i = 0; i < pdata->num_row_gpios; i++)
			enable_irq_wake(ak_gpio_to_irq(pdata->row_gpios[i]));

	return 0;
}

/**
*  @brief       matrix_keypad_resume
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *pdev
*  @return      int
*/
static int matrix_keypad_resume(struct platform_device *pdev)
{
	struct matrix_keypad *keypad = platform_get_drvdata(pdev);
	const struct matrix_keypad_platform_data *pdata = keypad->pdata;
	int i;

	if (device_may_wakeup(&pdev->dev))
		for (i = 0; i < pdata->num_row_gpios; i++)
			disable_irq_wake(ak_gpio_to_irq(pdata->row_gpios[i]));

	matrix_keypad_start(keypad->input_dev);

	return 0;
}
#else
#define matrix_keypad_suspend	NULL
#define matrix_keypad_resume	NULL
#endif

/**
*  @brief       init_matrix_gpio
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *pdev
*  @param[in]   *keypad
*  @return      int __devinit
*/
static int __devinit init_matrix_gpio(struct platform_device *pdev,
					struct matrix_keypad *keypad)
{
	struct matrix_keypad_platform_data *pdata = keypad->pdata;
	int i, err = -(EINVAL);
	unsigned long flags;

	/* to fix the bug: can not start OS if one key is pressed on OS starts */
	keypad->start_close_int = false;
	/* initialized strobe lines as outputs, activated */
	for (i = 0; i < pdata->num_col_gpios; i++) {
			
		err = ak_gpio_request(pdata->col_gpios[i], "matrix_kbd_col");
		if (err) {
			dev_err(&pdev->dev,
				"failed to request GPIO%d for COL%d\n",
				pdata->col_gpios[i], i);
			goto err_free_cols;
		}
		pdata->col_gpios_cfginfo.pin = pdata->col_gpios[i];
		ak_gpio_set(&(pdata->col_gpios_cfginfo));	
		
	}

	for (i = 0; i < pdata->num_row_gpios; i++) {
		
		err = ak_gpio_request(pdata->row_gpios[i], "matrix_kbd_row");
		if (err) {
			dev_err(&pdev->dev,
				"failed to request GPIO%d for ROW%d\n",
				pdata->row_gpios[i], i);
			goto err_free_rows;
		}
		pdata->row_gpios_cfginfo.pin = pdata->row_gpios[i];
		ak_gpio_set(&(pdata->row_gpios_cfginfo));
		ak_gpio_intcfg(pdata->row_gpios[i], AK_GPIO_INT_ENABLE);		
	}


	for (i = 0; i < pdata->num_row_gpios; i++) {
		err = request_irq(ak_gpio_to_irq(pdata->row_gpios[i]),
				matrix_keypad_interrupt,
				IRQF_DISABLED,
				"matrix-keypad", keypad);
		if (err) {
			dev_err(&pdev->dev,
				"Unable to acquire interrupt for GPIO line %i\n",
				pdata->row_gpios[i]);
			goto err_free_irqs;
		}
	}
	
	/* update input status, needed if keypad is connected to AW9523 */
	ak_gpio_getpin(pdata->row_gpios[0]);
	
	spin_lock_irqsave(&keypad->lock, flags);
	/* initialized as disabled - enabled by input->open */
	if (!keypad->start_close_int)
	{
		disable_row_irqs(keypad);
		keypad->start_close_int = true;
	}
	spin_unlock_irqrestore(&keypad->lock, flags);
	return 0;

err_free_irqs:
	while (--i >= 0)
		free_irq(ak_gpio_to_irq(pdata->row_gpios[i]), keypad);
	i = pdata->num_row_gpios;
err_free_rows:
	while (--i >= 0)
		ak_gpio_free(pdata->row_gpios[i]);
	i = pdata->num_col_gpios;
err_free_cols:
	while (--i >= 0)
		ak_gpio_free(pdata->col_gpios[i]);

	return err;
}

/**
*  @brief       matrix_keypad_probe
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *pdev
*  @return      int __devinit
*/
static int __devinit matrix_keypad_probe(struct platform_device *pdev)
{
	struct matrix_keypad_platform_data *pdata;
	const struct matrix_keymap_data *keymap_data;
	struct matrix_keypad *keypad;
	struct input_dev *input_dev;
	unsigned short *keycodes;
	unsigned int row_shift;
	int err;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data defined\n");
		return -EINVAL;
	}

	keymap_data = pdata->keymap_data;
	if (!keymap_data) {
		dev_err(&pdev->dev, "no keymap data defined\n");
		return -EINVAL;
	}

	row_shift = get_count_order(pdata->num_col_gpios);
	if (pdata->grounding)
		row_shift++;

	keypad = kzalloc(sizeof(struct matrix_keypad), GFP_KERNEL);
	keycodes = kzalloc((pdata->num_row_gpios << (row_shift)) *
				sizeof(*keycodes),
			   GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!keypad || !keycodes || !input_dev) {
		err = -ENOMEM;
		goto err_free_mem;
	}

	keypad->input_dev = input_dev;
	keypad->pdata = pdata;
	keypad->keycodes = keycodes;
		
	keypad->row_shift = row_shift;	
	keypad->stopped = true;
	INIT_DELAYED_WORK(&keypad->work, matrix_keypad_scan);
	spin_lock_init(&keypad->lock);

	input_dev->name		= pdev->name;
	input_dev->id.bustype	= BUS_HOST;
	input_dev->dev.parent	= &pdev->dev;
	input_dev->evbit[0]	= BIT_MASK(EV_KEY);// | BIT_MASK(EV_REP);
	input_dev->open		= matrix_keypad_start;
	input_dev->close	= matrix_keypad_stop;

	input_dev->keycode	= keycodes;
	input_dev->keycodesize	= sizeof(*keycodes);
	input_dev->keycodemax	= pdata->num_row_gpios << (row_shift);

	matrix_keypad_build_keymap(keymap_data, row_shift,
				   input_dev->keycode, input_dev->keybit);

	input_set_capability(input_dev, EV_MSC, MSC_SCAN);
	input_set_drvdata(input_dev, keypad);

	err = init_matrix_gpio(pdev, keypad);
	if (err)
		goto err_free_mem;

	err = input_register_device(keypad->input_dev);
	if (err)
		goto err_free_mem;

	device_init_wakeup(&pdev->dev, pdata->wakeup);
	platform_set_drvdata(pdev, keypad);

	return 0;

err_free_mem:
	input_free_device(input_dev);
	kfree(keycodes);
	kfree(keypad);
	return err;
}

/**
*  @brief       called when the device is removed
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   *pdev
*  @return      int __devexit
*/
static int __devexit matrix_keypad_remove(struct platform_device *pdev)
{
	struct matrix_keypad *keypad = platform_get_drvdata(pdev);
	const struct matrix_keypad_platform_data *pdata = keypad->pdata;
	int i;

	device_init_wakeup(&pdev->dev, 0);

	for (i = 0; i < pdata->num_row_gpios; i++) {
		free_irq(ak_gpio_to_irq(pdata->row_gpios[i]), keypad);
		ak_gpio_free(pdata->row_gpios[i]);
	}

	for (i = 0; i < pdata->num_col_gpios; i++)
		ak_gpio_free(pdata->col_gpios[i]);

	input_unregister_device(keypad->input_dev);
	platform_set_drvdata(pdev, NULL);
	kfree(keypad->keycodes);
	kfree(keypad);

	return 0;
}

static struct platform_driver matrix_keypad_driver = {
	.probe		= matrix_keypad_probe,
	.remove		= __devexit_p(matrix_keypad_remove),
	.suspend	= matrix_keypad_suspend,
	.resume		= matrix_keypad_resume,
	.driver		= {
		.name	= "matrix-keypad",
		.owner	= THIS_MODULE,
	},
};

/**
*  @brief       matrix_keypad_init
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   void
*  @return      int __init
*/
static int __init matrix_keypad_init(void)
{
	return platform_driver_register(&matrix_keypad_driver);
}

/**
*  @brief       matrix_keypad_exit
*  @author      zhou wenyong
*  @date        2011-08-05
*  @param[in]   void
*  @return      void __exit
*/
static void __exit matrix_keypad_exit(void)
{
	platform_driver_unregister(&matrix_keypad_driver);
}

module_init(matrix_keypad_init);
module_exit(matrix_keypad_exit);

MODULE_AUTHOR("Anyka <xxx@anyka.oa>");
MODULE_DESCRIPTION("GPIO Driven Matrix Keypad Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:matrix-keypad");

