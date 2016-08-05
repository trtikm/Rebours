// It always returns 0
int OneLoop(int n) {
    int i = 0;
    while (i < n) i += 4;
    return i == 15;
}

// Test driver

int main(int argc, char* argv[])
{
    if (argc != 2)
        return -1;
    if (OneLoop((int)argv[1][0]))
    { unsigned int i; for (i = 0U; 1; ++i); }
    return 0;
}

