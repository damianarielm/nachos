/// Routines for managing the disk file header (in UNIX, this would be called
/// the i-node).
///
/// The file header is used to locate where on disk the file's data is
/// stored.  We implement this as a fixed size table of pointers -- each
/// entry in the table points to the disk sector containing that portion of
/// the file data (in other words, there are no indirect or doubly indirect
/// blocks). The table size is chosen so that the file header will be just
/// big enough to fit in one disk sector,
///
/// Unlike in a real system, we do not keep track of file permissions,
/// ownership, last modification date, etc., in the file header.
///
/// A file header can be initialized in two ways:
///
/// * for a new file, by modifying the in-memory data structure to point to
///   the newly allocated data blocks;
/// * for a file already on disk, by reading the file header from disk.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "file_header.hh"
#include "threads/system.hh"

FileHeader::FileHeader(unsigned hdrSector, const char* fileName) {
    sector = hdrSector;
    name = fileName;
    FetchFromDisk();
}

/// Initialize a fresh file header for a newly created file.  Allocate data
/// blocks for the file out of the map of free disk blocks.  Return false if
/// there are not enough free blocks to accomodate the new file.
///
/// * `freeMap` is the bit map of free disk sectors.
/// * `fileSize` is the bit map of free disk sectors.
bool
FileHeader::Allocate(Bitmap *freeMap, unsigned fileSize) {
    DEBUG('f', "Trying to allocate %u bytes for file %s... ", fileSize, name);
    ASSERT(freeMap);

    raw.numBytes = fileSize;
    raw.numSectors = DivRoundUp(fileSize, SECTOR_SIZE);
    if (!fileSize) {
        DEBUG_CONT('f', "Succeed.\n");
        return true; // Empty file.
    }

    // Skip to the first unasigned sector.
    unsigned i = 0;
    while (i != NUM_DIRECT && raw.dataSectors[i]) i++;

    if (freeMap->CountClear() < raw.numSectors - i) {
        DEBUG_CONT_ERROR('f', "Failed. Space left: %u bytes.\n",
                freeMap->CountClear() * SECTOR_SIZE);
        return false;  // Not enough space.
    }

    for (; i < raw.numSectors; i++) {
        // New header needed.
        if (i == NUM_DIRECT) {
            raw.nextHeader = freeMap->Find();
            if (raw.nextHeader == -1) return false;
            DEBUG_CONT('f', "\n");
            DEBUG('f', "Creating extra header for %s on sector %d...\n",
                    name, raw.nextHeader);

            FileHeader* header = new FileHeader(raw.nextHeader, name);
            header->ClearRaw();
            header->raw.level = raw.level + 1;
            bool tmp = header->Allocate(freeMap, raw.numBytes - MAX_FILE_SIZE);
            raw.numBytes = MAX_FILE_SIZE;
            raw.numSectors = NUM_DIRECT;
            header->WriteBack();

            delete header;
            return tmp;
        }

        raw.dataSectors[i] = freeMap->Find();
    }

    DEBUG_CONT('f', "Succeed.\n");

    return true;
}

/// De-allocate all the space allocated for data blocks for this file.
///
/// * `freeMap` is the bit map of free disk sectors.
void
FileHeader::Deallocate(Bitmap *freeMap) {
    DEBUG('f', "Deallocating %u bytes from file %s.\n", raw.numBytes, name);
    ASSERT(freeMap);

    for (unsigned i = 0; i < NUM_DIRECT; i++)
        if (freeMap->Test(raw.dataSectors[i]) && raw.dataSectors[i])
            freeMap->Clear(raw.dataSectors[i]);

    if (raw.nextHeader) {
        FileHeader* header = new FileHeader(raw.nextHeader, name);
        header->Deallocate(freeMap);
        freeMap->Clear(raw.nextHeader);
        delete header;
    }
}

/// Fetch contents of file header from disk.
///
/// * `sector` is the disk sector containing the file header.
void
FileHeader::FetchFromDisk() {
    DEBUG('F', "Reading header for %s from sector %u.\n", name, sector);

    synchDisk->ReadSector(sector, (char *) &raw);
}

/// Write the modified contents of the file header back to disk.
///
/// * `sector` is the disk sector to contain the file header.
void
FileHeader::WriteBack() {
    DEBUG('F', "Writing header of %s to sector %u.\n", name, sector);

    synchDisk->WriteSector(sector, (char *) &raw);
}

/// Return which disk sector is storing a particular byte within the file.
/// This is essentially a translation from a virtual address (the offset in
/// the file) to a physical address (the sector where the data at the offset
/// is stored).
///
/// * `offset` is the location within the file of the byte in question.
unsigned
FileHeader::ByteToSector(unsigned offset) {
    unsigned idx = (offset / SECTOR_SIZE) % NUM_DIRECT;

    if (offset / (NUM_DIRECT * SECTOR_SIZE) != raw.level) {
        ASSERT(raw.nextHeader);

        FileHeader* header = new FileHeader(raw.nextHeader, name);
        unsigned tmp = header->ByteToSector(offset);
        delete header;
        return tmp;
    } else {
        return raw.dataSectors[idx];
    }
}

/// Return the number of bytes in the file.
unsigned
FileHeader::FileLength() const {
    return raw.numBytes;
}

/// Print the contents of the file header, and the contents of all the data
/// blocks pointed to by the file header.
void
FileHeader::Print() {
    char *data = new char [SECTOR_SIZE];
    FileHeader *header = this;

    printf("Size: %u bytes.\nBlock numbers: ", raw.numBytes);
    for (unsigned i = 0; i < raw.numSectors; i++) {
        if (i && !(i % NUM_DIRECT)) {
            ASSERT(header->raw.nextHeader);

            printf(BOLD WHITE "|| " RESET);
            int tmp = header->raw.nextHeader;
            if (header != this) delete header;
            header = new FileHeader(tmp, name);
        }
        printf("%u ", header->raw.dataSectors[i % NUM_DIRECT]);
    }
    printf("\n");

    if (header != this) {
        delete header;
        header = this;
    }

    for (unsigned i = 0, k = 0; i < raw.numSectors; i++) {
        if (i && !(i % NUM_DIRECT)) {
            printf(YELLOW "[%u] ", header->raw.nextHeader);
            printf("--------------------------------------------------------------\n" RESET);
            int tmp = header->raw.nextHeader;
            if (header != this) delete header;
            header = new FileHeader(tmp, name);
        }

        synchDisk->ReadSector(header->raw.dataSectors[i % NUM_DIRECT], data);

        printf(YELLOW "[%u] " RESET, header->raw.dataSectors[i % NUM_DIRECT]);
        for (unsigned j = 0; j < SECTOR_SIZE && k < raw.numBytes; j++, k++)
            PrintByte(data[j]);
        printf("\n");
    }

    if (header != this) delete header;
    delete [] data;
}

const RawFileHeader *
FileHeader::GetRaw() const {
    return &raw;
}

const char*
FileHeader::GetName() {
    return name;
}

void
FileHeader::ClearRaw() {
    for (unsigned i = 0; i < NUM_DIRECT; i++) raw.dataSectors[i] = 0;
    raw.nextHeader = raw.level = raw.numBytes = raw.numSectors = 0;
}

void
FileHeader::Expand(unsigned numBytes) {
    ASSERT(numBytes);

    // Always update file size on the main header and last header.
    if (!raw.level || !raw.nextHeader) {
        raw.numBytes += numBytes;
        raw.numSectors = DivRoundUp(raw.numBytes, SECTOR_SIZE);
        WriteBack();
    }

    // If there is a next header, delegate the request.
    if (raw.nextHeader) {
        FileHeader* header = new FileHeader(raw.nextHeader, name);
        header->Expand(numBytes);
        delete header;
        return;
    }

    // Skip to the first unasigned sector. Maybe we don't really need to expand.
    unsigned i = 0;
    while (i != NUM_DIRECT && raw.dataSectors[i]) i++;

    if (i < raw.numSectors) {
        // Loads the map of free sectors.
        OpenFile* freeMapFile = new OpenFile(0, ITALIC "Free Sectors Map" FAINT);
        Bitmap* freeMap = new Bitmap(NUM_SECTORS);
        freeMap->FetchFrom(freeMapFile);

        Allocate(freeMap, raw.numBytes);
        WriteBack();
        freeMap->WriteBack(freeMapFile);
    }
}
