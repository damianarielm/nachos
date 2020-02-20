#include <unistd.h>

#include "lib/colors.hh"
#include "synch.hh"
#include "system.hh"

int* buffer;
int in, out, produced, buffersize, time;
static int b;
static Lock *l;

Condition *full;
Condition *empty;

void PrintBuffer(int pos, char const* color) {
    printf("( ");
    for (int i = 0; i < buffersize; i++) {
        if (i == pos) printf(color);
        printf("%d ", buffer[i]);
        printf(RESET);
    }
    printf(")\n");
}

void productor(void* arg) {
    while(1) {
        currentThread->Yield();
        l->Acquire();
            currentThread->Yield();
            if (b) while(produced >= buffersize) full->Wait();
            else while(produced >= buffersize) currentThread->Yield();
            currentThread->Yield();
            buffer[in] = 1;
            currentThread->Yield();
            printf(GREEN "[%s] Item produced:\t" RESET, currentThread->GetName()+9);
            PrintBuffer(in, GREEN);
            usleep(time);
            currentThread->Yield();
            in = (in + 1) % buffersize;
            currentThread->Yield();
            produced++;
            currentThread->Yield();
            if (b) empty->Signal();
            currentThread->Yield();
        l->Release();
        currentThread->Yield();
    }
}

void consumidor(void* arg) {
    while(1) {
        currentThread->Yield();
        l->Acquire();
            currentThread->Yield();
            if (b) while(produced <= 0) empty->Wait();
            else while(produced <=0) currentThread->Yield();
            currentThread->Yield();
            buffer[out] = 0;
            currentThread->Yield();
            printf(RED "[%s] Item consumed:\t" RESET, currentThread->GetName()+9);
            PrintBuffer(out, RED);
            usleep(time);
            currentThread->Yield();
            out = (out + 1) % buffersize;
            currentThread->Yield();
            produced--;
            currentThread->Yield();
            if (b) full->Signal();
            currentThread->Yield();
        l->Release();
        currentThread->Yield();
    }
}

void ProdCons() {
    in = out = produced = 0;

    printf("0 - Producer/consumer without condition variables.\n");
    printf("1 - Producer/consumer with condition variables.\n");
    printf("Enter a number: ");
    scanf("%d", &b);

    printf("Size of the buffer: ");
    scanf("%d", &buffersize);
    buffer = new int[buffersize];

    int ps, cs;
    printf("How many producers: ");
    scanf("%d", &ps);
    printf("How many consumers: ");
    scanf("%d", &cs);

    printf("Time delay (ms): ");
    scanf("%d", &time);
    time *= 1000;

    l = new Lock("ProdCons");
    full = new Condition("Full", l);
    empty = new Condition("Empty", l);

    for (int i = 1; i <= ps; i++) {
        char *name = new char[10];
        sprintf(name, "%s %d", "Producer", i);
        Thread *t = new Thread(name, false);
        t->Fork(productor, nullptr);
    }

    for (int i = 1; i <= cs; i++) {
        char *name = new char[10];
        sprintf(name, "%s %d", "Consumer", i);
        Thread *t = new Thread(name, false);
        t->Fork(consumidor, nullptr);
    }
}
