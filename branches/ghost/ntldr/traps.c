#include <types.h>
#include <stdlib.h>

#include <ntldr.h>
#include <gdt.h>
#include <idt.h>

#include <traps.h>


// �����쳣��������� traps_s.asm �ж���
// ��Щ�����ʵ�ֲ��ܱ� C ���Եȸ߼����Եĺ���ʵ��
// ԭ���Ǹ߼��������˴�������Ĺ���
// �����жϷ������ʹ��ָ�� iret ����
// ������ C �������� ret ����ָ��
// ��������Щ����ź��һ�㶼���ᱻ����..�����еĶ������
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

// emsg: ������Ϣ��
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

//ȱҳ����Ĵ�����
void trap_page_fault(void);



// reserced(): ����������
static void reserved(void)
{
	// "ERROR: IDT is reserved."
	NTLDR_PRINTF("ERROR: IDT is reserved\n");
	return;
}

// traps_init(): ��ʼ���쳣�����������
void traps_init(void)
{
	int i;

	// ���� INTEL i386 Ҫ��� 0 �� 31 ���쳣�������
	idt_settrapgate(0, divide_error);
	idt_settrapgate(1, debug);
	idt_settrapgate(2, nmi);
	// 3 - 5 �������û�����ʹ��(���Գ���)
	//idt_setusergate(3, xint3);
	//idt_setusergate(4, overflow);
	//idt_setusergate(5, bound);
	idt_settrapgate(3, xint3);
	idt_settrapgate(4, overflow);
	idt_settrapgate(5, invalid_bound);

	
	// 6 - 15 ����ϵͳ��Ȩ��
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
	// ���µ������ű� INTEL ����
	// ���µ� x86 ������ PI/II/III/IV ʹ�� 16, 17 ��������
	
	for(i = 16; i < 32; i++)
		idt_settrapgate(i, reserved);

	return;
}

// doexception(): �쳣���� C �ӳ���
// �������� traps_s.asm �Ļ�����������

void doexception(IDT_REGS regs)
{
	/*

	if (regs.idx == 8) {
		// ����ϵͳ�쳣��ʱ������쳣�������� misc_snprintf ��������
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
