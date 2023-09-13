# Context switch
.text
.align 8
.intel_syntax noprefix
.globl start_task
.type start_task, @function
start_task:
	# Save all registers
	push rax
	push rbx
	push rcx
	push rdx
	# Swap stack pointers
	mov [rdi], rsp # Save main stack pointer
	mov rsp, [rdi+8] # Load coroutine stack pointer
	ret # This return will make a jump to coroutine

.globl switch_to_task
.type switch_to_task, @function
switch_to_task:
	# Save all registers
	push rax
	push rbx
	push rcx
	push rdx
	# Swap stack pointers
	mov [rdi], rsp # Save main stack pointer
	mov rsp, [rdi+8] # Load coroutine stack pointer
	# Restore registers
	pop rdx
    pop rcx
    pop rbx
    pop rax
	ret # This return will make a jump to coroutine

.globl yield_task
.type yield_task, @function
yield_task:
	# Store task registers
	push rax
    push rbx
    push rcx
    push rdx
    # Swap stack pointers
	mov [rdi+8], rsp # Save coroutine stack pointer
	mov rsp, [rdi] # Load main stack pointer
	# Restore registers
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret # Return to main program
