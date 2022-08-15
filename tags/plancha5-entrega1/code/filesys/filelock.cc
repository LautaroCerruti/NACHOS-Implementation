#include "filelock.hh"
#include "threads/lock.hh"

FileLock::FileLock()
{
    readersLock = new Lock("Readers Lock");
    room = new Semaphore("Readers Semaphore", 1);
    turnstile = new Semaphore("Turnstile Semaphore", 1);
    readers = 0;
}

FileLock::~FileLock()
{
    delete readersLock;
    delete room;
    delete turnstile;
}

void
FileLock::WriteAcquire() {
    turnstile->P();
    room->P();
}

void
FileLock::WriteRelease() {
    turnstile->V();
    room->V();
}

void
FileLock::ReadAcquire() {
    turnstile->P();
    turnstile->V();
    readersLock->Acquire();
    readers++;
    if (readers == 1)
        room->P();
        
    readersLock->Release();
}

void
FileLock::ReadRelease() {
    readersLock->Acquire();
    readers--;
    if (readers == 0)
        room->V();
    readersLock->Release();
}