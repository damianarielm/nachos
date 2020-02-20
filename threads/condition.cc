#include "synch.hh"
#include "system.hh"

Condition::Condition(const char *debugName, Lock *conditionLock) {
    name = debugName;
    lock = conditionLock;
    queue = new List<Semaphore*>;
}

Condition::~Condition() {
    delete queue;
}

const char *
Condition::GetName() const {
    return name;
}

void
Condition::Wait() {
    DEBUG('c', "Thread %s doing " BOLD RED "Wait", currentThread->GetName());
    DEBUG_CONT('c', " on condition variable %s with %u other threads.\n",
            name, queue->Length());

    Semaphore* semaphore = new Semaphore(name, 0);
    queue->Append(semaphore);
    lock->Release();
        semaphore->P();
        delete semaphore;
    lock->Acquire();
}

void
Condition::Signal() {
    DEBUG('c', "Thread %s doing " BOLD GREEN "Signal", currentThread->GetName());
    DEBUG_CONT('c', " on condition variable %s with %u other threads.\n",
            name, queue->Length());
    ASSERT(lock->IsHeldByCurrentThread());

    if (!queue->IsEmpty()) queue->Pop()->V();
}

void
Condition::Broadcast() {
    DEBUG('c', "Thread %s doing " BOLD UNDERLINE GREEN "Broadcast", currentThread->GetName());
    DEBUG_CONT('c', " on condition variable %s with %u other threads.\n",
            name, queue->Length());
    ASSERT(lock->IsHeldByCurrentThread());

    while (!queue->IsEmpty()) queue->Pop()->V();
}
