#include "thread.hh"
#include "lib/list.hh"

/// This class defines a “semaphore”, which has a positive integer as its
/// value.
///
/// Semaphores offer only two operations:
///
/// * `P` -- wait until `value > 0`, then decrement `value`.
/// * `V` -- increment `value`, awaken a waiting thread if any.
///
/// Observe that this interface does *not* allow to read the semaphore value
/// directly -- even if you were able to read it, it would serve for nothing,
/// because meanwhile another thread could have modified the semaphore, in
/// case you have lost the CPU for some time.
class Semaphore {
public:

    /// Constructor: give an initial value to the semaphore.
    ///
    /// Set initial value.
    Semaphore(const char *debugName, int initialValue);

    ~Semaphore();

    /// For debugging.
    const char *GetName() const;

    /// The only public operations on the semaphore.
    ///
    /// Both of them must be *atomic*.
    void P();
    void V();

private:

    /// For debugging.
    const char *name;

    /// Semaphore value, it is always `>= 0`.
    int value;

    /// Queue of threads waiting on `P` because the value is zero.
    List<Thread *> *queue;

};
