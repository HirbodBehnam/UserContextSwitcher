#include <stdio.h>
#include "coroutine.h"

void *counter(struct coroutine *this, void *arg) {
    for (int i = 0; i < 3; i++) {
        printf("%p: counter %d\n", arg, i);
        coroutine_yield(this);
    }
    return NULL;
}

// Main functions
int main() {
    struct coroutine coroutine1, coroutine2;
    puts("Starting scheduler...");
    coroutine_create(&coroutine1, 1024 * 1024);
    coroutine_create(&coroutine2, 1024 * 1024);
    // Start both of them
    coroutine_start(&coroutine1, counter, (void *) 0x1);
    coroutine_start(&coroutine2, counter, (void *) 0x2);
    while (coroutine1.status != STATUS_DONE) {
        coroutine_continue(&coroutine1);
        coroutine_continue(&coroutine2);
    }
    puts("Done!");
    printf("Exited with %p and %p\n", coroutine1.exit_value, coroutine2.exit_value);
    coroutine_free(&coroutine1);
    coroutine_free(&coroutine2);
    return 0;
}
