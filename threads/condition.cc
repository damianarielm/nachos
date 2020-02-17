#include "synch.hh"

Condition::Condition(const char *debugName, Lock *conditionLock)
{}

Condition::~Condition()
{}

const char *
Condition::GetName() const {
    return name;
}

void
Condition::Wait()
{}

void
Condition::Signal()
{}

void
Condition::Broadcast()
{}
