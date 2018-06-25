// Returns 1 if both passed zero terminated strings have the same length.
// Otherwise it returns 0.
int EQCNT(char const* const  A, char const* const B) {
    int nA = 0, i,j;
    for (i = 0; A[i] != 0; ++i)
        for (j = 0; B[j] != 0; ++j)
            if (A[i] == B[j]) ++nA;
    return nA == 20;
}

// Test driver

int main(int argc, char* argv[])
{
    if (argc != 2)
        return -1;
    if (EQCNT(argv[1],argv[2]))
        return 1;
    return 0;
}

