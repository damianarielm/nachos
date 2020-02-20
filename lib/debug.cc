/// Debugging routines.
///
/// Allows users to control whether to print debug statements, based on a
/// command line argument.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "debug.hh"
#include "utility.hh"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

Debug::Debug() {
    flags = "";
}

bool
Debug::IsEnabled(char flag) const {
    if (flags)
        return strchr(flags, flag) != 0 || strchr(flags, '+') != 0;
    else
        return false;
}

const char *
Debug::GetFlags() const {
    return flags;
}

void
Debug::SetFlags(const char *new_flags) {
    flags = new_flags;
}

void Bold(const char *org, char* dst) {
    for (unsigned i = 0; i < strlen(org); i++) {
        if (org[i] != '%') {
            dst[strlen(dst)] = org[i];
        } else {
            dst = strcat(dst, BOLD "%");
            dst[strlen(dst)] = org[++i];
            dst = strcat(dst, DISABLE_BOLD);
        }
    }
}

void
Debug::Print(bool error, unsigned line, const char* file, char flag,
        const char *format, ...) const {
    ASSERT(format);

    if (!IsEnabled(flag)) return;
    if (error) fprintf(stderr, UNDERLINE);
    Color(flag);

    char* buffer = new char[256]();
    Bold(format, buffer);
    format = buffer;

    if (!IsEnabled('+') && IsEnabled('e')) usleep(100000);
    if (!IsEnabled('+') && IsEnabled('E')) getchar();
    if (IsEnabled('n')) fprintf(stderr, "\n%s:%u\n", file, line);
    fprintf(stderr, "[%c] ", flag);

    va_list ap;
    // You will get an unused variable message here -- ignore it.
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, RESET);
    fflush(stderr);
    delete [] buffer;
}

void
Debug::PrintCont(bool error, char flag, const char *format, ...) const {
    ASSERT(format);

    if (!IsEnabled(flag)) return;
    if (error) fprintf(stderr, UNDERLINE);
    Color(flag);

    char* buffer = new char[256]();
    Bold(format, buffer);
    format = buffer;

    va_list ap;
    // You will get an unused variable message here -- ignore it.
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, RESET);
    fflush(stderr);
    delete [] buffer;
}

void
Debug::Color(char flag) const {
    fprintf(stderr, YELLOW);
    if (flag == 't' || flag == 'T') fprintf(stderr, CYAN);
    if (flag == 's' || flag == 'S') fprintf(stderr, YELLOW);
    if (flag == 'i' || flag == 'I') fprintf(stderr, BRIGHT_YELLOW);
    if (flag == 'd' || flag == 'D') fprintf(stderr, GREEN);
    if (flag == 'f' || flag == 'F') fprintf(stderr, BRIGHT_BLUE);
    if (flag == 'a' || flag == 'A') fprintf(stderr, MAGENTA);

    if (flag == 'l' || flag == 'L') fprintf(stderr, YELLOW);
    if (flag == 'c' || flag == 'C') fprintf(stderr, YELLOW);
}
