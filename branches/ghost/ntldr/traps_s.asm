

segment .text	; �����
bits 32	; 32 λ����ģʽ

; ����ָ��
align 4
global divide_error
divide_error:
	; û�г������
	push byte 0
	; 0, ������� (����)
	push byte 0
	jmp exception

align 4
global debug
debug:
	; û�г������
	push byte 0
	; 1, ���Ժ͵��������ж� (����/����)
	push byte 1
	jmp exception

align 4
global nmi
nmi:
	; û�г������
	push byte 0
	; 2, �������ж�
	push byte 2
	jmp exception

align 4
global xint3
xint3:
	; û�г������
	push byte 0
	; 3, ���ֽ�INT3 (����)
	push byte 3
	jmp exception

align 4
global overflow
overflow:
	; û�г������
	push byte 0
	; 4, ��� (����)
	push byte 4
	jmp exception

align 4
global invalid_bound
invalid_bound:
	; û�г������
	push byte 0
	; 5,  �߽��� (����)
	push byte 5
	jmp exception

align 4
global invalid_opcode
invalid_opcode:
	; û�г������
	push byte 0
	; 6, �Ƿ�ָ�� (����)�����ı��������ɲ��˷Ƿ�ָ����񡣡���
	;������ֿ�һ����д������Ĵ���
	push byte 6
	jmp	exception

align 4
global no_coprocessor
no_coprocessor:
	; û�г������
	push byte 0
	; 7, Э�������������ж� (����)...(����Ҳ�����ã���ѧ�������
	push byte 7
	jmp exception

align 4
global double_fault
double_fault:
	; �г������
	nop
	nop
	; 8, ˫�ع��� (��ֹ)
	push byte 8
	jmp exception

align 4
global coprocessor_segment_overrun
coprocessor_segment_overrun:
	; û�г������
	push byte 0
	; 9, Э������Խ�� (��ֹ)
	push byte 9
	jmp exception

align 4
global invalid_tss
invalid_tss:
	; �г������
	nop
	nop
	; 10, ��Ч TSS (����)
	push byte 10
	jmp exception

align 4
global segment_not_present
segment_not_present:
	; �г������
	; ���� nop ��Ϊ�˺�һ�� push byte 0 ָ���ֽ������
	nop
	nop
	; 11, �β����� (����)
	push byte 11
	jmp exception

align 4
global stack_fault
stack_fault:
	; �г������
	nop
	nop
	; 12, ��ջ���쳣 (����)
	push byte 12
	jmp exception

align 4
global general_protection_fault
general_protection_fault:
	; �г������
	nop
	nop
	; 13, ͨ�ñ����쳣 (����)note:vware�ϴ��һ�Σ����ǻ��˸�bochs��û����,���
	push byte 13
	jmp exception

align 4
global page_fault
page_fault:
	; �г������
	nop
	nop
	; 14, ȱҳ�ж�(����)
	push byte 14
	jmp exception

align 4
global coprocessor_fault
coprocessor_fault:
	; û�г������
	push byte 0
	; 15, Э���������� (����)....����û�ã��Ǻ�
	push byte 15
	jmp exception

align 4
; exception: �쳣�������
global exception
exception:
	cld
	; ѹ�� 4 ����ѡ���ӼĴ���
	push gs
	push fs
	push es
	push ds
	; ѹ�� 8 ��ͨ�üĴ��� EAX ECX EDX EBX ESP EBP ESI EDI
	pusha
	; ȡ��ϵͳ���ݶ�ѡ����
SYS_DATASEL equ 0x10
	mov eax, SYS_DATASEL
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	; ���� C �������
extern doexception
	call doexception
	; ���� 8 ��ͨ�üĴ���
	popa
	; ���� 4 ���μĴ���
	pop ds
	pop es
	pop fs
	pop gs
	; �����쳣��ʹ�����
	add esp, 8
	iret

; traps_dodivide(): ��������������
global traps_dodivide
traps_dodivide:
	; 1 2 3 4 5 6 �źöӿ�������Щ���Ǻ�
	mov eax, 0
	mov ecx, 1
	mov edx, 2
	mov ebx, 3
	mov ebp, 4
	mov esi, 5
	mov edi, 6
	div eax
	ret

