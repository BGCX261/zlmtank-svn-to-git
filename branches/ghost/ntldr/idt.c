#include <types.h>
#include <stdlib.h>
#include <idt.h>
#include <bootinfo.h>
#include <syscfg.h>

// i386 �ж�������һ�� 2K ��С������, �� lidt ָ��λ��
// ÿ����Ŀ 8 ���ֽڣ�һ���� 256 ��������
// ���������� 0 - 31 �������ţ��� Intel ���� CPU �쳣
// 32 - 47 ���ж��ţ��� IBM PC ��Ƭ 8259A Ӳ���жϺ��ν�
// 48 - 255 ������Ϊ����ϵͳ��ϵͳ����
// һ������� OS ֻʹ������һ����������Ϊϵͳ���ú�
// ÿ���ž��в�ͬ�����ԡ���Ȩ��

IDTGATE *IdtPtr = (void *)IDT_BASE_PA;
DWORD IdtNum = IDT_ITEM_NUM;

void idt_init(void)
{
	IDTR idtr = {0};
    memset(IdtPtr, 0, IdtNum * sizeof(IDTGATE));
	idtr.limit = sizeof(IDTGATE) * IDT_ITEM_NUM;
	idtr.base = (unsigned)IdtPtr;//���ǵ��ж�������λ��

	// ���� IDT
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
// ָ�� IDT ������ n �ķ�������ַ addr
// ������Ϊ type ��Ȩ��Ϊ dpl
void Idt_SetEntry(DWORD vector, BYTE desc_type, void* handler)
{
	DWORD base		=	(DWORD)handler;
	IDTGATE* desc		=	IdtPtr + vector;
	desc->offset_low	=	base & 0xFFFF;
	desc->selector		=	IDT_KERNEL_CODE_SEL;	//系统代码段
	desc->dcount		=	0;
	desc->attr		=	desc_type ;
	desc->offset_high	=	(base >> 16) & 0xFFFF;
}

