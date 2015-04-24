/*
 *  linux/fs/fat/bmp_tree.c
 *
 *  Written 1992,1993 by Werner Almesberger
 *  VFAT extensions by Gordon Chaffee, merged with msdos fs by Henrik Storner
 *  Rewritten for the constant inumbers support by Al Viro
 *
 *  author:lixinhai
 *  date  :2013-11-01
 *
 *	
 */

#include <linux/slab.h>
#include <linux/log2.h>
#include <asm/unaligned.h>
#include <linux/err.h>
#include "bmp_tree.h"

//#define FAT_BMPTREE_DBG

#ifdef FAT_BMPTREE_DBG
#ifdef __KERNEL__
#define PDEBUG(fmt, args...) 	printk(KERN_INFO "bmptree:" fmt, ##args)
#else
#define PDEBUG(fmt, args...) 	fprintf(stderr, "%s %d:" fmt,__FILE__, __LINE__, ## args)
#endif
#else
#define PDEBUG(fmt, args...) 
#endif


static inline unsigned long bmptree_maxindex(struct bmptree *bt, 
		unsigned long height)
{
	return bt->height_to_maxidx[height];
}


static unsigned long __calc_maxidx(unsigned long height)
{
	unsigned long width = height * BMPTREE_NODE_MAP_SHIFT;
	int shift = BMPTREE_TREE_INDEX_BITS - width;

	if(shift < 0)
		return ~0UL;
	if(shift > BMPTREE_TREE_INDEX_BITS)
		return 0UL;

	return ~0UL >> shift;
}

static void bmptree_init_maxindex(struct bmptree *bt)
{
	int i;

	for(i=0; i<ARRAY_SIZE(bt->height_to_maxidx); i++)
		bt->height_to_maxidx[i] = __calc_maxidx(i);
}

static struct bmptree_node *bmptree_node_alloc(struct bmptree *bt) 
{
	struct bmptree_node *node;

	bt->used_ncount++;

	/*first, get node in free list.*/
	if(bt->free_ncount > 0) {
		struct list_head *free;

		PDEBUG("free node list count:%d.\n", bt->free_ncount);
		free = bt->free_nlist.next;
		bt->free_ncount--;
		list_del(free);
		node = list_entry(free, struct bmptree_node, n_head);

		return node;
	}

	node = kmem_cache_alloc(bt->bt_ncache, bt->gfp_mask);
	if(node == NULL) {
		return NULL;
	}
	return node;
}


static struct bmptree_leaf *bmptree_leaf_alloc(struct bmptree *bt) 
{
	struct bmptree_leaf *leaf;

	bt->used_lcount++;

	/*first, get leaf in free list.*/
	if(bt->free_lcount > 0) {
		struct list_head *free;

		PDEBUG("free leaf list count:%d.\n", bt->free_lcount);
		bt->free_lcount--;
		free = bt->free_llist.next;
		list_del(free);
		leaf = list_entry(free, struct bmptree_leaf, head);

		return leaf;
	}

	leaf = kmem_cache_alloc(bt->bt_lcache, bt->gfp_mask);

	if(leaf == NULL) {
		return NULL;
	}
	return leaf;
}


static void bmptree_free_node(struct bmptree *bt, struct bmptree_node *node)
{
	bt->used_ncount--;

	PDEBUG("free node. curr used count:%d.\n", bt->used_ncount);
	list_add_tail(&node->n_head, &bt->free_nlist);
	bt->free_ncount++;

	if(bt->free_ncount > bt->recliam_high) {
		struct bmptree_node *free_node;
		struct list_head *free;

		PDEBUG("free node, free_ncount:[%d] > recliam_high:[%d].\n",
			   	bt->free_ncount, bt->recliam_high);

		while(bt->free_ncount > bt->recliam_low) {
			bt->free_ncount--;
			free = bt->free_nlist.next;
			list_del(free);
			free_node = list_entry(free, struct bmptree_node, n_head);

			kmem_cache_free(bt->bt_ncache, free_node);
		}
	}
}


static void bmptree_free_leaf(struct bmptree *bt, struct bmptree_leaf *leaf)
{
	bt->used_lcount--;
	PDEBUG("free leaf. curr used count:%d.\n", bt->used_lcount);

	list_add_tail(&leaf->head, &bt->free_llist);
	bt->free_lcount++;

	if(bt->free_lcount > bt->recliam_high) {
		struct bmptree_leaf *free_leaf;
		struct list_head *free;

		PDEBUG("free leaf, free_lcount[%d] > recliam_high[%d].\n",
			   	bt->free_lcount, bt->recliam_high);

		while(bt->free_lcount > bt->recliam_low) {
			bt->free_lcount--;
			free = bt->free_llist.next;

			list_del(free);
			free_leaf = list_entry(free, struct bmptree_leaf, head);

			kmem_cache_free(bt->bt_lcache, free_leaf);
		}
	}
}


static int bmptree_free_subtree(struct bmptree *bt, struct bmptree_node *node)
{
	struct bmptree_node *slot = NULL;
	int offset = 0;

	if(!node) {
		return 0;
	}

	PDEBUG("free subtree. height:%d, count:%d.\n", 
			node->n_height, node->n_count);

	while(node->n_count > 0) {
		BUG_ON(offset >= BMPTREE_NODE_MAP_SIZE);
		slot = node->slots[offset];
		if(!slot) {
			offset++;
			continue;
		}

		if(slot->n_height > 0)
			bmptree_free_subtree(bt, slot);
		else
			bmptree_free_leaf(bt, (struct bmptree_leaf *)slot);

		node->slots[offset] = NULL;
		node->n_count--;
		offset++;
	}

	bmptree_free_node(bt, node);
	return 0;
}


static int bmptree_free_tree(struct bmptree *bt)
{
	return bmptree_free_subtree(bt, bt->rnode);
}

static int bmptree_node_insert(struct bmptree *bt, 
		struct bmptree_leaf *new, struct bmptree_node *parent, int index)
{
	PDEBUG("insert node to slot[%d].\n", index);

	if(unlikely(parent == NULL)) {
		if(bt->height < new->height) {
			struct bmptree_node *node = bt->rnode;

			PDEBUG("insert the root node, height=%d.\n", new->height);
			parent = bt->rnode = (struct bmptree_node *)new;
			new->parent = NULL;
			bt->height++;

			/*when this node is first node, return.*/
			if(node == NULL)
				return 0;

			new = (struct bmptree_leaf *)node;
			index = 0;
		} else {
			printk("Fat: node insert to subtree, but parent is NULL.\n");
			return -EINVAL;
		}
	}

	/*when only a part of bit assigned. not set the bit */
	/*set_bit(index, &parent->n_tags);*/
	parent->n_count++;
	parent->slots[index] = new;
	new->parent = parent;
	return 0;
}


static int bmptree_init_leaf(struct bmptree_leaf *leaf, int height, int is_full)
{
	unsigned int i;

	leaf->parent = NULL;
	leaf->height = height;
	if(is_full) {
		for(i=0; i<BMPTREE_NODE_TAG_LONGS; i++)
			leaf->tags[i] = ~0UL;
	} else {
		for(i=0; i<BMPTREE_NODE_TAG_LONGS; i++)
			leaf->tags[i] = 0;
	}
	
	return 0;
}

static int bmptree_init_node(struct bmptree *bt,
	   	struct bmptree_node *node, unsigned int height, int is_full)
{
	memset(node->slots, 0, BMPTREE_NODE_MAP_SIZE*sizeof(void *));

	return bmptree_init_leaf((struct bmptree_leaf *)node, height, is_full);
}

#define RECLIAM_REASON_NODEEMPTY 	(0) 
#define RECLIAM_REASON_NODEFULL 	(1)

static int bmptree_recliam_node(struct bmptree *bt, 
		struct bmptree_node *node, int index, int reason)
{
	struct bmptree_node *parent;
	int shift;
	int pos;

	/*pos: leaf position in parent.*/
	shift = (node->n_height + 1) * BMPTREE_NODE_MAP_SHIFT;
	pos = (index >> shift) & BMPTREE_NODE_MAP_MASK;

	BUG_ON(node->n_count != 0);
	PDEBUG("try to recliam node,node pos in parent: %d, height:%d.\n",
		   	pos, node->n_height);
	parent = node->n_parent;
	if(!parent)
		return 0;

	/*
	 * if all bit has been set in this node, set parent bit, and 
	 * free current node.
	 * */
	if(reason == RECLIAM_REASON_NODEFULL) {
		PDEBUG("recliam node,position in parent:%d, all assign.\n", pos);
		set_bit(pos, parent->n_tags);
		parent->slots[pos] = NULL;
	} else if(reason ==  RECLIAM_REASON_NODEEMPTY) {
		PDEBUG("recliam node,position in parent:%d, all empty.\n", pos);
		/*not necessary*/
		clear_bit(pos, parent->n_tags);	
		parent->slots[pos] = NULL;
	} else {
		//printk("node bmp counter error(%d).\n", node->n_count);
		BUG();
		return 0;
	}

	bmptree_free_node(bt, node);

	if(--parent->n_count == 0) {
		int bit;
		bit = (reason == RECLIAM_REASON_NODEEMPTY) ?
			find_first_bit(parent->n_tags, BMPTREE_NODE_MAP_SIZE):
			find_first_zero_bit(parent->n_tags, BMPTREE_NODE_MAP_SIZE);

		if(bit >= BMPTREE_NODE_MAP_SIZE)
			return bmptree_recliam_node(bt, parent, index, reason);
	}
	return 0;
}


static int bmptree_recliam_leaf(struct bmptree *bt, 
		struct bmptree_leaf *leaf, int index, int reason)
{
	struct bmptree_node *parent;
	int shift;
	int pos;

	/*pos: leaf position in parent.*/
	shift = (leaf->height + 1) * BMPTREE_NODE_MAP_SHIFT;
	pos = (index >> shift) & BMPTREE_NODE_MAP_MASK;

	PDEBUG("try to recliam leaf,leaf position in parent: %d.\n", pos);
	parent = leaf->parent;
	/*
	 * if all bit has been set in this node, set parent bit, and 
	 * free current node.
	 * */
	if(reason == RECLIAM_REASON_NODEFULL) {
		PDEBUG("recliam leaf,position in parent: %d, all assign.\n", pos);
		set_bit(pos, parent->n_tags);
		parent->slots[pos] = NULL;
	} else if(reason ==  RECLIAM_REASON_NODEEMPTY) {
		PDEBUG("recliam leaf,position in parent: %d, all empty.\n", pos);
		clear_bit(pos, parent->n_tags);	
		parent->slots[pos] = NULL;
	} else {
		BUG();
		return 0;
	}

	bmptree_free_leaf(bt, leaf);

	if(--parent->n_count == 0 ) {
		int bit;
		bit = (reason == RECLIAM_REASON_NODEEMPTY) ?
			find_first_bit(parent->n_tags, BMPTREE_NODE_MAP_SIZE):
			find_first_zero_bit(parent->n_tags, BMPTREE_NODE_MAP_SIZE);

		if(bit >= BMPTREE_NODE_MAP_SIZE)
			return bmptree_recliam_node(bt, parent, index, reason);
	}
	return 0;
}


int bmptree_check(struct bmptree *bt, unsigned long index)
{
	struct bmptree_node *node, *slot = NULL;
    struct bmptree_leaf *leaf;
	unsigned int height, shift;
	int offset;

	height = bt->height;
	node = bt->rnode;
	shift = height * BMPTREE_NODE_MAP_SHIFT;
	offset = (index >> shift) & BMPTREE_NODE_MAP_MASK;
	
	while(height > 1) {
		offset = (index >> shift) & BMPTREE_NODE_MAP_MASK;
		slot = node->slots[offset];
		height--;

		if(test_bit(offset, node->n_tags)) {
			return 1;
		}

		if(!slot){
			return 0;
		}
		shift -= BMPTREE_NODE_MAP_SHIFT;
		node = slot;
	}

	/*leaf operation*/
	leaf = node->slots[index & BMPTREE_NODE_MAP_MASK];
	if(!leaf) {
		return 0;
	}

	return test_bit(index & BMPTREE_NODE_MAP_MASK, leaf->tags);
}

static int bmptree_find_free_in_leaf(struct bmptree *bt, 
		struct bmptree_leaf *leaf, unsigned long start)
{
	int offset;

	/*leaf operation*/
	offset = find_next_zero_bit(leaf->tags, BMPTREE_NODE_MAP_SIZE,
		   	(start & BMPTREE_NODE_MAP_MASK));

	PDEBUG("%s:offset:%d, start:%d, tags:%08lx.\n", 
			__func__, offset, start, leaf->tags[0]);

	if(offset >= BMPTREE_NODE_MAP_SIZE) {
		return -EAGAIN;
	}

	return (start& ~(BMPTREE_NODE_MAP_MASK)) | offset;
}


static int bmptree_find_free_in_node(struct bmptree *bt, 
		struct bmptree_node *node, unsigned long start)
{
	struct bmptree_node *slot;
	unsigned int height, shift, slot_mask;
	int ofs, new_ofs;
	int ret = -EAGAIN;

	height = node->n_height;
	shift = height * BMPTREE_NODE_MAP_SHIFT;
	ofs = (start >> shift) & BMPTREE_NODE_MAP_MASK;
	slot_mask = ~((1 << (shift + BMPTREE_NODE_MAP_SHIFT)) - 1);

	PDEBUG("%s:tree height:%d, shift:%d, offset:%d, start:%d.\n", 
			__func__, height, shift, ofs, start);

	do {
		new_ofs = find_next_zero_bit(node->n_tags, BMPTREE_NODE_MAP_SIZE, ofs);
		if(ofs != new_ofs) {
			/*re calc the start pos.*/
			ofs = new_ofs;
			start = (start & slot_mask) | (ofs << shift);
		}
		if(ofs >= BMPTREE_NODE_MAP_SIZE) {
			return -EAGAIN;
		}
		PDEBUG("height:%d, slot[%d], tags:%08lx.\n", 
				height, ofs, node->n_tags[0]);
		slot = node->slots[ofs];

		if(!slot){
			/*all sub node is empty.*/
			ret = start;
			goto found;
		}

		if(slot->n_height > 0)
			ret = bmptree_find_free_in_node(bt, slot, start);
		else
			ret = bmptree_find_free_in_leaf(bt, (struct bmptree_leaf *)slot, start);

		if(ret >= 0)
			goto found;

		start = (start & slot_mask) | ((++ofs) << shift);
	}while(ofs < BMPTREE_NODE_MAP_SIZE);

	return -EAGAIN;
found:
	return ret;
}

static inline void bmptree_set_cache(struct bmptree *bt,
	   	struct bmptree_leaf *leaf, unsigned long index)
{
	struct bmptree_cache *cache = &bt->cache;

	cache->leaf = leaf;
	cache->path_idx = index >> BMPTREE_NODE_MAP_SHIFT; 
}

static inline void bmptree_init_cache(struct bmptree *bt)
{
	bmptree_set_cache(bt, NULL, 0);
}

static inline struct bmptree_leaf *bmptree_find_leaf_cache(struct bmptree *bt,
	   	unsigned long index)
{
	struct bmptree_cache *cache = &bt->cache;

	if((cache->leaf == NULL) || 
			(cache->path_idx != index >> BMPTREE_NODE_MAP_SHIFT)) 
		return NULL;

	PDEBUG("leaf has found in cache. path index:%08lx, index:%08lx.\n", 
			cache->path_idx, index);

	return cache->leaf;
}


static int bmptree_find_free_in_cache(struct bmptree *bt, int start)
{
	int ret;
	struct bmptree_leaf *leaf;
	struct bmptree_cache *cache = &bt->cache;

	leaf = bmptree_find_leaf_cache(bt, start);
	if(!leaf)
		return -EAGAIN;

	ret = find_next_zero_bit(leaf->tags, BMPTREE_NODE_MAP_SIZE, start & BMPTREE_NODE_MAP_MASK);

	if(ret >= BMPTREE_NODE_MAP_SIZE)
		return -EAGAIN;
	
	return (cache->path_idx << BMPTREE_NODE_MAP_SHIFT) | ret;
}

static int bmptree_extend(struct bmptree *bt, unsigned long index)
{
	struct bmptree_node *node;
	unsigned int height;

	height = bt->height;
	node = bt->rnode;

	while(index > bmptree_maxindex(bt, height)) {
		PDEBUG("grow up the bmptree.\n");

		node = bmptree_node_alloc(bt);
		bmptree_init_node(bt, node, ++height, 0);
		bmptree_node_insert(bt, (struct bmptree_leaf *)node, NULL, 0);
		bt->rnode = node;
	}

	return 0;
}

int bmptree_set(struct bmptree *bt, unsigned long index)
{
	int err = 0;
	struct bmptree_node *node, *slot;
    struct bmptree_leaf *leaf;
	unsigned int height, shift;
	int offset;

	leaf = bmptree_find_leaf_cache(bt, index);
	if(leaf != NULL)
		goto found;

	height = bt->height;
	node = bt->rnode;
	shift = height * BMPTREE_NODE_MAP_SHIFT;
	offset = (index >> shift) & BMPTREE_NODE_MAP_MASK;

	PDEBUG("%s:tree height:%d, shift:%d, offset:%d, index:%lu.\n", 
			__func__, height, shift, offset, index);

	if(unlikely(index > bmptree_maxindex(bt, height))) {
		err = bmptree_extend(bt, index);
		if(err)
			return err;
	}

	/*leaf node: height equals 0*/
	while(height > 1) {
		slot = node->slots[offset];
		height--;

		if(!slot) {
			//BUG_ON(test_bit(offset, &node->n_tags));
			if(test_bit(offset, node->n_tags)) {
				PDEBUG("%s:set bit exist\n", __func__);
				return -EEXIST;
			}

			if(!(slot = bmptree_node_alloc(bt))) {
				return -ENOMEM;
			}

			bmptree_init_node(bt, slot, height, 0);
			bmptree_node_insert(bt, (struct bmptree_leaf *)slot, node, offset);
		}
		shift -= BMPTREE_NODE_MAP_SHIFT;
		offset = (index >> shift) & BMPTREE_NODE_MAP_MASK;
		node = slot;
	}

	BUG_ON(!node);
	/*leaf operation*/
	leaf = node->slots[offset];
	if(!leaf) {
		//BUG_ON(test_bit(offset, &node->n_tags));
		if(test_bit(offset, node->n_tags)) {
			PDEBUG("%s:set bit exist.\n", __func__);
			return -EEXIST;
		}

		if(!(leaf = bmptree_leaf_alloc(bt))) {
			return -ENOMEM;
		}

		bmptree_init_leaf(leaf, 0, 0);
		bmptree_node_insert(bt, leaf, node, offset);
	}

	bmptree_set_cache(bt, leaf, index);
found:
	if(test_and_set_bit(index & BMPTREE_NODE_MAP_MASK, leaf->tags)) {
		PDEBUG("%s:set bit exist..\n", __func__);
		return -EEXIST;
	}

	if(find_first_zero_bit(leaf->tags, BMPTREE_NODE_MAP_SIZE) >= BMPTREE_NODE_MAP_SIZE)
		bmptree_recliam_leaf(bt, leaf, index, RECLIAM_REASON_NODEFULL);

	return 0;
}

int bmptree_clear(struct bmptree *bt, unsigned long index)
{
	struct bmptree_node *node, *slot;
    struct bmptree_leaf *leaf;
	unsigned int height, shift;
	int offset;

	leaf = bmptree_find_leaf_cache(bt, index);
	if(leaf != NULL)
		goto found;

	height = bt->height;
	node = bt->rnode;
	shift = height * BMPTREE_NODE_MAP_SHIFT;
	offset = (index >> shift) & BMPTREE_NODE_MAP_MASK;
	
	PDEBUG("%s:tree height:%d, shift:%d, offset:%d, index:%lu.\n", 
			__func__, height, shift, offset, index);

	while(height > 1) {
		slot = node->slots[offset];
		height--;

		if(!slot && test_bit(offset, node->n_tags)) {
			if(!(slot = bmptree_node_alloc(bt))) {
				return -ENOMEM;
			}

			bmptree_init_node(bt, slot, height, 1);
			bmptree_node_insert(bt, (struct bmptree_leaf *)slot, node, offset);
			clear_bit(offset, node->n_tags);
		}
		shift -= BMPTREE_NODE_MAP_SHIFT;
		offset = (index >> shift) & BMPTREE_NODE_MAP_MASK;
		node = slot;
	}

	BUG_ON(!node);

	/*leaf operation*/
	leaf = node->slots[offset];
	if(!leaf) {
		if(!(leaf = bmptree_leaf_alloc(bt))) {
			return -ENOMEM;
		}

		bmptree_init_leaf(leaf, 0, 1);
		bmptree_node_insert(bt, leaf, node, offset);
		clear_bit(offset, node->n_tags);
	}
	bmptree_set_cache(bt, leaf, index);

found:
	clear_bit(index & BMPTREE_NODE_MAP_MASK, leaf->tags);

	if(find_first_bit(leaf->tags, BMPTREE_NODE_MAP_SIZE) >= BMPTREE_NODE_MAP_SIZE)
		bmptree_recliam_leaf(bt, leaf, index, RECLIAM_REASON_NODEEMPTY);

	return 0;
}


static inline int verify_vaild_pos(int pos, unsigned long min, unsigned long max)
{
	return (pos >=min) && (pos <= max);
}

int bmptree_find_next_free_pos(struct bmptree *bt, unsigned long start,
	   	unsigned long min, unsigned long max)
{
	int ret;
	struct bmptree_node *root;

	PDEBUG("find next free pos, max index:%lu, start pos:%d.\n",
		   	max, start);

	if(unlikely(start > max)) {
		return -EINVAL;
	}

	/*frist:search the cache node.*/
	ret = bmptree_find_free_in_cache(bt, start);
	if(verify_vaild_pos(ret, min, max))
		return ret;

	/*not hit in cache node , search in tree.*/
	root = bt->rnode;
	ret = bmptree_find_free_in_node(bt, root, start);
	if(unlikely(!verify_vaild_pos(ret, min, max))) {

		/*if the tree max index less than max index, next index is
		 * 1 << ((root->height + 1) * BMPTREE_NODE_MAP_SHIFT).
		 * */
		ret = 1 << ((root->n_height + 1) * BMPTREE_NODE_MAP_SHIFT);
		if(ret <= max) {
			PDEBUG("free bit in parent of curr root node, pos:%d\n", ret);
			return ret;	
		}
		/*research from start pos.*/
		ret = bmptree_find_free_in_node(bt, root, min);
	}

	return verify_vaild_pos(ret, min, max) ? ret : -EAGAIN;	
}

static void bmptree_node_ctor(void *node)
{
	memset(node, 0, sizeof(struct bmptree_node));
}

static void bmptree_leaf_ctor(void *leaf)
{
	return;
}

void *get_bmptree_priv(struct bmptree *bt)
{
	return bt->priv_data;
}

/*
 * fat bitmap tree initialize.
 * */
struct bmptree *bmptree_init(char *name, void *priv)
{
	struct bmptree *bt;
	struct bmptree_node *node;
	char cache_name[BMPTREE_NAME_LEN + 8] = {0};

	PDEBUG("fat bitmap tree initilize.\n");

	bt = kmalloc(sizeof(struct bmptree), GFP_KERNEL);

	memset(bt, 0, sizeof(struct bmptree));

	strlcpy(bt->name, name, sizeof(bt->name));

	snprintf(cache_name, sizeof(cache_name), "%s_ncache", bt->name);
	bt->bt_ncache = kmem_cache_create(cache_name, 
			sizeof(struct bmptree_node), 0, 
		   	SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD, bmptree_node_ctor);
	if(!bt->bt_ncache)
		return ERR_PTR(-ENOMEM);

	snprintf(cache_name, sizeof(cache_name), "%s_lcache", bt->name);
	bt->bt_lcache = kmem_cache_create(cache_name, 
			sizeof(struct bmptree_leaf), 0, 
		   	SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD, bmptree_leaf_ctor);
	if(!bt->bt_lcache)
		return ERR_PTR(-ENOMEM);

	bt->height = 0;
	bt->gfp_mask = GFP_NOFS;
	bt->rnode = NULL;
	bt->free_ncount = bt->free_lcount = 0;
	bt->recliam_low = BMPTREE_FREE_NODE_MIN_LIMIT;
	bt->recliam_high = BMPTREE_FREE_NODE_MAX_LIMIT;
	INIT_LIST_HEAD(&bt->free_nlist);
	INIT_LIST_HEAD(&bt->free_llist);

	bt->used_ncount = bt->used_lcount = 0;

	bt->priv_data = priv;
	bmptree_init_cache(bt);
	bmptree_init_maxindex(bt);

	/*alloc a root node.*/
	if(!(node = bmptree_node_alloc(bt))) {
		return ERR_PTR(-ENOMEM);
	}

	bmptree_init_node(bt, node, 1, 0);
	bmptree_node_insert(bt, (struct bmptree_leaf *)node, NULL, 0);

	PDEBUG("bitmap tree initilize success.\n");
	return bt;
}

void bmptree_release(struct bmptree *bt)
{
	struct bmptree_node *n, *ntmp;
	struct bmptree_leaf *l, *ltmp;

	PDEBUG("fat bitmap tree release.\n");
	if(!bt) {
		return;
	}

	bmptree_free_tree(bt);

	list_for_each_entry_safe(n, ntmp, &bt->free_nlist, n_head) {
		list_del(&n->n_head);
		kmem_cache_free(bt->bt_ncache, n);
	}

	list_for_each_entry_safe(l, ltmp, &bt->free_llist, head) {
		list_del(&l->head);
		kmem_cache_free(bt->bt_lcache, l);
	}

	kmem_cache_destroy(bt->bt_ncache);
	kmem_cache_destroy(bt->bt_lcache);
	kfree(bt);
}


static void bmptree_dump_leaf(struct bmptree_leaf *leaf)
{
	int i;
	//printk("tree height:%d, count:%d.\n", node->height, node->n_count);

	for(i=0; i<BMPTREE_NODE_MAP_SIZE; i++) {
		printk("%2d ", i);
	}
	printk("\n");

	for(i=0; i<BMPTREE_NODE_MAP_SIZE; i++) {
		printk(" %s ", (test_bit(i, leaf->tags)) ? "X":"O");
	}

	printk("\n");
}


static void bmptree_dump_node(struct bmptree_node *node)
{
	int i;
	struct bmptree_node *slot;

	printk("tree height:%d, count:%d.\n", node->n_height, node->n_count);
	for(i=0; i<BMPTREE_NODE_MAP_SIZE; i++) {
		printk("%2d ", i);
	}
	printk("\n");

	for(i=0; i<BMPTREE_NODE_MAP_SIZE; i++) {
		slot = node->slots[i];
		if(slot != NULL) {
			printk(" %s ", (test_bit(i, node->n_tags)) ? "Err":"P");
		}else 
			printk(" %s ", (test_bit(i, node->n_tags)) ? "X":"O");
	}

	printk("\n");

	for(i=0; i<BMPTREE_NODE_MAP_SIZE; i++) {
		slot = node->slots[i];
		if(slot != NULL) {

			if(slot->n_height > 0) {
				printk("node slot[%d]:\n", i);
				bmptree_dump_node(slot);
			} else {
				printk("leaf slot[%d]:\n", i);
				bmptree_dump_leaf((struct bmptree_leaf *)slot);
			}
		}
	}
}

void bmptree_dump_tree(struct bmptree *bt)
{
#if 0
	printk("note: \nErr: bitmap error. \n"
			"P: a part of node has been assign(have subtree.).\n"
			"O: all node is free. \n"
			"X:all node has been assign.\n");
#endif
	printk("-----------------------tree----------------------------\n");
	if(bt->rnode == NULL) {
		printk("[empty tree]\n");
	} else
		bmptree_dump_node(bt->rnode);

	printk("-------------------------------------------------------\n");
	return;
}

void show_tree_amount(struct bmptree *bt)
{
	/*use for debug.*/
	printk("bmptree(%s) infomation:\n", bt->name);

	printk("tree height:%d, maxindex:%lu.\n",
		   	bt->height, bt->height_to_maxidx[bt->height]);

	printk("total used node count:%d(totals %dBytes), leaf count:%d(totals %dBytes).\n", 
			bt->used_ncount, sizeof(struct bmptree_node)*bt->used_ncount,
		   	bt->used_lcount, sizeof(struct bmptree_leaf)*bt->used_lcount);
}


int bmptree_testmyself(void)
{
#define CHK_RET(r) 	\
	do { \
		if(r < 0) { \
			printk("ERR:%d.\n", __LINE__); \
			return r; \
		} \
	}while(0)

	int ret = 0;
	int i;
   	unsigned long	t = 0, c = 0;
	struct bmptree *bt = NULL;

	printk("bmptree start test myself.\n");
	bt = bmptree_init("bmptree_test", NULL);
	CHK_RET(ret);

	bmptree_dump_tree(bt);

	printk("start test set bit.\n");
	for(i=0; i<1000; i++) {
		//printk("set pos:%d.\n", i*2);
		ret = bmptree_set(bt, i*2);
		CHK_RET(ret);
	//	bmptree_dump_tree(bt);
	}
	show_tree_amount(bt);

	for(i=200; i<600; i++) {
		//printk("set pos:%d.\n", i*2+1);
		ret = bmptree_set(bt, i*2+1);
		CHK_RET(ret);
	//	bmptree_dump_tree(bt);
	}
	show_tree_amount(bt);
	bmptree_dump_tree(bt);

//#define TEST_FIND_FREE_CLUSTER

#ifdef TEST_FIND_FREE_CLUSTER
	printk("start test find free bit.\n");
	for(i=0; i<2000; i++) {
		ret = bmptree_find_next_free_pos(bt, i, 0, 3000);
		printk("find bit:first pos:%d, free bit:%d.\n", i, ret);
	}

	for(i=2000; i<2500; i++) {
		ret = bmptree_find_next_free_pos(bt, i, 1, 2500);
		printk("find bit:first pos:%d, free bit:%d.\n", i, ret);
	}

	for(i=300; i<800; i++) {
		ret = bmptree_find_next_free_pos(bt, i, 2, 1000);
		printk("find bit:first pos:%d, free bit:%d.\n", i, ret);
	}

	for(i=32760; i<32780; i++) {
		ret = bmptree_find_next_free_pos(bt, i, 2, 40000);
		printk("find bit:first pos:%d, free bit:%d.\n", i, ret);
	}
#endif


#define TEST_CLEAR_CLUSTER

#ifdef TEST_CLEAR_CLUSTER
	printk("start test clear bit.\n");
	bmptree_dump_tree(bt);
	for(i=0; i<1000; i++) {
		//printk("clear pos:%d.\n", i*2);
		bmptree_clear(bt, i*2);
		CHK_RET(ret);
	//	bmptree_dump_tree(bt);
	}

	bmptree_dump_tree(bt);
	show_tree_amount(bt);

	for(i=200; i<600; i++) {
		//printk("clear pos:%d.\n", i*2+1);
		ret = bmptree_clear(bt, i*2+1);
		CHK_RET(ret);
	//	bmptree_dump_tree(bt);
	}

	show_tree_amount(bt);
#endif
	
#define TEST_FIND_TIMES
#ifdef TEST_FIND_TIMES
	printk("start insert data.\n");
	t = jiffies;
	for(i=0; i<16*1024*1024; i++) {
		ret = bmptree_set(bt, i*2);
		CHK_RET(ret);
	}
	t = ((jiffies - t)*1000)/HZ;
	c = (t*1000) /(16*1024*1024/10000);
	printk("insert 16M counts data use %lums. avg:%lu.%lu ms/10000cnt\n",
		  t, c/1000, (c%1000)/10);

	show_tree_amount(bt);

	printk("start find data.\n");
	t = jiffies;
	for(i=0; i<1024*1024; i++) {
		ret = bmptree_find_next_free_pos(bt, 17*i, 0, 16*1024*1024);
	}

	t = ((jiffies - t)*1000)/HZ;
	c = (t*1000) /(1024*1024/10000);
	printk("find 1M counts use %lums. avg:%lu.%lu ms/10000cnt.\n",
		  t, c/1000, (c%1000)/10);


	printk("start clear data.\n");
	t = jiffies;
	for(i=0; i<16*1024*1024; i++) {
		ret = bmptree_clear(bt, i*2);
		CHK_RET(ret);
	}

	t = ((jiffies - t)*1000)/HZ;
	c = (t*1000) /(16*1024*1024/10000);
	printk("delete 16M counts use %lums. avg:%lu.%lu ms/10000cnt.\n",
		  t, c/1000, (c%1000)/10);

	show_tree_amount(bt);

#endif


	printk("test release bmptree.\n");
	bmptree_dump_tree(bt);
	bmptree_release(bt);

	printk("test bmptree finish.\n");
	return ret;
}


