#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <iostream>

class MyThread : public QThread
{
public:
    MyThread()
        :next(nullptr), number(0)
    {
        //nothing to do
    }
    void set(MyThread *next, int n)
    {
        this->next = next;
        this->number = n;
    }
    void go()
    {
        sem.release(1);
    }
protected:
    void run()
    {
        sem.acquire(1);
        printf("Thread #%d\n", number);
        if (next != nullptr)
            next->go();
    }
private:
    QSemaphore sem;
    MyThread *next;
    int number;
};

#endif // MYTHREAD_H
