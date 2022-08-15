/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_prod_cons.hh"
#include "synch_list.hh"
#include "system.hh"

#include <stdio.h>
#include <cstring>

static unsigned products = 0;
SynchList<Product*> *lista = new SynchList<Product*>();
Lock *lockB = new Lock("Lock buffer");
Condition *conditionB = new Condition("Condition buffer", lockB);


Product::Product(const char *name_, int number_) {
    number = number_;
    nameProducer = name_;
    printf("Producer %s: produced product %d\n", nameProducer, number);
}

Product::~Product(){}

void
Product::Consume(const char *nameConsumer){
    printf("Consumer %s: consumed product %d from %s\n", nameConsumer, number, nameProducer);
}

void
Producer(void *name_)
{
    const char *name = (char *) name_;

    for (unsigned i = 0; i < MAX_ITERS; ++i) {
        lockB->Acquire();

        while(products == BUFFER) {
            conditionB->Wait();
        }

        lista->Append(new Product(name, i));
        ++products;
        lockB->Release();
    }
}

void
Consumer(void *name_)
{
    const char *name = (char *) name_;

    for (unsigned i = 0; i < MAX_ITERS; ++i) {
        Product* product = lista->Pop();

        lockB->Acquire();
        --products;
        conditionB->Signal();
        product->Consume(name);
        delete product;
        lockB->Release();
    }
}

void
ThreadTestProdCons()
{
    for (unsigned i = 0; i < CANT_PRODUCERS; i++) {
        char *name = new char [64];
        sprintf(name, "Producer %d", i + 1);
        Thread *newThread = new Thread(name);
        newThread->Fork(Producer, (void *) name);
    }

    for (unsigned i = 0; i < CANT_CONSUMERS; i++) {
        char *name = new char [64];
        sprintf(name, "Consumer %d", i + 1);
        Thread *newThread = new Thread(name);
        newThread->Fork(Consumer, (void *) name);
    }

    currentThread->Yield();    
    currentThread->Finish();
}
