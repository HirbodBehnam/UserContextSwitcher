# Context switch.
# Read more here: https://wiki.osdev.org/System_V_ABI#x86-64
#include "coroutine_offsets.h"

.text
.align 8
.intel_syntax noprefix
.globl coroutine_start_inner
.type coroutine_start_inner, @function
coroutine_start_inner:
	# Save all registers
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	# Swap stack pointers
	mov [rdi+RETURN_STACK_POINTER_OFFSET], rsp # Save main stack pointer
	mov rsp, [rdi+COROUTINE_STACK_POINTER_OFFSET] # Load coroutine stack pointer
	# We dont care about the base pointer here
	# rdi is the pointer to coroutine struct
	# rsi is the pointer to the argument
	ret # This return will make a jump to coroutine

.globl coroutine_continue_inner
.type coroutine_continue_inner, @function
coroutine_continue_inner:
	# Save all registers
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	# Swap stack pointers
	mov [rdi+RETURN_STACK_POINTER_OFFSET], rsp # Save main stack pointer
	mov rsp, [rdi+COROUTINE_STACK_POINTER_OFFSET] # Load coroutine stack pointer
	# Restore registers
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret # This return will make a jump to coroutine

.globl coroutine_yield
.type coroutine_yield, @function
coroutine_yield:
	# Store task registers
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	# Swap stack pointers
	mov [rdi+COROUTINE_STACK_POINTER_OFFSET], rsp # Save coroutine stack pointer
	mov rsp, [rdi+RETURN_STACK_POINTER_OFFSET] # Load main stack pointer
	# Restore registers
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret # Return to main program
