

segment .text	; 代码段
bits 32	; 32 位保护模式

; 对齐指令
align 4
global irq0:
irq0:
	push byte 0
	jmp irq

align 4
global irq1:
irq1:
	push byte 1
	jmp irq

align 4
global irq2:
irq2:
	push byte 2
	jmp irq

align 4
global irq3:
irq3:
	push byte 3
	jmp irq

align 4
global irq4:
irq4:
	push byte 4
	jmp irq

align 4
global irq5:
irq5:
	push byte 5
	jmp irq

align 4
global irq6:
irq6:
	push byte 6
	jmp irq

align 4
global irq7:
irq7:
	push byte 7
	jmp irq

align 4
global irq8:
irq8:
	push byte 8
	jmp irq

align 4
global irq9:
irq9:
	push byte 9
	jmp irq

align 4
global irq10:
irq10:
	push byte 10
	jmp irq

align 4
global irq11:
irq11:
	push byte 11
	jmp irq

align 4
global irq12:
irq12:
	push byte 12
	jmp irq

align 4
global irq13:
irq13:
	push byte 13
	jmp irq

align 4
global irq14:
irq14:
	push byte 14
	jmp irq

align 4
global irq15:
irq15:
	push byte 15
	jmp irq

;系统调用先顺便走中断流程,有两个问题,门权限问题;参数问题...
global irq_syscall
irq_syscall:
	push dword 0x80
	jmp irq

align 4
; exception: 异常处理程序
global irq
irq:
	cld
	; 压入 4 个段选择子寄存器
	push gs
	push fs
	push es
	push ds
	; 压入 8 个通用寄存器 EAX ECX EDX EBX ESP EBP ESI EDI
	pusha
	; 取得系统数据段选择子
SYS_DATASEL equ 0x10
	mov eax, SYS_DATASEL
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	; 调用 C 处理程序
extern doirq
	call doirq
	cli
	; 弹出 8 个通用寄存器
	popa
	; 弹出 4 个段寄存器
	pop ds
	pop es
	pop fs
	pop gs
	; 弹出中断码
	add esp, 4
	iret

;下面的代码暂时没有用,有问题.目前系统调用与中断走一个流程,可能以后用户级别进程调用时权限不够
align 4
global systemcall
extern SystemCallTable
systemcall:
;//系统调用中断底层处理函数
_SystemCallService:
	;//这里开启中断应该没问题，
	;//当硬件中断发生时，会保存线程在内核下的上下文。
	sti
	push	0x0	;//err_code
	push	0x80	;//int_no
	;//保护现场
	pusha
	push	ds
	push	es
	push	fs
	push	gs
	;保存eax, eax里有调用号
	push	eax
	;//use kernel segment selector
	mov	ax,0x10
	mov	ds,ax
	mov	es,ax
	;//恢复%eax
	pop	eax
	;//把调用参数压栈,先压4个,其实只用了2个
	push	edi
	push	edx
	push	ecx
	push	ebx
	call	[SystemCallTable + eax * 4]
	add	esp,16
	;//保存返回值
	;movl	eax, ret_val
	;//恢复现场
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popa
	add	esp,8
	;//取出返回值
	;movl	ret_val, eax
	iret
