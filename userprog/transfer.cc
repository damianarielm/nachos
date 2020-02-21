#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"

bool ReadStringFromUser(int userAddress, char *outString, unsigned maxByteCount) {
    ASSERT(userAddress != 0);
    ASSERT(outString);
    ASSERT(maxByteCount);

    unsigned count = 0;
    do {
        int temp;
        count++;
        if (!machine->ReadMem(userAddress++, 1, &temp))
#ifdef USE_TLB
            ASSERT(machine->ReadMem(userAddress - 1, 1, &temp));
#else
            ASSERT(false);
#endif
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}

void ReadBufferFromUser(int userAddress, char *outBuffer, unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(outBuffer);

    for (; byteCount--; outBuffer++)
         if (!machine->ReadMem(userAddress++, 1, (int*) outBuffer))
#ifdef USE_TLB
            ASSERT(machine->ReadMem(userAddress - 1, 1, (int*) outBuffer));
#else
            ASSERT(false);
#endif
}

void WriteBufferToUser(int userAddress, const char *buffer, unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(buffer);

    for (; byteCount--; buffer++)
        if (!machine->WriteMem(userAddress++, 1, *buffer))
#ifdef USE_TLB
            ASSERT(machine->WriteMem(userAddress - 1, 1, *buffer));
#else
            ASSERT(false)
#endif
}

void WriteStringToUser(const char *string, int userAddress) {
    ASSERT(string);

    for (; *string; string++)
        if (!machine->WriteMem(userAddress++, 1, *string))
#ifdef USE_TLB
            ASSERT(machine->WriteMem(userAddress - 1, 1, *string));
#else
            ASSERT(false);
#endif
}
