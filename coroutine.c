#include <stdlib.h>
#include <assert.h>
#include "coroutine.h"
#include "coroutine_offsets.h"

// Assembly functions

extern void coroutine_start_inner(struct coroutine *c, void *);

extern void coroutine_continue_inner(struct coroutine *c);

extern void coroutine_finish(void);

/**
 * Gets the initial top of stack pointer of a coroutine
 * @param c The coroutine to get its top of stack
 * @return The RSP
 */
static void initial_top_of_stack(struct coroutine *c) {
    // Remember that stack grows from top to bottom.
    // First 8 bytes are function address.
    // Next 8 bytes is the address to coroutine_finish method in switch.S
    // Last 8 bytes is the address of the coroutine itself.
    uint64_t stack_pointer = (uint64_t) &(c->stack[c->stack_size - 3 * 8]);
    // Read more: https://stackoverflow.com/a/51072200/4213397
    while (stack_pointer % 16 != 0)
        stack_pointer--;
    c->stack_pointer = stack_pointer;
}

/**
 * Setups the stack of the a coroutine
 * @param c The coroutine
 * @param f The initial function to call
 */
static void setup_stack(struct coroutine *c, void (f)(struct coroutine *, void *)) {
    initial_top_of_stack(c);
    uint64_t *stack = (uint64_t *) (c->stack_pointer);
    stack[0] = (uint64_t) f; // first ret will redirect us to this function
    stack[1] = (uint64_t) coroutine_finish; // after the given function is done this subroutine will be called
    stack[2] = (uint64_t) c; // pointer to coroutine
}

int coroutine_create(struct coroutine *c, size_t stack_size) {
    // Assert to don't fuck up in start_task_inner
    _Static_assert(offsetof(struct coroutine, return_stack_pointer) == RETURN_STACK_POINTER_OFFSET,
                   "return_stack_pointer");
    _Static_assert(offsetof(struct coroutine, stack_pointer) == COROUTINE_STACK_POINTER_OFFSET, "stack_pointer");
    _Static_assert(offsetof(struct coroutine, status) == COROUTINE_STATUS_OFFSET, "status");
    _Static_assert(COROUTINE_STATUS_DONE_VALUE == STATUS_DONE, "status value");
    // Allocate stack
    char *stack = malloc(stack_size);
    if (stack == NULL)
        return 1;
    // Set result
    c->stack = stack;
    c->stack_size = stack_size;
    c->stack_pointer = 0; // better be safe than sorry
    c->return_stack_pointer = 0; // not needed but whatever
    c->status = STATUS_CREATED;
    return 0;
}

int coroutine_start(struct coroutine *c, void (f)(struct coroutine *, void *), void *argument) {
    // Check coroutine status
    if (c->status != STATUS_CREATED)
        return 1;
    // Extract the address of function to execute
    setup_stack(c, f);
    assert(c->stack_pointer % 16 == 0);
    // Start it!
    c->status = STATUS_RUNNING;
    coroutine_start_inner(c, argument);
    return 0;
}

int coroutine_continue(struct coroutine *c) {
    if (c->status != STATUS_RUNNING)
        return 1;
    coroutine_continue_inner(c);
    return 0;
}

void coroutine_free(struct coroutine *c) {
    free(c->stack);
    c->stack = NULL;
    c->stack_size = 0;
    c->stack_pointer = 0;
    c->return_stack_pointer = 0;
    c->status = STATUS_INVALID;
}
