#include <stdio.h>
#include "system.hh"
#include "lock.hh"

Lock* lock;

void High(void* args) {
    lock->Acquire();
    lock->Release();

    printf("High priority task done.\n");
}

void Medium(void* args) {
    printf("Medium priority infinite loop...\n");

    while (1) currentThread->Yield();
}

void Low(void* args) {
    lock->Acquire();
        currentThread->Yield();
    lock->Release();

    printf("Low priority task done.\n");
}
void ThreadTestProdCons() {
#ifndef INVERSION
    printf("The priority inversion patch isn't activated. ");
    printf("In order to change the behaviour just add the INVERSION flag to ");
    printf("thread/Makefile.\n");
#else
    printf("The priority inversion patch is activated. ");
    printf("In order to change the behaviour just remove the INVERSION flag to ");
    printf("thread/Makefile.\n");
#endif

    lock = new Lock("Lock");
    Thread *t4 = new Thread("High", false, 0);
    Thread *t3 = new Thread("Medium 1", false, 1);
    Thread *t2 = new Thread("Medium 2", false, 1);
    Thread *t1 = new Thread("Low", false, 2);

    t1->Fork(Low, nullptr);
    currentThread->Yield();
    t2->Fork(Medium, nullptr);
    t3->Fork(Medium, nullptr);
    t4->Fork(High, nullptr);
}
