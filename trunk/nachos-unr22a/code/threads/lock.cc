/// Routines for synchronizing threads.
///
/// The implementation for this primitive does not come with base Nachos.
/// It is left to the student.
///
/// When implementing this module, keep in mind that any implementation of a
/// synchronization routine needs some primitive atomic operation.  The
/// semaphore implementation, for example, disables interrupts in order to
/// achieve this; another way could be leveraging an already existing
/// primitive.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "lock.hh"
#include "system.hh"


/// Dummy functions -- so we can compile our later assignments.

Lock::Lock(const char *debugName)
{
    name = debugName;
    semaphore = new Semaphore("Semaphore lock", 1);
    holder = nullptr;
    previousPriority = -1;
}

Lock::~Lock()
{
    delete semaphore;
}

const char *
Lock::GetName() const
{
    return name;
}

void
Lock::Acquire()
{
    ASSERT(!IsHeldByCurrentThread());
    DEBUG('s', "Thread \"%s\" aquiring lock \"%s\"\n", currentThread->GetName(), name);
    if (holder != nullptr && currentThread->GetPriority() < holder->GetPriority()) {
        previousPriority = holder->GetPriority();
        scheduler->TransferPriority(holder, currentThread->GetPriority());
        semaphore->PP();
    } else {
        semaphore->P();
    }
    holder = currentThread;
}

void
Lock::Release()
{
    ASSERT(IsHeldByCurrentThread());
    DEBUG('s', "Thread \"%s\" releasing lock \"%s\"\n", currentThread->GetName(), name);
    if (previousPriority != -1) {
        scheduler->TransferPriority(holder, previousPriority);
        previousPriority = -1;
    }
    holder = nullptr;
    semaphore->V();
}

bool
Lock::IsHeldByCurrentThread() const
{
    return holder == currentThread;
}
