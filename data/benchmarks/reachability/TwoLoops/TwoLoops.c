void TwoLoops(int n) {
    int i = 0, j = 0;
    while (i < n) i += 4;
    while (i != j + 7) j += 2;
}

// Test driver

int main(void)//int argc, char* argv[])
{
    int N;
    TwoLoops(N);
    return 0;
}
