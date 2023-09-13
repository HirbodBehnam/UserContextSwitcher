# Context switch.
# Read more here: https://wiki.osdev.org/System_V_ABI#x86-64
#include "coroutine_offsets.h"

.text
.align 8
.intel_syntax noprefix

.macro save_context
	push rbx
	push r12
	push r13
	push r14
	push r15
	push rbp
.endm

.macro restore_context
	pop rbp
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
.endm

.globl coroutine_start_inner
.type coroutine_start_inner, @function
coroutine_start_inner:
	# Save all registers
	save_context
	# Swap stack pointers
	mov [rdi + RETURN_STACK_POINTER_OFFSET], rsp # Save main stack pointer
	mov rsp, [rdi + COROUTINE_STACK_POINTER_OFFSET] # Load coroutine stack pointer
	# We dont care about the base pointer here
	# rdi is the pointer to coroutine struct
	# rsi is the pointer to the argument
	ret # This return will make a jump to coroutine

.globl coroutine_continue_inner
.type coroutine_continue_inner, @function
coroutine_continue_inner:
	# Save all registers
	save_context
	# Swap stack pointers
	mov [rdi + RETURN_STACK_POINTER_OFFSET], rsp # Save main stack pointer
	mov rsp, [rdi + COROUTINE_STACK_POINTER_OFFSET] # Load coroutine stack pointer
	# Restore registers
	restore_context
	ret # This return will make a jump to coroutine

.globl coroutine_yield
.type coroutine_yield, @function
coroutine_yield:
	# Store task registers
	save_context
	# Swap stack pointers
	mov [rdi + COROUTINE_STACK_POINTER_OFFSET], rsp # Save coroutine stack pointer
	mov rsp, [rdi + RETURN_STACK_POINTER_OFFSET] # Load main stack pointer
	# Restore registers
	restore_context
	ret # Return to main program

.globl coroutine_finish
.type coroutine_finish, @function
coroutine_finish:
	pop rdi # pop the address of coroutine
	mov DWORD PTR [rdi + COROUTINE_STATUS_OFFSET], COROUTINE_STATUS_DONE_VALUE # set status to done
	mov [rdi + COROUTINE_EXIT_VALUE_OFFSET], rax # save exit value
	jmp coroutine_yield
