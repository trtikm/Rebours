#include <stdlib.h>
#include <assert.h>

typedef int elem_type;

int matrIR(elem_type const* const A[], int const m, int const n)
{
    int w = 0;
    for (int i = 0; i < m; ++i)
    {
        int k = 0;
        for (int j = i; j < n; ++j)
            if (A[i][j] > 10 && A[i][j] < 100)
                ++k;
        if (k > 15)
        {
            w = 1;
            break;
        }
    }
    return m > 20 && n > 20 && w == 1;
}

// Test driver

//extern void* malloc(unsigned int n);
//extern void free(void* p);

int main(int argc, char* argv[])
{
    int M,N;
    if (M > 0 && N > 0)
    {
        elem_type** A = (elem_type**)malloc(M * sizeof(elem_type*));
        for (int i = 0; i < M; ++i)
            A[i] = (elem_type*)malloc(N * sizeof(elem_type));
        if (matrIR(A,M,N))
        {
            assert(0);
        }
        for (int i = 0; i < M; ++i)
            free(A[i]);
        free(A);
    }
    return 0;
}
