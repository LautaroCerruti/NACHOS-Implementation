#include "channel.hh"

Channel::Channel() {
    sendLock = new Lock("Send lock");
    lock = new Lock("Receive lock");
    senderCondition = new Condition("Send Condition", lock);
    receiveCondition = new Condition("Receive Condition", lock);
    lista = new SynchList<int*>();
}

Channel::~Channel() {
    delete sendLock;
    delete lock;
    delete senderCondition;
    delete receiveCondition;
    delete lista;
}

void
Channel::Receive(int *message) {
    lock->Acquire();
    lista->Append(message);
    senderCondition->Signal();
    receiveCondition->Wait();
    DEBUG('t', "Thread \"%s\" received message \"%d\"\n", currentThread->GetName(), *message);
    lock->Release();
}

void
Channel::Send(int message) {
    sendLock->Acquire();

    lock->Acquire();
    if(lista->IsEmpty()) {
        senderCondition->Wait();
    }

    while (!lista->IsEmpty()) {
        int *dir = lista->Pop();
        *dir = message;
    }

    receiveCondition->Broadcast();
    DEBUG('t', "Thread \"%s\" sent message \"%d\"\n", currentThread->GetName(), message);
    lock->Release();
    sendLock->Release();
    currentThread->Yield();
}