/// Test program to do matrix multiplication on large arrays.
///
/// Intended to stress virtual memory system.
///
/// Ideally, we could read the matrices off of the file system, and store the
/// result back to the file system!

#include "stdio.h"
#include "string.h"

/// Sum total of the arrays does not fit in physical memory.
#define DIM  20

static int A[DIM][DIM];
static int B[DIM][DIM];
static int C[DIM][DIM];

void printMatrix(int M[DIM][DIM]) {
    char tmp[5];

    print("┌ ");
    for (int j = 0; j < DIM; j++) print("\t");
    print("┐ ");
    print("\n");

    for (int i = 0; i < DIM; i++) {
        print("│ ");
        for (int j = 0; j < DIM; j++) {
            itoa(tmp, M[i][j]);
            print(tmp);
            print("\t");
        }
        print("│\n");
    }

    print("└ ");
    for (int j = 0; j < DIM; j++) print("\t");
    print("┘ ");
    print("\n");
}

int
main(void) {
    int i, j, k;

    // First initialize the matrices.
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM; j++) {
            A[i][j] = i;
            B[i][j] = j;
            C[i][j] = 0;
        }

    // Then multiply them together.
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM; j++)
            for (k = 0; k < DIM; k++)
                C[i][j] += A[i][k] * B[k][j];

    print("Multiplication finished.\n");

    // And then we are done.

    printMatrix(A);
    printMatrix(B);
    printMatrix(C);

    Exit(0);
}
