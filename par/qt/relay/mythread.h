#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <iostream>

class MyThread : public QThread
{
    Q_OBJECT

public:
    MyThread()
        :number(0)
    {
        //nothing to do
    }
    void set(int n)
    {
        this->number = n;
    }
    void go()
    {
        printf("Thread #%d\n", number);
        emit go_next();
        quit();
    }
signals:
    void go_next();
protected:
    void run()
    {
        exec();
    }
public slots:
    void do_go() {
        go();
    }
private:
    int number;
};

#endif // MYTHREAD_H
