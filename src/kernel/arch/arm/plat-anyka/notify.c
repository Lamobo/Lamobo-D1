/*
 * power_notify.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/device.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <plat-anyka/notify.h>

static BLOCKING_NOTIFIER_HEAD(power_notifier_list);
static BLOCKING_NOTIFIER_HEAD(addetect_notifier_list);

/*
 * @brief register a power client notifier
 * @author Li Xiaoping
 * @date 2011-08-02
 * @param [in] nb notifier block to callback on events
 * @return 0
 */
int power_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&power_notifier_list, nb);
}
EXPORT_SYMBOL(power_register_client);

/*
 * @brief unregister a power client notifier
 * @author Li Xiaoping
 * @date 2011-08-02
 * @param [in] nb notifier block to callback on events
 * @return 0
 */
int power_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&power_notifier_list, nb);
}
EXPORT_SYMBOL(power_unregister_client);

/*
 * @brief notify clients of power_events
 * @author Li Xiaoping
 * @date 2011-08-02
 * @param [in] val - notifier events dispatch to clients
 * @param [in] v - event data associated with events
 * @return 0
 */
int power_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&power_notifier_list, val, v);
}
EXPORT_SYMBOL_GPL(power_notifier_call_chain);


/*
 * @brief register a ad detect notifier
 * @author caolianming
 * @date 2012-09-28
 * @param [in] nb notifier block to callback on events
 * @return 0
 */
int addetect_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&addetect_notifier_list, nb);
}
EXPORT_SYMBOL(addetect_register_client);

/*
 * @brief unregister a ad detect client notifier
 * @author caolianming
 * @date 2012-09-28
 * @param [in] nb notifier block to callback on events
 * @return 0
 */
int addetect_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&addetect_notifier_list, nb);
}
EXPORT_SYMBOL(addetect_unregister_client);

/*
 * @brief notify clients of ad deteced events
 * @author caolianming
 * @date 2012-09-28
 * @param [in] val - notifier events dispatch to clients
 * @param [in] v - event data associated with events
 * @return 0
 */
int addetect_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&addetect_notifier_list, val, v);
}
EXPORT_SYMBOL_GPL(addetect_notifier_call_chain);

