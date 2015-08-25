#ifndef _IDT_H
#define _IDT_H

#include <gdt.h>

// 数据结构按字节对齐，即不自动扩展对齐
//#pragma pack(push, 1)

// IDTGATE: 中断服务门，8 个字节
// i386 下，数据结构基地址必须是 4 的倍数(32 位 DWORD)
// i386 有三种门：硬件中断/系统调用、异常/陷阱、任务切换
// 所谓门只是切换的意思，从一个地方切换另一个地方，通过门
// 切换的原因是硬件设备独立运行时想报告到 CPU (中断)
// 正在执行的指令发生异常，无法继续正常运行 (陷阱)
// 用户程序要使用系统内核中实现的功能 (调用)
// 这些都会打断 CPU 正常的指令执行序列(或指令执行状态)
// 那么，i386 使用一个全局的数组为每一种情况制定处理方式
// 每一个处理项目就是一个门，遇到什么进什么门
// 当发生这些东东，CPU 停止当前的动作，保护现场状态
// 根据门的信息会设置 CPU 的状态，并转到门的那一头开始执行
// 等到门里面指令执行完毕后转回门的另一头，继续原先的指令
// 使用门而不是直接用指令跳转或调用是因为：门是不确定的
// 无法预先获知什么时候硬件发出信息以及指令执行异常
// 另外调用门需要 CPU 从用户态转到内核态(ring3 -> ring0)
// 来回通过门时，CPU 也需要进行一系列的既定操作
#if 0
typedef struct {
	// 中断执行代码偏移量的低 16 位
	unsigned short offset_low;
	// 选择器，也就是寄存器
	// 存在于 GDT 中的偏移值
	// 任务门此处为 TSS 段选择符
	unsigned short selector;
	// 保留位，始终为零
	unsigned char reserved;
	// IDT 中的门的类型
	// 当一个目标选择器没有能牵涉到正确的描述符类型时，就会引起异常 13
	unsigned char type:4;
	// 段标识位 = 0
	unsigned char segflag:1;
	// 中断门的权限等级 DPL
	unsigned char dpl:2;
	// 呈现标志位 P
	// 判定描述符内容是否有效 = 1 有效
	unsigned char present:1;
	// 中断执行代码偏移量的高 16 位
	unsigned short offset_high;
} IDTGATE;
#else
typedef struct {
  WORD offset_low;
  WORD selector;
  BYTE dcount;     //系统定义为reserved的
  BYTE attr;
  WORD offset_high;
}IDTGATE;

#endif

//#pragma pack(pop)



#if 0
// idt 服务程序索引类型
// 0101b i386 任务门 进行任务切换
#define TYPE_TASK 0x05

// 1100b i386 调用门 改变特权级别(这个调用门不在 IDT 中 GDT/LDT)
// 1110b i386 中断门 驱动硬件中断服务子程序
#define TYPE_INTERRUPT 0x0E
// 1111b i386 陷阱门 确定异常处理子程序
#define TYPE_TRAP 0x0F

// idt 服务程序特权级
// 00b ring0 表示内核级(ring好像是windows的说法,抄)
#define DPL_KERNEL 0
// 11b ring3 表示用户级
#define DPL_USER 3
//大家似乎都不用ring1,ring2,藕也不用
#endif

// idt_init(): 初始化 idt(设置 IDT 位置)
void idt_init(void);

// idt_setgate(): 设置 idt 服务
// 指定 IDT 索引号 n 的服务程序地址 addr
// 其类型为 type 特权级为 dpl
void Idt_SetEntry(DWORD vector, BYTE desc_type, void* handler);


// 设置任务门，特权级 0
//#define idt_settaskgate(n, addr) \
//	idt_setgate((n), TYPE_TASK, DPL_KERNEL, (unsigned)(addr))
#define idt_settaskgate(n, addr) \
	Idt_SetEntry((n), DA_TaskGate, (unsigned)(addr))


// 设置中断门，特权级 0
//#define idt_setintrgate(n, addr) \
//	idt_setgate((n), TYPE_INTERRUPT, DPL_KERNEL, (unsigned)(addr))
#define idt_setintrgate(n, addr) \
	Idt_SetEntry((n), DA_386IGate, (addr))

// 设置系统调用，特权级 3

#define idt_setusergate(n, addr) \
	Idt_SetEntry((n), DA_386IGate, (addr))


// 设置陷阱门，特权级 0
//#define idt_settrapgate(n, addr) \
//	idt_setgate((n), TYPE_TRAP, DPL_KERNEL, (unsigned)(addr))
#define idt_settrapgate(n, addr) \
	Idt_SetEntry((n), DA_386TGate, (addr))


// 设置用户级调用门，特权级 3
//#define idt_setusergate(n, addr) \
//	idt_setgate((n), TYPE_TRAP, DPL_USER, (unsigned)(addr))

// IDT_REGS: 中断处理寄存器入栈结构
typedef struct {
	// pusha 8 个通用寄存器
	unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
	// 分别 push 的 4 个段寄存器
	unsigned ds, es, fs, gs;
	// 中断索引号和错误号
	unsigned idx, errno;
	// 代码地址和段，标志寄存器，用户 ESP 和 SS
	unsigned eip, cs, eflags, uesp, uss;
} IDT_REGS;

typedef struct _idtr{
  // 表限长，字节
  unsigned short limit;
  // 基地址，实际物理地址
  unsigned long base;
}IDTR;


#endif
