#include <stdio.h>
#include <omp.h>
#include <stdbool.h>

int main()
{
    int move = 0;
    
    omp_lock_t lock;
    omp_init_lock(&lock);

    #pragma omp parallel 
    {
        bool done = false;
        int tid = omp_get_thread_num();
        while (!done)
        {
            while ((tid != move) || !omp_test_lock(&lock));
            if (tid == move)
            {
                printf("%d\n", tid);
                move++;
                done = true;
            }
            omp_unset_lock(&lock);
        }
    }

    omp_destroy_lock(&lock);

    return 0;
}
