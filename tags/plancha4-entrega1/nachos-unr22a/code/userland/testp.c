#include "syscall.h"
#include "lib.c"


#define DIM  1024

static int A[DIM];

int
main(void)
{
    int i, j, tmp;

    for (i = 0; i < DIM; i++) {
        A[i] = i;
    }

    for (i = 0; i < DIM; i++) {
        A[i] = i;
    }
    
    // char res[10];
    // itoa(A[1023], res);
    // puts2(res);
    // puts2("\n");
    Halt();
    return A[1023];
}
