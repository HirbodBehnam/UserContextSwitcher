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
    // Status of this coroutine
    enum {
        STATUS_INVALID, STATUS_CREATED, STATUS_RUNNING, STATUS_DONE,
    } status;
    // Exit value of this coroutine
    void *exit_value;
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
 * @param f The function to start executing. The first argument will be the pointer to the coroutine, the
 * second pointer is given by user in argument parameter.
 * @param argument The argument to give to coroutine.
 * @return 0 if coroutine started and then either yielded or finished. 1 if it's not possible to run the
 * coroutine. For example a coroutine in RUNNING state is given as argument.
 */
int coroutine_start(struct coroutine *c, void *(f)(struct coroutine *, void *), void *argument);

/**
 * Continue the coroutine until it yields or finishes.
 * @param c The coroutine to continue or yield
 * @return 0 if the coroutine yielded or finished. Otherwise 1. 1 Means that the status of the coroutine
 * is not STATUS_RUNNING.
 */
int coroutine_continue(struct coroutine *c);

/**
 * Frees coroutine resources
 * @param c The coroutine to start the function in
 */
void coroutine_free(struct coroutine *c);

// Assembly functions

extern void coroutine_yield(struct coroutine *this);

#endif //USERCONTEXTSWITCHER_COROUTINE_H
