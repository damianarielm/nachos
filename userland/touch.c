#include "stdio.h"

void
main(int argc, char** argv) {
    if (argc < 2) print("Syntax error.\n");
    else Create(argv[1], 0);

    Exit(0);
}
