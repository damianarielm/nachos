/// Miscellaneous useful definitions, including debugging utilities.
///
/// See also `lib/debug.hh`.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_LIB_UTILITY__HH
#define NACHOS_LIB_UTILITY__HH

#include "colors.hh"
#include "debug.hh"

/// Miscellaneous useful routines.

//#define min(a, b)  (((a) < (b)) ? (a) : (b))
//#define max(a, b)  (((a) > (b)) ? (a) : (b))

/// Typedef for host memory references, expressed in numerical (integer)
/// form.

#ifdef HOST_x86_64
typedef unsigned long HostMemoryAddress;
#else
typedef unsigned int HostMemoryAddress;
#endif

/// Divide and either round up or down.

template <typename T>
inline T
DivRoundDown(T n, T s) {
    return n / s;
}

template <typename T>
inline T
DivRoundUp(T n, T s) {
    return n / s + (n % s > 0 ? 1 : 0);
}

/// This declares the type `VoidFunctionPtr` to be a “pointer to a
/// function taking a pointer argument and returning nothing”.
///
/// With such a function pointer (say it is "func"), we can call it like
/// this:
///    func (arg);
/// or:
///    (*func) (arg);
///
/// This is used by `Thread::Fork` and for interrupt handlers, as well as a
/// couple of other places.
typedef void (*VoidFunctionPtr)(void *arg);

typedef void (*VoidNoArgFunctionPtr)();

// Include interface that isolates us from the host machine system library.
// Requires definition of `VoidFunctionPtr`.
#include "machine/.system_dep.hh"

/// Global object for debug output.
extern Debug debug;
extern void PrintByte(char byte);

#define DEBUG_ERROR(...)      (debug.Print)(true, __LINE__, __FILE__, __VA_ARGS__)
#define DEBUG_CONT_ERROR(...) (debug.PrintCont)(true, __VA_ARGS__)
#define DEBUG(...)            (debug.Print)(false, __LINE__, __FILE__, __VA_ARGS__)
#define DEBUG_CONT(...)       (debug.PrintCont)(false, __VA_ARGS__)

#define ERROR(string) UNDERLINE string DISABLE_UNDERLINE

/// If `condition` is false, print a message and dump core.
///
/// Useful for documenting assumptions in the code.
///
/// NOTE: needs to be a `#define`, to be able to print the location where the
/// error occurred.
#define ASSERT(condition)                                                \
    if (!(condition)) {                                                  \
        fprintf(stderr, ERROR("\nAssertion failed!\n")                   \
                        "Expression: " BOLD "%s" DISABLE_BOLD".\n"       \
                        "Location: file " BOLD "%s" DISABLE_BOLD" line " \
                        BOLD "%u" DISABLE_BOLD".\n\n",                   \
                        #condition, __FILE__, __LINE__);                 \
        fflush(stderr);                                                  \
        abort();                                                         \
    }

#endif
