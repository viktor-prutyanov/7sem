#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void merge_sort(std::vector<int>::iterator begin, std::vector<int>::iterator end, int level = 0)
{
    size_t size = end - begin;
    if (size <= 1)
        return;

    size_t left_size = size / 2;
    size_t right_size = size - left_size;

    {
        merge_sort(begin, begin + left_size, level + 1);
        merge_sort(begin + left_size, end, level + 1);
    }
    
    size_t li = 0, ri = left_size, i = 0;
    std::vector<int> tmp_v(size);

    while (li < left_size || ri < size)
        if (((li != left_size) && *(begin + li) < *(begin + ri)) || (ri == size)) {
            tmp_v[i] = *(begin + li);
            ++i;
            ++li;
        } else {
            tmp_v[i] = *(begin + ri);
            ++i;
            ++ri;
        }

    std::copy(tmp_v.begin(), tmp_v.begin() + size, begin);
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
    merge_sort(v.begin(), v.end());
    fprintf(stderr, "%lf sec\n", omp_get_wtime() - begin);

    if (!std::is_sorted(v.begin(), v.end())) {
        fprintf(stderr, "Sort error\n");
        return -1;
    }

    return 0;
}
