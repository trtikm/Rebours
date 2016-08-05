// Returns 1 if difference between counts of ('U' and 'u') and ('V' and 'v') are
// 10 and 15 respectively. Otherwise it returns 0.
int DOIFex(char const* const P) {
    int U = 0, V = 0, ic;
    for (ic = 0; P[ic] != 'T'; ++ic) {
        if (P[ic] == 'U') ++U;
        if (P[ic] == 'V') ++V;
    }
    for (ic = 0; P[ic] != 'T'; ++ic) {
        if (P[ic] == 'u') --U;
        if (P[ic] == 'v') --V;
    }
    return U == 10 && V == 15;
}

// Test driver

int main(int argc, char* argv[])
{
    if (argc != 2)
        return -1;
    if (DOIFex(argv[1]))
        return 1;
    return 0;
}
