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
        elem_type const H[] = { 'H', 'e', 'l', 'l', 'o', 0 };
        elem_type const W[] = { 'W', 'o', 'r', 'l', 'd', 0 };
        elem_type const T[] = { 'A', 't', 0 };
        elem_type const M[] = { 'M', 'i', 'c', 'r', 'o', 's', 'o', 'f', 't', '!', 0 };
        elem_type* const A = (elem_type*)malloc(N * sizeof(elem_type));
        A[N-1] = 0;
        int const h = contains(A,H);
        int const w = contains(A,W);
        int const t = contains(A,T);
        int const m = contains(A,M);
        if (h == 1 && w == 1 && t == 1 && m == 1)
        {
            assert(0);
        }
        free(A);
    }
    return 0;
}
