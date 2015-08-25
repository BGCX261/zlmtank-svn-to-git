
#ifndef _IRQ_H
#define _IRQ_H

// x86 STI 指令，允许中断
#define irq_enable()	\
	__asm__ (	\
		"sti"	\
		:	\
		:	\
	)

// x86 CLI 指令，禁止中断
#define irq_disable()	\
	__asm__ (	\
		"cli"	\
		:	\
		:	\
	)

// IRQ_NUM: 硬件中断类型
typedef enum {
	IRQ_TIME = 0,	// 日时钟
	IRQ_KBD = 1,	// 键盘
	IRQ_COM2 = 3,	// 串行口 2
	IRQ_COM1 = 4,	// 串行口 1
	IRQ_LPT2 = 5,	// 并行口 2
	IRQ_FD = 6,		// 软盘
	IRQ_LPT1 = 7,	// 并行口 1
	IRQ_CLOCK = 8,	// 实时时钟
	IRQ_MOUSE = 12,	// PS/2 鼠标
	IRQ_MATH = 13,	// 数学协处理器
	IRQ_HD = 14,	// 硬盘
	IRQ_MIN = 0,	// 最小值
	IRQ_MAX = 15,	// 最大值
} IRQ_NUM;

#define SYSCALL_INT  0x80

// 允许硬件中断 IRQn
extern void irq_enableirq(IRQ_NUM n);

// 禁止硬件中断 IRQn
extern void irq_disableirq(IRQ_NUM n);

// irq_init(): 初始化硬件中断服务
extern void irq_init(void);

// IRQ_FUN: 硬件中断处理程序类型
typedef void (*IRQ_FUN)(void);

// irq_setirqfun(): 给 #irq 指定处理程序 fn
extern int irq_setirqfun(IRQ_NUM irq, IRQ_FUN fn);

#endif
