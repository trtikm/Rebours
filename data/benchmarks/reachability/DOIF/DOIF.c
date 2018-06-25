// Returns 1 if the passed 'T'-terminated string contains 'U' 10 times and 'V'
// 15 times. Otherwise it returns 0.
#include <stdio.h>

int DOIF(char const* const P) {
    int U = 0, V = 0, ic;
    for (ic = 0; P[ic] != 'T'; ++ic) {
        if (P[ic] == 'U') ++U;
        if (P[ic] == 'V') ++V;
    }
    return U == 10 && V == 15;
}

// Test driver

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        puts("Wrong count of paramenters.\n");
        return -1;
    }
        
    if (DOIF(argv[1]))
    {
        puts("The condition was satisfied.\n");
        return 1;
    }
    puts("The condition was NOT satisfied.\n");
    return 0;
}

