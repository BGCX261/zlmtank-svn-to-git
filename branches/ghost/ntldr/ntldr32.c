#include <types.h>
#include <vga.h>
#include <bootinfo.h>
#include <stdlib.h>
#include <dbg.h>

#include <gdt.h>
#include <idt.h>
#include <irq.h>
#include <traps.h>
#include <page.h>
#include <time.h>
#include <serial.h>
#include <ntldr.h>
#include <floppy.h>
#include <fat.h>
//�ոս���c���
//��ʱ������4M��һ����λ��
//�ոս��뱣��ģʽ
//GDTֻ��3�ű�
//code / data���ص�,����0-4G


//��Ҫ��������
//������ҳ,ϵͳʹ�ø�2G (0x80000000-0xffffffff)
//�û�����ʹ�õ�2G 0x00000000-0x7fffffff
//���µ���GDT,IDT,PTE

//�μ�syscfg.h
SYSTEMCONFIG syscfg = {0};




void ntldr_print_bootinfo();
void ntldr_init_syscfg();
void ntldr_test();


void ntldr_main()
{
  irq_disable();

  ntldr_init_syscfg();

  // ��ʼ������,�������������Ϣ
  InitSerialPort();

  //biosĬ�����ı�ģʽ,���ǳ�ʼ��һ��
  vga_init();
  
  // ����
  vga_clrscr();

  // ���ñ�׼����ӿ�
  SetStd((stdputs)vga_puts);

  
  ntldr_print_bootinfo();
  

  // ��ʼ��GDT
  gdt_init();

  //��ʼ��IDT
  idt_init();

  //��ʼ��irq
  irq_init();

  //��ʼ��trap
  traps_init();

  page_init();

  //��ʼ����ʱ��
  timer_init();

  floppy_init();

  //fat12_init();
  
  SerialTest();

  irq_enable();

  for (int ii = 0; ii <16;ii++)
  irq_enableirq(ii);

  ntldr_test();

  for(;;);
  
  return;
}

CHAR ntldr_buf[1000];
DWORD ntldr_printf(CHAR *format,...)
{
	int i;
	va_list args;

	va_start(args, format);
	i = vsnprintf(ntldr_buf, sizeof(ntldr_buf), format, args);
	va_end(args);

	///SerialWriteString(DBGPORT, ntldr_buf);
	vga_puts(ntldr_buf);
	return i;
}

CHAR ntldr_dbg_buf[1000];
DWORD ntldr_dbg(CHAR *format,...)
{
	int i;
	int color = 0;
	va_list args;

	va_start(args, format);
	i = vsnprintf(ntldr_dbg_buf, sizeof(ntldr_dbg_buf), format, args);
	va_end(args);

	///SerialWriteString(DBGPORT, ntldr_buf);
	color = vga_textcolor(RED);
	vga_puts(ntldr_dbg_buf);
	vga_textcolor(color);
	return i;
}

void ntldr_hlt(CHAR *file,DWORD line)
{
  NTLDR_PRINTF(" NTLDR32 : hlt...<file=%s, line=%d>",file, line);
  for(;;);
}


void ntldr_print_bootinfo()
{
  BYTE *ptr = (BYTE *)BOOTINFO_PTR;
  BOOTINFO *bootinfo = (BOOTINFO *)ptr;
  E820_MAP_ITEM *map_item_ptr = NULL;
  DWORD ii = 0;
  
  NTLDR_DBG(" bootdriver = %x\n",
  	     bootinfo->bootdriver);
  
  NTLDR_DBG(" e801 : memsize = %x (%d kb)\n",
  	     bootinfo->e801_mem_len,
  	     bootinfo->e801_mem_len);

  map_item_ptr = bootinfo->e820_map_ptr;

  NTLDR_DBG(" e820 : map num = %d \n",
  	     bootinfo->e820_map_num);
/*
  for (ii = 0; ii < bootinfo->e820_map_num; ii++)
  {
    NTLDR_DBG(" e820: 0x%x%x, 0x%x%x, 0x%x\n",
		   map_item_ptr->mem_base_up32,
		   map_item_ptr->mem_base_low32,
		   map_item_ptr->mem_len_up32,
		   map_item_ptr->mem_len_low32,
		   map_item_ptr->mem_type);
  }
*/

  // ��ӡ����ָ��
  NTLDR_DBG("KERNEL_BASE_LA=%x, KERNEL_BASE_PA=%x, size=%x\n",
            KERNEL_BASE_LA,
            KERNEL_BASE_PA,
            KERNEL_BASE_SIZE);

  NTLDR_DBG("BIOS_BASE_LA=%x, BIOS_BASE_PA=%x, size=%x\n",
            BIOS_BASE_LA,
            BIOS_BASE_PA,
            BIOS_BASE_SIZE);

  NTLDR_DBG("NTLDR_BASE_LA=%x, NTLDR_BASE_PA=%x, size=%x\n",
            NTLDR_BASE_LA,
            NTLDR_BASE_PA,
            NTLDR_BASE_SIZE);

  NTLDR_DBG("PTE_DIR_BASE_LA=%x, PTE_DIR_BASE_PA=%x, size=%x\n",
            PTE_DIR_BASE_LA,
            PTE_DIR_BASE_PA,
            PTE_DIR_BASE_SIZE);

  NTLDR_DBG("PTE_TAB_BASE_LA=%x, PTE_TAB_BASE_PA=%x, size=%x\n",
            PTE_TAB_BASE_LA,
            PTE_TAB_BASE_PA,
            PTE_TAB_BASE_SIZE);

  NTLDR_DBG("GDT_BASE_LA=%x, GDT_BASE_PA=%x, size=%x\n",
            GDT_BASE_LA,
            GDT_BASE_PA,
            GDT_BASE_SIZE);

  NTLDR_DBG("IDT_BASE_LA=%x, IDT_BASE_PA=%x, size=%x\n",
            IDT_BASE_LA,
            IDT_BASE_PA,
            IDT_BASE_SIZE);
}

SYSTEMCONFIG *ntldr_get_syscfg()
{
  return &syscfg;
}

void ntldr_init_syscfg()
{
  SYSTEMCONFIG *syscfg = ntldr_get_syscfg();

  // ����bootinfoָ��
  syscfg->boot_info = (BOOTINFO *)BOOTINFO_PTR;

  // ���õ�ǰ��������������
  syscfg->boot_drive_no = syscfg->boot_info->bootdriver;
  
  // ��bootinfo�����ȡ�ڴ�
  syscfg->phy_mem_size = (syscfg->boot_info->e801_mem_len) * 1024;

  //
  syscfg->phy_mem_min_size = SYSTEM_MIN_MEM_SIZE;

  // �����ڴ�����С,Ŀǰֻ����32λ����
  syscfg->linear_mem_max_size = SYSTEM_MAX_MEM_SIZE;

  // ��bootinfo�����ȡ���������������
}

#include <syscall.h>
void ntldr_test()
{
  //syscall
  syscall_null();

  //floppy
  BYTE buffer[512] = {0};
  floppy_read(0, 1, buffer);
  dumpmem(buffer,64);
  
}
