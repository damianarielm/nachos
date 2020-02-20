/// Data structures to export a synchronous interface to the raw disk device.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "synch_console.hh"

/// Dummy functions because C++ is weird about pointers to member functions.

static void
ReadAvailDummy(void *args) {
    ASSERT(args);

    ((SynchConsole*) args)->ReadAvail();
}

static void
WriteDoneDummy(void *args) {
    ASSERT(args);

    ((SynchConsole*) args)->WriteDone();
}

SynchConsole::SynchConsole() {
    console = new Console(nullptr, nullptr, ReadAvailDummy, WriteDoneDummy, this);
    reading = new Lock("Console reading");
    writing = new Lock("Console writing");
    readAvail = new Semaphore("Console readers", 0);
    writeDone = new Semaphore("Console waiters", 0);
}

SynchConsole::~SynchConsole() {
    delete console;
    delete reading;
    delete writing;
    delete readAvail;
    delete writeDone;
}

void
SynchConsole::PutChar(char ch) {
    writing->Acquire();
        console->PutChar(ch);
        writeDone->P();
    writing->Release();
}

char
SynchConsole::GetChar() {
    reading->Acquire();
        readAvail->P();
        char ch = console->GetChar();
    reading->Release();

    return ch;
}

void
SynchConsole::ReadAvail() {
    readAvail->V();
}

void
SynchConsole::WriteDone() {
    writeDone->V();
}
