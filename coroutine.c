#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "coroutine.h"

/**
 * Gets the initial top of stack pointer of a coroutine
 * @param c The coroutine to get its top of stack
 * @return The RSP
 */
static uint64_t initial_top_of_stack(struct coroutine *c) {
    // Remember that stack grows from top to bottom.
    // First 8 bytes are function address
    uint64_t stack_pointer = (uint64_t) &(c->stack[c->stack_size - 8]);
    // Read more: https://stackoverflow.com/a/51072200/4213397
    while (stack_pointer % 16 != 0)
        stack_pointer--;
    return stack_pointer;
}

/**
 * Extracts the address of a function and puts it on top of stack.
 * @param f The function to extract its address
 * @param stack Top of stack
 */
static void extract_function_address(void (f)(void), char *stack) {
    const uint64_t function_address = (uint64_t) f;
    const uint32_t low = function_address & UINT32_MAX, high = (function_address >> 32) & UINT32_MAX;
    uint32_t *casted_stack = (uint32_t *) stack;
    casted_stack[0] = low;
    casted_stack[1] = high;
}

extern void coroutine_start_inner(struct coroutine *c);

int coroutine_create(struct coroutine *c, size_t stack_size) {
    // Assert to don't fuck up in start_task_inner
    _Static_assert(offsetof(struct coroutine, return_stack_pointer) == 0, "return_stack_pointer");
    _Static_assert(offsetof(struct coroutine, stack_pointer) == 8, "stack_pointer");
    _Static_assert(offsetof(struct coroutine, stack) == 16, "stack");
    // Allocate stack
    char *stack = malloc(stack_size);
    if (stack == NULL)
        return 1;
    // Set result
    c->stack = stack;
    c->stack_size = stack_size;
    c->stack_pointer = initial_top_of_stack(c);
    assert(c->stack_pointer % 16 == 0);
    c->return_stack_pointer = 0; // not needed but whatever
    return 0;
}

void coroutine_start(struct coroutine *c, void (f)(void)) {
    // Extract the address of function to execute
    extract_function_address(f, (char *) (c->stack_pointer));
    // Start it!
    coroutine_start_inner(c);
}

void coroutine_free(struct coroutine *c) {
    free(c->stack);
    c->stack = NULL;
    c->stack_size = 0;
    c->stack_pointer = 0;
    c->return_stack_pointer = 0;
}