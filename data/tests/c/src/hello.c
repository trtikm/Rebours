extern void* malloc(unsigned int n);
extern void free(void* p);

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

#include "./bugst_utils.h"

int main(void)//int argc, char* argv[])
{
    int N;
    BUGST_IGNORE_UNINITIALISED_INT(&N);
    if (N > 0)
    {
        elem_type const hello[] = { 72, 101, 108, 108, 111, 0 }; // "Hello\0"
        elem_type* const A = (elem_type*)malloc(N * sizeof(elem_type));
        A[N-1] = 0;
        BUGST_IGNORE_UNINITIALISED_INT_ARRAY(A,N-1);
        if (contains(A,hello))
        {
            BUGST_REPORT_TARGET_LOCATION_REACHED();
            BUGST_TERMINATE_WHOLE_PROGRAM_ANALYSIS();
        }
        free(A);
    }
    return 0;
}
