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
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "file_header.hh"
#include "threads/system.hh"

#include <ctype.h>
#include <stdio.h>


/// Initialize a fresh file header for a newly created file.  Allocate data
/// blocks for the file out of the map of free disk blocks.  Return false if
/// there are not enough free blocks to accomodate the new file.
///
/// * `freeMap` is the bit map of free disk sectors.
/// * `fileSize` is the bit map of free disk sectors.
bool
FileHeader::Allocate(Bitmap *freeMap, unsigned fileSize)
{
    ASSERT(freeMap != nullptr);

    if (fileSize > MAX_FILE_SIZE) {
        return false;
    }
    raw.numBytes = fileSize;
    raw.numSectors = DivRoundUp(fileSize, SECTOR_SIZE);

    unsigned fhSectorsToAllocate = 0;
    int remainingSectors = raw.numSectors - NUM_DIRECT;
    if (remainingSectors > 0) {
        fhSectorsToAllocate++;
        raw.fiQuantity = std::min((unsigned)remainingSectors, NUM_DIRECT2);
        remainingSectors -= NUM_DIRECT2;
        if (remainingSectors > 0) {
            fhSectorsToAllocate++;
            raw.siQuantity = remainingSectors;
            fhSectorsToAllocate += DivRoundUp((unsigned)remainingSectors, NUM_DIRECT2);
        }
    }

    if (freeMap->CountClear() < raw.numSectors + fhSectorsToAllocate) {
        return false;  // Not enough space.
    }

    remainingSectors = raw.numSectors;
    unsigned min = std::min((unsigned)remainingSectors, NUM_DIRECT);
    for (unsigned i = 0; i < min; i++) {
        raw.dataSectors[i] = freeMap->Find();
    }
    remainingSectors -= NUM_DIRECT;
    if (remainingSectors > 0) {
        raw.firstIndirection = freeMap->Find();
        for (unsigned i = 0; i < raw.fiQuantity; i++) {
            firstInd.dataSectors[i] = freeMap->Find();
        }
        remainingSectors -= NUM_DIRECT2;

        if (remainingSectors > 0) {
            raw.secondIndirection = freeMap->Find();
            unsigned i = 0;
            while (remainingSectors > 0) {
                secondInd.dataSectors[i] = freeMap->Find();
                RawFileIndirection aux;
                min = std::min((unsigned)remainingSectors, NUM_DIRECT2);
                for (unsigned j = 0; j < min; j++) {
                    aux.dataSectors[j] = freeMap->Find();
                }
                secondIndArray.push_back(aux);
                remainingSectors -= NUM_DIRECT2;
                i++;
            }
        }
    }
    return true;
}

bool
FileHeader::Extend(Bitmap *freeMap, unsigned extendSize)
{
    DEBUG('f', "Extending file from actual size %u by %u\n", raw.numBytes, extendSize);

    ASSERT(freeMap != nullptr);

    if (raw.numBytes + extendSize > MAX_FILE_SIZE) {
        return false;
    }

    if (extendSize <= (raw.numSectors * SECTOR_SIZE - raw.numBytes)) {
        raw.numBytes += extendSize;
        return true;
    }

    unsigned sectorsToAllocate = DivRoundUp(raw.numBytes + extendSize, SECTOR_SIZE) - raw.numSectors;
    unsigned totalSectors = sectorsToAllocate + raw.numSectors;
    unsigned headerSectorsToAllocate = 0;

    if(raw.firstIndirection == -1 && totalSectors > NUM_DIRECT) {
        headerSectorsToAllocate++;
    } 
    if (totalSectors > NUM_DIRECT + NUM_DIRECT2) {
        if (raw.secondIndirection == -1) {
            headerSectorsToAllocate += DivRoundUp(totalSectors - (NUM_DIRECT + NUM_DIRECT2), NUM_DIRECT2) + 1;
        } else {
            headerSectorsToAllocate += (DivRoundUp(totalSectors - (NUM_DIRECT + NUM_DIRECT2), NUM_DIRECT2) - DivRoundUp(raw.siQuantity, NUM_DIRECT2));
        }
    }

    if(freeMap->CountClear() < sectorsToAllocate + headerSectorsToAllocate) {
        return false;
    }

    if(raw.numSectors < NUM_DIRECT) {
        for (unsigned i = raw.numSectors; i < NUM_DIRECT && sectorsToAllocate != 0; i++) {
            raw.dataSectors[i] = freeMap->Find();
            sectorsToAllocate--;
        }
    }

    if(sectorsToAllocate != 0 && raw.numSectors < NUM_DIRECT + NUM_DIRECT2) {
        if (raw.firstIndirection == -1)
            raw.firstIndirection = freeMap->Find();
        for (unsigned i = raw.fiQuantity; i < NUM_DIRECT2 && sectorsToAllocate != 0; i++) {
            firstInd.dataSectors[i] = freeMap->Find();
            sectorsToAllocate--;
            raw.fiQuantity++;
        }
    }
    if(sectorsToAllocate != 0) {
        if (raw.secondIndirection == -1)
            raw.secondIndirection = freeMap->Find();
        
        unsigned index = DivRoundDown(raw.siQuantity, NUM_DIRECT2);

        if(secondIndArray.size() == index + 1) {
            for (unsigned i = raw.siQuantity % NUM_DIRECT2; i < NUM_DIRECT2 && sectorsToAllocate != 0; i++) {
                secondIndArray[index].dataSectors[i] = freeMap->Find();
                raw.siQuantity++;
                sectorsToAllocate--;
            }
            index++;
        }

        while (sectorsToAllocate > 0) {
            RawFileIndirection aux;
            secondInd.dataSectors[index] = freeMap->Find();
            unsigned min = std::min((unsigned)sectorsToAllocate, NUM_DIRECT2);
            for (unsigned i = 0; i < min; i++) {
                aux.dataSectors[i] = freeMap->Find();
                raw.siQuantity++;
            }
            secondIndArray.push_back(aux);
            sectorsToAllocate -= min;
            index++;
        }
    }

    raw.numBytes += extendSize;
    raw.numSectors = DivRoundUp(raw.numBytes, SECTOR_SIZE);
    return true;
}

/// De-allocate all the space allocated for data blocks for this file.
///
/// * `freeMap` is the bit map of free disk sectors.
void
FileHeader::Deallocate(Bitmap *freeMap)
{
    ASSERT(freeMap != nullptr);

    unsigned min = std::min(raw.numSectors, NUM_DIRECT);
    for (unsigned i = 0; i < min; i++) {
        ASSERT(freeMap->Test(raw.dataSectors[i]));  // ought to be marked!
        freeMap->Clear(raw.dataSectors[i]);
    }
    
    if (raw.firstIndirection != -1) {
        for (unsigned i = 0; i < raw.fiQuantity; i++) {
            ASSERT(freeMap->Test(firstInd.dataSectors[i]));  // ought to be marked!
            freeMap->Clear(firstInd.dataSectors[i]);
        }

        ASSERT(freeMap->Test(raw.firstIndirection));  // ought to be marked!
        freeMap->Clear(raw.firstIndirection);   
    }

    if (raw.secondIndirection != -1) {
        unsigned index = 0;
        for (unsigned i = 0; i < raw.siQuantity; i++) {
            unsigned pos = i%NUM_DIRECT2;
            ASSERT(freeMap->Test(secondIndArray[index].dataSectors[pos]));  // ought to be marked!
            freeMap->Clear(secondIndArray[index].dataSectors[pos]);
            if ((i+1)%NUM_DIRECT2 == 0) {
                ASSERT(freeMap->Test(secondInd.dataSectors[index]));  // ought to be marked!
                freeMap->Clear(secondInd.dataSectors[index]);   
                index++;
            }
        }
        ASSERT(freeMap->Test(secondInd.dataSectors[index]));  // ought to be marked!
        freeMap->Clear(secondInd.dataSectors[index]);  

        ASSERT(freeMap->Test(raw.secondIndirection));  // ought to be marked!
        freeMap->Clear(raw.secondIndirection);   
    }
}

/// Fetch contents of file header from disk.
///
/// * `sector` is the disk sector containing the file header.
void
FileHeader::FetchFrom(unsigned sector)
{
    synchDisk->ReadSector(sector, (char *) &raw);
    if (raw.firstIndirection != -1) {
        synchDisk->ReadSector(raw.firstIndirection, (char *) &firstInd);
        if (raw.secondIndirection != -1) {
            synchDisk->ReadSector(raw.secondIndirection, (char *) &secondInd);
            secondIndArray.clear();
            RawFileIndirection aux;
            unsigned read = 0;
            for (unsigned i = 0; read < raw.siQuantity; i++) {
                synchDisk->ReadSector(secondInd.dataSectors[i], (char *) &aux);
                secondIndArray.push_back(aux);
                read += NUM_DIRECT2;
            }
        }
    }
}

/// Write the modified contents of the file header back to disk.
///
/// * `sector` is the disk sector to contain the file header.
void
FileHeader::WriteBack(unsigned sector)
{
    synchDisk->WriteSector(sector, (char *) &raw);
    if (raw.firstIndirection != -1) {
        synchDisk->WriteSector(raw.firstIndirection, (char *) &firstInd);
        if (raw.secondIndirection != -1) {
            synchDisk->WriteSector(raw.secondIndirection, (char *) &secondInd);

            RawFileIndirection aux;
            unsigned wrote = 0;
            for (unsigned i = 0; wrote < raw.siQuantity; i++) {
                aux = secondIndArray[i];
                synchDisk->WriteSector(secondInd.dataSectors[i], (char *) &aux);
                wrote += NUM_DIRECT2;
            }
        }
    }
}

/// Return which disk sector is storing a particular byte within the file.
/// This is essentially a translation from a virtual address (the offset in
/// the file) to a physical address (the sector where the data at the offset
/// is stored).
///
/// * `offset` is the location within the file of the byte in question.
unsigned
FileHeader::ByteToSector(unsigned offset)   
{
    return *(GetSector(offset / SECTOR_SIZE));
}

unsigned*
FileHeader::GetSector(unsigned index)
{
    if (index < NUM_DIRECT)
        return &(raw.dataSectors[index]);

    index -= NUM_DIRECT;
    if (index < NUM_DIRECT2)
        return &(firstInd.dataSectors[index]);

    index -= NUM_DIRECT2;
    unsigned tabnum = index/NUM_DIRECT2;
    return &(secondIndArray[tabnum].dataSectors[index%NUM_DIRECT2]);
}

/// Return the number of bytes in the file.
unsigned
FileHeader::FileLength() const
{
    return raw.numBytes;
}

/// Print the contents of the file header, and the contents of all the data
/// blocks pointed to by the file header.
void
FileHeader::Print(const char *title)
{
    char *data = new char [SECTOR_SIZE];

    if (title == nullptr) {
        printf("File header:\n");
    } else {
        printf("%s file header:\n", title);
    }

    printf("    first indirection quantity: %u\n    second indirection quantity: %u\n", raw.fiQuantity, raw.siQuantity);
    if (raw.firstIndirection != -1)
        printf("    first indirection sector: %u\n", raw.firstIndirection);
    if (raw.secondIndirection != -1) {
        printf("    second indirection table sector: %u\n    second indirection sectors:\n", raw.secondIndirection);
        unsigned index = 0;
        for (unsigned i = 0; i < raw.siQuantity; i+=32)
        {
            printf("        %u\n", secondInd.dataSectors[index]);
            index++;
        }
        
    }
    printf("\n");
    printf("    size: %u bytes\n"
           "    block indexes: ",
           raw.numBytes);

    for (unsigned i = 0; i < raw.numSectors; i++) {
        printf("%u ", *GetSector(i));
    }
    printf("\n");
    for (unsigned i = 0, k = 0; i < raw.numSectors; i++) {
        printf("    contents of block %u:\n", *GetSector(i));
        synchDisk->ReadSector(*GetSector(i), data);
        for (unsigned j = 0; j < SECTOR_SIZE && k < raw.numBytes; j++, k++) {
            if (isprint(data[j])) {
                printf("%c", data[j]);
            } else {
                printf("\\%X", (unsigned char) data[j]);
            }
        }
        printf("\n");
    }
    delete [] data;
}

const RawFileHeader *
FileHeader::GetRaw() const
{
    return &raw;
}
