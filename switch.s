# Context switch
.text
.align 8
.intel_syntax noprefix
.globl switch_to
.type switch_to, @function
switch_to:
	# Save all registers
	push rax
	push rbx
	push rcx
	push rdx
	# Swap stack pointers
	mov [rdi], rsp # Save main stack pointer
	mov rsp, [rdi+8] # Load coroutine stack pointer
	ret # This return will make a jump to coroutine

.globl yield
.type yield, @function
yield:
	mov [rdi+8], rsp # Save coroutine stack pointer
	mov rsp, [rdi] # Load main stack pointer
	# Restore registers
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret # Return to main program
