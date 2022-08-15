#ifndef NACHOS_USERPROG_SYNCHCONSOLE__HH
#define NACHOS_USERPROG_SYNCHCONSOLE__HH

#include "machine/console.hh"
#include "threads/lock.hh"
#include "threads/semaphore.hh"

/// The following class defines a "synchronous" console abstraction.
///
/// This class provides the abstraction that for any individual thread making
/// a request, it waits around until the operation finishes before returning.
class SynchConsole {
public:

    SynchConsole(const char *name);

    ~SynchConsole();

    void Read(char *data, unsigned size);
    void Write(char *data, unsigned size);

    void WriteDone();
    void ReadAvail();

private:
    Console *console; 
    Semaphore *readAvail;
    Semaphore *writeDone;
    Lock *lockRead;
    Lock *lockWrite;
};

#endif
