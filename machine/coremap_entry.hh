#ifndef NACHOS_MACHINE_COREMAP__HH
#define NACHOS_MACHINE_COREMAP__HH

#include "lib/utility.hh"
class Thread;

class CoreMapEntry {
public:

    /// The virtual page of the process.
    unsigned virtualPage;

    /// The thread to which this page belongs.
    Thread* thread;

};

#endif
