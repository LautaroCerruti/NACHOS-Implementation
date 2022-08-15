#include "channel.hh"

Channel::Channel() {
    lock = new Lock("Receive lock");
    senderCondition = new Condition("Send Condition", lock);
    receiveCondition = new Condition("Receive Condition", lock);
}

Channel::~Channel() {
    delete lock;
    delete senderCondition;
    delete receiveCondition;
}

void
Channel::Receive(int *message) {
    lock->Acquire();
    
    while (buffer != nullptr)
        senderCondition->Wait();

    buffer = message;

    receiveCondition->Signal();
    senderCondition->Wait();
    buffer = nullptr;
    DEBUG('t', "Thread \"%s\" received message \"%d\"\n", currentThread->GetName(), *message);
    lock->Release();
}

void
Channel::Send(int message) {
    lock->Acquire();
    
    while (buffer == nullptr)
        receiveCondition->Wait();

    *buffer = message;

    senderCondition->Signal();
    DEBUG('t', "Thread \"%s\" sent message \"%d\"\n", currentThread->GetName(), message);
    lock->Release();
}