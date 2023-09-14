# User Context Switcher

Context switching in user space, a.k.a. coroutines in C programming language.

## What is a coroutine?

Coroutine is a function which can cooperatively give control back to a master thread, or in other words, yield.
In other programming languages you might have come across `async/await` pattern. These patterns tend to use coroutines
under the hood in most cases.

This pattern is very useful when you are doing asynchronous IO programming such
as [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html)
or [io_uring](https://man7.org/linux/man-pages/man7/io_uring.7.html).

### API

Five methods are available in order to interact with coroutines. I'll just copy and paste the comments from code here:

```c
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
 * second pointer is given by user in argument parameter. This function might optionally return something,
 * and later it can be seen in c->exit_value
 * @param argument The argument to give to coroutine.
 * @return 0 if coroutine started and then either yielded or finished. 1 if it's not possible to run the
 * coroutine. For example a coroutine in RUNNING state is given as argument.
 */
int coroutine_start(struct coroutine *c, void *(f)(struct coroutine *, void *), void *argument);

/**
 * Yield this current coroutine
 * @param this The coroutine to yield
 */
void coroutine_yield(struct coroutine *this);

/**
 * Continue the coroutine until it yields or finishes.
 * @param c The coroutine to continue or yield
 * @return 0 if the coroutine yielded or finished. Otherwise, 1. 1 Means that the status of the coroutine
 * is not STATUS_RUNNING.
 */
int coroutine_continue(struct coroutine *c);

/**
 * Frees coroutine resources
 * @param c The coroutine to start the function in
 */
void coroutine_free(struct coroutine *c);
```

The coroutine struct is defined like this:

```c
struct coroutine {
    // Actually, used in assembly.
    // The stack pointer of main thread.
    // DO NOT TOUCH THIS
    uint64_t return_stack_pointer;
    // The stack pointer of this thread
    // DO NOT TOUCH THIS
    uint64_t stack_pointer;
    // The stack
    // DO NOT TOUCH THIS
    char *stack;
    // Size of the stack
    // DO NOT TOUCH THIS
    size_t stack_size;
    // Status of this coroutine
    enum {
        STATUS_INVALID, STATUS_CREATED, STATUS_RUNNING, STATUS_DONE,
    } status;
    // Exit value of this coroutine
    void *exit_value;
};
```

## How does it work?

Here comes the fun part. Each coroutine has its own stack allocated in the program's heap. When using `coroutine_create`
the library allocates specified amount of memory using `calloc` to be used as stack. The usage of `calloc` over `malloc`
is for the fact that we don't want data leak in coroutine stacks. However, the stack is not initialized in this
function.

Upon calling `coroutine_start`, the top of stack is initialized like this:

1. Top of the stack gets allocated to the address of `f`.
2. Next to top is the address of an internal function called `coroutine_finish`, located in `switch.S`.
3. After that, the address of the given coroutine (`c`) is stored.

Library also makes sure that the stack pointer is aligned to 16 bytes. Then, we jump to `coroutine_start_inner` which is
a subroutine written in assembly.

According to [SysV ABI](https://wiki.osdev.org/System_V_ABI#x86-64), the only registers which need to be saved
are `rbx`, `rbp` and `r12` ~ `r15`. These registered are saved on the stack of the callee and a stack pointer swap is
performed, i.e. changing the stack pointer from real program's stack pointer to the coroutine's stack pointer. At
last, `ret` instruction is executed. This instruction pops the top of the stack and places it in `rip` and thus,
performs a jump. This jump basically acts as calling the given function `f`.

Now the coroutine is running! It is worth mentioning that `rdi` and `rsi` registers are pointing to `c` and `f`
respectively, so we don't need to change them upon calling `f` in regard to function arguments.

If a coroutine wants to yield, it calls `coroutine_yield` with the coroutine in which it is running in, a.k.a. the first
argument of the initial function. This function is defined in `switch.S`. Initially, it stores all variables on the
stack of coroutine, then it switches the stack pointer with the real stack pointer of the program, and restores all
registers of the main coroutine. A `ret` will cause the program to continue the execution of the coroutine which called
either `coroutine_start` or `coroutine_start`.

`coroutine_continue` will continue a paused coroutine. It does such by saving the callee registers, swapping the stack
pointer and then loading the coroutine's registers. Just like `coroutine_start` but with an additional step of loading
the saved registers. The `ret` in the function will cause the coroutine to start executing after the `coroutine_yield`
function which was called inside the coroutine. Thus, the coroutine resumes.

When the coroutine returns, the `ret` instruction will cause the CPU to pop the next 8 bytes from stack and jump to it.
This equals to the address of `coroutine_finish` method. This method is hidden from the user, in `switch.S` file. In
this function, the function will pop the last element in stack into `rdi` register which equals to the address of the
exited coroutine. The status of the coroutine struct is set to DONE and the returned value is saved. Moving forward,
the `coroutine_yield` method is called (jumped to) in order to switch back to main coroutine.

The main coroutine must check the status of each task after each call to `coroutine_yield` or even the first call
to `coroutine_start` to see whether it is done or not.

### FAQ

* Does it support multi threading? Yes, because there is no shared state between coroutines. You just need to ensure
  that you don't call `coroutine_continue` while the coroutine is running.
* Can you create a coroutine from another coroutine? Yes you can. But starting or continuing the coroutine will cause
  the program counter to return the calling point. So you can stack coroutines.
* Does this work on Windows? Probably not because Windows uses
  another [calling convention](https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170).
* Does this work on other ISA for example x86? No. This library only works with amd64 or x86_64 because it uses its
  assembly.