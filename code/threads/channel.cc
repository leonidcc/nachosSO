#include "channel.hh"

Channel::Channel(){

    buff = NULL;
    ready = false;
    lock = new Lock("Lock");
    send = new Condition("Send channel", lock);
    receive = new Condition("Receive channel", lock);
    empty = new Condition("Empty info", lock);
}

Channel::~Channel(){

    delete lock;
    delete send;
    delete receive;
    delete empty;
}

void Channel::Send(int msg){

    lock->Acquire();
    while(ready)
        receive->Wait();

    buff = msg;
    ready = true;

    send->Signal();
    empty->Signal();
    lock->Release();
}

void Channel::Receive(int *msg){

    lock->Acquire();
    while(!ready)
        empty->Wait();

    *msg = buff;
    ready = false;

    receive->Signal();
    send->Wait();
    lock->Release();
}
