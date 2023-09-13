#include <stdio.h>
#include "coroutine.h"

void *counter(struct coroutine *this, void *arg) {
    printf("arg is %p\n", arg);
    for (int i = 0; i < 10; i++) {
        printf("counter %d\n", i);
        coroutine_yield(this);
    }
    printf("Yielding one last time!\n");
    coroutine_yield(this);
    puts("Exiting...");
    return (void *) 0x100;
}

// Main functions
int main() {
    struct coroutine my_coroutine;
    printf("Hello, World!\n");
    coroutine_create(&my_coroutine, 1024 * 1024);
    // Jump to it!
    coroutine_start(&my_coroutine, counter, (void *) 0x200);
    while (my_coroutine.status != STATUS_DONE)
        coroutine_continue(&my_coroutine);
    printf("Done!\n");
    printf("Exited with %p\n", my_coroutine.exit_value);
    coroutine_free(&my_coroutine);
    return 0;
}
