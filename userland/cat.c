#include "syscall.h"
#include "stdio.h"

int main(int argc, char** argv) {
    int fd; char c[1];

    if (argc != 2)
        print("Syntax error.\n");
    else if ((fd = Open(argv[1])) == -1) {
        print("Can't open the file ");
        print(argv[1]);
        print(".\n");
    } else
        while (Read(c, 1, fd)) Write(c, 1, CONSOLE_OUTPUT);

    Exit(0);
}
