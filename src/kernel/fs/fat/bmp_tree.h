#ifndef __BMP_TREE_H__
#define __BMP_TREE_H__


#define BMPTREE_TREE_INDEX_BITS 	(8 /*char bit*/ * sizeof(unsigned long))

#define BMPTREE_NODE_MAP_SHIFT 		(5)
#define BMPTREE_NODE_MAP_SIZE 	(1UL << BMPTREE_NODE_MAP_SHIFT)
#define BMPTREE_NODE_MAP_MASK 	(BMPTREE_NODE_MAP_SIZE - 1)

#define BMPTREE_MAX_PATH 	\
	(DIV_ROUND_UP(BMPTREE_TREE_INDEX_BITS, BMPTREE_NODE_MAP_SHIFT))

#define BMPTREE_NODE_TAG_LONGS  \
	((BMPTREE_NODE_MAP_SIZE + BITS_PER_LONG -1)/BITS_PER_LONG)

#define BMPTREE_FREE_NODE_MAX_LIMIT 	(64)
#define BMPTREE_FREE_NODE_MIN_LIMIT 	(8)

struct bmptree_leaf
{
	unsigned long 	tags[BMPTREE_NODE_TAG_LONGS]; /*curr only one tags*/
	uint8_t 	 	height;
	uint8_t 		reserved; /*align*/
	union {
		struct bmptree_node *parent;
		struct list_head head;
	};
};


struct bmptree_node
{
	struct bmptree_leaf  base;

#define n_tags 		base.tags
#define n_height 	base.height
#define n_parent 	base.parent
#define n_head 		base.head

	uint8_t n_count;
	void *slots[BMPTREE_NODE_MAP_SIZE];
};

struct bmptree_cache
{
	struct bmptree_leaf *leaf;
	unsigned long path_idx;

};

#define BMPTREE_NAME_LEN 	(32)
struct bmptree
{
	char name[BMPTREE_NAME_LEN];
	unsigned int height;
	gfp_t gfp_mask;
	struct bmptree_node *rnode;
	struct bmptree_cache cache; /*use for fast set/clear*/

	unsigned long height_to_maxidx[BMPTREE_MAX_PATH + 1];

	struct kmem_cache *bt_ncache; /*bmptree_node cache*/
	struct kmem_cache *bt_lcache; /*bmptree_leaf cache*/

	struct list_head free_nlist; /*free node list*/
	struct list_head free_llist; /*free leaf list*/
	int free_ncount;  /*free node count*/
	int free_lcount;  /*free leaf count*/

	int recliam_low;  /**/
	int recliam_high;

	int used_ncount; /*total used node count, used for debug*/
	int used_lcount; /*total used leaf count, used for debug*/
	void *priv_data;
};

void *get_bmptree_priv(struct bmptree *bt);

int bmptree_check(struct bmptree *bt, unsigned long index);
int bmptree_set(struct bmptree *bt, unsigned long index);
int bmptree_clear(struct bmptree *bt, unsigned long index);

int bmptree_find_next_free_pos(struct bmptree *bt, unsigned long start,
	   unsigned long min, unsigned long max);

/*
 * fat bitmap tree initialize.
 * */
struct bmptree *bmptree_init(char *name, void *priv);


void bmptree_release(struct bmptree *bt);

void bmptree_dump_tree(struct bmptree *bt);
int bmptree_testmyself(void);

#endif
