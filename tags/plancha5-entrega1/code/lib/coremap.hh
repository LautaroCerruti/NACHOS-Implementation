#ifndef NACHOS_LIB_COREMAP__HH
#define NACHOS_LIB_COREMAP__HH

#include "userprog/address_space.hh"
#include "lib/bitmap.hh"

class Coremap {
public:
    Coremap(unsigned numPhysPages);

    ~Coremap();

    unsigned ReplacePage(AddressSpace* newSpace);

    void Clear(AddressSpace* space);

    unsigned GetVictim();
    void UpdateTimers(unsigned pageUsed);

    void ClearPageIndex(unsigned pageUsed);

private:
    AddressSpace** CoreMap;
    unsigned* timers;
    unsigned numPhysPages;
#ifndef USE_LRU
    unsigned victimIndex = 0;
#endif
    Bitmap *pages;
};


#endif
