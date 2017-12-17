#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "btree.h"

#define NR_INS 1000000UL
#define NR_DEL 15

int main()
{
    btree_t btree;
   
    fprintf(stdout, "Insertion test:\n");
    btree_create(&btree);
   
    clock_t clk = clock(); 
    for (long int i = 0; i < NR_INS; ++i) {
        uint64_t key = random() % NR_INS;
        btree_insert(&btree, key, i);
    }
    fprintf(stderr, "%lums - %lu keys\n", (clock() - clk) * 1000 / CLOCKS_PER_SEC, NR_INS);
    
    btree_destroy(&btree);
    
    fprintf(stdout, "Deletion test:\n");
    btree_create(&btree);
    
    for (long int i = 0; i < NR_DEL; ++i) {
        uint64_t key = random() % NR_DEL;
        btree_insert(&btree, key, i);
    }
    btree_dump(&btree);
    fprintf(stdout, "delete ");
    for (long int i = 0; i < NR_DEL; ++i) {
        uint64_t key = random() % NR_DEL;
        fprintf(stdout, "%lu ", key);
        btree_delete_key(&btree, key);
    }
    fprintf(stdout, "\n");
    btree_dump(&btree);
    fprintf(stdout, "insert at ");
    for (long int i = 0; i < NR_DEL; ++i) {
        uint64_t key = random() % NR_DEL;
        fprintf(stdout, "%lu ", key);
        btree_insert(&btree, key, i);
    }
    fprintf(stdout, "\n");
    btree_dump(&btree);
    
    btree_destroy(&btree);
    
    return 0;
}
