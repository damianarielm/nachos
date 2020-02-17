/// Routines for converting Words and Short Words to and from the simulated
/// machine's format of little endian.  These end up being NOPs when the host
/// machine is also little endian (DEC and Intel).

#include ".endianness.hh"
#include "lib/utility.hh"

unsigned
WordToHost(unsigned word) {
#ifdef HOST_IS_BIG_ENDIAN
     unsigned long result;
     result  = word >> 24 & 0x000000FF;
     result |= word >>  8 & 0x0000FF00;
     result |= word <<  8 & 0x00FF0000;
     result |= word << 24 & 0xFF000000;
     return result;
#else
     return word;
#endif
}

unsigned short
ShortToHost(unsigned short shortword) {
#ifdef HOST_IS_BIG_ENDIAN
     unsigned short result;
     result  = shortword << 8 & 0xFF00;
     result |= shortword >> 8 & 0x00FF;
     return result;
#else
     return shortword;
#endif
}

unsigned
WordToMachine(unsigned word) {
    return WordToHost(word);
}

unsigned short
ShortToMachine(unsigned short shortword) {
    return ShortToHost(shortword);
}

/// Do little endian to big endian conversion on the bytes in the object file
/// header, in case the file was generated on a little endian machine, and we
/// are re now running on a big endian machine.
void
SwapHeader(noffHeader *noffH)
{
    ASSERT(noffH != nullptr);

    noffH->noffMagic              = WordToHost(noffH->noffMagic);
    noffH->code.size              = WordToHost(noffH->code.size);
    noffH->code.virtualAddr       = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr        = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size          = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr   = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr    = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size        = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr  = WordToHost(noffH->uninitData.inFileAddr);
}
