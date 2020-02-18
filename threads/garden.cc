#include <stdio.h>
#include "synch.hh"
#include "system.hh"

int visitors, total;
static int b;
static Lock *l;

void Turnstile(void *arg) {
    for (int i = 0; i < total; i++) {
        if (b) l->Acquire();
            currentThread->Yield();

            // Simulates low level operations.
            int temp = visitors;
            currentThread->Yield();
            temp++;
            currentThread->Yield();
            visitors = temp;

            currentThread->Yield();
        if (b) l->Release();
    }

    printf("%s finished. %d visitors in total.\n", currentThread->GetName(), visitors);
}

void Garden() {
    printf("0 - Garden without lock.\n");
    printf("1 - Garden with lock.\n");
    printf("Enter a number: ");
    scanf("%d", &b);

    int turnstiles;
    printf("How many turnstiles?: ");
    scanf("%d", &turnstiles);

    printf("Hoy many visitors on each turnstile?: ");
    scanf("%d", &total);

    l = new Lock("Garden");

    for (int i = 0; i < turnstiles; i++) {
        char* name = new char[15];
        sprintf(name, "%s %d", "Turnstile", i);
        Thread* newThread = new Thread(name);
        newThread->Fork(Turnstile, nullptr);
    }
}
