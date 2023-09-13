#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

// Coroutines
#define COROUTINE_STACK_SIZE (1024 * 1024)

struct coroutine {
    uint64_t return_stack_pointer;
    uint64_t stack_pointer;
    char stack[COROUTINE_STACK_SIZE];
};

/**
 * Gets the initial top of stack pointer of a coroutine
 * @param c The coroutine to get its top of stack
 * @return The RSP
 */
uint64_t initial_top_of_stack(struct coroutine *c) {
    // Remember that stack grows from top to bottom.
    // First 8 bytes are function address
    uint64_t stack_pointer = (uint64_t) &(c->stack[COROUTINE_STACK_SIZE - 8]);
    // Read more: https://stackoverflow.com/a/51072200/4213397
    while (stack_pointer % 16 != 0)
        stack_pointer--;
    return stack_pointer;
}

void extract_function_address(void (f)(void), char *stack) {
    const uint64_t function_address = (uint64_t) f;
    const uint32_t low = function_address & UINT32_MAX, high = (function_address >> 32) & UINT32_MAX;
    uint32_t *casted_stack = (uint32_t *) stack;
    casted_stack[0] = low;
    casted_stack[1] = high;
}

// Assembly functions
extern void start_task(struct coroutine *c);

extern void switch_to_task(struct coroutine *c);

extern void yield_task(struct coroutine *this);

// Program coroutines
struct coroutine my_coroutine;

void counter(void) {
    for (int i = 0; i < 10; i++) {
        printf("counter %d\n", i);
        yield_task(&my_coroutine);
    }
    yield_task(&my_coroutine);
}

// Main functions
int main() {
    _Static_assert(offsetof(struct coroutine, return_stack_pointer) == 0, "return_stack_pointer");
    _Static_assert(offsetof(struct coroutine, stack_pointer) == 8, "stack_pointer");
    _Static_assert(offsetof(struct coroutine, stack) == 16, "stack");
    printf("Hello, World!\n");
    my_coroutine.stack_pointer = initial_top_of_stack(&my_coroutine);
    assert(my_coroutine.stack_pointer % 16 == 0);
    extract_function_address(&counter, (char *) (my_coroutine.stack_pointer));
    // Jump to it!
    start_task(&my_coroutine);
    for (int i = 0; i < 5; i++)
        switch_to_task(&my_coroutine);
    printf("Done!\n");
    return 0;
}
