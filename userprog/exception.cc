/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2019 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "transfer.hh"
#include "syscall.h"
#include "filesys/directory_entry.hh"
#include "threads/system.hh"
#include "userprog/args.hh"

void Forker(void* args) {
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    if (args) {
        machine->WriteRegister(4, WriteArgs((char**) args));
        machine->WriteRegister(5, machine->ReadRegister(STACK_REG) + 16);
    }

    machine->Run();
}

static void
IncrementPC() {
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et) {
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);

    ASSERT(false);
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void
SyscallHandler(ExceptionType _et) {
    int scid = machine->ReadRegister(2);

    switch (scid) {
        case SC_HALT:
            DEBUG('y', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);
            int isDirectory = machine->ReadRegister(5);
            char filename[FILE_NAME_MAX_LEN + 1];

            if (!filenameAddr)
                DEBUG_ERROR('y', "Error: address to filename string is null.\n");
            else if (!ReadStringFromUser(filenameAddr, filename, sizeof filename))
                DEBUG_ERROR('y', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
            else if (!fileSystem->Create(filename, 0, isDirectory))
                DEBUG_ERROR('y', "Error: cannot create file %s.\n", filename);
            else
                DEBUG('y', "File %s created.\n", filename);

            break;
        }

        case SC_WRITE: {
            int stringAddr = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            OpenFileId fd = machine->ReadRegister(6);
            DEBUG('y', "Write request. String addres: %d; Size: %d, Fd: %d.\n",
                    stringAddr, size, fd);

            if (!stringAddr)
                DEBUG_ERROR('y', "Error: address to filename string is null.\n");
            else if (size < 0)
                DEBUG_ERROR('y', "Error: invalid size.\n");
            else if (fd < CONSOLE_OUTPUT)
                DEBUG_ERROR('y', "Error: invalid file descriptor.\n");
            else if (!currentThread->openFiles->HasKey(fd))
                DEBUG_ERROR('y', "Error: file descriptor not opened.\n");
            else {
                char towrite[size];

                ReadBufferFromUser(stringAddr, towrite, size);
                DEBUG('y', "String to write: %s.\n", towrite);

                if (fd == CONSOLE_OUTPUT)
                    for (int i = 0; i < size; i++) synchConsole->PutChar(towrite[i]);
                else
                    currentThread->openFiles->Get(fd)->Write(towrite, size);
            }

            break;
        }

        case SC_READ: {
            int usrAddr = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            OpenFileId fd = machine->ReadRegister(6);
            DEBUG('y', "Read request. User address: %d, Size: %d, Fd: %d.\n",
                  usrAddr, size, fd);

            int i;
            if (!usrAddr)
                DEBUG_ERROR('y', "Error: address to filename string is null.\n");
            else if (size < 0)
                DEBUG_ERROR('y', "Error: invalid size.\n");
            else if (fd == CONSOLE_OUTPUT || fd < 0)
                DEBUG_ERROR('y', "Error: invalid file descriptor.\n");
            else if (!currentThread->openFiles->HasKey(fd))
                DEBUG_ERROR('y', "Error: file descriptor not opened.\n");
            else {
                char readed[size + 1];

                if (fd == CONSOLE_INPUT) {
                    for(i = 0; i < size; i++) readed[i] = synchConsole->GetChar();
                    readed[i--] = '\0';
                } else {
                    i = currentThread->openFiles->Get(fd)->Read(readed, size);
                }

                WriteStringToUser(readed, usrAddr);
                DEBUG('y', "String readed: %s. Size: %d\n", readed, i);
            }
            machine->WriteRegister(2, i);

            break;
        }

        case SC_OPEN: {
            int filenameAddr = machine->ReadRegister(4);
            DEBUG('y', "Open request. Filename address: %d.\n", filenameAddr);

            char filename[FILE_NAME_MAX_LEN + 1];
            OpenFile *o;
            machine->WriteRegister(2, -1);
            if (!filenameAddr)
                DEBUG_ERROR('y', "Error: address to filename string is null.\n");
            else if (!ReadStringFromUser(filenameAddr, filename, sizeof filename))
                DEBUG_ERROR('y', "Error: filename string too long.\n");
            else if ((o = fileSystem->Open(filename)) == nullptr)
                DEBUG_ERROR('y', "Error: cannot open file `%s`.\n", filename);
            else {
                int fd = currentThread->openFiles->Add(o);
                DEBUG('y', "File %s opened. Assigned Fd: %d.\n", filename, fd);
                machine->WriteRegister(2, fd);
            }

            break;
        }

        case SC_CLOSE: {
            int fid = machine->ReadRegister(4);
            DEBUG('y', "Close requested for id %u.\n", fid);

            if (fid < 0)
                DEBUG_ERROR('y', "Error: invalid file descriptor.\n");
            else if (!currentThread->openFiles->HasKey(fid))
                DEBUG_ERROR('y', "Error: file descriptor not opened.\n");
            else {
                currentThread->openFiles->Remove(fid);
                DEBUG('y', "File closed.\n");
            }

            break;
        }

        case SC_EXIT: {
            int status = machine->ReadRegister(4);
            DEBUG('y', "Exiting with status: %d.\n", status);

            currentThread->Finish(status);

            break;
        }

        case SC_JOIN: {
            int threadId = machine->ReadRegister(4);
            DEBUG('y', "Join requested. PID: %d.\n", threadId);

            if (threadId < 0 || !threadTable->HasKey(threadId))
                DEBUG_ERROR('y', "Error: invalid PID.\n");
            else
                machine->WriteRegister(2, threadTable->Get(threadId)->Join());

            break;
        }

        case SC_EXEC: {
#ifdef MULTIPROGRAMMING
            int filenameAddr = machine->ReadRegister(4);
            char** args = SaveArgs(machine->ReadRegister(5));
            int join = machine->ReadRegister(6);
            DEBUG('y', "Exec requested. Filename address: %d.\n", filenameAddr);

            char* filename = new char[FILE_NAME_MAX_LEN + 1];
            OpenFile *o;
            if (!filenameAddr)
                DEBUG_ERROR('y', "Error: address to filename string is null.\n");
            else if (!ReadStringFromUser(filenameAddr, filename, sizeof filename * FILE_NAME_MAX_LEN))
                DEBUG_ERROR('y', "Error: filename string too long.\n");
            else if ((o = fileSystem->Open(filename)) == nullptr)
                DEBUG_ERROR('y', "Error: cannot open file %s.\n", filename);
            else {
                Thread *t = new Thread(filename, join, currentThread->GetPriority());
                t->space = new AddressSpace(o, t);
                t->Fork(Forker, args);

                machine->WriteRegister(2, t->threadId);
            }
#else
            DEBUG_ERROR('y', "Error: this machine doesn't support multiprogramming.\n");
#endif

            break;
        }

        case SC_REMOVE: {
            int filenameAddr = machine->ReadRegister(4);
            char filename[FILE_NAME_MAX_LEN + 1];

            if (!filenameAddr)
                DEBUG_ERROR('y', "Error: address to filename string is null.\n");
            else if (!ReadStringFromUser(filenameAddr, filename, sizeof filename))
                DEBUG_ERROR('y', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
            else if (!fileSystem->Remove(filename))
                DEBUG_ERROR('y', "Error: cannot remove file %s.\n", filename);
            else
                DEBUG('y', "File %s deleted.\n", filename);

            break;
        }

#ifdef FILESYS
        case SC_LS: {
            fileSystem->List();
            break;
        }

        case SC_CD: {
            int filenameAddr = machine->ReadRegister(4);
            char filename[FILE_NAME_MAX_LEN + 1];

            if (!filenameAddr)
                DEBUG_ERROR('y', "Error: address to filename string is null.\n");
            else if (!ReadStringFromUser(filenameAddr, filename, sizeof filename))
                DEBUG_ERROR('y', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
            else if (!fileSystem->ChangeDirectory(filename))
                DEBUG_ERROR('y', "Error: can't change to directory %s.\n", filename);
            else
                DEBUG('y', "Changing to directory %s.\n", filename);

            break;
        }
#endif

        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);
    }

    IncrementPC();
}

static void
PageFaultHandler(ExceptionType et) {
    int virtualAddr = machine->ReadRegister(BAD_VADDR_REG);
    int virtualPage = virtualAddr / PAGE_SIZE;
    TranslationEntry* entry = &currentThread->space->pageTable[virtualPage];
    stats->numPageFaults++;
    ASSERT(virtualPage >= 0);

    DEBUG('b', "Page fault in %s. Virtual address: %u, VPN: %u.\n",
            currentThread->GetName(), virtualAddr, virtualPage);

    entry->valid = true;

#ifdef DEMAND_LOADING
    // Not in memory.
    if (entry->virtualPage == currentThread->space->numPages + 1) {
        entry->physicalPage = currentThread->space->LoadPage(virtualPage);
        entry->virtualPage = virtualPage;

    #ifdef PAGINATION
        memMap->coreMap[entry->physicalPage].thread = currentThread;
        memMap->coreMap[entry->physicalPage].virtualPage = virtualPage;
    #endif
    }
#endif

    machine->GetMMU()->TLBLoadEntry(entry);
}

/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers() {
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &PageFaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
