#include "synch_console.hh"

static void
readHandler(void *arg)
{
    ASSERT(arg != nullptr);
    SynchConsole *consola = (SynchConsole *)arg;
    consola->ReadAvail();
}

static void
writeHandler(void *arg)
{
    ASSERT(arg != nullptr);
    SynchConsole *console = (SynchConsole *)arg;
    console->WriteDone();
}

void
SynchConsole::ReadAvail()
{
    readAvail->V();
}

void
SynchConsole::WriteDone()
{
    writeDone->V();
}

SynchConsole::SynchConsole(const char *name) {
    console = new Console(nullptr, nullptr, readHandler, writeHandler, this);
    writeDone = new Semaphore("write done", 0);
    readAvail = new Semaphore("read avail", 0);
    lockWrite = new Lock("write console");
    lockRead = new Lock("read console");
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete writeDone;
    delete readAvail;
    delete lockWrite;
    delete lockRead;
}

void
SynchConsole::Write(char *buffer, unsigned size)
{
    lockWrite->Acquire();
    for(unsigned i = 0; i < size; ++i) {
        console->PutChar(buffer[i]);
        writeDone->P();
    }
    lockWrite->Release();
}

void
SynchConsole::Read(char *buffer, unsigned size)
{
    lockRead->Acquire();
    for(unsigned i = 0; i < size; ++i) {
        readAvail->P();
        buffer[i] = console->GetChar();
    }
    lockRead->Release();
}
