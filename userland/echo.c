#include "stdio.h"

int
main(int argc, char* argv[]) {
    for (unsigned i = 1; i < argc; i++) {
        print(argv[i]);
        print(" ");
    }
    print("\n");
}
