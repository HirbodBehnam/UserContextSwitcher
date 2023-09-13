#include <stdio.h>
#include "coroutine.h"


void counter(struct coroutine *this, void *arg) {
    printf("arg is %p\n", arg);
    for (int i = 0; i < 10; i++) {
        printf("counter %d\n", i);
        coroutine_yield(this);
    }
    printf("Yielding one last time!\n");
    coroutine_yield(this);
    puts("Exiting...");
}

// Main functions
int main() {
    struct coroutine my_coroutine;
    printf("Hello, World!\n");
    coroutine_create(&my_coroutine, 1024 * 1024);
    // Jump to it!
    coroutine_start(&my_coroutine, counter, NULL);
    while (my_coroutine.status != STATUS_DONE)
        coroutine_continue(&my_coroutine);
    printf("Done!\n");
    coroutine_free(&my_coroutine);
    return 0;
}
