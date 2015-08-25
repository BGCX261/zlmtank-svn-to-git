#include <types.h>
#include <stdlib.h>
#include <irq.h>
#include <io.h>
#include <ntldr.h>
#include <idt.h>

// 硬件中断 IDT 起始索引号
#define IRQBASE 0x20
// 每个 8259 有 8 个 IRQ
#define I8259NUM 8


// 8259 中断控制器
#define IRQ_MASTERBASE    0x20
#define IRQ_MASTERCTLMASK 0x21
#define IRQ_SLAVEBASE     0xA0
#define IRQ_SLAVECTLMASK  0xA1


// init8259(): 初始化 8259 中断控制器
// 大学微型计算机技术书上面有
// 本函数的处理过程是必须的，而且是唯一的
// IO 操作后必须空转 CPU 等待 8259 处理命令字
//
// ICW Initialization Command Words 初始化命令字
// OCW Operation Command Words 操作命令字
//
// 主片 从片 功能
// 20h  A0h  写入 ICW1 OCW2 OCW3
// 写入 ICW1 时 bit4 = 1
// 写入 OCW2 和 OCW3 bit4 = 0
// 根据 bit3 = 0 为 OCW2 否则 = 1 为 OCW3
//
// 主片 从片 功能
// 21h  A1h  写入 ICW2 ICW3 ICW4 和 OCW1
// 写入 ICW2 ICW3 ICW4 看 ICW1 选择后依次写入
// 没有操作 ICW1 默认为写入 OCW1
static void init8259(void)
{
  // 发 ICW1 命令字到 20h(主) 和 A0h(从) 端口
  // ICW1 是 8259 开始工作前必须的，命令字节如下：
  // bit0 = 1 指明向 21h/A1h 要发送 ICW4 命令(ICW2 ICW3 ICW4 顺序发送)
  // bit1 = 0 级连方式。两片 8259 级连，不是单片(8088)
  // bit3 = 0 边沿触发方式。= 1 为电平触发
  // bit4 = 1 必须等于 1， = 0 时为写入 OCW2 或 OCW3
  // 其他位保留为 0
  io_writebyte_p(IRQ_MASTERBASE, 0x11);
  io_writebyte_p(IRQ_SLAVEBASE, 0x11);
  
  // 根据 ICW1 bit0 = 1 依次发送 ICW2
  // ICW2 为中断向量字，设置硬件中断到 IDT 的中断向量号
  // bit3-7 为向量号高 5 位，低 3 位(bit0-2)由 8259 自动填写
  // 也就是说 ICW2 只能指定 8259 第一个引脚 IRQ0
  // 并且制定向量数为 8 的倍数
  // 这样 8259 会根据第一个引脚的向量号自动计算其他 7 个引脚
  // ICW2 ICW3 ICW4 使用端口 21h(主) 和 A1h(从)
  // 主片向量为 20h 到 27h
  io_writebyte_p(IRQ_MASTERCTLMASK, IRQBASE);
  // 从片向量紧接为 28h 到 2Fh
  io_writebyte_p(IRQ_SLAVECTLMASK, IRQBASE + I8259NUM);
  
  // 根据 ICW1 bit0 = 1 发送完 ICW2 后，必须继续发送 ICW3
  // ICW3 是级连命令字。主从片都需写入，含义不同
  // 主片 bit0-7 = 1 是表明对应的 IRQ0-7 接有从片
  // 04h: bit2 = 1 IRQ2 接从片
  io_writebyte_p(IRQ_MASTERCTLMASK, 0x04);
  // 从片 bit0-2 有效，说明其级连在主片那个引脚
  // bit3-7 保留，必须为 0
  // 02h = 2 接在主片 IRQ2 上
  io_writebyte_p(IRQ_SLAVECTLMASK, 0x02);
  
  // 根据 ICW1 bit0 = 1 发送完 ICW3 后，必须继续发送 ICW4
  // ICW4 中断方式字，设置 8259 的基本工作方式
  // bit0 = 1 CPU 为 16 位 80x86，= 0 是 8 位 8080/8085
  // bit1 = 1 自动中断结束， = 0 非自动
  // bit2 = 1 主片 = 0 从片
  // bit3 = 1 数据线缓冲 = 0 非缓冲方式
  // bit4 = 1 特殊全嵌套方式 = 0 普通全嵌套方式
  // bit5-7 保留，必须为 0
  // 普通全嵌套方式下，中断优先权从高到低顺序为
  // IRQ0 IRQ1 IRQ2  IRQ8 IRQ9 IRQ10 IRQ11 IRQ12
  // IRQ13 IRQ14 IRQ15  IRQ3 IRQ4 IRQ5 IRQ6 IRQ7
  io_writebyte_p(IRQ_MASTERCTLMASK, 0x01);
  io_writebyte_p(IRQ_SLAVECTLMASK, 0x01);
  ///? 以上关于 ICW3 引自《16/32 位微机原理、汇编语言及接口技术》p181
  ///? 钱晓捷、陈涛编著，机械工业出版社 2001.7 ISBN 7-111-08912-X
  ///? 似乎与这里的命令字 01h 要求不一样(主从片一样的命令字)
  
  // 根据 ICW1 bit0 = 1 发送 ICW2 ICW3 ICW4 完成
  
  // 默认发送为 OCW1 命令
  // OCW1 屏蔽命令字，内容写入中断屏蔽寄存器 IMR
  // bit0-7 对应 8259 引脚 IRQ0-7
  // = 1 屏蔽该中断，= 0 允许该中断
  // FFh 禁止所有
  io_writebyte_p(IRQ_MASTERCTLMASK, 0xFF);
  io_writebyte_p(IRQ_SLAVECTLMASK, 0xFF);
  return;
}

// irq_enableirq(): 允许 IRQn 硬件中断
// IRQn 主片      n+  从片
// 0    日时钟     8 实时时钟
// 1    键盘       9 改向 0Ah 中断
// 2    级连从片  10 保留
// 3    串行口 2  11 保留
// 4    串行口 1  12 PS/2 鼠标(以前保留)
// 5    并行口 2  13 数学协处理器
// 6    软盘      14 硬盘(以前保留)
// 7    并行口 1  15 保留
void irq_enableirq(IRQ_NUM n)
{
	unsigned char mask;
	unsigned int port;

	// 选择操作端口
	port = n < 8 ? IRQ_MASTERCTLMASK : IRQ_SLAVECTLMASK;
	// read: xxxx xxxx xxxx xxxx
	// 1<<n: 1111 1111 11n1 1111 : [n] = 0
	// mask: xxxx xxxx xx0x xxxx
	// 移位可以循环移位
	mask = io_readbyte_p(port) & (~(1 << n));
	io_writebyte_p(port, mask);
	return;
}

// irq_disableirq(): 屏蔽 IRQn 硬件中断
void irq_disableirq(IRQ_NUM n)
{
	unsigned char mask;
	unsigned int port;

	// 选择操作端口
	port = n < 8 ? IRQ_MASTERCTLMASK : IRQ_SLAVECTLMASK;
	// read: xxxx xxxx xxxx xxxx
	// 1<<n: 0000 0000 00n0 0000 : [n] = 1
	// mask: xxxx xxxx xx1x xxxx
	// 移位可以循环移位
	mask = io_readbyte_p(port) | (1 << n);
	io_writebyte_p(port, mask);
	return;
}

// 下列中断处理程序在 irq_s.asm 中定义
// 这些程序的实现不能被 C 语言等高级语言的函数实现
// 原因是高级语言做了大量额外的工作
// 另外中断服务程序使用指令 iret 返回
// 而不是 C 程序函数的 ret 返回指令
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// IRQ 需要放入 IDT 的中断服务处理程序
static IRQ_FUN irqfun[I8259NUM*2];

// unexpected(): 缺省硬件中断处理程序
static void unexpected(int irq)
{
	// 硬件中断处理程序类型原本是没有参数的。。。
	// "Unexpected IRQ #%d\n"
	NTLDR_PRINTF("Unexpected IRQ #%d\n", irq);
	return;
}

// irq_setirqfun(): 给 #irq 指定处理程序 fn
int irq_setirqfun(IRQ_NUM irq, IRQ_FUN fn)
{
	// #irq 是否正确
	if (irq < 0 || irq >= sizeof(irqfun) / sizeof(irqfun[0])) {
		// "Invalid IRQ #%d\n"
		NTLDR_PRINTF("Invalid IRQ #%d\n", irq);
	}

	// 不需要设置
	if (irqfun[irq] == fn)
		return 0;

	// 重复设置
	if( irqfun[irq] != (IRQ_FUN)unexpected) {
		// "That have set one handler on IRQ #%d\n"
		NTLDR_PRINTF("That have set one handler on IRQ #%d\n", irq);
	}

	// 操作前先禁止该硬件中断
	// 其实在这之前该硬件中断本来就是禁止的
	irq_disableirq(irq);
	//设置irq的处理函数
	irqfun[irq] = fn;
	// 主要是为了打开
	irq_enableirq(irq);

	return 0;
}

// irq_init(): 初始化硬件中断服务
void irq_init(void)
{
	int i;
	// 硬件中断芯片初始化
	init8259();
	// 设置 idt 服务程序
	idt_setintrgate(IRQBASE + 0, irq0);
	idt_setintrgate(IRQBASE + 1, irq1);
	idt_setintrgate(IRQBASE + 2, irq2);
	idt_setintrgate(IRQBASE + 3, irq3);
	idt_setintrgate(IRQBASE + 4, irq4);
	idt_setintrgate(IRQBASE + 5, irq5);
	idt_setintrgate(IRQBASE + 6, irq6);
	idt_setintrgate(IRQBASE + 7, irq7);
	idt_setintrgate(IRQBASE + 8, irq8);
	idt_setintrgate(IRQBASE + 9, irq9);
	idt_setintrgate(IRQBASE + 10, irq10);
	idt_setintrgate(IRQBASE + 11, irq11);
	idt_setintrgate(IRQBASE + 12, irq12);
	idt_setintrgate(IRQBASE + 13, irq13);
	idt_setintrgate(IRQBASE + 14, irq14);
	idt_setintrgate(IRQBASE + 15, irq15);
	// 初始化处理函数组，这里是自己赋值
	for (i = 0; i < sizeof(irqfun) / sizeof(irqfun[0]); i++)
		irqfun[i] = (IRQ_FUN)unexpected;
	//设置系统调用
extern int *systemcall;
extern void irq_syscall(void);
	idt_setusergate(SYSCALL_INT ,irq_syscall);
	return;
}

// ackicq(): 避免中断循环
static void ackirq(int irq)
{
#define ENABLEINT 0x20
	// 0x20 = 0010 0000b
	// bit3-4 = 00b -> OCW2 中断结束和优先权命令字
	// bit0-3 = 000b 取消优先权自动循环
	io_writebyte(IRQ_MASTERBASE, ENABLEINT);
	if(irq > 8)
		io_writebyte(IRQ_SLAVEBASE, ENABLEINT);
	return;
}
extern void sc_null();

// doirq(): 硬件中断处理 C 子程序
// 本函数供 irq_s.asm 的汇编服务程序调用
void doirq(IDT_REGS regs)
{
	irq_disableirq(regs.idx);
	ackirq(regs.idx);
	irq_disable();
	if (regs.idx == 0x80)
	{
		sc_null();
	}
	else
	{
		irqfun[regs.idx]();
	}
	irq_enable();
	irq_enableirq(regs.idx);
	return;
}





void* SystemCallTable[] = {
	//0
	sc_null
};

void sc_null()
{
	NTLDR_PRINTF("syscall: NULL");
}
