
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

// change to 0 and see that the clock cycles significantly increas
int secret = 1;

int main() {
    volatile int x = 0;
    int training_value = 1; // train predictor to expect 1

    // train predictor
    for (int i = 0; i < 1000000; i++) {
        if (training_value) x++;
        else x--;
    }

    // measure branch controlled by secret
    uint64_t t1 = __rdtsc();
    if (secret) x++;   // predictor expects this
    else x--;          // predictor mispredicts
    uint64_t t2 = __rdtsc();

    printf("Cycles: %llu\n", (unsigned long long)(t2 - t1));
    return 0;
}
