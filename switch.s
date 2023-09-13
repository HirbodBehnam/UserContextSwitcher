# Context switch
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
	mov [rdi], rsp # Save main stack pointer
	mov rsp, [rdi+8] # Load coroutine stack pointer
	# We dont care about the base pointer here
	ret # This return will make a jump to coroutine

.globl coroutine_continue
.type coroutine_continue, @function
coroutine_continue:
	# Save all registers
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	# Swap stack pointers
	mov [rdi], rsp # Save main stack pointer
	mov rsp, [rdi+8] # Load coroutine stack pointer
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
	mov [rdi+8], rsp # Save coroutine stack pointer
	mov rsp, [rdi] # Load main stack pointer
	# Restore registers
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret # Return to main program
