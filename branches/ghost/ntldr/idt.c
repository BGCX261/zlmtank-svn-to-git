#include <types.h>
#include <stdlib.h>
#include <idt.h>
#include <bootinfo.h>
#include <syscfg.h>

// i386 ÖÐ¶ÏÏòÁ¿ÊÇÒ»¸ö 2K ´óÐ¡µÄÊý×é, ÓÉ lidt Ö¸¶¨Î»ÖÃ
// Ã¿¸öÏîÄ¿ 8 ¸ö×Ö½Ú£¬Ò»¹²ÊÇ 256 ¸öÏòÁ¿ºÅ
// ÆäÖÐÏòÁ¿ºÅ 0 - 31 ÊÇÏÝÚåÃÅ£¬ÓÉ Intel ¶¨Òå CPU Òì³£
// 32 - 47 ÊÇÖÐ¶ÏÃÅ£¬Óë IBM PC Á½Æ¬ 8259A Ó²¼þÖÐ¶ÏºÅÏÎ½Ó
// 48 - 255 ¿ÉÒÔ×÷Îª²Ù×÷ÏµÍ³µÄÏµÍ³µ÷ÓÃ
// Ò»°ãÇé¿öÏÂ OS Ö»Ê¹ÓÃÆäÖÐÒ»¸öÏòÁ¿À´×÷ÎªÏµÍ³µ÷ÓÃºÅ
// Ã¿¸öÃÅ¾ßÓÐ²»Í¬µÄÊôÐÔ¡¢ÌØÈ¨¼¶

IDTGATE *IdtPtr = (void *)IDT_BASE_PA;
DWORD IdtNum = IDT_ITEM_NUM;

void idt_init(void)
{
	IDTR idtr = {0};
    memset(IdtPtr, 0, IdtNum * sizeof(IDTGATE));
	idtr.limit = sizeof(IDTGATE) * IDT_ITEM_NUM;
	idtr.base = (unsigned)IdtPtr;//ÎÒÃÇµÄÖÐ¶ÏÏòÁ¿±íÎ»×Ó

	// ÉèÖÃ IDT
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
// Ö¸¶¨ IDT Ë÷ÒýºÅ n µÄ·þÎñ³ÌÐòµØÖ· addr
// ÆäÀàÐÍÎª type ÌØÈ¨¼¶Îª dpl
void Idt_SetEntry(DWORD vector, BYTE desc_type, void* handler)
{
	DWORD base		=	(DWORD)handler;
	IDTGATE* desc		=	IdtPtr + vector;
	desc->offset_low	=	base & 0xFFFF;
	desc->selector		=	IDT_KERNEL_CODE_SEL;	//ç³»ç»Ÿä»£ç æ®µ
	desc->dcount		=	0;
	desc->attr		=	desc_type ;
	desc->offset_high	=	(base >> 16) & 0xFFFF;
}

