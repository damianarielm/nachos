#include "synch.hh"

// This class defined a “condition variable”.
//
// A condition variable does not have any value.  It is used for enqueuing
// threads that are waiting (`Wait`) that another thread informs them of
// something (`Signal`).  Condition variables are bound to a lock (`Lock`).
//
// These are the three operations on condition variables:
//
// * `Wait` -- free the lock and displace the thread from the CPU.  The
//   thread will wait until someone sends it a `Signal`.
// * `Signal` -- if there is someone waiting on the variable, awaken one of
//   the threads. If there is none waiting, nothing occurs.
// * `Broadcast` -- awaken all waiting threads.
//
// All operations on a condition variable must be performed after having
// acquired the lock.  This means that operations on condition variables must
// be executed in mutual exclusion.
//
// Nachos' condition variables should work according to the “Mesa” style.
// When a `Signal` or `Broadcast` awakens another thread, this is put in the
// ready queue.  The woken thread is responsible for acquiring the lock
// again.  This has to be implemented in the body of the `Wait` function.
//
// In contrast, there exists another style of condition variables, the
// “Hoare” style: according to it, `Signal` loses control of the lock and
// delivers the CPU to the woken thread; this is run immediately and when the
// lock is freed, the thread returns control to the thread that performed the
// `Signal`.
//
// The “Mesa” style is somewhat simpler to implement, but it does not
// guarantee that the woken thread recover the control of the lock
// immediately.
class Condition {
public:

    // Constructor: indicate which lock the condition variable belongs to.
    Condition(const char *debugName, Lock *conditionLock);

    ~Condition();

    const char *GetName() const;

    /// The three operations on condition variables.
    ///
    /// The thread that invokes any of these operations must hold the
    /// corresponding lock; otherwise an error must occur.

    void Wait();
    void Signal();
    void Broadcast();

private:

    const char *name;

    // Other needed fields are to be added here.
    Lock* lock;

    List<Semaphore*> *queue;
};
