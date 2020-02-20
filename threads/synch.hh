/// Synchronization primitives
///
/// Data structures for synchronizing threads.
///
/// Three synchronization mechanisms are defined here: semaphores, locks and
/// condition variables. Only semaphores are implemented; for locks and
/// condition variables, only the interface is provided. Precisely, the first
/// task involves developing this implementation.
///
/// All synchronization objects have a `name` parameter in the constructor;
/// its only aim is to ease debugging the program.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2000      José Miguel Santos Espino - ULPGC.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_SYNCH__HH
#define NACHOS_THREADS_SYNCH__HH

#include "semaphore.hh"
#include "lock.hh"
#include "condition.hh"
#include "port.hh"

#endif
