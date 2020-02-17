/// Copyright (c) 2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include <ctype.h>
#include "utility.hh"

Debug debug;

void
PrintByte(char byte) {
    if (isprint(byte)) printf(BRIGHT_WHITE "%c" RESET, byte);
    else if (byte == '\t') printf(YELLOW "»" RESET);
    else if (byte == '\b') printf(YELLOW "«" RESET);
    else if (byte == '\n') printf(YELLOW "¶" RESET);
    else if (byte == '\0') printf(YELLOW "•" RESET);
    else printf("%x", byte);
}
