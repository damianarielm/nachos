/// Simple program to test whether running a user program works.
///
/// Just do a “syscall” that shuts down the OS.
///
/// NOTE: for some reason, user programs with global data structures
/// sometimes have not worked in the Nachos environment.  So be careful out
/// there!  One option is to allocate data structures as automatics within a
/// procedure, but if you do this, you have to be careful to allocate a big
/// enough stack to hold the automatics!

#include "stdio.h"

int
main(void) {
    print("Creating file test.txt.\n");
    Create("test.txt");

    print("Opening file test.txt.\n");
    OpenFileId o = Open("test.txt");

    print("Writing `Hello world` to file test.txt.\n");
    Write("Hello world.\n", 13, o);

    print("Closing file test.txt.\n");
    Close(o);
    // Not reached.
}
