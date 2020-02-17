#include "synch.hh"

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Lock::Lock(const char *debugName)
{}

Lock::~Lock()
{}

const char *
Lock::GetName() const {
    return name;
}

void
Lock::Acquire()
{}

void
Lock::Release()
{}

bool
Lock::IsHeldByCurrentThread() const
{
    return false; // TODO: complete...
}
