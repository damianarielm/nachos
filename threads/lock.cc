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

#ifdef INVERSION
    if (owner && owner->GetPriority() < currentThread->GetPriority()) {
        DEBUG('L', "Promoting thread %s from priority %u to priority %u.\n",
                owner->GetName(), owner->GetPriority(), currentThread->GetPriority());

        owner->SetPriority(currentThread->GetPriority());
        scheduler->UpdatePriority(owner);
    }
#endif


    semaphore->P();
    owner = currentThread;
}

void
Lock::Release() {
    DEBUG('l', "Thread %s doing " BOLD GREEN "Release ", currentThread->GetName());
    DEBUG_CONT('l', "on lock %s taken by %s.\n", name, owner ? owner->GetName() : "nobody");
    ASSERT(IsHeldByCurrentThread());

#ifdef INVERSION
    if (currentThread->GetOldPriority() != currentThread->GetPriority()) {
        DEBUG('L', "Degrading thread %s from priority %u to priority %u.\n",
                owner->GetName(), currentThread->GetPriority(), owner->GetOldPriority());

        currentThread->SetPriority(currentThread->GetOldPriority());
        scheduler->UpdatePriority(currentThread);
    }
#endif

    owner = nullptr;
    semaphore->V();
}

bool
Lock::IsHeldByCurrentThread() const {
    return owner == currentThread;
}
