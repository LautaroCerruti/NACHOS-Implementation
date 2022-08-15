/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_channel.hh"
#include "system.hh"
#include "thread.hh"
#include "lib/list.hh"
#include "channel.hh"

#include <stdio.h>
#include <string.h>
#include <string>

Channel *channel;

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.

void
RecieveMessage(void *name_) 
{
    char *name = (char *)name_;
    int *message = new int();
    channel->Receive(message);
    printf("Thread %s received %d\n", name, *message);
    currentThread->Finish();
}

void
ThreadTestChannel()
{
    channel = new Channel();
    for (unsigned i = 0; i < 4; i++) {
        char *name = new char [64];
        sprintf(name, "%d", i + 1);
        Thread *newThread = new Thread(name);
        newThread->Fork(RecieveMessage, (void *) name);
    }

    currentThread->Yield();

    channel->Send(1);
    printf("Thread father finished sending\n");
}
