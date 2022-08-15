/// Test program to do matrix multiplication on large arrays.
///
/// Intended to stress virtual memory system.
///
/// Ideally, we could read the matrices off of the file system, and store the
/// result back to the file system!


#include "syscall.h"
#include "lib.c"


/// Sum total of the arrays does not fit in physical memory.
#define DIM  20

static int A[DIM][DIM];
static int B[DIM][DIM];
static int C[DIM][DIM];

int
main(void)
{
    int i, j, k;

    // First initialize the matrices.
    for (i = 0; i < DIM; i++) {
        for (j = 0; j < DIM; j++) {
            A[i][j] = i;
            B[i][j] = j;
            C[i][j] = 0;
        }
    }

    // Then multiply them together.
    for (i = 0; i < DIM; i++) {
        for (j = 0; j < DIM; j++) {
            for (k = 0; k < DIM; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    // And then we are done.
    // Imprimimos el resultado y luego llamamos a Halt para poder ver las estadisticas de ejecucion,
    // ya que al tener el sistema la consola activa, siempre hay interrupciones pendientes por lo que nunca finaliza por si solo
    // char res[10];
    // itoa(C[DIM - 1][DIM - 1], res);
    // puts2(res);
    // puts2("\n");
    Halt();
    return C[DIM - 1][DIM - 1];
}
