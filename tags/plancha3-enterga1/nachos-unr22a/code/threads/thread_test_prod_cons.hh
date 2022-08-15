/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_THREADTESTPRODCONS__HH
#define NACHOS_THREADS_THREADTESTPRODCONS__HH

static const unsigned MAX_ITERS = 10;
static const unsigned BUFFER = 5;
static const unsigned CANT_CONSUMERS = 10;
static const unsigned CANT_PRODUCERS = 10;

class Product
{
    private:

    int number;

    const char* nameProducer;

    public:

    Product(const char *name, int number);

    ~Product();

    void Consume(const char *nameConsumer);
};

void ThreadTestProdCons();


#endif
