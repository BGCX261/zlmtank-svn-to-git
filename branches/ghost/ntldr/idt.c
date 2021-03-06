#include <types.h>
#include <stdlib.h>
#include <idt.h>
#include <bootinfo.h>
#include <syscfg.h>

// i386 中断向量是一个 2K 大小的数组, 由 lidt 指定位置
// 每个项目 8 个字节，一共是 256 个向量号
// 其中向量号 0 - 31 是陷阱门，由 Intel 定义 CPU 异常
// 32 - 47 是中断门，与 IBM PC 两片 8259A 硬件中断号衔接
// 48 - 255 可以作为操作系统的系统调用
// 一般情况下 OS 只使用其中一个向量来作为系统调用号
// 每个门具有不同的属性、特权级

IDTGATE *IdtPtr = (void *)IDT_BASE_PA;
DWORD IdtNum = IDT_ITEM_NUM;

void idt_init(void)
{
	IDTR idtr = {0};
    memset(IdtPtr, 0, IdtNum * sizeof(IDTGATE));
	idtr.limit = sizeof(IDTGATE) * IDT_ITEM_NUM;
	idtr.base = (unsigned)IdtPtr;//我们的中断向量表位子

	// 设置 IDT
	/*
	__asm__ (
		"lidt (%0)"
		:
		:"r" ((unsigned char *)&idtr)
	);
	*/
		__asm__ __volatile__  ( "lidt %0" : "=m"( idtr ) ) ;

	return;
}

// idt_setgate(): 
// 指定 IDT 索引号 n 的服务程序地址 addr
// 其类型为 type 特权级为 dpl
void Idt_SetEntry(DWORD vector, BYTE desc_type, void* handler)
{
	DWORD base		=	(DWORD)handler;
	IDTGATE* desc		=	IdtPtr + vector;
	desc->offset_low	=	base & 0xFFFF;
	desc->selector		=	IDT_KERNEL_CODE_SEL;	//绯荤粺浠ｇ爜娈�
	desc->dcount		=	0;
	desc->attr		=	desc_type ;
	desc->offset_high	=	(base >> 16) & 0xFFFF;
}

