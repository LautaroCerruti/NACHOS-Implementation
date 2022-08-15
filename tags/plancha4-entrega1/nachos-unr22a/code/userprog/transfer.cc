/// Copyright (c) 2019-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"


void ReadBufferFromUser(int userAddress, char *outBuffer,
                        unsigned byteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outBuffer != nullptr);
    ASSERT(byteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
#ifdef USE_TLB
        unsigned i;
        for(i = 0;  i < NUMBER_OF_TRIES; ++i){
            if(machine->ReadMem(userAddress, 1, &temp)){
                userAddress++;
                break;
            }
        }
        ASSERT(i!=NUMBER_OF_TRIES);
#else
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
#endif
        *outBuffer = (unsigned char) temp;
        outBuffer++;
    } while (count < byteCount);
}

bool ReadStringFromUser(int userAddress, char *outString,
                        unsigned maxByteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outString != nullptr);
    ASSERT(maxByteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
#ifdef USE_TLB
        unsigned i;
        for(i = 0;  i < NUMBER_OF_TRIES; ++i){
            if(machine->ReadMem(userAddress, 1, &temp)){
                userAddress++;
                break;
            }
        }
        ASSERT(i!=NUMBER_OF_TRIES);
#else
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
#endif
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}

void WriteBufferToUser(const char *buffer, int userAddress,
                       unsigned byteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(buffer != nullptr);
    ASSERT(byteCount != 0);

    unsigned count = 0;
    do {
        int temp = (int)buffer[count];
        count++;
#ifdef USE_TLB
        unsigned i;
        for(i = 0;  i < NUMBER_OF_TRIES; ++i){
            if(machine->WriteMem(userAddress, 1, temp)){
                userAddress++;
                break;
            }
        }
        ASSERT(i!=NUMBER_OF_TRIES);
#else
        ASSERT(machine->WriteMem(userAddress++, 1, temp));
#endif
    } while (count < byteCount);
}

void WriteStringToUser(const char *string, int userAddress)
{
    ASSERT(userAddress != 0);
    ASSERT(string != nullptr);

    do {
        int temp = (int) *string;
#ifdef USE_TLB
        unsigned i;
        for(i = 0;  i < NUMBER_OF_TRIES; ++i){
            if(machine->WriteMem(userAddress, 1, temp)){
                userAddress++;
                break;
            }
        }
        ASSERT(i!=NUMBER_OF_TRIES);
#else
        ASSERT(machine->WriteMem(userAddress++, 1, temp));
#endif
    } while (*string++ != '\0');
}
