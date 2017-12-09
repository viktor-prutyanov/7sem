#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>

#define NR_SIGNS 20

inline double fact(int n);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s num_iters\n", argv[0]);
        return 1;
    }

    int num = atoi(argv[1]);
    double result = 0;

    #pragma omp parallel for reduction (+: result) 
    for (int i = 0; i < num; ++i)
    {
        result += 1/fact(i);
    }

    printf("%4.*lf\n", NR_SIGNS, result);

    return 0;
}

double fact(int n)
{
    double result = 1.0;
    for (int i = 2; i <= n; ++i)
        result *= i;

    return result;
}
