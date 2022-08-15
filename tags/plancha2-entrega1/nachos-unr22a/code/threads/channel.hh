#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "synch_list.hh"
#include "system.hh"

class Channel {
public:

    Channel();

    ~Channel();

    void Send(int message);
    void Receive(int *message);

    private:
    Lock *sendLock;
    Lock *lock;
    Condition *receiveCondition;
    Condition *senderCondition;
    SynchList<int*> *lista;
};

#endif
