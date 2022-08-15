/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_join.hh"
#include "system.hh"
#include "thread.hh"
#include "lib/list.hh"

#include <stdio.h>
#include <string.h>
#include <string>

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.

void
ChildThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    printf("!!! Thread `%s` has finished\n", name);
}



void
ThreadTestJoin()
{
    List<Thread*> *listathreads = new List<Thread*>();  
    for (unsigned i = 0; i < CHILD_THREADS; i++) {
        char *name = new char [64];
        sprintf(name, "%d", i + 1);
        Thread *newThread = new Thread(name, true);
        newThread->Fork(ChildThread, (void *) name);
        listathreads->Append(newThread);
    }

    while (!listathreads->IsEmpty()) {
        Thread* child = listathreads->Pop();
        child->Join();
    }

    printf("Finished father");
}
