#ifndef NACHOS_FILE_LOCK__HH
#define NACHOS_FILE_LOCK__HH

//#include "threads/lock.hh"

class Lock;
class Semaphore;

class FileLock {
public:

    FileLock();

    ~FileLock();

    void ReadAcquire();
    void ReadRelease();
    void WriteAcquire();
    void WriteRelease();

private:
    Lock* readersLock;
    Semaphore* room;
    Semaphore* turnstile;
    unsigned readers;
};


#endif
