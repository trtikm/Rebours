#include <stdlib.h>
#include <assert.h>

//extern void* malloc(unsigned int n);
//extern void free(void* p);

typedef int elem_type;

int containsAt(elem_type const* const A, int const k, elem_type const* const X)
{
    int i = 0;
    for ( ; A[k+i] != 0 && X[i] != 0 && A[k+i] == X[i]; ++i)
        ;
    if (X[i] != 0)
        return 0;
    return 1;
}

int contains(elem_type const* const A, elem_type const* const X)
{
    for (int i = 0; A[i] != 0; ++i)
        if (containsAt(A,i,X))
            return 1;
    return 0;
}

int main(int argc, char* argv[])
{
    int N;
    if (N > 0)
    {
        elem_type const hello[] = { 72, 101, 108, 108, 111, 0 }; // "Hello\0"
        elem_type* const A = (elem_type*)malloc(N * sizeof(elem_type));
        A[N-1] = 0;
        if (contains(A,hello))
        {
            assert(0);
        }
        free(A);
    }
    return 0;
}
