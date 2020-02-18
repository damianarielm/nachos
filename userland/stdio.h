#include "syscall.h"

void
print(char* string) {
    unsigned len = 0;
    while(string[len]) len++;
    Write(string, len, CONSOLE_OUTPUT);
}

#define scan(buffer) { unsigned i = 0;                        \
                       buffer[0] = 0;                         \
                       do Read(&buffer[i], 1, CONSOLE_INPUT); \
                       while (buffer[i++] != '\n');           \
                       buffer[i - 1] = 0; }
