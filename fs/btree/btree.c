#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MIN_DEGREE 	2
#define MAX_NR_KEYS (2 * MIN_DEGREE - 1)
#define NR_CHILDREN(bnode) (bnode->nr_keys + 1)

typedef struct btree_node {
	uint64_t nr_keys;
	uint64_t keys[MAX_NR_KEYS];
	struct btree_node *children[MAX_NR_KEYS + 1];
	bool is_leaf;
} bnode_t;

bnode_t *bnode_alloc()
{
	bnode_t *bnode = (bnode_t *)malloc(sizeof(bnode_t));
	if (!bnode)
		return NULL;

	return bnode;
}

void bnode_free(bnode_t *bnode)
{
	free(bnode);	
}

/*
 * Splits full child node #i of non-empty node
 */
void bnode_split_child(bnode_t *bnode, uint64_t i)
{
	bnode_t *child = bnode->children[i];
	bnode_t *new_child = bnode_alloc();
	
	assert(bnode->nr_keys < MAX_NR_KEYS);
	assert(bnode->children[i]->nr_keys == MAX_NR_KEYS);
	
	new_child->is_leaf = child->is_leaf;
	new_child->nr_keys = MIN_DEGREE - 1;
		
	memcpy(new_child->keys, child->keys + MIN_DEGREE, (MIN_DEGREE - 1) * sizeof(bnode_t));
	if (!child->is_leaf)
		memcpy(new_child->children, child->children + MIN_DEGREE, MIN_DEGREE * sizeof(bnode_t));

	child->nr_keys = MIN_DEGREE - 1;

	for (uint64_t j = NR_CHILDREN(bnode); j > i + 1; --j)
		bnode->children[j] = bnode->children[j - 1];
	bnode->children[i + 1] = new_child;

	for (uint64_t j = bnode->nr_keys; j > i; --j)
		bnode->keys[j] = bnode->keys[j - 1];
	bnode->keys[i] = child->keys[MIN_DEGREE - 1];
	
	++bnode->nr_keys;
}

/*
uint64_t search(uint64_t *keys, uint64_t nr_keys, uint64_t k)
{
	uint64_t low, middle, high;

	low =  
}
*/


