#include <QCoreApplication>
#include <QAbstractAnimation>
#include <unistd.h>
#include "mythread.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s num_threads\n", argv[0]);
        return -1;
    }

    int n = atoi(argv[1]);

    std::vector<MyThread> threads(n);
    for (int i = 1; i < n; ++i)
        threads[i].set(&threads[i - 1], i);

    for (auto &t : threads)
        t.start();

    threads[n - 1].go();

    for (auto &t : threads)
        t.wait();

    return 0;
}
