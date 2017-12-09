#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    double *a = (double *)malloc(ISIZE * JSIZE * sizeof(*a));
    FILE *ff;
    double start;

    for (int i = 0; i < ISIZE; i++)
        for (int j = 0; j < JSIZE; j++)
            a[i * JSIZE + j] = 10 * i +j;
    
    start = omp_get_wtime();
    #pragma omp parallel
    {
        for (int i = 1; i < ISIZE; i++) 
        {
            #pragma omp for
            for (int j = 3; j < JSIZE - 1; j++)
                a[i * JSIZE + j] = sin(0.00001 * a[(i - 1) * JSIZE + (j - 3)]);
        }
    }
    fprintf(stderr, "%lg\n", omp_get_wtime() - start);

    ff = fopen("result.txt","w");
    
    for (int i = 0; i < ISIZE; i++)
        for (int j = 0; j < JSIZE; j++)
            fprintf(ff, "%f ", a[i * JSIZE + j]);
        fprintf(ff, "\n");

    free(a);
    fclose(ff);
}
