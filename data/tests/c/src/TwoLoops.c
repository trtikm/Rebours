void TwoLoops(int n) {
    int i = 0, j = 0;
    while (i < n) i += 4;
    while (i != j + 7) j += 2;
}

// Test driver

#include "./bugst_utils.h"

int main(void)//int argc, char* argv[])
{
    int N;
    BUGST_IGNORE_UNINITIALISED_INT(&N);
    TwoLoops(N);
    BUGST_REPORT_TARGET_LOCATION_REACHED();
    BUGST_TERMINATE_WHOLE_PROGRAM_ANALYSIS();
    return 0;
}
