/// Routines to manage address spaces (memory for executing user programs).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "executable.hh"
#include "threads/system.hh"
#include "lib/coremap.hh"

#include <string.h>
#include <limits.h>
#include <stdio.h>

unsigned int AddressSpace::Translate(unsigned int virtualAddr)
{
  uint32_t page = virtualAddr / PAGE_SIZE;
  uint32_t offset = virtualAddr % PAGE_SIZE;
  uint32_t physicalPage = pageTable[page].physicalPage;
  return physicalPage * PAGE_SIZE + offset;
}

AddressSpace::AddressSpace(OpenFile *executable_file, int spaceId)
{
    ASSERT(executable_file != nullptr);

    Executable exe (executable_file);
    ASSERT(exe.CheckMagic());
    executableFile = executable_file;

    // How big is address space?

    unsigned size = exe.GetSize() + USER_STACK_SIZE;
      // We need to increase the size to leave room for the stack.
    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

#ifndef SWAP
    ASSERT(numPages <= pages->CountClear());
#endif
      // Check we are not trying to run anything too big -- at least until we
      // have virtual memory.

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    // First, set up the translation.

    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
        pageTable[i].virtualPage  = i;
#ifdef DEMAND_LOADING
        pageTable[i].physicalPage = UINT_MAX;
        pageTable[i].valid        = false;
        pageTable[i].isInSwap     = false;
#else
        pageTable[i].physicalPage = pages->Find();
        pageTable[i].valid        = true;
#endif
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
          // If the code segment was entirely on a separate page, we could
          // set its pages to be read-only.
    }

#ifdef SWAP
    swapFileName = new char[10];
    sprintf(swapFileName, "SWAP.%d", spaceId);
    DEBUG('v', "Creating swap file %s\n", swapFileName);
    fileSystem->Create(swapFileName, size);
    swapFile = fileSystem->Open(swapFileName);
#endif
#ifndef DEMAND_LOADING
    char *mainMemory = machine->GetMMU()->mainMemory;

    // Zero out the entire address space, to zero the unitialized data
    // segment and the stack segment.
    // memset(mainMemory, 0, size);

    for (unsigned i = 0; i < numPages; i++) {
        memset(&mainMemory[pageTable[i].physicalPage * PAGE_SIZE], 0, PAGE_SIZE);
    }

    // Then, copy in the code and data segments into memory.
    uint32_t codeSize = exe.GetCodeSize();
    uint32_t initDataSize = exe.GetInitDataSize();
    if (codeSize > 0) {
        unsigned i = 0;
        uint32_t virtualAddr = exe.GetCodeAddr();
        DEBUG('a', "Initializing code segment, at 0x%X, size %u\n", virtualAddr, codeSize);
        while (i < codeSize) {
            uint32_t addr = Translate(virtualAddr + i);
            exe.ReadCodeBlock(&mainMemory[addr], 1, i);
            i++;
        }
    }
    if (initDataSize > 0) {
        unsigned i = 0;
        uint32_t virtualAddr = exe.GetInitDataAddr();
        DEBUG('a', "Initializing data segment, at 0x%X, size %u\n", virtualAddr, initDataSize);
        while (i < initDataSize) {
            uint32_t addr = Translate(virtualAddr + i);
            exe.ReadDataBlock(&mainMemory[addr], 1, i);
            i++;
        }
    }
#endif
}

/// Deallocate an address space.
AddressSpace::~AddressSpace()
{
#ifndef SWAP
    for (unsigned i = 0; i < numPages; i++)
        pages->Clear(pageTable[i].physicalPage);
#endif
    delete [] pageTable;
#ifndef DEMAND_LOADING
    delete executableFile;
#endif
#ifdef SWAP
    coreMap->Clear(this);
    delete swapFile;
    fileSystem->Remove(swapFileName);
    delete swapFileName;
#endif
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++) {
        machine->WriteRegister(i, 0);
    }

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void
AddressSpace::SaveState() {
#ifdef SWAP
    for (unsigned i=0; i<TLB_SIZE; ++i)
        SyncTlbEntry(i);
#endif
}

#ifdef USE_TLB
void
invalidateTLB() {
    for (unsigned i = 0; i<TLB_SIZE;++i)
        machine->GetMMU()->tlb[i].valid = false;
}
#endif

/// On a context switch, restore the machine state so that this address space
/// can run.
void
AddressSpace::RestoreState()
{
#ifdef USE_TLB
    invalidateTLB();
#else
    machine->GetMMU()->pageTable     = pageTable;
    machine->GetMMU()->pageTableSize = numPages;
#endif
}

TranslationEntry
AddressSpace::GetPageTableEntry(unsigned vpn) {
    return pageTable[vpn];
}

TranslationEntry
AddressSpace::LoadPage(unsigned vpnadd, unsigned frame) {
    unsigned vpn = vpnadd * PAGE_SIZE;
    ASSERT(vpn >= 0);
    Executable exe (executableFile);
    uint32_t physicalAddressToWrite = frame * PAGE_SIZE;
    DEBUG('v', "Loading Page Frame %lu from file\n", frame);

    char *mainMemory = machine->GetMMU()->mainMemory;
    memset(&mainMemory[physicalAddressToWrite], 0, PAGE_SIZE);

    unsigned readed = 0;

    uint32_t codeSize = exe.GetCodeSize();
    uint32_t initDataSize = exe.GetInitDataSize();
    uint32_t virtualDataAddr = exe.GetInitDataAddr();

    if (codeSize > 0 && vpn < codeSize) {
        uint32_t toRead = (codeSize - vpn) < PAGE_SIZE ? (codeSize - vpn) : PAGE_SIZE;

        exe.ReadCodeBlock(&mainMemory[physicalAddressToWrite], toRead, vpn);

        readed += toRead; 
    }

    if (initDataSize > 0 && vpn + readed < virtualDataAddr + initDataSize &&
        readed != PAGE_SIZE) {

        uint32_t toRead = (virtualDataAddr + initDataSize - vpn) < PAGE_SIZE ?
                                (virtualDataAddr + initDataSize) - (vpn + readed)
                            :
                                PAGE_SIZE - readed;

        readed ? 
            exe.ReadDataBlock(&mainMemory[physicalAddressToWrite + readed], toRead,  0)
        :
            exe.ReadDataBlock(&mainMemory[physicalAddressToWrite], toRead, vpn - codeSize);

        readed += toRead; 
    }

    if(vpn > codeSize + initDataSize) {
        readed = PAGE_SIZE;
    };
    pageTable[vpnadd].valid = true;
    pageTable[vpnadd].isInSwap = false;
    pageTable[vpnadd].dirty = false;
    pageTable[vpnadd].use = false;
    pageTable[vpnadd].physicalPage = frame;
    return pageTable[vpnadd];
}

#ifdef SWAP

TranslationEntry
AddressSpace::LoadFromSwap(unsigned vpn, unsigned physIndex)
{
    DEBUG('v', "Loading from the swap \n");
    char *mainMemory = machine->GetMMU()->mainMemory;
    swapFile->ReadAt(&mainMemory[physIndex * PAGE_SIZE], PAGE_SIZE, PAGE_SIZE * vpn);

    pageTable[vpn].valid = true;
    pageTable[vpn].physicalPage = physIndex;
    return pageTable[vpn];
}

void
AddressSpace::SyncTlbEntry(unsigned entry)
{
    DEBUG('v', "Synching from TLB \n");
    TranslationEntry* tlb = machine->GetMMU()->tlb;
    if (tlb[entry].valid) {
        pageTable[tlb[entry].virtualPage].dirty = tlb[entry].dirty;
        pageTable[tlb[entry].virtualPage].use = tlb[entry].use;
    }

    tlb[entry].valid = false;
}

unsigned
AddressSpace::GetPhysicalPageIndex(unsigned victim) {
    for (unsigned i=0; i < numPages; ++i)
        if (pageTable[i].physicalPage == victim) return i;
    ASSERT(false);
    return 0;
}

void
AddressSpace::SwapPage(unsigned vpn) 
{
    pageTable[vpn].valid = false;
    coreMap->ClearPageIndex(pageTable[vpn].physicalPage);
    
    if (currentThread->space == this) {
        TranslationEntry *tlb = machine->GetMMU()->tlb;

        for(unsigned i=0; i<TLB_SIZE; ++i)
            if(tlb[i].physicalPage == pageTable[vpn].physicalPage)
                SyncTlbEntry(i);
    }
    
    if (pageTable[vpn].dirty) {
        char *mainMemory = machine->GetMMU()->mainMemory;
        swapFile->WriteAt(&mainMemory[pageTable[vpn].physicalPage * PAGE_SIZE], PAGE_SIZE, PAGE_SIZE * vpn);
        pageTable[vpn].isInSwap = true;
        DEBUG('v', "Swap saved %lu \n", pageTable[vpn].physicalPage);
    }
    pageTable[vpn].physicalPage = UINT_MAX;
}

#endif