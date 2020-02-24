#include "syscall.h"
#include "stdio.h"

void main(int argc, char** argv) {
    int src, dst; char c[1];
    if (argc != 3)
        print("Syntax error.\n");
    else if ((src = Open(argv[1])) == -1) {
        print("Can't open the file ");
        print(argv[1]);
        print(".\n");
    } else {
        Create(argv[2], 0);
        dst = Open(argv[2]);
        while (Read(c, 1, src)) Write(c, 1, dst);
        Close(src); Close(dst);
    }

    Exit(0);
}
