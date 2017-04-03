/**
 * @FILENAME   akuio.h
 * @BRIEF      This file provide the APIs definition of libakuio.
 *             Copyright (C) 2011 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
 * @AUTHOR     Jacky Lau
 * @DATE       2011-03-15
 * @VERSION    0.1
 * @REF        Please refer to akuio.c
 */

#ifndef __AKUIO_H_
#define __AKUIO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef unsigned char uint8_t;

enum akuio_HW_type
{
    AKUIO_HWTYPE_JPEG,
    AKUIO_HWTYPE_2D,
};

  /**
   * @BRIEF     Clean up the legacy lock resouce of akuio
   * @AUTHOR    She shaohua
   * @DATE      2011-11-11
   * @PARAM     [in]  *lock_handle :  the handle of akuio lock
   * @RETURN    int :  if successful return 0, otherwise return negative
   * @RETVAL     0 :  Successfully clean up the akuio lock
   * @RETVAL    <0 :  Clean up the akuio lock failed.
   */
extern int   akuio_cleanup( void *lock_handle );

  /**
   * @BRIEF     Lock akuio with block mode and init pmem
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  hw_id :  the id of hardware which will be lock
   * @RETURN    void * :  if successful return lock handle, otherwise return NULL
   * @RETVAL    !NULL :  Successfully lock akuio and init pmem
   * @RETVAL     NULL :  Lock akuio or init pmem failed.
   */
extern void *akuio_lock_block_compatible(int hw_id);
  /**
   * @BRIEF     Lock akuio with unblock mode and init pmem
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  hw_id :  the id of hardware which will be lock
   * @RETURN    void * :  if successful return lock handle, otherwise return NULL
   * @RETVAL    !NULL :  Successfully lock akuio and init pmem
   * @RETVAL     NULL :  Lock akuio or init pmem failed.
   */
extern void *akuio_lock_unblock_compatible(int hw_id);
  /**
   * @BRIEF     Lock akuio with timeout mode and init pmem
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  hw_id :  the id of hardware which will be lock
   * @RETURN    void * :  if successful return lock handle, otherwise return NULL
   * @RETVAL    !NULL :  Successfully lock akuio and init pmem
   * @RETVAL     NULL :  Lock akuio or init pmem failed.
   */
extern void *akuio_lock_timewait_compatible(int hw_id);
  /**
   * @BRIEF     Unlock akuio and de-init pmem
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  *lock_handle :  the handle returned by akuio_lock_xxx()
   * @RETURN    int :  if successful return 0, otherwise return nagative
   * @RETVAL     0 :  Successfully unlock akuio and free pmem resource
   * @RETVAL    <0 :  Unlock akuio or de-init pmem failed.
   */
extern int akuio_unlock_compatible(void *lock_handle);


  /**************** DMA memory ****************/
  /**
   * @BRIEF     Init pmem
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM
   * @RETURN    int :  if successful return 0, otherwise return nagative
   * @RETVAL     0 :  Successfully init pmem
   * @RETVAL    <0 :  Init pmem failed.
   */
extern int akuio_pmem_init( void );
  /**
   * @BRIEF     Free pmem resource
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM
   * @RETURN    int :  if successful return 0, otherwise return nagative
   * @RETVAL     0 :  Successfully free pmem resource
   * @RETVAL    <0 :  Free pmem resource failed.
   */
extern int akuio_pmem_fini( void );

  /**
   * @BRIEF     Allocate a piece of physically contiguous memory
   * @AUTHOR    Jacky Lau
   * @DATE      2011-03-15
   * @PARAM     [in]  size: size of physically contigous memory region
   * @RETURN    void* :  if successful return a pointer of the physically contigous memory,
                         otherwise return NULL
   * @RETVAL    !NULL :  Allocation successful.
   * @RETVAL     NULL :  Allocation failed.
   */
extern void *akuio_alloc_pmem(unsigned int size);

  /**
   * @BRIEF     Free a piece of physically contiguous memory
   * @AUTHOR    Jacky Lau
   * @DATE      2011-03-15
   * @PARAM     [in]  p :  pointer of physically contigous memory
   * @RETURN    void
   * @RETVAL
   */
extern void  akuio_free_pmem(void *p);

  /**
   * @BRIEF     Convert virtual address to physical address
   * @AUTHOR    Jacky Lau
   * @DATE      2011-03-15
   * @PARAM     [in]  p :  pointer of physically contigous memory
   * @RETURN    unsigned long :  if successful return the physical address of physically contigous memory,
                                 otherwise return 0
   * @RETVAL    !0 :  convertion successful.
   * @RETVAL     0 :  convertion failed.
   */
extern unsigned long akuio_vaddr2paddr(void *p);

/**************** Device Registers mapping ****************/
  /**
   * @BRIEF     Lock akuio with block mode
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  hw_id :  the id of hardware which will be lock
   * @RETURN    void * :  if successful return lock handle, otherwise return NULL
   * @RETVAL    !NULL :  Successfully lock akuio
   * @RETVAL     NULL :  Lock akuio failed.
   */
extern void *akuio_lock_block(int hw_id);

  /**
   * @BRIEF     Lock akuio with unblock mode
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  hw_id :  the id of hardware which will be lock
   * @RETURN    void * :  if successful return lock handle, otherwise return NULL
   * @RETVAL    !NULL :  Successfully lock akuio
   * @RETVAL     NULL :  Lock akuio failed.
   */
extern void *akuio_lock_unblock(int hw_id);

  /**
   * @BRIEF     Lock akuio with timeout mode
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  hw_id :  the id of hardware which will be lock
   * @RETURN    void * :  if successful return lock handle, otherwise return NULL
   * @RETVAL    !NULL :  Successfully lock akuio
   * @RETVAL     NULL :  Lock akuio failed.
   */
extern void *akuio_lock_timewait(int hw_id);

  /**
   * @BRIEF     Unlock akuio
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  *lock_handle :  the handle returned by akuio_lock_xxx()
   * @RETURN    int :  if successful return 0, otherwise return nagative
   * @RETVAL     0 :  Successfully unlock akuio and free pmem resource
   * @RETVAL    <0 :  Unlock akuio or de-init pmem failed.
   */
extern int   akuio_unlock(void *lock);

  /**
   * @BRIEF     Acquire the android wake-lock, disable system suspend
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  HW_type :  the id of wake-lock
   * @RETURN    int :  if successful return 0, otherwise return nagative
   * @RETVAL     0 :  Successfully acquire the wake-lock
   * @RETVAL    <0 :  Acquire wake-lock failed.
   */
extern int akuio_begin( uint8_t HW_type );
  /**
   * @BRIEF     Release the android wake-lock, enable system suspend
   * @AUTHOR    She shaohua
   * @DATE      2011-11-25
   * @PARAM     [in]  HW_type :  the id of wake-lock
   * @RETURN    int :  if successful return 0, otherwise return nagative
   * @RETVAL     0 :  Successfully release the wake-lock
   * @RETVAL    <0 :  Release wake-lock failed.
   */
extern int akuio_end( uint8_t HW_type );

  /**
   * @BRIEF     Map the register region to application memory space
   * @AUTHOR    Jacky Lau
   * @DATE      2011-03-15
   * @PARAM     [in]  paddr :  the physical address of register region
   * @PARAM     [in]  size :  the size of register region
   * @RETURN    void* :  if successful return the pointer of register region
                         otherwise return NULL
   * @RETVAL    !NULL :  Mapping successful.
   * @RETVAL     NULL :  Mapping failed.
   */
extern void *akuio_map_regs(unsigned int paddr, unsigned int size);
  /**
   * @BRIEF     Unmap the register region from application memory space
   * @AUTHOR    Jacky Lau
   * @DATE      2011-03-15
   * @PARAM     [in]  vaddr :  the pointer returned by akuio_map_regs()
   * @PARAM     [in]  size :  the size of register region
   * @RETURN    void
   * @RETVAL
   */
extern void  akuio_unmap_regs(void * vaddr, unsigned int size);
  /**
   * @BRIEF     Modify the system register
   * @AUTHOR    Jacky Lau
   * @DATE      2011-03-15
   * @PARAM     [in]  paddr :  the physical address of system register
   * @PARAM     [in]  val :  the value which will be set to system register
   * @PARAM     [in]  mask :  the valid bit of the value
   * @RETURN    void
   * @RETVAL
   */
extern void  akuio_sysreg_write(unsigned int paddr, unsigned int val, unsigned int mask);
  /**
   * @BRIEF     Wait a interruption occur
   * @AUTHOR    Jacky Lau
   * @DATE      2011-03-15
   * @PARAM
   * @RETURN    void
   * @RETVAL
   */
extern void  akuio_wait_irq();
#ifndef ANDROID
extern void akuio_2D_wait_irq ();
#endif

  /**
   * @BRIEF     Invalid the (L2) cache
   * @AUTHOR    Jacky Lau
   * @DATE      2011-03-15
   * @PARAM
   * @RETURN    void
   * @RETVAL
   */
extern void  akuio_inv_cache();


#ifdef __cplusplus
}
#endif

#endif
