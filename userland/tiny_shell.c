#include "stdio.h"

int main(void) {
    char buffer[60];

    while (1) {
        print("> ");
        scan(buffer);

        if (buffer[0]) {
            SpaceId newProc = Exec(buffer);
            Join(newProc);
        }
    }
}
