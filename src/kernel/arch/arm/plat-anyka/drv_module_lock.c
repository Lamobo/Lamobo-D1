
/**
 * arch/arm/plat-anyka/drv_module_lock.c
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <plat-anyka/drv_module_lock.h>

//#define DRV_LOCK_DEBUG
#ifdef DRV_LOCK_DEBUG
#define PK(fmt...)	printk(const char * fmt,...)
#endif

#if defined CONFIG_ARCH_AK39
/* the param drv_shpin_lock[i].name indicate different lock */
struct ak_drv_module_lock drv_mod_lock[] = {
	{DRV_MODULE_SDIO,AK_MODULE_LOCK_1, TYPE_LOCK_SEMAPHORE, 0, ePIN_AS_SDIO},
	{DRV_MODULE_SPI,AK_MODULE_LOCK_1, TYPE_LOCK_SEMAPHORE, 0, ePIN_AS_SPI1},

};
#endif

static void *module_lock_array[AK_MODULE_COUNT] = { 0 };

#define GET_DRV_LOCK_INFO(type, table, len, lock_name, lock) \
{	\
    int i;  \
    type *drv_lock = table;		\
    for (i = 0; i < len; i++) {     \
        if (drv_lock[i].lock_name == lock_name) { \
            lock = &drv_lock[i];    \
            break;				\
        }   \
    }	\
}

static void ak_acquire_lock(void **lock_array, 
	E_LOCK_NAME lock_name, E_LOCK_TYPE lock_type, unsigned long *flags)
{
	unsigned long flag;
	
	switch(lock_type) {
		case TYPE_LOCK_MUTEX:
			mutex_lock((struct mutex *)lock_array[lock_name]);
			break;
		case TYPE_LOCK_SEMAPHORE:
			down((struct semaphore *)lock_array[lock_name]);
			break;
		case TYPE_LOCK_SPINLOCK:
			spin_lock((spinlock_t *)lock_array[lock_name]);
			break;
		case TYPE_LOCK_SPINLOCK_IRQ:
			spin_lock_irqsave((spinlock_t *)lock_array[lock_name], flag);
			*flags = flag;
			break;
		default:
			BUG();
	}
}

static void ak_release_lock(void **lock_array, 
	E_LOCK_NAME lock_name, E_LOCK_TYPE lock_type, unsigned long flags)
{
	switch(lock_type) {
		case TYPE_LOCK_MUTEX:
			mutex_unlock((struct mutex *)lock_array[lock_name]);
			break;
		case TYPE_LOCK_SEMAPHORE:
			up((struct semaphore *)lock_array[lock_name]);
			break;
		case TYPE_LOCK_SPINLOCK:
			spin_unlock((spinlock_t *)lock_array[lock_name]);
			break;
		case TYPE_LOCK_SPINLOCK_IRQ:
			spin_unlock_irqrestore((spinlock_t *)lock_array[lock_name], flags);
			break;
		default:
			BUG();
	}
}

static int ak_module_lock(E_DRV_MODULE drv_mod_name,
	struct ak_drv_module_lock *mod_lock, int len, void **lock_array)
{
	struct ak_drv_module_lock *lock = NULL;
	GET_DRV_LOCK_INFO(struct ak_drv_module_lock, mod_lock, len, drv_mod_name, lock);
	if (lock == NULL)
		return -1;

	ak_acquire_lock(lock_array, lock->lock_name, lock->lock_type, &lock->flags);
	ak_group_config(lock->sharepin_cfg);
	return 0;
}

static void ak_module_unlock(E_DRV_MODULE drv_mod_name,
	struct ak_drv_module_lock *mod_lock, int len, void **lock_array)
{
	struct ak_drv_module_lock *lock = NULL;
	GET_DRV_LOCK_INFO(struct ak_drv_module_lock, mod_lock, len, drv_mod_name, lock);
	if (lock == NULL)
		return;
	
	ak_release_lock(lock_array, lock->lock_name, lock->lock_type, lock->flags);
}

int ak_drv_module_lock(E_DRV_MODULE drv_mod_name) 
{
	return ak_module_lock(drv_mod_name, drv_mod_lock, 
				ARRAY_SIZE(drv_mod_lock), module_lock_array);
}
EXPORT_SYMBOL(ak_drv_module_lock);

void ak_drv_module_unlock(E_DRV_MODULE drv_mod_name) 
{
	ak_module_unlock(drv_mod_name, drv_mod_lock, 
				ARRAY_SIZE(drv_mod_lock), module_lock_array);
}
EXPORT_SYMBOL(ak_drv_module_unlock);


static void ak_init_lock_array(void **lock_array, int len)
{
	int i;

	for (i = 0; i < len; i++)
		lock_array[i] = NULL;
}

static void ak_init_lock(void **lock_array, E_LOCK_NAME lock_name, E_LOCK_TYPE lock_type)
{
	if (lock_array[lock_name] == NULL) {
		switch (lock_type) {
		case TYPE_LOCK_MUTEX:
			lock_array[lock_name] = kmalloc(sizeof(struct mutex), GFP_KERNEL);
			mutex_init((struct mutex *)lock_array[lock_name]);
			break;
		case TYPE_LOCK_SEMAPHORE:
			lock_array[lock_name] = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
			sema_init((struct semaphore *)lock_array[lock_name], 1);
			break;
		case TYPE_LOCK_SPINLOCK:
		case TYPE_LOCK_SPINLOCK_IRQ:
			lock_array[lock_name] = kmalloc(sizeof(spinlock_t), GFP_KERNEL);
			spin_lock_init((spinlock_t *)lock_array[lock_name]);
			break;
		default:
			BUG();
		}
	}		
}

static int ak_init_module_locks(void)
{
	int i, len;

	if ((len = ARRAY_SIZE(drv_mod_lock)) == 0)
		return -1;
	
	ak_init_lock_array(module_lock_array, AK_MODULE_COUNT);

	for (i = 0; i < len; i++) {
		ak_init_lock(module_lock_array, drv_mod_lock[i].lock_name, drv_mod_lock[i].lock_type);
		drv_mod_lock[i].flags = 0;
	}

	return 0;
}


/* ************* CONFIG GPIO' SHARE FUNC  FOR MACHINE ************ */
struct ak_drv_sharepin_lock *shpin_lock = NULL;
int shpin_lock_count = 0;
void **shpin_lock_array = NULL;

/**
 * ak_set_sharepin_lock_table - get machine sharepin lock table valid callback
 */
void ak_set_sharepin_lock_table(
	struct ak_drv_sharepin_lock *shpin_lock_table,
	int count, void **lock_array)
{
	shpin_lock = shpin_lock_table;
	shpin_lock_count = count;
	shpin_lock_array = lock_array;
}
EXPORT_SYMBOL(ak_set_sharepin_lock_table);

static int ak_sharepin_lock(E_DRV_MODULE drv_mod_name, 
	struct ak_drv_sharepin_lock *shpin_lock, int len, void **lock_array)
{
	struct ak_drv_sharepin_lock *lock = NULL;
	GET_DRV_LOCK_INFO(struct ak_drv_sharepin_lock, shpin_lock, len, drv_mod_name, lock);
	if (lock == NULL)
		return -1;

	ak_acquire_lock(lock_array, lock->lock_name, lock->lock_type, &lock->flags);
	if (lock->config_share_pin != NULL)
		lock->config_share_pin();
	return 0;
}

static void ak_sharepin_unlock(E_DRV_MODULE drv_mod_name, 
	struct ak_drv_sharepin_lock *shpin_lock, int len, void **lock_array)
{
	struct ak_drv_sharepin_lock *lock = NULL;
	GET_DRV_LOCK_INFO(struct ak_drv_sharepin_lock, shpin_lock, len, drv_mod_name, lock);
	if (lock == NULL)
		return;

	ak_release_lock(lock_array, lock->lock_name, lock->lock_type, lock->flags);
}

int ak_drv_sharepin_lock(E_DRV_MODULE drv_mod_name)
{
	return ak_sharepin_lock(drv_mod_name, shpin_lock, 
						shpin_lock_count, shpin_lock_array);
}
EXPORT_SYMBOL(ak_drv_sharepin_lock);

void ak_drv_sharepin_unlock(E_DRV_MODULE drv_mod_name)
{
	ak_sharepin_unlock(drv_mod_name, shpin_lock, 
						shpin_lock_count, shpin_lock_array);
}
EXPORT_SYMBOL(ak_drv_sharepin_unlock);


/* Brief: Init sharepin locks for  drivers module
 * param: void
 * ret: 
 *	0: if acquire lock successed; 
 *	<0: indicate the Board or products  being not sharepin for mutual drivers
 */
static int ak_init_sharepin_locks(void)
{
	int i;

	if (shpin_lock_count == 0)
		return -1;

	ak_init_lock_array(shpin_lock_array, AK_MODULE_COUNT);

	for (i = 0; i < shpin_lock_count; i++) {
		ak_init_lock(shpin_lock_array, shpin_lock[i].lock_name, shpin_lock[i].lock_type);
		shpin_lock[i].flags = 0;
	}

	return 0;
}


void ak_drv_module_protect(E_DRV_MODULE drv_mod_name)
{
	ak_drv_module_lock(drv_mod_name);
	ak_drv_sharepin_lock(drv_mod_name);
}
EXPORT_SYMBOL(ak_drv_module_protect);

void ak_drv_module_unprotect(E_DRV_MODULE drv_mod_name)
{
	ak_drv_sharepin_unlock(drv_mod_name);
	ak_drv_module_unlock(drv_mod_name);
}
EXPORT_SYMBOL(ak_drv_module_unprotect);

static int __init ak_init_module_lock(void)
{
	printk("Anyka platform share gpio locks initialize.\n");
	
	ak_init_module_locks();
	ak_init_sharepin_locks();
	return 0;
}
subsys_initcall(ak_init_module_lock);

