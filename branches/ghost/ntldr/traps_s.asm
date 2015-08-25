

segment .text	; 代码段
bits 32	; 32 位保护模式

; 对齐指令
align 4
global divide_error
divide_error:
	; 没有出错代码
	push byte 0
	; 0, 除零出错 (故障)
	push byte 0
	jmp exception

align 4
global debug
debug:
	; 没有出错代码
	push byte 0
	; 1, 调试和单步运行中断 (故障/陷阱)
	push byte 1
	jmp exception

align 4
global nmi
nmi:
	; 没有出错代码
	push byte 0
	; 2, 非屏蔽中断
	push byte 2
	jmp exception

align 4
global xint3
xint3:
	; 没有出错代码
	push byte 0
	; 3, 单字节INT3 (陷阱)
	push byte 3
	jmp exception

align 4
global overflow
overflow:
	; 没有出错代码
	push byte 0
	; 4, 溢出 (陷阱)
	push byte 4
	jmp exception

align 4
global invalid_bound
invalid_bound:
	; 没有出错代码
	push byte 0
	; 5,  边界检查 (故障)
	push byte 5
	jmp exception

align 4
global invalid_opcode
invalid_opcode:
	; 没有出错代码
	push byte 0
	; 6, 非法指令 (故障)正常的编译器生成不了非法指令，好像。。。
	;最近发现可一故意写个出错的代码
	push byte 6
	jmp	exception

align 4
global no_coprocessor
no_coprocessor:
	; 没有出错代码
	push byte 0
	; 7, 协处理器不可用中断 (故障)...(有我也不会用，数学差。。。）
	push byte 7
	jmp exception

align 4
global double_fault
double_fault:
	; 有出错代码
	nop
	nop
	; 8, 双重故障 (中止)
	push byte 8
	jmp exception

align 4
global coprocessor_segment_overrun
coprocessor_segment_overrun:
	; 没有出错代码
	push byte 0
	; 9, 协处理器越界 (中止)
	push byte 9
	jmp exception

align 4
global invalid_tss
invalid_tss:
	; 有出错代码
	nop
	nop
	; 10, 无效 TSS (故障)
	push byte 10
	jmp exception

align 4
global segment_not_present
segment_not_present:
	; 有出错代码
	; 两个 nop 是为了和一个 push byte 0 指令字节码对齐
	nop
	nop
	; 11, 段不存在 (故障)
	push byte 11
	jmp exception

align 4
global stack_fault
stack_fault:
	; 有出错代码
	nop
	nop
	; 12, 堆栈段异常 (故障)
	push byte 12
	jmp exception

align 4
global general_protection_fault
general_protection_fault:
	; 有出错代码
	nop
	nop
	; 13, 通用保护异常 (故障)note:vware上错过一次，但是换了个bochs就没错了,奇怪
	push byte 13
	jmp exception

align 4
global page_fault
page_fault:
	; 有出错代码
	nop
	nop
	; 14, 缺页中断(故障)
	push byte 14
	jmp exception

align 4
global coprocessor_fault
coprocessor_fault:
	; 没有出错代码
	push byte 0
	; 15, 协处理器出错 (故障)....还是没用，呵呵
	push byte 15
	jmp exception

align 4
; exception: 异常处理程序
global exception
exception:
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
extern doexception
	call doexception
	; 弹出 8 个通用寄存器
	popa
	; 弹出 4 个段寄存器
	pop ds
	pop es
	pop fs
	pop gs
	; 弹出异常码和错误码
	add esp, 8
	iret

; traps_dodivide(): 产生除零错误代码
global traps_dodivide
traps_dodivide:
	; 1 2 3 4 5 6 排好队看的整齐些，呵呵
	mov eax, 0
	mov ecx, 1
	mov edx, 2
	mov ebx, 3
	mov ebp, 4
	mov esi, 5
	mov edi, 6
	div eax
	ret

