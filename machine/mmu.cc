/// Routines to translate virtual addresses to physical addresses.
///
/// Software sets up a table of legal translations.  We look up in the table
/// on every memory reference to find the true physical memory location.
///
/// Two types of translation are supported here.
///
/// Linear page table -- the virtual page # is used as an index into the
/// table, to find the physical page #.
///
/// Translation lookaside buffer -- associative lookup in the table to find
/// an entry with the same virtual page #.  If found, this entry is used for
/// the translation.  If not, it traps to software with an exception.
///
/// In practice, the TLB is much smaller than the amount of physical memory
/// (16 entries is common on a machine that has 1000's of pages).  Thus,
/// there must also be a backup translation scheme (such as page tables), but
/// the hardware does not need to know anything at all about that.
///
/// Note that the contents of the TLB are specific to an address space.
/// If the address space changes, so does the contents of the TLB!
///
/// DO NOT CHANGE -- part of the machine emulation
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "mmu.hh"
#include ".endianness.hh"
#include "threads/system.hh"

MMU::MMU() {
    mainMemory = new char [MEMORY_SIZE];
    for (unsigned i = 0; i < MEMORY_SIZE; i++) mainMemory[i] = 0;

#ifdef USE_TLB
    tlb = new TranslationEntry[TLB_SIZE];
    for (unsigned i = 0; i < TLB_SIZE; i++) tlb[i].valid = false;
    pageTable = nullptr;
#else  // Use linear page table.
    tlb = nullptr;
    pageTable = nullptr;
#endif
}

MMU::~MMU() {
    delete [] mainMemory;
    if (tlb) delete [] tlb;
}

/// Read `size` (1, 2, or 4) bytes of virtual memory at `addr` into
/// the location pointed to by `value`.
///
/// Returns false if the translation step from virtual to physical memory
/// failed.
///
/// * `addr` is the virtual address to read from.
/// * `size` is the number of bytes to read (1, 2, or 4).
/// * `value` is the place to write the result.
ExceptionType
MMU::ReadMem(unsigned addr, unsigned size, int *value) {
    DEBUG('A', "Reading VA 0x%X, size %u.\n", addr, size);
    ASSERT(value);

    unsigned physicalAddress;
    ExceptionType e = Translate(addr, &physicalAddress, size, false);
    if (e != NO_EXCEPTION) return e;

    int data;
    switch (size) {
        case 1:
            data = mainMemory[physicalAddress];
            *value = data;
            break;

        case 2:
            data = *(unsigned short *) &mainMemory[physicalAddress];
            *value = ShortToHost(data);
            break;

        case 4:
            data = *(unsigned *) &mainMemory[physicalAddress];
            *value = WordToHost(data);
            break;

        default:
            ASSERT(false);
    }

    DEBUG('A', "\tValue read: %X.\n", *value);
    return NO_EXCEPTION;
}

/// Write `size` (1, 2, or 4) bytes of the contents of `value` into virtual
/// memory at location `addr`.
///
/// Returns false if the translation step from virtual to physical memory
/// failed.
///
/// * `addr` is the virtual address to write to.
/// * `size` is the number of bytes to be written (1, 2, or 4).
/// * `value` is the data to be written.
ExceptionType
MMU::WriteMem(unsigned addr, unsigned size, int value) {
    DEBUG('A', "Writing VA 0x%X, size %u, value %d.\n", addr, size, value);

    unsigned physicalAddress;
    ExceptionType e = Translate(addr, &physicalAddress, size, true);
    if (e != NO_EXCEPTION) return e;

    switch (size) {
      case 1:
          mainMemory[physicalAddress] = (unsigned char) (value & 0xFF);
          break;

      case 2:
          *(unsigned short *) &mainMemory[physicalAddress]
            = ShortToMachine((unsigned short) (value & 0xFFFF));
          break;

      case 4:
          *(unsigned *) &mainMemory[physicalAddress] = WordToMachine((unsigned) value);
          break;

      default:
          ASSERT(false);
    }

    return NO_EXCEPTION;
}

ExceptionType
MMU::RetrievePageEntry(unsigned vpn, TranslationEntry **entry) const {
    ASSERT(entry);

    if (!tlb) {
        // Use a page table; `vpn` is an index in the table.

        if (vpn >= pageTableSize) {
            DEBUG_CONT_ERROR('A', "virtual page # %u too large for page table size %u!\n",
                       vpn, pageTableSize);
            return ADDRESS_ERROR_EXCEPTION;
        } else if (!pageTable[vpn].valid) {
            DEBUG_CONT_ERROR('A', "virtual page # %u too large for page table size %u!\n",
                       vpn, pageTableSize);
            return PAGE_FAULT_EXCEPTION;
        }

        *entry = &pageTable[vpn];
        return NO_EXCEPTION;
    } else {
        // Use the TLB.

        unsigned i;
        for (i = 0; i < TLB_SIZE; i++)
            if (tlb[i].valid && tlb[i].virtualPage == vpn) {
                *entry = &tlb[i];  // FOUND!
                return NO_EXCEPTION;
            }

        // Not found.
        DEBUG_CONT_ERROR('A', "no valid TLB entry found for this virtual page!\n");
        return PAGE_FAULT_EXCEPTION;  // Really, this is a TLB fault, the
                                      // page may be in memory, but not in
                                      // the TLB.
    }
}

/// Translate a virtual address into a physical address, using
/// either a page table or a TLB.
///
/// Check for alignment and all sorts of other errors, and if everything is
/// ok, set the use/dirty bits in the translation table entry, and store the
/// translated physical address in "physAddr".  If there was an error,
/// returns the type of the exception.
///
/// * `virtAddr" is the virtual address to translate.
/// * `physAddr" is the place to store the physical address.
/// * `size" is the amount of memory being read or written.
/// * `writing` -- if true, check the “read-only” bit in the TLB.
ExceptionType
MMU::Translate(unsigned virtAddr, unsigned *physAddr, unsigned size, bool writing) {
    DEBUG('A', "\tTranslate: ");

    // We must have either a TLB or a page table, but not both!
    ASSERT((!tlb) != (!pageTable));
    ASSERT(physAddr);

    // Check for alignment errors.
    if ((size == 4 && virtAddr & 0x3) || (size == 2 && virtAddr & 0x1)) {
        DEBUG_CONT_ERROR('A', "alignment problem at 0x%X, size %u!\n", virtAddr, size);
        return ADDRESS_ERROR_EXCEPTION;
    }

    // Calculate the virtual page number, and offset within the page,
    // from the virtual address.
    unsigned vpn    = (unsigned) virtAddr / PAGE_SIZE;
    unsigned offset = (unsigned) virtAddr % PAGE_SIZE;

    TranslationEntry *entry;
    ExceptionType exception = RetrievePageEntry(vpn, &entry);
    if (exception != NO_EXCEPTION) return exception;

    if (entry->readOnly && writing) {  // Trying to write to a read-only
                                       // page.
        DEBUG_CONT_ERROR('A', "0x%X mapped read-only!\n", virtAddr);
        return READ_ONLY_EXCEPTION;
    }

    unsigned pageFrame = entry->physicalPage;

    // If the `pageFrame` is too big, there is something really wrong!  An
    // invalid translation was loaded into the page table or TLB.
    if (pageFrame >= NUM_PHYS_PAGES) {
        DEBUG_CONT_ERROR('A', "frame %u > %u!\n", pageFrame, NUM_PHYS_PAGES);
        return BUS_ERROR_EXCEPTION;
    }

    *physAddr = pageFrame * PAGE_SIZE + offset;

    DEBUG_CONT('A', "physical address 0x%X.\n", *physAddr);
    ASSERT(*physAddr >= 0 && *physAddr + size <= MEMORY_SIZE);

    // Set the `use` and `dirty` flags.
    entry->use = true;
    if (writing) {
        DEBUG('a', "Marking virtual page %u as dirty.\n", vpn);
        entry->dirty = true;
    }

    return NO_EXCEPTION;
}

void
MMU::PrintTLB() const {
    printf("TLB Content:\n");
    for (unsigned i = 0; i < TLB_SIZE; i++)
        if (tlb[i].valid)
            printf("[%u] VPN: %d, Frame: %d, Valid: %d, RO: %d, Use: %d, Dirty: %d.\n",
                    i, tlb[i].virtualPage, tlb[i].physicalPage, tlb[i].valid,
                    tlb[i].readOnly, tlb[i].use, tlb[i].dirty);
}

void
MMU::TLBLoadEntry(TranslationEntry* entry) {
#ifdef PAGINATION
    TLBSaveEntry(tlbIdx);
#endif
    tlb[tlbIdx].virtualPage  = entry->virtualPage;
    tlb[tlbIdx].physicalPage = entry->physicalPage;
    tlb[tlbIdx].valid        = entry->valid;
    tlb[tlbIdx].readOnly     = entry->readOnly;
    tlb[tlbIdx].use          = entry->use;
    tlb[tlbIdx].dirty        = entry->dirty;

    tlbIdx++;
    tlbIdx %= TLB_SIZE;
}

#ifdef PAGINATION
void
MMU::TLBSaveEntry(unsigned index) {
    if (tlb[index].valid) {
        tlb[index].valid = false;

        unsigned physicalPage   = tlb[index].physicalPage;
        Thread* thread          = memMap->coreMap[physicalPage].thread;
        unsigned virtualPage    = memMap->coreMap[physicalPage].virtualPage;
        TranslationEntry* entry = &thread->space->pageTable[virtualPage];

        entry->virtualPage  = tlb[index].virtualPage;
        entry->physicalPage = tlb[index].physicalPage;
        entry->valid        = tlb[index].valid;
        entry->readOnly     = tlb[index].readOnly;
        entry->use          = tlb[index].use;
        entry->dirty        = tlb[index].dirty;
    }
}

unsigned
MMU::ChooseFrame() {
    return Random() % NUM_PHYS_PAGES;
}
#endif

