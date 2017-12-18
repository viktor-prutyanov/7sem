#include <QCoreApplication>
#include <QAbstractAnimation>
#include <unistd.h>
#include "mythread.h"

int main(int argc, char *argv[])
{
    QCoreApplication loop(argc, argv);

    if (argc != 2)
    {
        printf("usage: %s num_threads\n", argv[0]);
        return -1;
    }

    int n = atoi(argv[1]);

    std::vector<MyThread> threads(n);
    for (int i = 1; i < n; ++i)
        threads[i].set(i);

    for (int i = 0; i < n - 1; i++)
        QObject::connect(&threads[i], SIGNAL(go_next()), &threads[i+1], SLOT(do_go()));

    for (auto &t : threads)
        t.start();

    threads[0].go();

    for (auto &t : threads)
        t.wait();

    return 0;
}
