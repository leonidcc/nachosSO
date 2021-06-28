#include "channel.hh"

Channel::Channel(){

    buff = nullptr;
    ready = false;
    lock = new Lock("Lock");
    send = new Condition("Send channel", lock);
    receive = new Condition("Receive channel", lock);
}

Channel::~Channel(){

    delete lock;
    delete send;
    delete receive;
}

void Channel::Send(int msg){

    lock->Acquire();
    while(ready)
        receive->Wait();

    *buff = msg;
    ready = true;

    send->Signal();

    lock->Release();
}

void Channel::Receive(int *msg){

    lock->Acquire();
    while(!ready)
        send->Wait();

    ASSERT(msg);
    msg = buff;
    ready = false;

    receive->Signal();

    lock->Release();
}
