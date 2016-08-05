typedef int elem_type;

int matrIR(elem_type const* const A, int const m, int const n)
{
    int w = 0;
    for (int i = 0; i < m; ++i)
    {
        int k = 0;
        for (int j = i; j < n; ++j)
            if (A[i*n+j] > 10 && A[i*n+j] < 100)
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

extern void* malloc(unsigned int n);
extern void free(void* p);

#include "./bugst_utils.h"

int main(void)//int argc, char* argv[])
{
    int M,N;
    BUGST_IGNORE_UNINITIALISED_INT(&M);
    BUGST_IGNORE_UNINITIALISED_INT(&N);
    if (M > 0 && N > 0)
    {
        elem_type A[M][N];
        BUGST_IGNORE_UNINITIALISED_INT_ARRAY(&A[0][0],M*N);
        if (matrIR(&A[0][0],M,N))
        {
            BUGST_REPORT_TARGET_LOCATION_REACHED();
            BUGST_TERMINATE_WHOLE_PROGRAM_ANALYSIS();
        }
    }
    return 0;
}
