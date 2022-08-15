#include "file_table.hh"
#include "threads/lock.hh"

FileTable::FileTable()
{
    lock = new Lock("Read Write Lock");
    last = nullptr;
    first = nullptr;
}

FileTable::~FileTable()
{
    delete lock;
    ListEntry* aux;
    for (aux = first; aux != nullptr; aux = first)
    {
        first = first->next;
        delete aux->RWLock;
        delete aux;
    }
}

void
FileTable::LockAcquire() {
    lock->Acquire();
}

void
FileTable::LockRelease() {
    lock->Release();
}

FileLock* 
FileTable::OpenFile(int fSector){
    ListEntry* aux;
    for (aux = first; aux != nullptr; aux = aux->next)
    {
        if (aux->sector == fSector)
            break;
    }

    if (!aux) {
        aux = new ListEntry();
        aux->sector = fSector;
        aux->opened = 1;
        aux->next = nullptr;
        aux->toRemove = false;
        aux->RWLock = new FileLock();
        if (!first) {
            first = last = aux;
        } else {
            last->next = aux;
            last = aux;
        }
    } else {
        if (aux->toRemove)
            return nullptr;
        aux->opened++;
    }
    return aux->RWLock;
}

bool 
FileTable::CloseFile(int fSector) {
    ListEntry *aux, *father = nullptr;
    for (aux = first; aux != nullptr; father = aux, aux = aux->next)
    {
        if (aux->sector == fSector)
            break;
    }

    ASSERT(aux != nullptr);
    if (aux->opened > 1) {
        aux->opened--;
        return false;
    } else {
        bool toR = aux->toRemove;

        if (aux == first) {
            first = aux->next;
            if (aux == last)
                last = nullptr;
        } else {
            if (aux == last) {
                father->next = nullptr;
                last = father;
            } else
                father->next = aux->next;
        }

        delete aux->RWLock;
        delete aux;
        return toR;
    }
}

bool 
FileTable::SetRemove(int fSector) {
    ListEntry* aux;
    for (aux = first; aux != nullptr; aux = aux->next)
    {
        if (aux->sector == fSector)
            break;
    }

    if (!aux) {
        return true;
    } else {
        aux->toRemove = true;
        return false;
    }
}