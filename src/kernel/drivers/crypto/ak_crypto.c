/*
 * Cryptographic API.
 *
 * Support for Anyka AES/DES/3DES encryption.
 *
 * Copyright (C) 2013 Anyka Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */	 
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
	 
#include <crypto/algapi.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <crypto/ctr.h>

#include <plat-anyka/ak_crypto.h>
#include <mach/reset.h>

#define __hdbg__	//printk("===%s===%s===%d\n", __FILE__, __func__, __LINE__);

//#define CRYPT_DBG

#ifdef CRYPT_DBG
#ifdef __KERNEL__
#define PDEBUG(fmt, args...) 	printk(KERN_INFO "akcrypto:" fmt, ##args)
#define DUMP_DATA(fmt, args...) dump_data(fmt, ##args)
#else
#define PDEBUG(fmt, args...) 	fprintf(stderr, "%s %d:" fmt,__FILE__, __LINE__, ## args)
#define DUMP_DATA(fmt, args...)
#endif
#else
#define PDEBUG(fmt, args...) 
#define DUMP_DATA(fmt, args...) 
#endif


struct ak_cipher_reqctx {
	unsigned long mode;
};

struct ak_crypto {
	struct device 			*dev;
	struct clk 				*clk;
	void __iomem 			*iobase;
	struct resource			*ioarea;
	spinlock_t 				lock;
	int 					irq;
	struct crypto_queue		queue;
	struct list_head		list;

	unsigned int 			flags;
#define FLAGS_BUSY 				(1<<0)
#define FLAGS_MULTI_GROUP_MODE 	(1<<1)
	
	struct tasklet_struct	queue_task;	
	
	struct ablkcipher_request  *req;	
	struct ak_cipher_ctx	   *ctx;
	
	struct scatterlist		*sg_src;
	u32 	src_ofs;
	struct scatterlist		*sg_dst;
	u32 	dst_ofs;
};


#define AES_MAX_GRP_INPUT_SIZE	(16)


#define aes_key 	key_u.aes
#define des_key 	key_u.des
#define des3_key	key_u.des3


struct ak_cipher_ctx {
	struct ak_crypto		*dev;

	union {
		u8		aes[AES_MAX_KEY_SIZE];
		u8		des[DES_KEY_SIZE];
		u8		des3[3 * DES_KEY_SIZE];
	} key_u;

	int 	keylen;
	unsigned int 	width_sel;
	int encrypt_mode;
};

struct ak_cipher_alg {
	struct crypto_alg	alg;
	struct list_head	entry;
	u32 		enc_type;
	u32 		dec_type; /*not used*/
};

struct ak_cipher_tmpl
{
	const char		*name;
	const char		*drv_name;
	u8				block_size;
	u32				enc_type;
	struct ablkcipher_alg	ablkcipher;
};

/* keep registered devices data here */
static LIST_HEAD(dev_list);
static DEFINE_SPINLOCK(dev_list_lock);


/* keep registered devices data here */
static LIST_HEAD(cipher_list);
static DEFINE_SPINLOCK(list_lock);

static void dump_regs(struct ak_crypto *crypto, const char *desc)
{
	int i;
	u32 con, stat, timeout;
    u32 input[4], iv[4], key[8], output[4];
	u32 plint_addr, datalen, cipher_addr;

	PDEBUG("crypto reg list(%s):\n", desc);
	con = readl(crypto->iobase + AKENC_CONTROL_REG);
	stat = readl(crypto->iobase + AKENC_INT_STATUS_REG);
	timeout = readl(crypto->iobase + AKENC_TIMEOUT_REG);

	PDEBUG("con:[%08x], irq_stat:[%08x], timeout_period:[%08x]\n",
		   	con, stat, timeout);

	for(i=1; i<=4; i++)	{
		input[i]= readl(crypto->iobase + AKENC_GRP_INPUT_REG(i));
		PDEBUG("input(%d):[%08x], ", i, input[i]);
	}
	PDEBUG("\n");

	for(i=1; i<=4; i++)	{
		iv[i] = readl(crypto->iobase + AKENC_VEC_INPUT_REG(i));
		PDEBUG("iv(%d):[%08x], ", i, iv[i]);
	}
	PDEBUG("\n");

	for(i=1; i<=8; i++)	{
		key[i] = readl(crypto->iobase + AKENC_KEY_INPUT_REG(i));
		PDEBUG("key(%d):[%08x], ", i, key[i]);
	}
	PDEBUG("\n");

	for(i=1; i<=4; i++)	{
		output[i] = readl(crypto->iobase + AKENC_GRP_OUTPUT_REG(i));
		PDEBUG("output(%d):[%08x], ", i, output[i]);
	}
	PDEBUG("\n");

	plint_addr = readl(crypto->iobase + AKENC_PLAINT_ADDR_REG);
	datalen = readl(crypto->iobase + AKENC_DATALEN_REG);
	cipher_addr = readl(crypto->iobase + AKENC_CIPHER_ADDR_REG);

	PDEBUG("plint_addr:[%08x], datalen:[%08x], cipher_addr:[%08x]\n",
		   	plint_addr, datalen, cipher_addr);
}

static inline void dump_data(char *desc, void *buf, int len)
{
	int i;
	unsigned char *data = buf;
	if(!data)
		return;

	if(desc != NULL)
		printk("%s. len=%d", desc, len);

	for(i=0; i<len; i++) {
		if((i%16)==0)
			printk("\n");
		printk("%02x ", *(data++));
	}
	printk("\n");
}

static inline struct ak_cipher_alg *tfm_to_cipher_alg(struct crypto_tfm *tfm)
{
	return container_of(tfm->__crt_alg, struct ak_cipher_alg, alg);
}

static struct ak_crypto *ak_cipher_find_dev(struct ak_cipher_ctx *ctx)
{
	struct ak_crypto *dd = NULL, *tmp;

	spin_lock_bh(&dev_list_lock);
	if (!ctx->dev) {
		list_for_each_entry(tmp, &dev_list, list) {
			/* FIXME: take fist available aes core */
			dd = tmp;
			break;
		}
		ctx->dev = dd;
	} else {
		/* already found before */
		dd = ctx->dev;
	}
	spin_unlock_bh(&dev_list_lock);

	return dd;
}


static int akenc_wait_for_idle(struct ak_crypto *dev, unsigned long timeout)
{
	u32 regval;

	regval = readl(dev->iobase + AKENC_CONTROL_REG);
	
	while(regval & AK_ENC_OPT_STATUS);
	return 0;
}


/**
 * @brief: notify complet working.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] crypto: akcrypto dev pointer.
 * @param [in] err: is error or not.
 * @return void
 */
static void akenc_encrypt_complete(struct ak_crypto *dev, int err)
{
	struct ak_cipher_ctx *ctx = dev->ctx;

	/* holding a lock outside */
	dev->req->base.complete(&dev->req->base, err);
	dev->flags &= ~FLAGS_BUSY;

	if(ctx->encrypt_mode == CRYPTO_MULTI_GROUP_MODE) {
		dma_unmap_sg(dev->dev, dev->sg_src, 1, DMA_TO_DEVICE);
		dma_unmap_sg(dev->dev, dev->sg_dst, 1, DMA_FROM_DEVICE);
	}
}


/**
 * @brief: set single/mutli group input data
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] crypto: akcrypto dev pointer.
 * @param [in] sg: input scatterlist data
 * @return int
 */
static int akenc_grp_indata(struct ak_crypto *dev, struct scatterlist *sg)
{
	void *grp;
	int remain;
	int len;
	struct ak_cipher_ctx *ctx = dev->ctx;

	if(ctx->encrypt_mode == CRYPTO_MULTI_GROUP_MODE) {
		dma_addr_t phyaddr_src;
		dma_addr_t phyaddr_dst;
		PDEBUG("%s use for multi group mode.\n", __func__);

		phyaddr_src = sg_dma_address(dev->sg_src) + dev->src_ofs;
		phyaddr_dst = sg_dma_address(dev->sg_dst) + dev->dst_ofs;
		writel(phyaddr_src, dev->iobase + AKENC_PLAINT_ADDR_REG);
		writel(phyaddr_dst, dev->iobase + AKENC_CIPHER_ADDR_REG);
		writel(dev->sg_src->length, dev->iobase + AKENC_DATALEN_REG);
		dev->src_ofs += dev->sg_src->length;
	} else {
		grp = sg_virt(sg) + dev->src_ofs;
		remain = dev->sg_src->length - dev->src_ofs;
		len = (remain > ctx->width_sel) ? ctx->width_sel : remain;

		memcpy(dev->iobase + AKENC_GRP_INPUT_REG(1), grp, len);
		dev->src_ofs += len;

		DUMP_DATA("indata", grp, len);	
	}

next:
	if((dev->src_ofs == dev->sg_src->length) && (!sg_is_last(sg))) {
		dev->sg_src = sg = sg_next(sg);
		dev->src_ofs = 0;
		if(dev->sg_src->length == 0) {
			goto next;
		}
		if(len < ctx->width_sel) {
			
		}
	}

	return 0;
}

/**
 * @brief: set single/mutli group output data
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] crypto: akcrypto dev pointer.
 * @param [in] sg: output scatterlist data
 * @return int
 */
static int akenc_grp_outdata(struct ak_crypto *dev, struct scatterlist *sg)
{
	void *grp;
	int len;
	int remain;
	struct ak_cipher_ctx *ctx = dev->ctx;

	if(ctx->encrypt_mode == CRYPTO_MULTI_GROUP_MODE) {
		dev->dst_ofs+= dev->sg_dst->length;
		PDEBUG("%s use for multi group mode.\n", __func__);
	} else {
		grp = sg_virt(sg) + dev->dst_ofs;
		remain = dev->sg_dst->length - dev->dst_ofs;
		len = (remain > ctx->width_sel) ? ctx->width_sel : remain;

		memcpy(grp, dev->iobase + AKENC_GRP_OUTPUT_REG(1), len);
		dev->dst_ofs += len;
		DUMP_DATA("outdata", grp, len);	
	}

	if(dev->dst_ofs == dev->sg_dst->length) {
		if(!sg_is_last(sg)) {
			dev->sg_dst = sg_next(sg);
			dev->dst_ofs = 0;
		} else
			akenc_encrypt_complete(dev, 0);
	}

	return 0;
}

/**
 * @brief: set encode key and iv.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in]: iv: input vector
 * @param [in]: ivlen: input vector length.
 * @param [in]: key
 * @param [in]: keylen
 * @return int
 */
static inline void akenc_set_key_iv(struct ak_crypto *dev,
			uint8_t *iv, unsigned int ivlen, uint8_t *key, unsigned int keylen)
{
	if(ivlen > 0) {
		DUMP_DATA("iv", iv, ivlen);
		memcpy(dev->iobase + AKENC_VEC_INPUT_REG(1), iv, ivlen);
	}

	if(keylen > 0) {
		DUMP_DATA("key", key, keylen);
		memcpy(dev->iobase + AKENC_KEY_INPUT_REG(1), key, keylen);
	}
}


/**
 * @brief: working mode selection
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in]: mode: encrypt mode
 * @return int
 */
static inline int akenc_work_mode(int mode)
{
	int val;
	mode &= FLAGS_WORK_MODE_MASK;
	switch(mode) {
		case FLAGS_ECB:
			val = AK_ENC_OPT_MODE(ENC_WORK_MODE_ECB);
			break;
		case FLAGS_CBC:
			val = AK_ENC_OPT_MODE(ENC_WORK_MODE_CBC);
			break;
		case FLAGS_CFB:
			val = AK_ENC_OPT_MODE(ENC_WORK_MODE_CFB);
			break;
		case FLAGS_OFB:
			val = AK_ENC_OPT_MODE(ENC_WORK_MODE_OFB);
			break;
		case FLAGS_CTR:
			val = AK_ENC_OPT_MODE(ENC_WORK_MODE_CTR);
			break;
		default:
			BUG();
	}
	return val;
}


/**
 * @brief: encrypt algorithm selection
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in]: mode: encrypt mode
 * @param [in]: width: bit width.
 * @return int
 */
static inline int akenc_alg_sel(int mode, int key_len)
{
	int val;

	PDEBUG("alg mode:%d, key len:%d.\n", mode, key_len);

	mode &= FLAGS_ALG_MODE_MASK;
	if(mode & FLAGS_AES) {
		if (key_len == AES_KEYSIZE_256)
			val = AK_ENC_ALG_SEL(ENC_TYPE_ALG_AES256);
		else if (key_len == AES_KEYSIZE_192)
			val = AK_ENC_ALG_SEL(ENC_TYPE_ALG_AES192);
		else
			val = AK_ENC_ALG_SEL(ENC_TYPE_ALG_AES128);
	} else if(mode & FLAGS_DES) {
		val = AK_ENC_ALG_SEL(ENC_TYPE_ALG_DES);
	} else if(mode & FLAGS_3DES) {
		if (key_len == DES_KEY_2)
			val = AK_ENC_ALG_SEL(ENC_TYPE_ALG_3DES_2KEY);
		else
			val = AK_ENC_ALG_SEL(ENC_TYPE_ALG_3DES_3KEY);
	}	

	return val;
}


/**
 * @brief: data bit width selection
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in]: mode: encrypt mode
 * @param [in]: width: bit width.
 * @return int
 */
static inline int akenc_width_sel(int mode, int width)
{
	int val;

	mode &= FLAGS_ALG_MODE_MASK;
	if(mode & FLAGS_AES) {
		val = AK_ENC_WIDTH_SEL(ENC_WIDTH_AES_128BIT);
	} else if(mode & (FLAGS_DES|FLAGS_3DES)) {
		val = AK_ENC_WIDTH_SEL(ENC_WIDTH_DES_64BIT);
	}	

	return val;

}


/**
 * @brief: config bit sequence.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in]: mode: encrypt mode
 * @return int
 */
static inline int akenc_set_bit_seq(int mode)
{
	int val;
	/*ecb enc need set the input_bit, output_bit, iv_bit
	  besides need set the key bit if used for cbc enc. */

	mode &= FLAGS_ALG_MODE_MASK;
	if(mode & FLAGS_AES) {
		val =  AK_CRYPT_AES_BIT_SEQ;
	} else if(mode & (FLAGS_DES|FLAGS_3DES)) {
		val =  AK_CRYPT_DES_BIT_SEQ;
	}
	return val;
}

static inline void akenc_start_encrypt(struct ak_crypto *dev)
{
	u32 con;
	con = readl(dev->iobase + AKENC_CONTROL_REG);
	con |= AK_ENC_START;
	writel(con, dev->iobase + AKENC_CONTROL_REG);
}

/**
 * @brief: encrypt work function, configure encrypt register, start 
 * the hardware encrypt working. 
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] crypto: akcrypto dev pointer.
 * @param [in]: mode: encrypt mode
 * @return int
 */
static void ak_cipher_crypto_start(struct ak_crypto *dev, unsigned long mode)
{
	u32 con, ret = 0;
	struct ablkcipher_request  *req = dev->req;
	struct ak_cipher_ctx *ctx = dev->ctx;

	struct crypto_ablkcipher *tfm = crypto_ablkcipher_reqtfm(req);
	unsigned ivsize = crypto_ablkcipher_ivsize(tfm);

	PDEBUG("%s enter\n", __func__);
	con = AK_ENC_INT_EN|AK_ENC_TIMEOUT_INT_EN;
	akenc_wait_for_idle(dev, 0);

	if(mode & FLAGS_AES)
		ctx->width_sel = 16;
	else 
		ctx->width_sel = 8;

	if((dev->flags & FLAGS_MULTI_GROUP_MODE) && !(mode & FLAGS_CTR))
		ctx->encrypt_mode = CRYPTO_MULTI_GROUP_MODE;
	else 
		ctx->encrypt_mode = CRYPTO_SINGLE_GROUP_MODE;

	dev->sg_src = req->src;
	dev->src_ofs = 0;
	dev->sg_dst = req->dst;
	dev->dst_ofs = 0;

	if(ctx->encrypt_mode == CRYPTO_MULTI_GROUP_MODE) {
		PDEBUG("%s use for multi group mode.\n", __func__);
		dma_map_sg(dev->dev, dev->sg_src, 1, DMA_TO_DEVICE);
		dma_map_sg(dev->dev, dev->sg_dst, 1, DMA_FROM_DEVICE);
	}

	ret = akenc_grp_indata(dev, req->src);
	if(ret) {
		goto out;
	}

	writel(MAX_TIMEOUT, dev->iobase + AKENC_TIMEOUT_REG);

	akenc_set_key_iv(dev, req->info, ivsize, ctx->aes_key, ctx->keylen);

	con |= akenc_alg_sel(mode, ctx->keylen);
	con |= akenc_width_sel(mode, ctx->width_sel); 

	if(ctx->encrypt_mode == CRYPTO_MULTI_GROUP_MODE)
		con |= AK_ENC_MULT_GRP_CIPHER;

	con |= AK_ENC_CLK_EN;
	con |= akenc_set_bit_seq(mode);
	
	con |= akenc_work_mode(mode);

	/* iv mode sel, vaild for cbc, cfb, ofb modes, choosing 0, 
	 * applies for a data packet to be encrypted for several times,
	 * in which case, iv is set only once 
	 * */
	con |= AK_ENC_IV_MODE;

	writel(con, dev->iobase + AKENC_CONTROL_REG);

	dump_regs(dev, __func__);
	akenc_start_encrypt(dev);	
	PDEBUG("ak crypto con(%08x):%08x.\n", 
			(unsigned int)(dev->iobase + AKENC_CONTROL_REG),
		   	readl(dev->iobase + AKENC_CONTROL_REG));
	return;

out:
	akenc_encrypt_complete(dev, ret);
}

/**
 * @brief: encrypt tasklet handle.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in]: data:private data.
 * @return int
 */
static void akenc_tasklet_cb(unsigned long data)
{
	struct ak_crypto *dev = (struct ak_crypto *)data;
	struct crypto_async_request *async_req, *backlog;
	struct ak_cipher_reqctx *reqctx;
	unsigned long flags;

	PDEBUG("tasklet running.\n");
	spin_lock_irqsave(&dev->lock, flags);
	backlog   = crypto_get_backlog(&dev->queue);
	async_req = crypto_dequeue_request(&dev->queue);
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!async_req)
		return;

	if (backlog)
		backlog->complete(backlog, -EINPROGRESS);

	dev->req = ablkcipher_request_cast(async_req);
	dev->ctx = crypto_tfm_ctx(dev->req->base.tfm);
	reqctx   = ablkcipher_request_ctx(dev->req);

	ak_cipher_crypto_start(dev, reqctx->mode);

}


/**
 * @brief: push the request to queue.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] crypto: akcrypto dev pointer.
 * @param [in] req: async cipher request pointer.
 * @return int
 */
static int ak_cipher_handle_queue(struct ak_crypto *crypto,
			       struct ablkcipher_request *req)
{
	unsigned long flags;
	int ret = 0;

	PDEBUG("%s enter\n", __func__);
	spin_lock_irqsave(&crypto->lock, flags);
	
	if (crypto->flags & FLAGS_BUSY) {
		spin_unlock_irqrestore(&crypto->lock, flags);
		return ret;
	}

	ret = ablkcipher_enqueue_request(&crypto->queue, req);

	crypto->flags |= FLAGS_BUSY;
	spin_unlock_irqrestore(&crypto->lock, flags);

	tasklet_schedule(&crypto->queue_task);

	return ret; /* return ret, which is enqueue return value */
}

/**
 * @brief: cipher encode entry, get ak_crypto alg from request, push 
 * the request to queue.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] req: async cipher request pointer.
 * @return int
 */
static int ak_cipher_encrypt(struct ablkcipher_request *req)
{
	struct ak_cipher_ctx *ctx;
	struct ak_cipher_reqctx *rctx;
	struct ak_crypto *crypto;
	struct ak_cipher_alg *alg;

	PDEBUG("%s enter\n", __func__);
 	rctx= ablkcipher_request_ctx(req);
	ctx = crypto_ablkcipher_ctx(crypto_ablkcipher_reqtfm(req));

	alg = tfm_to_cipher_alg(&crypto_ablkcipher_reqtfm(req)->base);

	if(!(alg->enc_type & FLAGS_ENCRYPT))
		return -EINVAL;

	PDEBUG("nbytes: %d, enc: %d, mode: %d\n", req->nbytes,
		  !!(alg->enc_type & FLAGS_ENCRYPT),
		  (alg->enc_type & FLAGS_WORK_MODE_MASK));

	crypto = ctx->dev;
	if (!crypto) {
		printk("warning!!\n");
		crypto = ctx->dev = ak_cipher_find_dev(ctx);
		if (!crypto) {
			printk("ak crypto: not found the crypto dev.\n");
			return -ENODEV;
		}
	}

	rctx->mode = alg->enc_type;

	return ak_cipher_handle_queue(crypto, req);
}

/**
 * @brief: set des encrypt key.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] *cipher: async block cipher
 * @param [in] *key
 * @param [in] key_len
 * @return int
 */
static int ak_des_setkey(struct crypto_ablkcipher *cipher, 
		const u8 *key, unsigned int keylen)
{
	struct crypto_tfm  *tfm = crypto_ablkcipher_tfm(cipher);
	struct ak_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	PDEBUG("%s enter\n", __func__);
	if (keylen != DES_KEY_SIZE) {
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	memcpy(ctx->des_key, key, keylen);
	ctx->keylen = keylen;

	PDEBUG("set des key success.\n");
	return 0;

}


/**
 * @brief: set 3des encrypt key.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] *cipher: async block cipher
 * @param [in] *key
 * @param [in] key_len
 * @return int
 */
static int ak_3des_setkey(struct crypto_ablkcipher *cipher, 
		const u8 *key, unsigned int keylen)
{
	struct crypto_tfm  *tfm = crypto_ablkcipher_tfm(cipher);
	struct ak_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	PDEBUG("%s enter\n", __func__);
	if (keylen != 3*DES_KEY_SIZE) {
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	memcpy(ctx->des_key, key, keylen);
	ctx->keylen = keylen;

	PDEBUG("set 3des key success.\n");
	return 0;
}


/**
 * @brief: set aes encrypt key.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] *cipher: async block cipher
 * @param [in] *key
 * @param [in] key_len
 * @return int
 */
static int ak_aes_setkey(struct crypto_ablkcipher *cipher, 
		const u8 *key, unsigned int keylen)
{
	struct crypto_tfm  *tfm = crypto_ablkcipher_tfm(cipher);
	struct ak_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	PDEBUG("%s enter\n", __func__);
	if (keylen != AES_KEYSIZE_128 &&
	    keylen != AES_KEYSIZE_192 &&
	    keylen != AES_KEYSIZE_256)
		return -EINVAL;

	memcpy(ctx->aes_key, key, keylen);
	ctx->keylen = keylen;

	PDEBUG("set aes key success.\n");
	return 0;

}


static inline int ak_null_decrypt(struct ablkcipher_request *req)
{
	printk("unsupport the decrypt.\n");
	return -EINVAL;
}

static inline int ak_cipher_cra_init(struct crypto_tfm *tfm)
{
	struct ak_cipher_ctx  *ctx = crypto_tfm_ctx(tfm);

	PDEBUG("%s enter\n", __func__);
	ctx->dev = ak_cipher_find_dev(ctx);
	tfm->crt_ablkcipher.reqsize = sizeof(struct ak_cipher_ctx);
	return 0;

}

static inline void ak_cipher_cra_exit(struct crypto_tfm *tfm)
{
	PDEBUG("%s enter\n", __func__);
}


/**
 * @brief: crypto interrupt handle, completion encrypt or timeout. 
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] *irq interrupt number.
 * @param [in] *devid dev id
 * @return irqreturn_t .
 */
static irqreturn_t ak_crypto_irq(int irq, void *dev_id)
{	
	struct ak_crypto   *dev  = dev_id;
	struct ak_cipher_reqctx *rctx;
	uint32_t	status;

	status = readl(dev->iobase + AKENC_INT_STATUS_REG);
		
	dump_regs(dev, __func__);
	PDEBUG("ak crypto interrupt : status is %08x.\n", status);
	if (status & AK_ENC_ENCRYPT_DONE) {
		akenc_grp_outdata(dev, dev->sg_dst);
		if((dev->src_ofs != dev->sg_src->length) || !sg_is_last(dev->sg_src)) {
			int ret;
			ret = akenc_grp_indata(dev, dev->sg_src);	
			if(ret) {
				goto out;
			}

			rctx = ablkcipher_request_ctx(dev->req);
			if(!(rctx->mode & FLAGS_CTR)) {
				u32 con = readl(dev->iobase + AKENC_CONTROL_REG);
				con &= ~AK_ENC_IV_MODE;
				writel(con, dev->iobase + AKENC_CONTROL_REG);
			} else {
				u32 con = readl(dev->iobase + AKENC_CONTROL_REG);
				con |= AK_ENC_IV_MODE;
				writel(con, dev->iobase + AKENC_CONTROL_REG);
			}

			akenc_start_encrypt(dev);	
		}
		
	} else if(status & AK_ENC_ENCRYPT_TIMEOUT) {
		/**FIXME: add the timeout handle.*/
		printk("crypto timeout.\n");
		akenc_encrypt_complete(dev, -EAGAIN);
	}

out:	
	return IRQ_HANDLED;
}

static struct ak_cipher_tmpl cipher_tmpls[] = {
	{
		.name = "ecb(aes)",
		.drv_name = "ecb-aes",
		.block_size = AES_BLOCK_SIZE,
		.enc_type = (FLAGS_ECB|FLAGS_AES|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.ivsize		= AES_BLOCK_SIZE,
			.setkey 	= ak_aes_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "cbc(aes)",
		.drv_name = "cbc-aes",
		.block_size = AES_BLOCK_SIZE,
		.enc_type = (FLAGS_AES|FLAGS_CBC|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.ivsize		= AES_BLOCK_SIZE,
			.setkey 	= ak_aes_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "cfb(aes)",
		.drv_name = "cfb-aes",
		.block_size = AES_BLOCK_SIZE,
		.enc_type = (FLAGS_AES|FLAGS_CFB|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.ivsize		= AES_BLOCK_SIZE,
			.setkey 	= ak_aes_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "ofb(aes)",
		.drv_name = "ofb-aes",
		.block_size = AES_BLOCK_SIZE,
		.enc_type = (FLAGS_AES|FLAGS_OFB|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.ivsize		= AES_BLOCK_SIZE,
			.setkey 	= ak_aes_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "ctr(aes)",
		.drv_name = "ctr-aes",
		.block_size = AES_BLOCK_SIZE,
		.enc_type = (FLAGS_AES|FLAGS_CTR|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.ivsize		= AES_BLOCK_SIZE,
			.setkey 	= ak_aes_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "ecb(des)",
		.drv_name = "ecb-des",
		.block_size = DES_BLOCK_SIZE,
		.enc_type = (FLAGS_DES|FLAGS_ECB|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES_KEY_SIZE,
			.max_keysize	= DES_KEY_SIZE,
			.ivsize		= DES_BLOCK_SIZE,
			.setkey 	= ak_des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "cbc(des)",
		.drv_name = "cbc-des",
		.block_size = DES_BLOCK_SIZE,
		.enc_type = (FLAGS_DES|FLAGS_CBC|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES_KEY_SIZE,
			.max_keysize	= DES_KEY_SIZE,
			.ivsize		= DES_BLOCK_SIZE,
			.setkey 	= ak_des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "cfb(des)",
		.drv_name = "cfb-des",
		.block_size = DES_BLOCK_SIZE,
		.enc_type = (FLAGS_DES|FLAGS_CBC|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES_KEY_SIZE,
			.max_keysize	= DES_KEY_SIZE,
			.ivsize		= DES_BLOCK_SIZE,
			.setkey 	= ak_des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "cfb(des)",
		.drv_name = "cfb-des",
		.block_size = DES_BLOCK_SIZE,
		.enc_type = (FLAGS_DES|FLAGS_CFB|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES_KEY_SIZE,
			.max_keysize	= DES_KEY_SIZE,
			.ivsize		= DES_BLOCK_SIZE,
			.setkey 	= ak_des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "ofb(des)",
		.drv_name = "ofb-des",
		.block_size = DES_BLOCK_SIZE,
		.enc_type = (FLAGS_DES|FLAGS_OFB|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES_KEY_SIZE,
			.max_keysize	= DES_KEY_SIZE,
			.ivsize		= DES_BLOCK_SIZE,
			.setkey 	= ak_des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "ecb(des3_ede)",
		.drv_name = "ecb-des3-ede",
		.block_size = DES3_EDE_BLOCK_SIZE,
		.enc_type = (FLAGS_3DES|FLAGS_ECB|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES3_EDE_KEY_SIZE,
			.max_keysize	= DES3_EDE_KEY_SIZE,
			.ivsize 	= DES3_EDE_BLOCK_SIZE,
			.setkey 	= ak_3des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "cbc(des3_ede)",
		.drv_name = "cbc-des3-ede",
		.block_size = DES3_EDE_BLOCK_SIZE,
		.enc_type = (FLAGS_3DES|FLAGS_CBC|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES3_EDE_KEY_SIZE,
			.max_keysize	= DES3_EDE_KEY_SIZE,
			.ivsize 	= DES3_EDE_BLOCK_SIZE,
			.setkey 	= ak_3des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "cfb(des3_ede)",
		.drv_name = "cfb-des3-ede",
		.block_size = DES3_EDE_BLOCK_SIZE,
		.enc_type = (FLAGS_3DES|FLAGS_CFB|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES3_EDE_KEY_SIZE,
			.max_keysize	= DES3_EDE_KEY_SIZE,
			.ivsize 	= DES3_EDE_BLOCK_SIZE,
			.setkey 	= ak_3des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
	{
		.name = "ofb(des3_ede)",
		.drv_name = "ofb-des3-ede",
		.block_size = DES3_EDE_BLOCK_SIZE,
		.enc_type = (FLAGS_3DES|FLAGS_OFB|FLAGS_ENCRYPT),
		.ablkcipher = {
			.min_keysize	= DES3_EDE_KEY_SIZE,
			.max_keysize	= DES3_EDE_KEY_SIZE,
			.ivsize 	= DES3_EDE_BLOCK_SIZE,
			.setkey 	= ak_3des_setkey,
			.encrypt	= ak_cipher_encrypt,
			.decrypt	= ak_null_decrypt,
		}
	},
};

/**
 * @brief: convert ak_cipher_tmpl to alg, register a crypto algorithm.
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] *tmpl information of crypto algorithm.
 * @return int.
 * @retval -EINVAL no platform data , fail;
 * @retval -EBUSY  requset mem  fail;
 * @retval -ENOMEM  alloc mem fail;
 */
static int __devinit ak_register_one_cipher(const struct ak_cipher_tmpl *tmpl)
{
	struct crypto_alg *alg;
	int err;
	struct ak_cipher_alg *p = kzalloc(sizeof(*p), GFP_KERNEL);

	if (!p)
		return -ENOMEM;

	alg = &p->alg;

	snprintf(alg->cra_name, CRYPTO_MAX_ALG_NAME, "%s", tmpl->name);
	snprintf(alg->cra_driver_name, CRYPTO_MAX_ALG_NAME, "ak-%s", tmpl->drv_name);
	alg->cra_priority = AK_CRA_PRIORITY;
	alg->cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER |
			 	CRYPTO_ALG_ASYNC;
	
	alg->cra_blocksize = tmpl->block_size;
	alg->cra_ctxsize = sizeof(struct ak_cipher_ctx);
	alg->cra_type = &crypto_ablkcipher_type;
	alg->cra_u.ablkcipher = tmpl->ablkcipher;
	alg->cra_init = ak_cipher_cra_init;
	alg->cra_exit = ak_cipher_cra_exit;
	alg->cra_module = THIS_MODULE;
	
	p->enc_type = tmpl->enc_type;

	spin_lock(&list_lock);
	list_add(&p->entry, &cipher_list);
	spin_unlock(&list_lock);

	/*register a algorithm.*/
	err = crypto_register_alg(alg);
	if (err) {
		pr_err("%s alg registration failed\n", alg->cra_name);
		list_del(&p->entry);
		kfree(p);
	} else {
		pr_info("%s alg registered\n", alg->cra_name);
	}
	
	return err;
}

/**
 * @brief unregister all algorithm
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param void.
 * @return void.
 */
static void ak_unregister_algs(void)
{
	struct ak_cipher_alg *cipher, *cipher_tmp;

	list_for_each_entry_safe(cipher, cipher_tmp, &cipher_list, entry) {
		crypto_unregister_alg(&cipher->alg);
		spin_lock(&list_lock);
		list_del(&cipher->entry);
		spin_unlock(&list_lock);
		kfree(cipher);
	}
}

/**
 * @brief crypto driver probe and initilize. 
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] *pdev information of platform device ,getting the crypto
 * driver resource .
 * @return int.
 * @retval -EINVAL no platform data , fail;
 * @retval -EBUSY  requset mem  fail;
 * @retval -ENOMEM  alloc mem fail;
 */
static int ak_crypto_probe(struct platform_device *pdev)
{
	int i, ret = 0;
	struct resource *res;
	struct ak_crypto *crypto_dev;
	struct device *dev = &pdev->dev;
	struct ak_crypto_plat_data *pdata;

	pdata = pdev->dev.platform_data;

	if(!pdata) {
		printk("not found crypto platform data.");
		ret = -EINVAL;
		goto probe_out;
	}

	crypto_dev = kzalloc(sizeof(struct ak_crypto), GFP_KERNEL);
	if (crypto_dev == NULL) {
		dev_err(dev, "unable to alloc data struct.\n");
		goto err_nomem;
	}
	crypto_dev->dev = dev;

	platform_set_drvdata(pdev, crypto_dev);

	ak_soft_reset(AK_SRESET_ENCRY);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Cannot get resource.\n");
		ret = -ENXIO;
		goto err_no_iores;
	}

	crypto_dev->ioarea = request_mem_region(res->start, resource_size(res),
					pdev->name);

	if (crypto_dev->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		ret = -ENXIO;
		goto err_no_iores;
	}

	crypto_dev->iobase = ioremap(crypto_dev->ioarea->start, 
								resource_size(crypto_dev->ioarea));
	if (crypto_dev->iobase == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		ret = -ENXIO;
		goto err_no_iomap;
	}

	crypto_dev->irq = platform_get_irq(pdev, 0);
	if (crypto_dev->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		ret = -ENOENT;
		goto err_no_irq;
	}

	ret = request_irq(crypto_dev->irq, ak_crypto_irq, 0, pdev->name, crypto_dev);
	if (ret) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_no_irq;
	}

	crypto_dev->clk = clk_get(dev, "encryption");
	if (IS_ERR(crypto_dev->clk)) {
		dev_err(dev, "failed to find secss clock source\n");
		ret = -ENOENT;
		goto err_no_clk;
	}
	
	ret = clk_enable(crypto_dev->clk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable clock source.\n");
		goto err_clk_en;
	}
	
	tasklet_init(&crypto_dev->queue_task, akenc_tasklet_cb, (unsigned long)crypto_dev);
	crypto_init_queue(&crypto_dev->queue, CRYPTO_QUEUE_LEN);
	
	crypto_dev->flags = 0;
	if(pdata->encrypt_mode == CRYPTO_MULTI_GROUP_MODE) {
		crypto_dev->flags |= FLAGS_MULTI_GROUP_MODE;
		printk("crypto use for multi group mode.\n");
	}
	INIT_LIST_HEAD(&crypto_dev->list);
	spin_lock(&dev_list_lock);
	list_add_tail(&crypto_dev->list, &dev_list);
	spin_unlock(&dev_list_lock);

	for (i = 0; i < ARRAY_SIZE(cipher_tmpls); i++) {		
		ret = ak_register_one_cipher(&cipher_tmpls[i]);
		if (ret) {
			ak_unregister_algs();
			goto err_algs;
		}
	}

	pr_info("Anyka crypto driver registered.\n");

	return 0;

err_algs:	
	clk_disable(crypto_dev->clk);
	
err_clk_en:
	clk_put(crypto_dev->clk);

err_no_clk:
	free_irq(crypto_dev->irq, crypto_dev);

err_no_irq:
	iounmap(crypto_dev->iobase);

err_no_iomap: 	
	release_mem_region(crypto_dev->ioarea->start, resource_size(crypto_dev->ioarea));

err_no_iores:	
	kfree(crypto_dev);

err_nomem:
probe_out:
	return ret;
}


/**
 * @brief crypto driver remove and release
 * 
 * @author lixinhai 
 * @date 2013-12-15
 * @param [in] *pdev information of platform device ,getting the crypto
 * driver resource .
 * @return int.
 */
static int ak_crypto_remove(struct platform_device *pdev)
{
	struct ak_crypto *crypto_dev;

	crypto_dev = platform_get_drvdata(pdev);

	if (!crypto_dev)
		return -ENODEV;
	
	spin_lock(&dev_list_lock);
	list_del(&crypto_dev->list);
	spin_unlock(&dev_list_lock);

	clk_disable(crypto_dev->clk);	
	clk_put(crypto_dev->clk);

	free_irq(crypto_dev->irq, crypto_dev);

	iounmap(crypto_dev->iobase);
	release_mem_region(crypto_dev->ioarea->start, resource_size(crypto_dev->ioarea));

	kfree(crypto_dev);
	
	return 0;
}

static struct platform_driver ak_crypto_driver = {
	.probe	= ak_crypto_probe,
	.remove	= ak_crypto_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "ak-crypto",
	},
};

static int __init ak_crypto_init(void)
{
	printk("AK Crypto Driver (c) 2013 ANYKA\n");
	return platform_driver_register(&ak_crypto_driver);
}

static void __exit ak_crypto_exit(void)
{
	return platform_driver_unregister(&ak_crypto_driver);
}

module_init(ak_crypto_init);
module_exit(ak_crypto_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anyka Inc.");
MODULE_DESCRIPTION("Anyka hardware crypto");

