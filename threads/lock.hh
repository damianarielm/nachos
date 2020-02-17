#include "synch.hh"

/// This class defines a “lock”.
///
/// A lock can have two states: free and busy. Only two operations are
/// allowed on locks:
///
/// * `Acquire` -- wait until the lock is free and mark is as busy.
/// * `Release` -- mark the lock as free, thereby awakening some other thread
///   that were blocked on an `Acquired`.
///
/// For convenience, nobody but the thread that holds the lock can free it.
/// There is no operation for reading the state of the lock.
class Lock {
public:

    /// Constructor: set up the lock as free.
    Lock(const char *debugName);

    ~Lock();

    /// For debugging.
    const char *GetName() const;

    /// Operations on the lock.
    ///
    /// Both must be *atomic*.
    void Acquire();
    void Release();

    /// Returns `true` if the current thread is the one that possesses the
    /// lock.
    ///
    /// Useful for checks in `Release` and in condition variables.
    bool IsHeldByCurrentThread() const;

private:

    /// For debugging.
    const char *name;

    // Add other needed fields here.
};
