#ifndef NACHOS_DIRECTORY_TABLE__HH
#define NACHOS_DIRECTORY_TABLE__HH

//#include "threads/lock.hh"

class Lock;

struct DirListEntry {
    int sector;
    unsigned opened;
    Lock* dirLock;
    DirListEntry* next;
};

class DirectoryTable {
public:

    DirectoryTable();

    ~DirectoryTable();

    void LockAcquire();
    void LockRelease();

    Lock* OpenDirectory(int fSector);

    void CloseDirectory(int fSector);

    bool CanRemove(int fSector);

private:
    Lock* lock;
    DirListEntry *first;
    DirListEntry *last;
};


#endif
