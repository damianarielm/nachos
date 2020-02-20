/// Data structures to export a synchronous interface to the console.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_SYNCHCONSOLE__HH
#define NACHOS_FILESYS_SYNCHCONSOLE__HH

#include "machine/console.hh"
#include "threads/synch.hh"

class SynchConsole {
public:

    /// Initialize a synchronous console, by initializing the console.
    SynchConsole();

    /// De-allocate the synch console data.
    ~SynchConsole();

    /// External interface -- Nachos kernel code can call these.

    /// Write `ch` to the console display, and return immediately.
    void PutChar(char ch);

    /// Poll the console input. If a char is available, return int.
    /// Otherwise, return EOF.
    char GetChar();

    // Internal emulation routines -- DO NOT call these.
    // Internal routines to signal console completion.
    void WriteDone();
    void ReadAvail();

private:
    Console *console;  ///< Console device.

    Lock *reading;  ///< Only one read request can be sent to the console at
                    ///< a time.

    Lock *writing;  ///< Only one write request can be sent to the console at
                    ///< a time.

    Semaphore *readAvail;

    Semaphore *writeDone;
};

#endif
