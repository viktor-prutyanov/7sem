#include <stdint.h>
#include <stdbool.h>

#define MIN_DEGREE 	3
#define MAX_NR_KEYS (2 * MIN_DEGREE - 1)

typedef struct btree_node {
	uint64_t nr_keys;
	uint64_t keys[MAX_NR_KEYS];
	uint64_t vals[MAX_NR_KEYS];
	struct btree_node *children[MAX_NR_KEYS + 1];
    bool is_pr[MAX_NR_KEYS];
	bool is_leaf;
} bnode_t;

typedef struct btree {
    bnode_t *root;
} btree_t;

int btree_create(btree_t *btree);
void btree_destroy(btree_t *btree);
void btree_insert(btree_t *btree, uint64_t key, uint64_t val);
bnode_t *btree_search(btree_t *btree, uint64_t key, uint64_t *val);
void btree_dump(btree_t *btree);
void btree_delete_key(btree_t *btree, uint64_t key);
