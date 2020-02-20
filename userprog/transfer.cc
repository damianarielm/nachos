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
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}

void ReadBufferFromUser(int userAddress, char *outBuffer, unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(outBuffer);

    for (; byteCount--; outBuffer++)
        ASSERT(machine->ReadMem(userAddress++, 1, (int*) outBuffer));
}

void WriteBufferToUser(int userAddress, const char *buffer, unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(buffer);

    for (; byteCount--; buffer++)
        ASSERT(machine->WriteMem(userAddress++, 1, *buffer));
}

void WriteStringToUser(const char *string, int userAddress) {
    ASSERT(string);

    for (; *string; string++)
        ASSERT(machine->WriteMem(userAddress++, 1, *string));
}
