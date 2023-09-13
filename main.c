#include <stdio.h>
#include "coroutine.h"

struct coroutine my_coroutine;

void counter(void) {
    for (int i = 0; i < 10; i++) {
        printf("counter %d\n", i);
        coroutine_yield(&my_coroutine);
    }
    coroutine_yield(&my_coroutine);
}

// Main functions
int main() {
    printf("Hello, World!\n");
    coroutine_create(&my_coroutine, 1024 * 1024);
    // Jump to it!
    coroutine_start(&my_coroutine, counter);
    for (int i = 0; i < 5; i++)
        coroutine_continue(&my_coroutine);
    printf("Done!\n");
    coroutine_free(&my_coroutine);
    return 0;
}
