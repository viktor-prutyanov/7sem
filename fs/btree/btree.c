#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

#define NR_CHILDREN(bnode) (bnode->nr_keys + 1)

static bnode_t *bnode_alloc()
{
	bnode_t *bnode = (bnode_t *)malloc(sizeof(bnode_t));
	if (!bnode)
		return NULL;

	return bnode;
}

void btree_create(btree_t *btree)
{
    btree->root = bnode_alloc();
    btree->root->is_leaf = true;
    btree->root->nr_keys = 0;
}

static void bnode_free(bnode_t *bnode)
{
	free(bnode);	
}

static void bnode_destroy_subtree(bnode_t *bnode)
{
    if (!bnode->is_leaf) {
        for (uint64_t i = 0; i < NR_CHILDREN(bnode); ++i) {
            bnode_destroy_subtree(bnode->children[i]);
        }
    }
    bnode_free(bnode);
}

void btree_destroy(btree_t *btree)
{
    bnode_destroy_subtree(btree->root);
}

/*
 * Splits full child node #i of non-full node
 */
static void bnode_split_child(bnode_t *bnode, uint64_t i)
{
	bnode_t *child = bnode->children[i];
	bnode_t *new_child = bnode_alloc();
	
	assert(bnode->nr_keys < MAX_NR_KEYS);
	assert(bnode->children[i]->nr_keys == MAX_NR_KEYS);
	
	new_child->is_leaf = child->is_leaf;
	new_child->nr_keys = MIN_DEGREE - 1;

	memcpy(new_child->keys, child->keys + MIN_DEGREE, 
            (MIN_DEGREE - 1) * sizeof(*child->keys));
	memcpy(new_child->vals, child->vals + MIN_DEGREE, 
            (MIN_DEGREE - 1) * sizeof(*child->vals));
	memcpy(new_child->is_pr, child->is_pr + MIN_DEGREE, 
            (MIN_DEGREE - 1) * sizeof(*child->is_pr));
	if (!child->is_leaf)
		memcpy(new_child->children, child->children + MIN_DEGREE, 
                MIN_DEGREE * sizeof(bnode_t *));

	child->nr_keys = MIN_DEGREE - 1;

	for (uint64_t j = NR_CHILDREN(bnode); j > i + 1; --j)
		bnode->children[j] = bnode->children[j - 1];
	bnode->children[i + 1] = new_child;

	for (uint64_t j = bnode->nr_keys; j > i; --j) {
		bnode->keys[j] = bnode->keys[j - 1];
		bnode->vals[j] = bnode->vals[j - 1];
		bnode->is_pr[j] = bnode->is_pr[j - 1];
    }
	bnode->keys[i] = child->keys[MIN_DEGREE - 1];
	bnode->vals[i] = child->vals[MIN_DEGREE - 1];
	bnode->is_pr[i] = child->is_pr[MIN_DEGREE - 1];
	
	++bnode->nr_keys;
}

/*
 * Inserts key in subtree formed by non-full node
 */
static void bnode_insert_nonfull(bnode_t *bnode, uint64_t key, uint64_t val)
{
    uint64_t i = bnode->nr_keys - 1;
    
	assert(bnode->nr_keys < MAX_NR_KEYS);
    
    if (bnode->is_leaf) {
        while (key < bnode->keys[i]) {
            bnode->keys[i + 1] = bnode->keys[i];
            bnode->vals[i + 1] = bnode->vals[i];
            bnode->is_pr[i + 1] = bnode->is_pr[i];
            if (i-- == 0)
                break;
        }

        bnode->keys[i + 1] = key;
        bnode->vals[i + 1] = val;
        bnode->is_pr[i + 1] = true;
        ++bnode->nr_keys;
    } else {
        while (key < bnode->keys[i]) {
            if (i-- == 0)
                break;
        }
        ++i;
        
        if (bnode->children[i]->nr_keys == MAX_NR_KEYS) {
            bnode_split_child(bnode, i);
            if (key > bnode->keys[i])
                ++i;
        }

        bnode_insert_nonfull(bnode->children[i], key, val);
    }
}

static void bnode_dump_subtree(bnode_t *bnode)
{
    fprintf(stdout, "(");
    
    for (uint64_t i = 0; i < bnode->nr_keys; ++i) {
        if (!bnode->is_leaf)
            bnode_dump_subtree(bnode->children[i]);
        fprintf(stdout, " %lu%c%lu ", bnode->keys[i],
                bnode->is_pr[i] ? ':' : '*', bnode->vals[i]);
    }
    if (!bnode->is_leaf)
        bnode_dump_subtree(bnode->children[NR_CHILDREN(bnode) - 1]);

    fprintf(stdout, ")");
}

void btree_dump(btree_t *btree)
{
    fprintf(stdout, "btree at 0x%p:\n\t", btree);
    bnode_dump_subtree(btree->root);
    fprintf(stdout, "\n");
}

static inline bnode_t *bnode_search(bnode_t *bnode, uint64_t key, 
                                    uint64_t *index)
{
    uint64_t i = 0;

    while ((i < bnode->nr_keys) && (key > bnode->keys[i]))
        ++i;

    if ((i <= bnode->nr_keys) && (key == bnode->keys[i])) {
        if (index)
            *index = i;
        return bnode;
    } else if (bnode->is_leaf)
        return NULL;
    else
        return bnode_search(bnode->children[i], key, index);
}

void btree_delete_key(btree_t *btree, uint64_t key)
{
    uint64_t index;
    bnode_t *bnode = bnode_search(btree->root, key, &index);

    if (bnode)
        bnode->is_pr[index] = false;
}

bnode_t *btree_search(btree_t *btree, uint64_t key, uint64_t *val)
{
    uint64_t index;
    bnode_t *bnode = bnode_search(btree->root, key, &index);
    
    if (bnode) {
        if (bnode->is_pr[index])
            *val = bnode->vals[index];
        else
            return NULL;
    }
 
    return bnode;
}

void btree_insert(btree_t *btree, uint64_t key, uint64_t val)
{
    bnode_t *root = btree->root;
    uint64_t index;
    bnode_t *bnode = bnode_search(root, key, &index);

    if (bnode) {
        bnode->vals[index] = val;
        bnode->is_pr[index] = true;
        return;
    }

    if (root->nr_keys != MAX_NR_KEYS)
        bnode_insert_nonfull(root, key, val);
    else {
        bnode_t *s = bnode_alloc();
        s->is_leaf = false;
        s->nr_keys = 0;
        s->children[0] = root;

        btree->root = s;
        bnode_split_child(s, 0);
        bnode_insert_nonfull(s, key, val);
    }
}
