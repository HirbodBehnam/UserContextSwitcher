#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

void assert_stack() {
    uint64_t rsp;
    asm volatile ("mov %0, rsp" : "=r" (rsp));
    assert(rsp % 16 == 0);
}

// Coroutines
#define COROUTINE_STACK_SIZE (1024 * 1024)

struct coroutine {
    uint64_t return_stack_pointer;
    uint64_t stack_pointer;
    char stack[COROUTINE_STACK_SIZE];
};

// Assembly functions
extern void switch_to(struct coroutine *c);

extern void yield(struct coroutine *this);

// Program coroutines
struct coroutine my_coroutine;

void counter(void) {
    for (int i = 0; i < 10; i++) {
        //puts("Kir?");
        printf("counter %d\n", i);
        yield(&my_coroutine);
    }
    yield(&my_coroutine);
}

// Main functions
void extract_function_address(void (f)(void), char *stack) {
    const uint64_t function_address = (uint64_t) f;
    const uint32_t low = function_address & UINT32_MAX, high = (function_address >> 32) & UINT32_MAX;
    uint32_t *casted_stack = (uint32_t *) stack;
    casted_stack[0] = low;
    casted_stack[1] = high;
}

int main() {
    assert_stack();
    _Static_assert(offsetof(struct coroutine, return_stack_pointer) == 0, "return_stack_pointer");
    _Static_assert(offsetof(struct coroutine, stack_pointer) == 8, "stack_pointer");
    _Static_assert(offsetof(struct coroutine, stack) == 16, "stack");
    printf("Hello, World!\n");
    // Remember that stack grows from top to bottom.
    // First 8 bytes are function address
    my_coroutine.stack_pointer = (uint64_t) &my_coroutine.stack[COROUTINE_STACK_SIZE - 8];
    assert((my_coroutine.stack_pointer + 8) % 16 == 0);
    extract_function_address(&counter, &my_coroutine.stack[COROUTINE_STACK_SIZE - 8]);
    // Jump to it!
    switch_to(&my_coroutine);
    printf("Done!\n");
    return 0;
}
