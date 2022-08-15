/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_simple.hh"
#include "semaphore.hh"
#include "system.hh"

#include <stdio.h>
#include <string.h>
#include <string>

#ifdef SEMAPHORE_TEST
    Semaphore semaphore("Simple Test Semaphore", 3);
#endif

bool done[NEW_THREADS];

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *pos_)
{
    // Reinterpret arg `name` as a string.
    unsigned *pos = (unsigned *) pos_;

    #ifdef SEMAPHORE_TEST
        semaphore.P();
        DEBUG('s',"Semaforo decrementado por hilo %s\n", currentThread->GetName());
    #endif

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n", currentThread->GetName(), num);
        currentThread->Yield();
    }

    #ifdef SEMAPHORE_TEST
        semaphore.V();
        DEBUG('s',"Semaforo incrementado por hilo %s\n", currentThread->GetName());
    #endif

    if (*pos >= 2)
        done[(*pos) - 2] = true;

    delete pos;

    printf("!!! Thread `%s` has finished\n", currentThread->GetName());

    currentThread->Yield();
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void
ThreadTestSimple()
{
    for (unsigned i = 0; i < NEW_THREADS; i++) {
        done[i] = false;
        char *name = new char [64];
        unsigned *pos = new unsigned();
        *pos = i + 2;
        sprintf(name, "%d", i + 2);
        Thread *newThread = new Thread(name);
        newThread->Fork(SimpleThread, pos);
    }

    unsigned *pos = new unsigned();
    *pos = 1;
    SimpleThread((void *) pos);

    for (unsigned i = 0; i < NEW_THREADS; ++i) {
        while(!done[i])
            currentThread->Yield();
    }
}
