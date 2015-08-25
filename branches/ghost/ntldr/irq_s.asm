

segment .text	; �����
bits 32	; 32 λ����ģʽ

; ����ָ��
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

;ϵͳ������˳�����ж�����,����������,��Ȩ������;��������...
global irq_syscall
irq_syscall:
	push dword 0x80
	jmp irq

align 4
; exception: �쳣�������
global irq
irq:
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
extern doirq
	call doirq
	cli
	; ���� 8 ��ͨ�üĴ���
	popa
	; ���� 4 ���μĴ���
	pop ds
	pop es
	pop fs
	pop gs
	; �����ж���
	add esp, 4
	iret

;����Ĵ�����ʱû����,������.Ŀǰϵͳ�������ж���һ������,�����Ժ��û�������̵���ʱȨ�޲���
align 4
global systemcall
extern SystemCallTable
systemcall:
;//ϵͳ�����жϵײ㴦����
_SystemCallService:
	;//���￪���ж�Ӧ��û���⣬
	;//��Ӳ���жϷ���ʱ���ᱣ���߳����ں��µ������ġ�
	sti
	push	0x0	;//err_code
	push	0x80	;//int_no
	;//�����ֳ�
	pusha
	push	ds
	push	es
	push	fs
	push	gs
	;����eax, eax���е��ú�
	push	eax
	;//use kernel segment selector
	mov	ax,0x10
	mov	ds,ax
	mov	es,ax
	;//�ָ�%eax
	pop	eax
	;//�ѵ��ò���ѹջ,��ѹ4��,��ʵֻ����2��
	push	edi
	push	edx
	push	ecx
	push	ebx
	call	[SystemCallTable + eax * 4]
	add	esp,16
	;//���淵��ֵ
	;movl	eax, ret_val
	;//�ָ��ֳ�
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popa
	add	esp,8
	;//ȡ������ֵ
	;movl	ret_val, eax
	iret
