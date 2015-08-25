segment .text
bits 32

INT_SYSCALL_NO		equ		0x80
SYSCALL_NULL_NO		equ		0




; ∂‘∆Î÷∏¡Ó
align 4
global syscall_null
syscall_null:
	;mov		eax, SYSCALL_NULL_NO
	int		INT_SYSCALL_NO
	ret

