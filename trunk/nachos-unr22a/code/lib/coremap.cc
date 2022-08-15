#include "coremap.hh"

#include <stdio.h>

Coremap::Coremap(unsigned physPages)
{
    ASSERT(physPages > 0);
    numPhysPages = physPages;
    CoreMap = new AddressSpace*[numPhysPages];
    timers = new unsigned[numPhysPages];
    pages = new Bitmap(numPhysPages);
}

/// De-allocate a bitmap.
Coremap::~Coremap()
{
    delete[] CoreMap;
    delete[] timers;
    delete pages;
}
#ifdef SWAP
unsigned
Coremap::ReplacePage(AddressSpace* newSpace)
{
    int physIndex = pages->Find();

    if (physIndex == -1) {
        unsigned victim = GetVictim();
        AddressSpace* space = CoreMap[victim];
        space->SwapPage(space->GetPhysicalPageIndex(victim));
        physIndex = pages->Find();
        DEBUG('v', "Succesfully swapped, newP: %d\n", physIndex);
    }
    
    CoreMap[physIndex] = newSpace;
    return (unsigned) physIndex;
}
#endif
void
Coremap::Clear(AddressSpace* space)
{
    for (unsigned i=0;i<numPhysPages;++i) {
        if (CoreMap[i] == space)
            pages->Clear(i);
    }
}

unsigned
Coremap::GetVictim()
{
    DEBUG('v', "Getting Victims\n");
#ifdef USE_LRU
    unsigned victim, m = 0;
    for (unsigned i=0;i<numPhysPages;++i)
        if(timers[i] > m){
            victim = i;
            m = timers[i];
        }

    return victim;
#else
    return victimIndex++ % numPhysPages;
#endif
}
#ifdef USE_LRU
void
Coremap::UpdateTimers(unsigned pageUsed)
{
    DEBUG('v', "Updating Timers\n");
    for (unsigned i=0;i<numPhysPages;++i)
        timers[i]++;
    timers[pageUsed] = 0;
}
#endif

void
Coremap::ClearPageIndex(unsigned page)
{
    pages->Clear(page);
}