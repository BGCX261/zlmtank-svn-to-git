#include <types.h>
#include <stdlib.h>
#include <bootinfo.h>
#include <syscfg.h>
#include <gdt.h>
#include <ntldr.h>

// ¹À¼Æ³ýÁËÕâÀïÓÃÒ²Ã»±ðµÄµØ·½ÓÃ
#define load_gdtr(n) \
__asm__("lgdt (%%eax)\n\t" \
::"a" (n)); 

//¸ã¸öÖ¸Õë±ãÓÚ¹ÜÀí
GDTITEM *GdtAddr = (GDTITEM *)GDT_BASE_PA;
DWORD GdtItemNum = GDT_ITEM_NUM;

void gdt_init(void) 
{
  GDTR gdtr = {0};

  gdtr.GdtAddr = GdtAddr;
  gdtr.GdtLengthLimit = sizeof(GDTITEM) * GdtItemNum;
  memset(gdtr.GdtAddr, 0, gdtr.GdtLengthLimit);
  // ÏµÍ³¶Î,4G¶¼ÄÜ¸ã
  Gdt_SetEntry( 1, 0x0000, 0xFFFFF, DA_CR | DA_32 | DA_LIMIT_4K );
  Gdt_SetEntry( 2, 0x0000, 0xFFFFF, DA_DRW | DA_LIMIT_4K | DA_32 );

  // ÓÃ»§Ì¬Ê¹ÓÃµÄ¶Î
  Gdt_SetEntry( 3, 0x0000, 0xFFFFF, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3 );
  Gdt_SetEntry( 4, 0x0000, 0xFFFFF, DA_DRW | DA_LIMIT_4K | DA_32 | DA_DPL3 );

  //load_gdtr((DWORD)&gdtr); 
  __asm__ __volatile__ ( "lgdt %0" : "=m"( gdtr ) ) ; //è½½å…¥GDTè¡¨
  //NTLDR_HALT();
}

void Gdt_SetEntry(DWORD vector, DWORD base, DWORD limit, DWORD attribute )
{
	GDTITEM* desc	=	GdtAddr + vector;
	desc->limit_low		=	limit & 0x0FFFF;
	desc->base_low		=	base & 0x0FFFF;
	desc->base_mid		= 	(base >> 16) & 0x0FF;	
	desc->attr1		=	attribute & 0xFF;
	desc->limit_high_attr2	=	( (limit >> 16) & 0x0F ) | ((attribute >> 8) & 0xF0);
	desc->base_high		=	(base >> 24) & 0x0FF;
}

