#include "stdio.h"
#include "string.h"

#define MAX_LINE_SIZE  60
#define MAX_ARG_COUNT  32
#define ARG_SEPARATOR  ' '

#define NULL  ((void *) 0)

static int
PrepareArguments(char *line, char **argv, unsigned argvSize) {
    // TO DO: how to make sure that `line` and `argv` are not `NULL`?, and
    //        for `argvSize`, what precondition should be fulfilled?
    //
    // PENDIENTE: use `bool` instead of `int` as return type; for doing this,
    //            given that we are in C and not C++, it is convenient to
    //            include `stdbool.h`.

    unsigned argCount;

    argv[0] = line;
    argCount = 1;

    // Traverse the whole line and replace spaces between arguments by null
    // characters, so as to be able to treat each argument as a standalone
    // string.
    //
    // TO DO: what happens if there are two consecutive spaces?, and what
    //        about spaces at the beginning of the line?, and at the end?
    //
    // TO DO: what if the user wants to include a space as part of an
    //        argument?
    for (unsigned i = 0; line[i] != '\0'; i++)
        if (line[i] == ARG_SEPARATOR) {
            if (argCount == argvSize - 1)
                // The maximum of allowed arguments is exceeded, and
                // therefore the size of `argv` is too.  Note that 1 is
                // decreased in order to leave space for the NULL at the end.
                return 0;
            line[i] = '\0';
            argv[argCount] = &line[i + 1];
            argCount++;
        }

    argv[argCount] = NULL;
    return 1;
}

int
main(void) {
    char             line[MAX_LINE_SIZE];
    char            *argv[MAX_ARG_COUNT];

    for (;;) {
        print("> ");
        scan(line);
        const unsigned lineSize = strlen(line);
        if (!lineSize) Halt();

        if (PrepareArguments(line, argv, MAX_ARG_COUNT) == 0) {
            print("Error: too many arguments.\n");
            continue;
        }

        // Comment and uncomment according to whether command line arguments
        // are given in the system call or not.
        //const SpaceId newProc = Exec(line);
        const SpaceId newProc = Exec(line, argv, 1);

        // TO DO: check for errors when calling `Exec`; this depends on how
        //        errors are reported.

        Join(newProc);
        // TO DO: is it necessary to check for errors after `Join` too, or
        //        can you be sure that, with the implementation of the system
        //        call handler you made, it will never give an error?; what
        //        happens if tomorrow the implementation changes and new
        //        error conditions appear?
    }

    return 0;  // Never reached.
}
