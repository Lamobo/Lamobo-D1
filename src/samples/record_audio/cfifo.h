#ifndef _C_FIFO_H_
#define _C_FIFO_H_
#include <stdlib.h>
#include <semaphore.h>

struct cfifo
{
	unsigned int in;
	unsigned int out;
	unsigned int mask;
	unsigned int eorder;
	void* data;
	sem_t empty_sem;
	int cancel_wait;

};


/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */

static __always_inline int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

static inline int is_power_of_2(unsigned int n) 
{
	return (n != 0 && ((n & (n-1)) == 0));
}

static inline int roundup_pow_of_2(unsigned int n)
{
	return 1UL << fls(n - 1);
}

static inline struct cfifo* cfifo_init(unsigned int esize, unsigned int size_order)
{
	unsigned int size = 1<<size_order;
	unsigned int e_order;
	struct cfifo *cf = malloc(sizeof(struct cfifo));
	if(!cf)
		return NULL;
printf("esize=%d, size_order=%d.\n", esize, size_order);
	if(!is_power_of_2(esize)) {
		esize = roundup_pow_of_2(esize);
	}

	e_order = ffs(esize); //find frist set bit.
	cf->in = 0;
	cf->out = 0;
	cf->mask = size - 1;
	cf->eorder = e_order - 1;
	cf->data = malloc(size * esize);
	cf->cancel_wait = 0;

	printf("cfifo size:%d, mask:%d, eorder:%d, esize:%d.\n",
		   	size, cf->mask, cf->eorder, esize);
	return cf;
}

static inline void cfifo_release(struct cfifo* cf)
{
	if(cf->data)
		free(cf->data);
	free(cf);
}

/*
 * extern int sem_init __P ((sem_t *__sem, int __pshared, unsigned int __value));　　
 * sem为指向信号量结构的一个指针；pshared不为０时此信号量在进程间共享，否则只能为当前进程的所有线程共享；value给出了信号量的初始值。　
 * */
static inline void cfifo_enable_locking(struct cfifo* cf)
{
	sem_init(&cf->empty_sem, 0, 0);
	cf->cancel_wait = 0;
}

static inline int cfifo_len(struct cfifo* cf)
{
	return (cf->in+(cf->mask+1)-cf->out) & cf->mask;
}

static inline int cfifo_full(struct cfifo *cf)
{
	return (((cf->in+1) & cf->mask) == cf->out);
}

static inline int cfifo_empty(struct cfifo *cf)
{
	return (cf->in == cf->out);
}

static inline void cfifo_clear(struct cfifo *cf)
{
	cf->in = cf->out;
}

static inline int cfifo_in(struct cfifo* cf)
{
	if(cfifo_full(cf))
		return -1;
	cf->in = (cf->in+1) & cf->mask;
	return 0;
}

static inline int cfifo_in_signal(struct cfifo* cf)
{
	int sig = 0;
	if(cfifo_empty(cf))
		sig = 1;

	cfifo_in(cf);

	if(sig)
		sem_post(&cf->empty_sem);
	return 0;
}

static inline int cfifo_out(struct cfifo* cf)
{
	if(cfifo_empty(cf))
		return -1;
	cf->out = (cf->out+1) & cf->mask;
	return 0;
}

static inline void* cfifo_get_in(struct cfifo* cf)
{
	return ((char*)cf->data) + (cf->in << cf->eorder);
}

static inline const void* cfifo_get_out(struct cfifo* cf)
{
	return ((const char*)cf->data) + (cf->out << cf->eorder);
}

static inline void cfifo_wait_empty(struct cfifo *cf)
{
	while(cfifo_empty(cf) && !cf->cancel_wait)
	{
		sem_wait(&cf->empty_sem);
	}
	/* reset cancel wait */
//	cf->cancel_wait = 0;
}

static inline void cfifo_cancel_wait(struct cfifo* cf)
{
	cf->cancel_wait = 1;
	sem_post(&cf->empty_sem);
}

#endif




