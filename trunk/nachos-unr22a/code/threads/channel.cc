#include "channel.hh"

Channel::Channel() {
    lock = new Lock("Receive lock");
    senderCondition = new Condition("Send Condition", lock);
    receiveCondition = new Condition("Receive Condition", lock);
    readyCondition = new Condition("Ready Condition", lock);
    buffer = nullptr;
}

Channel::~Channel() {
    delete lock;
    delete senderCondition;
    delete receiveCondition;
    delete readyCondition;
}

void
Channel::Receive(int *message) {
    lock->Acquire();
    DEBUG('t', "Thread \"%s\" trying to receive\n", currentThread->GetName());
    while (buffer != nullptr)
        senderCondition->Wait();

    buffer = message;

    receiveCondition->Signal();
    readyCondition->Wait();
    senderCondition->Signal();
    DEBUG('t', "Thread \"%s\" received message \"%d\"\n", currentThread->GetName(), *message);
    lock->Release();
}

void
Channel::Send(int message) {
    lock->Acquire();
    DEBUG('t', "Thread \"%s\" trying to send message \"%d\"\n", currentThread->GetName(), message);
    while (buffer == nullptr)
        receiveCondition->Wait();

    *buffer = message;
    buffer = nullptr;

    readyCondition->Signal();
    DEBUG('t', "Thread \"%s\" sent message \"%d\"\n", currentThread->GetName(), message);
    lock->Release();
}