/// Data structures to keep track of executing user programs (address
/// spaces).
///
/// For now, we do not keep any information about address spaces.  The user
/// level CPU state is saved and restored in the thread executing the user
/// program (see `thread.hh`).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_USERPROG_ADDRESSSPACE__HH
#define NACHOS_USERPROG_ADDRESSSPACE__HH

#include <stdint.h>
#include "filesys/file_system.hh"
#include "machine/translation_entry.hh"

const unsigned USER_STACK_SIZE = 1024;  ///< Increase this as necessary!

class AddressSpace {
public:

    /// Create an address space, initializing it with the program stored in
    /// the file `executable`.
    ///
    /// * `executable` is the open file that corresponds to the program.
    AddressSpace(OpenFile *executable, Thread* thread);

    /// De-allocate an address space.
    ~AddressSpace();

    /// Initialize user-level CPU registers, before jumping to user code.
    void InitRegisters();

    /// Save/restore address space-specific info on a context switch.

    void SaveState();
    void RestoreState();

    /// Given a virtual address, returns the real address in memory.
    int Translate(int virtualAddr);

#ifdef DEMAND_LOADING
    /// Load the virtual page in memory. Returns the physical frame.
    unsigned LoadPage(unsigned virtualPage);
#endif

#ifdef PAGINATION
    /// Removes a pgae from memory and swap it if needed.
    void RemovePage();

    /// Writes a page to the swap file.
    void SwapPage(unsigned virtualPage);
#endif

    /// Assume linear page table translation for now!
    TranslationEntry *pageTable;

    /// Number of pages in the virtual address space.
    unsigned numPages;

private:

#ifdef DEMAND_LOADING
    /// Executable file.
    OpenFile* file;
#endif

#ifdef PAGINATION
    /// Swap file.
    OpenFile* swapFile;
#endif

    // Code segment information.
    uint32_t codeSize;        /// Size of the code segment.
    uint32_t codeVirtualAddr; /// Location of segment in virtual address space.
    uint32_t codeInFileAddr;  /// Position in the file where the code start.

    /// Initialized data segment information.
    uint32_t initDataSize;
    uint32_t initDataVirtualAddr;
    uint32_t initDataInFileAddr;

    /// Uninitialized data segment information.
    uint32_t uninitDataSize;

};

#endif
