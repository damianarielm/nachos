/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "system.hh"

/// Loops yielding the CPU to another ready thread each iteration.
void
SimpleThread(void* args) {
    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < * (unsigned*) args; num++) {
        printf("*** Thread %s is running: iteration %u.\n",
                currentThread->GetName(), num);
        currentThread->Yield();
    }
    printf("!!! Thread %s has finished.\n", currentThread->GetName());
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`.
void
ThreadTest() {
    char* name;
    Thread* newThread;
    unsigned* num = new unsigned;

    printf("How many iterations: ");
    scanf("%u", num);

    printf("Starting thread test.\n");

    for (unsigned i = 0; i < 2; i++) {
        name = new char [4];
        sprintf(name, "%uº", i + 1);
        newThread = new Thread(name);
        printf("Starting thread %s.\n", name);
        newThread->Fork(SimpleThread, num);
    }

    printf("End of thread test.\n");
}
