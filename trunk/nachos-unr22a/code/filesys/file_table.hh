#ifndef NACHOS_FILE_TABLE__HH
#define NACHOS_FILE_TABLE__HH

#include "filelock.hh"

struct ListEntry {
    int sector;
    bool toRemove;
    unsigned opened;
    FileLock* RWLock;
    ListEntry* next;
};

class FileTable {
public:

    FileTable();

    ~FileTable();

    void LockAcquire();
    void LockRelease();

    // returns nullpointer if the file that is being oppened was set to be removed
    FileLock* OpenFile(int fSector);

    // returns true if the file should be removed, false otherwise
    bool CloseFile(int fSector);

    // returns true if no other thread has the file open, otherwise returns false but sets toRemove flag
    bool SetRemove(int fSector);

private:
    Lock* lock;
    ListEntry *first;
    ListEntry *last;
};


#endif
