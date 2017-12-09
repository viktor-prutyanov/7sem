#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include <utility>

void bubble_sort(std::vector<int>::iterator begin, std::vector<int>::iterator end)
{
    size_t size = end - begin;

    int temp;
    for (size_t i = 0; i < size - 1; ++i) {
        for (size_t j = 0; j < size - i - 1; ++j) {
            if (*(begin + j) > *(begin + j + 1)) {
                temp = *(begin + j);
                *(begin + j) = *(begin + j + 1);
                *(begin + j + 1) = temp;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s nr_elements\n", argv[0]);
        return -1;
    }
    
    char *endptr;
    unsigned long nr_elems = strtoul(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid number\n");
        return -1;
    }

    std::vector<int> v;
    for (size_t i = 0; i < nr_elems; ++i)
        v.push_back(rand());
    
    if (std::is_sorted(v.begin(), v.end())) {
        fprintf(stderr, "Sorted already\n");
        return -1;
    }

    double begin = omp_get_wtime();
    bubble_sort(v.begin(), v.end());
    fprintf(stderr, "%lf sec\n", omp_get_wtime() - begin);

    if (!std::is_sorted(v.begin(), v.end())) {
        fprintf(stderr, "Sort error\n");
        for (auto &item : v) {
            fprintf(stderr, "%d ", item);
        }
        fprintf(stderr, "\n");
        return -1;
    }

    return 0;
}
