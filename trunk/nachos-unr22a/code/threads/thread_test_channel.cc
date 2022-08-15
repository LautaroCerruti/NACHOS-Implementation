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
static bool done[2];

void
ReceiveMessage(void *name_) 
{
    printf("Starting Receiver\n");
    char *name = (char *)name_;
    int *message = new int();
    for(int i = 0; i < 2; ++i) {
        channel->Receive(message);
        printf("Thread %s received %d\n", name, *message);
    }
    done[0] = true;
}

void
SendMessage(void *name_) 
{
    char *name = (char *)name_;
    channel->Send((int) name[0]);
    printf("Thread %s sent %d\n", name, (int) name[0]);
    done[1] = true;
}

void
ThreadTestChannel()
{
    channel = new Channel();

    char *name2 = new char [64];
    sprintf(name2, "%d", 10);
    Thread *receiveThread = new Thread(name2);
    receiveThread->Fork(ReceiveMessage, (void *) name2);

    char *name = new char [64];
    sprintf(name, "%d", 2);
    Thread *sendThread = new Thread(name);
    sendThread->Fork(SendMessage, (void *) name);

    channel->Send(1);

    currentThread->Yield();
    printf("Thread father finished sending\n");
    for (unsigned i = 0; i < 2; i++) {
        while (!done[i]) {
            currentThread->Yield();
        }
    }
    currentThread->Finish(1);
}
