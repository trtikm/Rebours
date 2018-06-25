// It always returns 0.
int EQCNTex(char const* const  A, char const* const B) {
    int nA = 0, nB = 0, i,j;
    for (i = 0; i < 3; ++i)
        for (j = 0; j < 3; ++j)
            if (A[i] == B[j]) ++nA;
    for (j = 0; j < 5; ++j)
        for (i = 0; i < 5; ++i)
            if (A[i] == B[j]) ++nB;
    return nA == 10 && nB == 13;
}

// Test driver

int main(int argc, char* argv[])
{
    if (argc != 2)
        return -1;
    if (EQCNTex(argv[1],argv[2]))
        return 1;
    return 0;
}
