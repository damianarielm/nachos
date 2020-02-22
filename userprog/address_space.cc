/// Routines to manage address spaces (executing user programs).
///
/// In order to run a user program, you must:
///
/// 1. Link with the `-N -T 0` option.
/// 2. Run `coff2noff` to convert the object file to Nachos format (Nachos
///    object code format is essentially just a simpler version of the UNIX
///    executable object code format).
/// 3. Load the NOFF file into the Nachos file system (if you have not
///    implemented the file system yet, you do not need to do this last
///    step).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "address_space.hh"
#include ".bin/noff.h"
#include "machine/.endianness.hh"
#include "threads/system.hh"
#include "filesys/directory_entry.hh"

#ifdef DEMAND_LOADING
    #ifndef MULTIPROGRAMMING
        #error Compilation flags set not supported.
    #endif
#endif
#ifdef PAGINATION
    #ifndef DEMAND_LOADING
        #error Compilation flags set not supported.
    #endif
    #ifndef MULTIPROGRAMMING
        #error Compilation flags set not supported.
    #endif
#endif

/// Create an address space to run a user program.
///
/// Load the program from a file `executable`, and set everything up so that
/// we can start executing user instructions.
///
/// Assumes that the object code file is in NOFF format.
///
/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
///
/// * `executable` is the file containing the object code to load into
///   memory.
AddressSpace::AddressSpace(OpenFile *executable, Thread* thread) {
    ASSERT(executable);

    InitSegments(); // Initialize segments metadata.
#ifdef DEMAND_LOADING
    file = executable;
#endif

    // How big is address space?
    unsigned size = codeSize + initDataSize + uninitDataSize + USER_STACK_SIZE;
      // We need to increase the size to leave room for the stack.

    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

#ifdef PAGINATION
    // Initialize swap file.
    char swapFileName[FILE_NAME_MAX_LEN];
    snprintf(swapFileName, FILE_NAME_MAX_LEN, "SWAP.%u", thread->threadId);
    fileSystem->Create(swapFileName, size);
    swapFile = fileSystem->Open(swapFileName);
    ASSERT(swapFile);
#endif

    DEBUG('a', "Initializing address space, num pages %u, size %u.\n", numPages, size);

    // Check we are not trying to run anything too big -- at least until we have virtual memory.
#ifdef MULTIPROGRAMMING
    #ifndef DEMAND_LOADING
    ASSERT(numPages <= memMap->CountClear());
    #endif
#else
    ASSERT(numPages <= NUM_PHYS_PAGES);
#endif

    // First, set up the translation.
    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
#ifdef DEMAND_LOADING
        pageTable[i].virtualPage  = numPages + 1; // Not on memory.
#else
        pageTable[i].virtualPage  = i;
#endif
#ifdef MULTIPROGRAMMING
    #ifndef DEMAND_LOADING
        pageTable[i].physicalPage = memMap->Find();
    #endif
#else
        pageTable[i].physicalPage = i;
#endif
        pageTable[i].valid        = true;
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
          // If the code segment was entirely on a separate page, we could
          // set its pages to be read-only.
    }

#ifndef DEMAND_LOADING
    char *mainMemory = machine->GetMMU()->mainMemory;
#endif

#ifndef MULTIPROGRAMMING
    // Zero out the entire address space, to zero the unitialized data
    // segment and the stack segment.
    memset(mainMemory, 0, size);
#endif

#ifndef DEMAND_LOADING
    // Copy in the code and data segments into memory.
    if (codeSize > 0) {
        DEBUG('a', "Initializing code segment. Num pages: %u; Size: %u.\n",
                codeSize / PAGE_SIZE, codeSize);
    #ifndef MULTIPROGRAMMING
        executable->ReadAt(&(mainMemory[codeVirtualAddr]), codeSize, codeInFileAddr);
    #else
        for (unsigned i = 0; i < codeSize; i++) {
            int realAddr = Translate(codeVirtualAddr + i);

            DEBUG('a', "Writing byte %d from virtual adress 0x%X to real addres 0x%X.\n",
                    i, codeVirtualAddr + i, realAddr);

            executable->ReadAt(&(mainMemory[realAddr]), 1, codeInFileAddr + i);
        }
    #endif
    }
    if (initDataSize > 0) {
        DEBUG('a', "Initializing data segment. Num pages: %u; Size: %u.\n",
                initDataSize / PAGE_SIZE, initDataSize);
    #ifndef MULTIPROGRAMMING
        executable->ReadAt(&(mainMemory[initDataVirtualAddr]), initDataSize, initDataInFileAddr);
    #else
        for (unsigned i = 0; i < initDataSize; i++) {
            int realAddr = Translate(initDataVirtualAddr + i);

            DEBUG('a', "Writing byte %d from virtual adress 0x%X to real addres 0x%X.\n",
                    i, codeVirtualAddr + i, realAddr);

            executable->ReadAt(&(mainMemory[realAddr]), 1, initDataInFileAddr + i);
        }
    #endif
    }
#endif
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace() {
#ifdef MULTIPROGRAMMING
    for (unsigned i = 0; i < numPages; i++) {
    #ifdef DEMAND_LOADING
        if (pageTable[i].virtualPage == numPages + 1) continue; // Not on memory.
    #endif
        unsigned physicalPage = pageTable[i].physicalPage;

        memMap->Clear(physicalPage);
        memset(&machine->GetMMU()->mainMemory[physicalPage * PAGE_SIZE], 0, PAGE_SIZE);
    }
#endif

    delete [] pageTable;
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters() {
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++) machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u.\n", numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void
AddressSpace::SaveState() {
#ifdef PAGINATION
    DEBUG('b', "Saving TLB.\n");
    for (unsigned i = 0; i < TLB_SIZE; i++)
        machine->GetMMU()->TLBSaveEntry(i);
#endif
}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void
AddressSpace::RestoreState() {
#ifndef USE_TLB
    machine->GetMMU()->pageTable     = pageTable;
    machine->GetMMU()->pageTableSize = numPages;
#else
    DEBUG('b', "Emptying TLB.\n");
    for (unsigned i = 0; i < TLB_SIZE; i++)
        machine->GetMMU()->tlb[i].valid = false;
#endif
}

/// Given a virtual address, returns the real address in memory.
int
AddressSpace::Translate(int virtualAddr) {
    int page   = virtualAddr / PAGE_SIZE;
    int offset = virtualAddr % PAGE_SIZE;
    int frame  = pageTable[page].physicalPage;

    return frame * PAGE_SIZE + offset;
}

#ifdef DEMAND_LOADING
unsigned
AddressSpace::LoadPage(unsigned virtualPage) {
    int physicalFrame = memMap->Find();
    #ifdef PAGINATION
    if (physicalFrame == -1) {
        DEBUG_ERROR('w', "No room for virtual page %u. ", virtualPage);
        RemovePage();
        physicalFrame = memMap->Find();
    }
    #endif
    int realAddr = physicalFrame * PAGE_SIZE;
    char* mainMemory = machine->GetMMU()->mainMemory;
    DEBUG('a', "Loading virtual page %u on physical frame %u",
            virtualPage, physicalFrame);
    ASSERT(physicalFrame != -1);

    memset(&(mainMemory[realAddr]), 0, PAGE_SIZE);
    #ifndef PAGINATION
    file->ReadAt(&(mainMemory[realAddr]), PAGE_SIZE, codeInFileAddr +
                                                     virtualPage * PAGE_SIZE);
    #else
    if (!pageTable[virtualPage].dirty) {
        DEBUG_CONT('a', " from executalbe");
        file->ReadAt(&(mainMemory[realAddr]), PAGE_SIZE, codeInFileAddr +
                                                         virtualPage * PAGE_SIZE);
    } else {
        DEBUG_CONT('a', " from swap");
        swapFile->ReadAt(&(mainMemory[realAddr]), PAGE_SIZE, virtualPage * PAGE_SIZE);
    }

    stats->numLoadedPages++;
    #endif
    DEBUG_CONT('a', ".\n");

    return physicalFrame;
}

    #ifdef PAGINATION
void
AddressSpace::RemovePage() {
    unsigned physicalFrame = machine->GetMMU()->ChooseFrame();
    unsigned virtualPage   = memMap->coreMap[physicalFrame].virtualPage;
    Thread* thread         = memMap->coreMap[physicalFrame].thread;
    DEBUG_CONT_ERROR('w', "Removing physical frame %d.\n", physicalFrame);

    TranslationEntry* tlb = machine->GetMMU()->tlb;

    // Invalidates tlb and page talbe entries.
    for (unsigned i = 0; i < TLB_SIZE; i++)
        if (tlb[i].physicalPage == physicalFrame && tlb[i].valid)
            machine->GetMMU()->TLBSaveEntry(i);
    thread->space->pageTable[virtualPage].virtualPage = thread->space->numPages + 1;

    // If it's a dirty page, swap it on disk.
    if (thread->space->pageTable[virtualPage].dirty)
        thread->space->SwapPage(virtualPage);

    memMap->Clear(physicalFrame);
}

void
AddressSpace::SwapPage(unsigned virtualPage) {
    DEBUG('W', "Writing virtual page %u on disk.\n", virtualPage);

    unsigned physicalAddress = pageTable[virtualPage].physicalPage * PAGE_SIZE;
    char* mainMemory = machine->GetMMU()->mainMemory;
    swapFile->WriteAt(&mainMemory[physicalAddress], PAGE_SIZE, virtualPage * PAGE_SIZE);
    stats->numSwappedPages++;
}
    #endif
#endif
