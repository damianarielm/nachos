#include "stdio.h"

void
main(int argc, char** argv) {
    if (argc < 2) print("Syntax error.\n");
    else Remove(argv[1]);

    Exit(0);
}
