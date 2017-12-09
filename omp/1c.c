#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int *numbers = malloc((argc - 1) * sizeof(*numbers));
    
    for (int i = 0; i < argc - 1; ++i)
    {
        numbers[i] = atoi(argv[i + 1]); 
    }

    int result = 0;

    #pragma omp parallel for reduction (max: result) 
    for (int i = 0; i < argc - 1; ++i)
    {
        if (numbers[i] > result)
            result = numbers[i];
    }

    printf("%d\n", result);

    free(numbers);

    return 0;
}
