#include "synch.hh"
#include "system.hh"

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Lock::Lock(const char *debugName) {
    name = debugName;
    owner = nullptr;
    semaphore = new Semaphore(name, 1);
}

Lock::~Lock() {
    delete semaphore;
}

const char *
Lock::GetName() const {
    return name;
}

void
Lock::Acquire() {
    DEBUG('l', "Thread %s doing " BOLD RED "Acquire ", currentThread->GetName());
    DEBUG_CONT('l', "on lock %s taken by %s.\n", name, owner ? owner->GetName() : "nobody");

    semaphore->P();
    owner = currentThread;
}

void
Lock::Release() {
    DEBUG('l', "Thread %s doing " BOLD GREEN "Release ", currentThread->GetName());
    DEBUG_CONT('l', "on lock %s taken by %s.\n", name, owner ? owner->GetName() : "nobody");
    ASSERT(IsHeldByCurrentThread());

    owner = nullptr;
    semaphore->V();
}

bool
Lock::IsHeldByCurrentThread() const {
    return owner == currentThread;
}
