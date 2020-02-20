#include <unistd.h>

#include "system.hh"
#include "synch.hh"

unsigned nports;
static unsigned time;
Port** ports;

void
PortThread(void *name_) {
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    while (1) {
        usleep(time);
        int port = Random() % nports;
        int message = Random() % 999;

        if (Random() % 2) {
            printf("Thread `%s` trying to send %d trough port `%s`...\n" RESET,
                    name, message, ports[port]->GetName());

            ports[port]->Send(message);

            printf("Thread `%s` successfully sended %d.\n" RESET,
                    name, message);
        } else {
            printf("Thread `%s` triying to receive trough port `%s`...\n" RESET,
                    name, ports[port]->GetName());

            ports[port]->Receive(&message);

            printf("Thread `%s` successfully received %d.\n" RESET, name, message);
        }
    }
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.
void
PortTest() {
    unsigned threads;

    printf("How many threads: ");
    scanf("%d", &threads);

    printf("How many ports: ");
    scanf("%d", &nports);
    ports = (Port**) malloc(sizeof(Port*) * nports);

    printf("Time delay (ms): ");
    scanf("%d", &time);
    time *= 1000;

    for (unsigned i = 0; i < nports; i++) {
        char* name = new char[4];
        sprintf(name, "%u", i);
        ports[i] = new Port(name);
    }

    for (unsigned i = 1; i <= threads; i++) {
        char *name = new char [4];
        sprintf(name, "%u", i);
        Thread *newThread = new Thread(name, false);
        newThread->Fork(PortThread, (void *) name);
    }
}
