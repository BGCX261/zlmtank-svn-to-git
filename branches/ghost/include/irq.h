
#ifndef _IRQ_H
#define _IRQ_H

// x86 STI ָ������ж�
#define irq_enable()	\
	__asm__ (	\
		"sti"	\
		:	\
		:	\
	)

// x86 CLI ָ���ֹ�ж�
#define irq_disable()	\
	__asm__ (	\
		"cli"	\
		:	\
		:	\
	)

// IRQ_NUM: Ӳ���ж�����
typedef enum {
	IRQ_TIME = 0,	// ��ʱ��
	IRQ_KBD = 1,	// ����
	IRQ_COM2 = 3,	// ���п� 2
	IRQ_COM1 = 4,	// ���п� 1
	IRQ_LPT2 = 5,	// ���п� 2
	IRQ_FD = 6,		// ����
	IRQ_LPT1 = 7,	// ���п� 1
	IRQ_CLOCK = 8,	// ʵʱʱ��
	IRQ_MOUSE = 12,	// PS/2 ���
	IRQ_MATH = 13,	// ��ѧЭ������
	IRQ_HD = 14,	// Ӳ��
	IRQ_MIN = 0,	// ��Сֵ
	IRQ_MAX = 15,	// ���ֵ
} IRQ_NUM;

#define SYSCALL_INT  0x80

// ����Ӳ���ж� IRQn
extern void irq_enableirq(IRQ_NUM n);

// ��ֹӲ���ж� IRQn
extern void irq_disableirq(IRQ_NUM n);

// irq_init(): ��ʼ��Ӳ���жϷ���
extern void irq_init(void);

// IRQ_FUN: Ӳ���жϴ����������
typedef void (*IRQ_FUN)(void);

// irq_setirqfun(): �� #irq ָ��������� fn
extern int irq_setirqfun(IRQ_NUM irq, IRQ_FUN fn);

#endif
