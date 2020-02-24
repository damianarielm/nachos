#include "syscall.h"
#include "stdio.h"

int main(int argc, char** argv) {
    if (argc != 2)
        print("Syntax error.\n");
    else if (!Cd(argv[1])) {
        print("Can't change to directory ");
        print(argv[1]);
        print(".\n");
    }

    Exit(0);
}
