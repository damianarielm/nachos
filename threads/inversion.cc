#include "system.hh"
#include "synch.hh"

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

void InversionTest() {
#ifndef INVERSION
    printf("The priority inversion patch " RED "isn't activated. " RESET);
    printf("In order to change the behaviour just add the INVERSION flag to ");
    printf("thread/Makefile.\n");
#else
    printf("The priority inversion patch " GREEN "is activated. " RESET);
    printf("In order to change the behaviour just remove the INVERSION flag to ");
    printf("thread/Makefile.\n");
#endif
    printf("To understeand better what's happening you should run nachos with ");
    printf(BOLD "-d Ll" RESET ".\n");

    lock = new Lock("Lock");

    Thread *t4 = new Thread("High", false, MAX_PRIORITY);
    Thread *t3 = new Thread("Medium 1", false, 3);
    Thread *t2 = new Thread("Medium 2", false, 3);
    Thread *t1 = new Thread("Low", false, 0);

    t1->Fork(Low, nullptr);
    currentThread->Yield();
    t2->Fork(Medium, nullptr);
    t3->Fork(Medium, nullptr);
    t4->Fork(High, nullptr);
}
