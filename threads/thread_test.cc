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

#ifdef SEMAPHORE_TEST
#include "synch.hh"
Semaphore* semaphore = new Semaphore("Ejercicio 15", 3);
#endif

/// Loops yielding the CPU to another ready thread each iteration.
void
SimpleThread(void* args) {
#ifdef SEMAPHORE_TEST
    semaphore->P();
#endif

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < * (unsigned*) args; num++) {
        printf("*** Thread %s is running: iteration %u.\n",
                currentThread->GetName(), num);
        currentThread->Yield();
    }
    printf("!!! Thread %s has finished.\n", currentThread->GetName());

#ifdef SEMAPHORE_TEST
    semaphore->V();
#endif
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`.
void
ThreadTest() {
    char* name;
    unsigned* num = new unsigned;
    unsigned threads = 5;
    static unsigned j, b;

    printf("How many iterations: ");
    scanf("%u", num);

    printf("How may threads: ");
    scanf("%u", &threads);

    printf("0 - Not joinable threads.\n");
    printf("1 - Joinable threads.\n");
    printf("Enter a number: ");
    scanf("%d", &b);

    printf("0 - Do not join threads.\n");
    printf("1 - Join threads.\n");
    printf("Enter a number: ");
    scanf("%d", &j);

    printf("Starting thread test.\n");

    Thread* ts[threads];
    for (unsigned i = 0; i < threads; i++) {
        name = new char [4];
        sprintf(name, "%uº", i + 1);
        ts[i] = new Thread(name, b);
        printf("Starting thread %s.\n", name);
        ts[i]->Fork(SimpleThread, num);
    }

    if (j) for (unsigned i = 0; i < threads; i++) ts[i]->Join();

    printf("End of thread test.\n");
}
