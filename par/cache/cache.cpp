#include <iostream>
#include <memory>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <sys/mman.h>

#define MAX_NR_BLOCKS 40
#define NR 100000000
#define BLOCK_SIZE (1 << 16)

using namespace std;

struct line {
    uint64_t data[8];
};

int main() {
    cout << "set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5\n" 
         << "plot '-' with linespoints ls 1\n";

    for (uint64_t nr_blocks = 1; nr_blocks <= MAX_NR_BLOCKS; ++nr_blocks)
    {
        void *buf = mmap(NULL, sizeof(line) * nr_blocks * BLOCK_SIZE, 
                PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        if (buf == MAP_FAILED)
            return -1;

        line *lines = (line *)buf;

        for (uint64_t j = 0; j < nr_blocks; j++) {
            for (uint64_t i = 0; i < nr_blocks - 1; i++) {
                lines[i * BLOCK_SIZE + j].data[0] = (i + 1) * BLOCK_SIZE + j;
            }
            lines[(nr_blocks - 1) * BLOCK_SIZE + j].data[0] = j + 1;
        }

        lines[(nr_blocks - 1) * BLOCK_SIZE + nr_blocks - 1].data[0] = 0;
        
        volatile register uint64_t tmp = 0;
        volatile register uint64_t next = 0;
        
        auto start = chrono::high_resolution_clock::now();
        for (uint64_t i = 0; i < NR; i++) {
            tmp = next;
            next = lines[next].data[0];
        }
        auto finish = chrono::high_resolution_clock::now();
        
        cout << nr_blocks << " " 
             << chrono::duration<uint64_t, nano>(finish - start).count() / NR
             << "\n";
        munmap((void *)lines, sizeof(line) * nr_blocks * BLOCK_SIZE);
    }

    cout << "e\n";

    return 0;
}
