#include <types.h>
#include <stdlib.h>

#include <ntldr.h>
#include <gdt.h>
#include <idt.h>

#include <traps.h>


// 下列异常处理程序在 traps_s.asm 中定义
// 这些程序的实现不能被 C 语言等高级语言的函数实现
// 原因是高级语言做了大量额外的工作
// 另外中断服务程序使用指令 iret 返回
// 而不是 C 程序函数的 ret 返回指令
// 而下面这些陷阱藕们一般都不会被用上..甚至有的都不理解
extern void divide_error(void);
extern void debug(void);
extern void nmi(void);
extern void xint3(void);
extern void overflow(void);
extern void invalid_bound(void);
extern void invalid_opcode(void);
extern void no_coprocessor(void);
extern void double_fault(void);
extern void coprocessor_segment_overrun(void);
extern void invalid_tss(void);
extern void segment_not_present(void);
extern void stack_fault(void);
extern void general_protection_fault(void);
extern void page_fault(void);
extern void coprocessor_fault(void);

// emsg: 出错信息表
static const char *emsg[] = {
	"Divide Error",
	"Debug",
	"NMI",
	"INT3",
	"Overflow",
	"Bound",
	"Invalid Opcode",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Coprocessor Fault"
};

//缺页错误的处理函数
void trap_page_fault(void);



// reserced(): 保留处理函数
static void reserved(void)
{
	// "ERROR: IDT is reserved."
	NTLDR_PRINTF("ERROR: IDT is reserved\n");
	return;
}

// traps_init(): 初始化异常处理程序设置
void traps_init(void)
{
	int i;

	// 设置 INTEL i386 要求的 0 至 31 号异常处理程序
	idt_settrapgate(0, divide_error);
	idt_settrapgate(1, debug);
	idt_settrapgate(2, nmi);
	// 3 - 5 可以由用户程序使用(调试程序)
	//idt_setusergate(3, xint3);
	//idt_setusergate(4, overflow);
	//idt_setusergate(5, bound);
	idt_settrapgate(3, xint3);
	idt_settrapgate(4, overflow);
	idt_settrapgate(5, invalid_bound);

	
	// 6 - 15 仍是系统特权级
	idt_settrapgate(6, invalid_opcode);
	idt_settrapgate(7, no_coprocessor);
	idt_settrapgate(8, double_fault);
	idt_settrapgate(9, coprocessor_segment_overrun);
	idt_settrapgate(10, invalid_tss);
	idt_settrapgate(11, segment_not_present);
	idt_settrapgate(12, stack_fault);
	idt_settrapgate(13, general_protection_fault);
	idt_settrapgate(14, page_fault);
	idt_settrapgate(15, coprocessor_fault);
	// 余下的索引号被 INTEL 保留
	// 最新的 x86 处理器 PI/II/III/IV 使用 16, 17 等索引号
	
	for(i = 16; i < 32; i++)
		idt_settrapgate(i, reserved);

	return;
}

// doexception(): 异常处理 C 子程序
// 本函数供 traps_s.asm 的汇编服务程序调用

void doexception(IDT_REGS regs)
{
	/*

	if (regs.idx == 8) {
		// 处理系统异常的时候出现异常，估计是 misc_snprintf 的问题了
		// "Exception: 8, double fault"
		NTLDR_PRINTF("Exception: 8, double fault\n");
	} 
	if(regs.idx == 14)
	{
		trap_page_fault();
	}else {
		int n = 0;

#define BUF &misc_buf[n], sizeof(misc_buf) - n
		// "Exception: %d, %s\n"
		n += SNPRINTF(BUF,"Exception: %d, %s\n", regs.idx, emsg[regs.idx]);
		if (regs.errno)
			// "Error Code: %d "
			n += SNPRINTF(BUF, "Error Code: %d ", regs.errno);
		n += SNPRINTF(BUF, "FLG: %08X\n", regs.eflags);
		n += SNPRINTF(BUF, "EAX: %08X  ECX: %08X  EDX: %08X  EBX: %08X\n", regs.eax, regs.ecx, regs.edx, regs.ebx);
		n += SNPRINTF(BUF, "ESP: %08X  EBP: %08X  ESI: %08X  EDI: %08X\n", regs.esp, regs.ebp, regs.esi, regs.edi);
		n += SNPRINTF(BUF, " DS: %08X   ES: %08X   FS: %08X   GS: %08X\n", regs.ds, regs.es, regs.fs, regs.gs);
		n += SNPRINTF(BUF, "EIP: %08X   CS: %08X  uSP: %08X  uSS: %08X\n", regs.eip, regs.cs, regs.uesp, regs.uss);
		PUTS(misc_buf);
		*/
        NTLDR_PRINTF("Exception: %d, %s\n", regs.idx, emsg[regs.idx]);
		NTLDR_PRINTF("Error Code: %d ", regs.errno);
		NTLDR_PRINTF("FLG: %08X\n", regs.eflags);
		NTLDR_PRINTF("EAX: %08X  ECX: %08X  EDX: %08X  EBX: %08X\n", regs.eax, regs.ecx, regs.edx, regs.ebx);
		NTLDR_PRINTF("ESP: %08X  EBP: %08X  ESI: %08X  EDI: %08X\n", regs.esp, regs.ebp, regs.esi, regs.edi);
		NTLDR_PRINTF(" DS: %08X   ES: %08X   FS: %08X   GS: %08X\n", regs.ds, regs.es, regs.fs, regs.gs);
		NTLDR_PRINTF("EIP: %08X   CS: %08X   SP: %08X   SS: %08X\n", regs.eip, regs.cs, regs.uesp, regs.uss);
		NTLDR_HALT();
	/*
	register unsigned long x;
		__asm__ __volatile__ ("movl %%cr2,%0" : "=r" (x));
		
		int oldc=vga_textcolor(RED);

	misc_printf(" TRAP:%d.%s linaer 0x%08x\n",regs.idx,emsg[regs.idx],x);
	vga_textcolor(oldc);
	*/
	return;
}






#if 0
void trap_page_fault(void)
{
		register unsigned long x;
		__asm__ __volatile__ ("movl %%cr2,%0" : "=r" (x));
		
		kconsole_textcolor(VESA_RED);
		PRINTF(" TRAP: CR2=%08x\n",x);
		return;
}
#endif
