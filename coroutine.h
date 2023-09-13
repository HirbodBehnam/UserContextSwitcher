#ifndef USERCONTEXTSWITCHER_COROUTINE_H
#define USERCONTEXTSWITCHER_COROUTINE_H

#include <stdint.h>
#include <stddef.h>

struct coroutine {
    // Actually, used in assembly.
    // The stack pointer of main thread.
    __attribute__((unused)) uint64_t return_stack_pointer;
    // The stack pointer of this thread
    uint64_t stack_pointer;
    // The stack
    char *stack;
    // Size of the stack
    size_t stack_size;
};

/**
 * Creates a coroutine to be executed
 * @param c The coroutine to initialize
 * @param stack_size The stack size of the task
 * @return 0 if ok or 1 if not ok (OOM)
 */
int coroutine_create(struct coroutine *c, size_t stack_size);

/**
 * Starts a task with a function
 * @param c The coroutine to start the function in
 * @param f The function to start executing
 */
void coroutine_start(struct coroutine *c, void (f)(void));

/**
 * Frees coroutine resources
 * @param c The coroutine to start the function in
 */
void coroutine_free(struct coroutine *c);

// Assembly functions
extern void coroutine_continue(struct coroutine *c);

extern void coroutine_yield(struct coroutine *this);

#endif //USERCONTEXTSWITCHER_COROUTINE_H
