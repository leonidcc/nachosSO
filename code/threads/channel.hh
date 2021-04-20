#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "lock.hh"
#include "condition.hh"

class Channel {
public:

    Channel();
    ~Channel();

    void Send(int msg);
    void Receive(int *msg);

private:
    bool ready;
    int buff;
    Lock* lock;

    Condition* send;
    Condition* receive;
    Condition* empty;
};

#endif
