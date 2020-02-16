/// Simple program to test whether running a user program works.
///
/// Just do a “syscall” that shuts down the OS.
///
/// NOTE: for some reason, user programs with global data structures
/// sometimes have not worked in the Nachos environment.  So be careful out
/// there!  One option is to allocate data structures as automatics within a
/// procedure, but if you do this, you have to be careful to allocate a big
/// enough stack to hold the automatics!

#include "syscall.h"

int
main(void) {
    Write("Creating file test.txt.\n", 24, CONSOLE_OUTPUT);
    Create("test.txt");

    Write("Opening file test.txt.\n", 23, CONSOLE_OUTPUT);
    OpenFileId o = Open("test.txt");

    Write("Writing `Hello world` to file test.txt.\n", 40, CONSOLE_OUTPUT);
    Write("Hello world.\n", 13, o);

    Write("Closing file test.txt.\n", 23, CONSOLE_OUTPUT);
    Close(o);
    // Not reached.
}
